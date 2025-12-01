#pragma once
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

#include <string>
#include <vector>
#include <cstdint>

namespace OpenXcom
{
namespace OXCCrypto
{
	/**
	 * Derive a 64-byte key from passphrase using scrypt.
	 * First 32 bytes are for AES-256 encryption key.
	 * Last 32 bytes are for HMAC-SHA256 key.
	 * 
	 * @param passphrase The password string
	 * @param salt 16-byte salt
	 * @return 64-byte derived key
	 */
	std::vector<uint8_t> deriveKey(const std::string& passphrase, const std::vector<uint8_t>& salt);

	/**
	 * Decrypt data using AES-256-CBC with HMAC-SHA256 verification (Encrypt-then-MAC).
	 * 
	 * @param encKey32 32-byte AES encryption key
	 * @param hmacKey32 32-byte HMAC key
	 * @param iv16 16-byte initialization vector
	 * @param ciphertext Encrypted data
	 * @param mac32 32-byte HMAC for verification
	 * @return Decrypted plaintext data
	 * @throws Exception if HMAC validation fails or decryption fails
	 */
	std::vector<uint8_t> decryptCbcHmac(
		const std::vector<uint8_t>& encKey32,
		const std::vector<uint8_t>& hmacKey32,
		const std::vector<uint8_t>& iv16,
		const std::vector<uint8_t>& ciphertext,
		const std::vector<uint8_t>& mac32
	);

	/**
	 * Compute HMAC-SHA256.
	 * 
	 * @param key HMAC key
	 * @param data1 First data chunk
	 * @param data2 Second data chunk
	 * @return 32-byte HMAC
	 */
	std::vector<uint8_t> hmacSha256(
		const std::vector<uint8_t>& key,
		const std::vector<uint8_t>& data1,
		const std::vector<uint8_t>& data2
	);

} // namespace OXCCrypto
} // namespace OpenXcom
