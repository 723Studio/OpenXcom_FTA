/*
 * Copyright 2010-2025 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "OXCCrypto.h"
#include "Exception.h"
#include "Logger.h"
#include <cstring>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
// For non-Windows platforms, we'll use a minimal implementation
// based on public domain/MIT licensed crypto primitives
#include "tiny_aes.h"
#endif

namespace OpenXcom
{
namespace OXCCrypto
{

// ============================================================================
// SCRYPT Implementation (simplified for our specific parameters)
// ============================================================================

// Simple scrypt implementation for N=16384, r=8, p=1
// Based on public domain reference implementations

static void xor_block(uint8_t* dest, const uint8_t* src, size_t len) {
	for (size_t i = 0; i < len; i++) {
		dest[i] ^= src[i];
	}
}

static uint32_t rotl32(uint32_t x, int b) {
	return (x << b) | (x >> (32 - b));
}

static void salsa20_8_core(uint32_t B[16]) {
	uint32_t x[16];
	memcpy(x, B, 64);

	for (int i = 0; i < 8; i += 2) {
		x[ 4] ^= rotl32(x[ 0] + x[12],  7); x[ 8] ^= rotl32(x[ 4] + x[ 0],  9);
		x[12] ^= rotl32(x[ 8] + x[ 4], 13); x[ 0] ^= rotl32(x[12] + x[ 8], 18);
		x[ 9] ^= rotl32(x[ 5] + x[ 1],  7); x[13] ^= rotl32(x[ 9] + x[ 5],  9);
		x[ 1] ^= rotl32(x[13] + x[ 9], 13); x[ 5] ^= rotl32(x[ 1] + x[13], 18);
		x[14] ^= rotl32(x[10] + x[ 6],  7); x[ 2] ^= rotl32(x[14] + x[10],  9);
		x[ 6] ^= rotl32(x[ 2] + x[14], 13); x[10] ^= rotl32(x[ 6] + x[ 2], 18);
		x[ 3] ^= rotl32(x[15] + x[11],  7); x[ 7] ^= rotl32(x[ 3] + x[15],  9);
		x[11] ^= rotl32(x[ 7] + x[ 3], 13); x[15] ^= rotl32(x[11] + x[ 7], 18);
		x[ 1] ^= rotl32(x[ 0] + x[ 3],  7); x[ 2] ^= rotl32(x[ 1] + x[ 0],  9);
		x[ 3] ^= rotl32(x[ 2] + x[ 1], 13); x[ 0] ^= rotl32(x[ 3] + x[ 2], 18);
		x[ 6] ^= rotl32(x[ 5] + x[ 4],  7); x[ 7] ^= rotl32(x[ 6] + x[ 5],  9);
		x[ 4] ^= rotl32(x[ 7] + x[ 6], 13); x[ 5] ^= rotl32(x[ 4] + x[ 7], 18);
		x[11] ^= rotl32(x[10] + x[ 9],  7); x[ 8] ^= rotl32(x[11] + x[10],  9);
		x[ 9] ^= rotl32(x[ 8] + x[11], 13); x[10] ^= rotl32(x[ 9] + x[ 8], 18);
		x[12] ^= rotl32(x[15] + x[14],  7); x[13] ^= rotl32(x[12] + x[15],  9);
		x[14] ^= rotl32(x[13] + x[12], 13); x[15] ^= rotl32(x[14] + x[13], 18);
	}

	for (int i = 0; i < 16; i++) {
		B[i] += x[i];
	}
}

static void blockmix_salsa8(uint32_t* B, uint32_t* Y, size_t r) {
	uint32_t X[16];
	memcpy(X, &B[(2 * r - 1) * 16], 64);

	for (size_t i = 0; i < 2 * r; i++) {
		xor_block((uint8_t*)X, (uint8_t*)&B[i * 16], 64);
		salsa20_8_core(X);
		memcpy(&Y[i * 16], X, 64);
	}

	for (size_t i = 0; i < r; i++) {
		memcpy(&B[i * 16], &Y[(i * 2) * 16], 64);
	}
	for (size_t i = 0; i < r; i++) {
		memcpy(&B[(i + r) * 16], &Y[(i * 2 + 1) * 16], 64);
	}
}

static void scryptROMix(uint32_t* B, size_t r, size_t N, uint32_t* V, uint32_t* XY) {
	uint32_t* X = XY;
	uint32_t* Y = &XY[32 * r];

	memcpy(X, B, 128 * r);

	for (size_t i = 0; i < N; i++) {
		memcpy(&V[i * (32 * r)], X, 128 * r);
		blockmix_salsa8(X, Y, r);
	}

	for (size_t i = 0; i < N; i++) {
		size_t j = X[(2 * r - 1) * 16] % N;
		xor_block((uint8_t*)X, (uint8_t*)&V[j * (32 * r)], 128 * r);
		blockmix_salsa8(X, Y, r);
	}

	memcpy(B, X, 128 * r);
}

// ============================================================================
// PBKDF2-HMAC-SHA256 (simplified for scrypt)
// ============================================================================

static void hmac_sha256_impl(const uint8_t* key, size_t keylen,
                             const uint8_t* data, size_t datalen,
                             uint8_t* out);

static void pbkdf2_sha256(const uint8_t* password, size_t passlen,
                          const uint8_t* salt, size_t saltlen,
                          size_t iterations, uint8_t* out, size_t outlen) {
#ifdef _WIN32
	BCRYPT_ALG_HANDLE hAlg = nullptr;

	NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG);
	if (!BCRYPT_SUCCESS(status)) {
		throw Exception("Failed to open PBKDF2 algorithm");
	}

	status = BCryptDeriveKeyPBKDF2(hAlg, (PUCHAR)password, (ULONG)passlen,
	                               (PUCHAR)salt, (ULONG)saltlen,
	                               iterations, out, (ULONG)outlen, 0);

	BCryptCloseAlgorithmProvider(hAlg, 0);

	if (!BCRYPT_SUCCESS(status)) {
		throw Exception("PBKDF2 derivation failed");
	}
#else
	// Simplified PBKDF2 implementation for non-Windows
	std::vector<uint8_t> U(32);
	std::vector<uint8_t> salt_block(saltlen + 4);

	memcpy(salt_block.data(), salt, saltlen);

	for (size_t block = 1; block * 32 <= outlen; block++) {
		salt_block[saltlen + 0] = (block >> 24) & 0xff;
		salt_block[saltlen + 1] = (block >> 16) & 0xff;
		salt_block[saltlen + 2] = (block >> 8) & 0xff;
		salt_block[saltlen + 3] = block & 0xff;

		hmac_sha256_impl(password, passlen, salt_block.data(), salt_block.size(), U.data());

		memcpy(&out[(block - 1) * 32], U.data(), 32);

		for (size_t iter = 1; iter < iterations; iter++) {
			hmac_sha256_impl(password, passlen, U.data(), 32, U.data());
			for (size_t i = 0; i < 32; i++) {
				out[(block - 1) * 32 + i] ^= U[i];
			}
		}
	}
#endif
}

// ============================================================================
// SHA-256 (needed for HMAC)
// ============================================================================

#ifdef _WIN32
static void sha256(const uint8_t* data, size_t len, uint8_t* out) {
	BCRYPT_ALG_HANDLE hAlg = nullptr;
	BCRYPT_HASH_HANDLE hHash = nullptr;

	BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
	BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0);
	BCryptHashData(hHash, (PUCHAR)data, (ULONG)len, 0);
	BCryptFinishHash(hHash, out, 32, 0);

	BCryptDestroyHash(hHash);
	BCryptCloseAlgorithmProvider(hAlg, 0);
}
#else
// Minimal SHA-256 implementation (public domain)
static const uint32_t K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t rotr32(uint32_t x, int n) {
	return (x >> n) | (x << (32 - n));
}

static void sha256_transform(uint32_t state[8], const uint8_t block[64]) {
	uint32_t W[64];
	uint32_t a, b, c, d, e, f, g, h, t1, t2;

	for (int i = 0; i < 16; i++) {
		W[i] = ((uint32_t)block[i * 4] << 24) | ((uint32_t)block[i * 4 + 1] << 16) |
		       ((uint32_t)block[i * 4 + 2] << 8) | ((uint32_t)block[i * 4 + 3]);
	}

	for (int i = 16; i < 64; i++) {
		uint32_t s0 = rotr32(W[i - 15], 7) ^ rotr32(W[i - 15], 18) ^ (W[i - 15] >> 3);
		uint32_t s1 = rotr32(W[i - 2], 17) ^ rotr32(W[i - 2], 19) ^ (W[i - 2] >> 10);
		W[i] = W[i - 16] + s0 + W[i - 7] + s1;
	}

	a = state[0]; b = state[1]; c = state[2]; d = state[3];
	e = state[4]; f = state[5]; g = state[6]; h = state[7];

	for (int i = 0; i < 64; i++) {
		uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
		uint32_t ch = (e & f) ^ (~e & g);
		t1 = h + S1 + ch + K[i] + W[i];
		uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		t2 = S0 + maj;

		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}

	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

static void sha256(const uint8_t* data, size_t len, uint8_t* out) {
	uint32_t state[8] = {
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};

	uint8_t block[64];
	size_t i = 0;

	while (len >= 64) {
		sha256_transform(state, data + i);
		i += 64;
		len -= 64;
	}

	memcpy(block, data + i, len);
	block[len] = 0x80;

	if (len >= 56) {
		memset(block + len + 1, 0, 64 - len - 1);
		sha256_transform(state, block);
		memset(block, 0, 56);
	} else {
		memset(block + len + 1, 0, 56 - len - 1);
	}

	uint64_t bitlen = (i + len) * 8;
	for (int j = 0; j < 8; j++) {
		block[63 - j] = bitlen & 0xff;
		bitlen >>= 8;
	}

	sha256_transform(state, block);

	for (int j = 0; j < 8; j++) {
		out[j * 4 + 0] = (state[j] >> 24) & 0xff;
		out[j * 4 + 1] = (state[j] >> 16) & 0xff;
		out[j * 4 + 2] = (state[j] >> 8) & 0xff;
		out[j * 4 + 3] = state[j] & 0xff;
	}
}
#endif

// ============================================================================
// HMAC-SHA256
// ============================================================================

static void hmac_sha256_impl(const uint8_t* key, size_t keylen,
                             const uint8_t* data, size_t datalen,
                             uint8_t* out) {
	uint8_t k[64];
	memset(k, 0, 64);

	if (keylen > 64) {
		sha256(key, keylen, k);
	} else {
		memcpy(k, key, keylen);
	}

	uint8_t ipad[64], opad[64];
	for (int i = 0; i < 64; i++) {
		ipad[i] = k[i] ^ 0x36;
		opad[i] = k[i] ^ 0x5c;
	}

	std::vector<uint8_t> inner(64 + datalen);
	memcpy(inner.data(), ipad, 64);
	memcpy(inner.data() + 64, data, datalen);

	uint8_t inner_hash[32];
	sha256(inner.data(), inner.size(), inner_hash);

	uint8_t outer[64 + 32];
	memcpy(outer, opad, 64);
	memcpy(outer + 64, inner_hash, 32);

	sha256(outer, 96, out);
}

std::vector<uint8_t> hmacSha256(
	const std::vector<uint8_t>& key,
	const std::vector<uint8_t>& data1,
	const std::vector<uint8_t>& data2
) {
	std::vector<uint8_t> combined;
	combined.insert(combined.end(), data1.begin(), data1.end());
	combined.insert(combined.end(), data2.begin(), data2.end());

	std::vector<uint8_t> result(32);
	hmac_sha256_impl(key.data(), key.size(), combined.data(), combined.size(), result.data());
	return result;
}

// ============================================================================
// Scrypt Key Derivation
// ============================================================================

std::vector<uint8_t> deriveKey(const std::string& passphrase, const std::vector<uint8_t>& salt) {
	const size_t N = 16384;
	const size_t r = 8;
	const size_t p = 1;
	const size_t dkLen = 64;

	std::vector<uint8_t> B(p * 128 * r);
	pbkdf2_sha256((uint8_t*)passphrase.data(), passphrase.size(),
	              salt.data(), salt.size(), 1, B.data(), B.size());

	std::vector<uint32_t> V(N * 32 * r);
	std::vector<uint32_t> XY(64 * r);

	for (size_t i = 0; i < p; i++) {
		scryptROMix((uint32_t*)(B.data() + i * 128 * r), r, N, V.data(), XY.data());
	}

	std::vector<uint8_t> result(dkLen);
	pbkdf2_sha256((uint8_t*)passphrase.data(), passphrase.size(),
	              B.data(), B.size(), 1, result.data(), result.size());

	return result;
}

// ============================================================================
// AES-256-CBC Decryption
// ============================================================================

std::vector<uint8_t> decryptCbcHmac(
	const std::vector<uint8_t>& encKey32,
	const std::vector<uint8_t>& hmacKey32,
	const std::vector<uint8_t>& iv16,
	const std::vector<uint8_t>& ciphertext,
	const std::vector<uint8_t>& mac32
) {
	// First, verify HMAC
	std::vector<uint8_t> expectedMac = hmacSha256(hmacKey32, iv16, ciphertext);

	bool macValid = (expectedMac.size() == mac32.size());
	if (macValid) {
		for (size_t i = 0; i < mac32.size(); i++) {
			if (expectedMac[i] != mac32[i]) {
				macValid = false;
				break;
			}
		}
	}

	if (!macValid) {
		throw Exception("HMAC validation failed - wrong password or corrupted data");
	}

	// Now decrypt
#ifdef _WIN32
	BCRYPT_ALG_HANDLE hAlg = nullptr;
	BCRYPT_KEY_HANDLE hKey = nullptr;

	NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, nullptr, 0);
	if (!BCRYPT_SUCCESS(status)) {
		throw Exception("Failed to open AES algorithm");
	}

	status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE,
	                          (PUCHAR)BCRYPT_CHAIN_MODE_CBC,
	                          sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw Exception("Failed to set CBC mode");
	}

	status = BCryptGenerateSymmetricKey(hAlg, &hKey, nullptr, 0,
	                                   (PUCHAR)encKey32.data(), 32, 0);
	if (!BCRYPT_SUCCESS(status)) {
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw Exception("Failed to generate symmetric key");
	}

	ULONG plaintextLen = 0;
	std::vector<uint8_t> ivCopy = iv16; // BCrypt modifies IV

	status = BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(),
	                      nullptr, ivCopy.data(), (ULONG)ivCopy.size(),
	                      nullptr, 0, &plaintextLen, BCRYPT_BLOCK_PADDING);

	if (!BCRYPT_SUCCESS(status)) {
		BCryptDestroyKey(hKey);
		BCryptCloseAlgorithmProvider(hAlg, 0);
		throw Exception("Failed to get plaintext length");
	}

	std::vector<uint8_t> plaintext(plaintextLen);
	ivCopy = iv16;

	status = BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(),
	                      nullptr, ivCopy.data(), (ULONG)ivCopy.size(),
	                      plaintext.data(), plaintextLen, &plaintextLen, BCRYPT_BLOCK_PADDING);

	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlg, 0);

	if (!BCRYPT_SUCCESS(status)) {
		throw Exception("AES decryption failed");
	}

	plaintext.resize(plaintextLen);
	return plaintext;
#else
	// For non-Windows platforms, we need a minimal AES implementation
	// Use embedded tiny-AES-c for AES-256-CBC decryption + PKCS#7 unpadding
	if (ciphertext.size() % 16 != 0) {
		throw Exception("Ciphertext size not multiple of AES block size");
	}

	AES_ctx ctx;
	AES_init_ctx_iv(&ctx, encKey32.data(), iv16.data());

	std::vector<uint8_t> buf = ciphertext; // will be decrypted in-place
	AES_CBC_decrypt_buffer(&ctx, buf.data(), buf.size());

	// PKCS#7 unpadding
	if (buf.empty()) {
		throw Exception("Decrypted plaintext is empty");
	}
	uint8_t pad = buf.back();
	if (pad == 0 || pad > 16 || pad > buf.size()) {
		throw Exception("Invalid PKCS#7 padding");
	}
	for (size_t i = 0; i < pad; ++i) {
		if (buf[buf.size() - 1 - i] != pad) {
			throw Exception("Invalid PKCS#7 padding bytes");
		}
	}

	buf.resize(buf.size() - pad);
	return buf;
#endif
}

} // namespace OXCCrypto
} // namespace OpenXcom
