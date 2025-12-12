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

#include "OXCContainer.h"
#include "OXCCrypto.h"
#include "Exception.h"
#include "Logger.h"
#include "../Engine/Yaml.h"
#include <cstring>
#ifndef _WIN32
#include <strings.h>
#endif
#include <sstream>
#include <algorithm>

#define MINIZ_NO_STDIO
#include "../../libs/miniz/miniz.h"

namespace OpenXcom
{
	inline int ciCompare(const std::string &lhs, const std::string &rhs)
	{
#ifdef _WIN32
		return _stricmp(lhs.c_str(), rhs.c_str());
#else
		return strcasecmp(lhs.c_str(), rhs.c_str());
#endif
	}

// Base64 decoding table
static const uint8_t base64_table[256] = {
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

std::vector<uint8_t> OXCContainer::_base64Decode(const std::string& encoded) {
	std::vector<uint8_t> result;
	result.reserve((encoded.size() * 3) / 4);

	uint32_t buffer = 0;
	int bits = 0;

	for (char c : encoded) {
		if (c == '=') break;
		uint8_t val = base64_table[(uint8_t)c];
		if (val == 64) continue; // Skip invalid characters

		buffer = (buffer << 6) | val;
		bits += 6;

		if (bits >= 8) {
			bits -= 8;
			result.push_back((buffer >> bits) & 0xFF);
		}
	}

	return result;
}

std::vector<uint8_t> OXCContainer::_readFixed(size_t pos, size_t len) {
	std::vector<uint8_t> buf(len);

	if (SDL_RWseek(_rwops, pos, RW_SEEK_SET) != (Sint64)pos) {
		throw Exception("Failed to seek in OXC container");
	}

	size_t read = SDL_RWread(_rwops, buf.data(), 1, len);
	if (read != len) {
		throw Exception("Failed to read expected bytes from OXC container");
	}

	return buf;
}

void OXCContainer::_parseContainer(const std::string& passphrase) {
	size_t pos = 0;

	// Read and verify magic
	std::vector<uint8_t> magic = _readFixed(pos, 8);
	pos += 8;

	const uint8_t expectedMagic[8] = {'O', 'X', 'F', 'T', 'A', '1', '\0', '\0'};
	if (memcmp(magic.data(), expectedMagic, 8) != 0) {
		throw Exception("Invalid OXC container: bad magic bytes");
	}
	Log(LOG_DEBUG) << "OXC magic OK for " << _fullpath;

	// Read version
	std::vector<uint8_t> versionBuf = _readFixed(pos, 1);
	pos += 1;
	uint8_t version = versionBuf[0];

	Log(LOG_DEBUG) << "OXC container version: " << (int)version;
	if (version < 1 || version > 3) {
		std::ostringstream err;
		err << "Unsupported OXC container version: " << (int)version;
		throw Exception(err.str());
	}
	if (version != 3) {
		Log(LOG_WARNING) << "Parsing OXC version " << (int)version << ", expected 3; attempting compatibility";
	}

	// Read salt
	std::vector<uint8_t> salt = _readFixed(pos, 16);
	pos += 16;

	// Read manifest IV
	std::vector<uint8_t> ivMan = _readFixed(pos, 16);
	pos += 16;

	// Read manifest length (endianness/semantics may differ across versions)
	std::vector<uint8_t> lenBuf = _readFixed(pos, 4);
	uint32_t lenBE = ((uint32_t)lenBuf[0] << 24) | ((uint32_t)lenBuf[1] << 16) |
	                 ((uint32_t)lenBuf[2] << 8) | (uint32_t)lenBuf[3];
	uint32_t lenLE = ((uint32_t)lenBuf[3] << 24) | ((uint32_t)lenBuf[2] << 16) |
	                 ((uint32_t)lenBuf[1] << 8) | (uint32_t)lenBuf[0];

	const uint64_t MAX_MAN64 = 64ull * 1024ull * 1024ull; // up to 64MB manifests
	const uint32_t MAX_MAN = 10 * 1024 * 1024;
	bool includeMac = true; // whether length includes the 32-byte MAC
	uint64_t lenVal64 = 0;  // chosen raw value from header
	std::string chosenFmt = "";

	auto plausible_inc32 = [&](uint32_t v){ return v >= 32 && v <= MAX_MAN; };
	auto plausible_exc32 = [&](uint32_t v){ return v >= 1 && v <= MAX_MAN; };
	auto plausible_inc64 = [&](uint64_t v){ return v >= 32 && v <= MAX_MAN64; };
	auto plausible_exc64 = [&](uint64_t v){ return v >= 1 && v <= MAX_MAN64; };

	// Try 4-byte variants first: BE+include, BE+exclude, LE+include, LE+exclude
	if (plausible_inc32(lenBE)) { lenVal64 = lenBE; includeMac = true; chosenFmt = "BE32+includeMAC"; pos += 4; }
	else if (plausible_exc32(lenBE)) { lenVal64 = lenBE; includeMac = false; chosenFmt = "BE32+excludeMAC"; pos += 4; }
	else if (plausible_inc32(lenLE)) { lenVal64 = lenLE; includeMac = true; chosenFmt = "LE32+includeMAC"; pos += 4; }
	else if (plausible_exc32(lenLE)) { lenVal64 = lenLE; includeMac = false; chosenFmt = "LE32+excludeMAC"; pos += 4; }
	else {
		// Try 8-byte variants without committing pos yet
		std::vector<uint8_t> lenBuf2 = _readFixed(pos + 4, 4);
		uint64_t be64 = ((uint64_t)lenBuf[0] << 56) | ((uint64_t)lenBuf[1] << 48) |
						((uint64_t)lenBuf[2] << 40) | ((uint64_t)lenBuf[3] << 32) |
						((uint64_t)lenBuf2[0] << 24) | ((uint64_t)lenBuf2[1] << 16) |
						((uint64_t)lenBuf2[2] << 8)  | (uint64_t)lenBuf2[3];
		uint64_t le64 = ((uint64_t)lenBuf2[3] << 56) | ((uint64_t)lenBuf2[2] << 48) |
						((uint64_t)lenBuf2[1] << 40) | ((uint64_t)lenBuf2[0] << 32) |
						((uint64_t)lenBuf[3]  << 24) | ((uint64_t)lenBuf[2]  << 16) |
						((uint64_t)lenBuf[1]  << 8)  | (uint64_t)lenBuf[0];
		if (plausible_inc64(be64)) { lenVal64 = be64; includeMac = true; chosenFmt = "BE64+includeMAC"; pos += 8; }
		else if (plausible_exc64(be64)) { lenVal64 = be64; includeMac = false; chosenFmt = "BE64+excludeMAC"; pos += 8; }
		else if (plausible_inc64(le64)) { lenVal64 = le64; includeMac = true; chosenFmt = "LE64+includeMAC"; pos += 8; }
		else if (plausible_exc64(le64)) { lenVal64 = le64; includeMac = false; chosenFmt = "LE64+excludeMAC"; pos += 8; }
		else {
			throw Exception("Invalid manifest length in OXC container");
		}
	}
	Log(LOG_DEBUG) << "OXC manifest length raw=" << (unsigned long long)lenVal64 << " format=" << chosenFmt;

	uint64_t encLen64 = includeMac ? (lenVal64 - 32ull) : lenVal64;
	if (encLen64 < 1 || encLen64 > MAX_MAN64) {
		throw Exception("Invalid manifest length in OXC container");
	}
	// Read encrypted manifest and MAC
	std::vector<uint8_t> encMan = _readFixed(pos, (size_t)encLen64);
	pos += (size_t)encLen64;
	std::vector<uint8_t> macMan = _readFixed(pos, 32);
	pos += 32;

	// Derive key from passphrase
	_key = OXCCrypto::deriveKey(passphrase, salt);

	std::vector<uint8_t> encKey(32);
	std::vector<uint8_t> hmacKey(32);
	std::copy(_key.begin(), _key.begin() + 32, encKey.begin());
	std::copy(_key.begin() + 32, _key.end(), hmacKey.begin());

	// Decrypt manifest
	std::vector<uint8_t> manifestPlaintext;
	try {
		manifestPlaintext = OXCCrypto::decryptCbcHmac(encKey, hmacKey, ivMan, encMan, macMan);
	} catch (const Exception& e) {
		Log(LOG_ERROR) << "Failed to decrypt OXC container manifest: " << e.what();
		throw Exception("Failed to decrypt OXC container - wrong password?");
	}

	// Parse manifest YAML using Engine/Yaml
	std::string manifestStr(manifestPlaintext.begin(), manifestPlaintext.end());
	// Sanitize manifest: strip BOM, normalize CRLF, and remove control chars except \n and \t
	if (!manifestStr.empty() && (unsigned char)manifestStr[0] == 0xEF && manifestStr.size() >= 3
		&& (unsigned char)manifestStr[1] == 0xBB && (unsigned char)manifestStr[2] == 0xBF)
	{
		manifestStr.erase(0, 3);
	}
	// normalize CRLF to LF
	manifestStr.erase(std::remove(manifestStr.begin(), manifestStr.end(), '\r'), manifestStr.end());
	// replace other ASCII control characters with spaces to avoid rapidyaml parse errors
	size_t sanitizedCount = 0;
	for (size_t i = 0; i < manifestStr.size(); ++i)
	{
		unsigned char ch = (unsigned char)manifestStr[i];
		if (ch < 0x20 && ch != '\n' && ch != '\t')
		{
			manifestStr[i] = ' ';
			++sanitizedCount;
		}
	}
	if (sanitizedCount > 0)
	{
		Log(LOG_WARNING) << "Sanitized " << (unsigned long)sanitizedCount << " control characters in OXC manifest";
	}
	YAML::YamlRootNodeReader manifestRoot(YAML::YamlString{manifestStr}, "manifest");
	try {
		// manifestRoot is already loaded
	} catch (...) {
		throw Exception("Failed to parse OXC container manifest YAML");
	}
	auto filesSeq = manifestRoot["files"].children();
	if (filesSeq.empty()) {
		throw Exception("Invalid or corrupted OXC container manifest");
	}

	// Store data section start position
	_dataSectionStart = pos;

	// Parse file entries
	for (const auto& fileNode : filesSeq) {
		FileEntry entry;
		fileNode.tryRead("path", entry.path);
		// normalize separators to forward slash
		std::replace(entry.path.begin(), entry.path.end(), '\\', '/');
		// trim leading/trailing whitespace from path
		auto notSpace = [](int ch){ return !std::isspace(ch); };
		entry.path.erase(entry.path.begin(), std::find_if(entry.path.begin(), entry.path.end(), notSpace));
		entry.path.erase(std::find_if(entry.path.rbegin(), entry.path.rend(), notSpace).base(), entry.path.end());
		while (!entry.path.empty() && (entry.path.front() == '/' || entry.path.front() == '.')) {
			// trim leading '/' or './'
			if (entry.path.front() == '/') { entry.path.erase(entry.path.begin()); }
			else if (entry.path.size() >= 2 && entry.path.substr(0,2) == "./") { entry.path.erase(0,2); }
			else { break; }
		}
		uint32_t offset = 0, length = 0;
		fileNode.tryRead("offset", offset);
		fileNode.tryRead("length", length);
		entry.offset = (size_t)offset;
		entry.length = (size_t)length;
		fileNode.tryRead("compressed", entry.compressed);
		fileNode.tryRead("iv", entry.iv);
		fileNode.tryRead("mac", entry.mac);

		_files[entry.path] = entry;
	}

	_loaded = true;
	Log(LOG_DEBUG) << "Loaded OXC container: " << _fullpath << " with " << _files.size() << " files";
}

OXCContainer::OXCContainer(const std::string& fullpath, const std::string& passphrase)
	: _fullpath(fullpath), _rwops(nullptr), _ownsRwops(true), _dataSectionStart(0), _loaded(false)
{
	_rwops = SDL_RWFromFile(fullpath.c_str(), "rb");
	if (!_rwops) {
		throw Exception("Failed to open OXC container file: " + fullpath);
	}

	try {
		_parseContainer(passphrase);
	} catch (...) {
		if (_ownsRwops && _rwops) {
			SDL_RWclose(_rwops);
		}
		throw;
	}
}

OXCContainer::OXCContainer(SDL_RWops* rwops, const std::string& fullpath, const std::string& passphrase, bool takeOwnership)
	: _fullpath(fullpath), _rwops(rwops), _ownsRwops(takeOwnership), _dataSectionStart(0), _loaded(false)
{
	if (!_rwops) {
		throw Exception("Invalid SDL_RWops for OXC container");
	}

	_parseContainer(passphrase);
}

OXCContainer::~OXCContainer() {
	if (_ownsRwops && _rwops) {
		SDL_RWclose(_rwops);
	}
}

bool OXCContainer::hasFile(const std::string& relpath) const {
	// Exact match
	if (_files.find(relpath) != _files.end()) return true;
	// Try common alias: .yml <-> .yaml
	if (relpath.size() >= 4) {
		if (relpath.size() >= 4 && relpath.substr(relpath.size()-4) == ".yml") {
			std::string alt = relpath;
			alt.replace(relpath.size()-4, 4, ".yaml");
			if (_files.find(alt) != _files.end()) return true;
		} else if (relpath.size() >= 5 && relpath.substr(relpath.size()-5) == ".yaml") {
			std::string alt = relpath;
			alt.replace(relpath.size()-5, 5, ".yml");
			if (_files.find(alt) != _files.end()) return true;
		}
	}
	// Case-insensitive match scan (container paths may differ in case)
	for (const auto& kv : _files) {
		if (ciCompare(kv.first, relpath) == 0) return true;
	}
	return false;
}

std::vector<std::string> OXCContainer::getFileList() const {
	std::vector<std::string> result;
	result.reserve(_files.size());

	for (const auto& pair : _files) {
		result.push_back(pair.first);
	}

	return result;
}

std::vector<uint8_t> OXCContainer::extractFile(const std::string& relpath) {
	auto it = _files.find(relpath);
	if (it == _files.end()) {
		// Try .yml <-> .yaml alias
		if (relpath.size() >= 4 && relpath.substr(relpath.size()-4) == ".yml") {
			std::string alt = relpath;
			alt.replace(relpath.size()-4, 4, ".yaml");
			it = _files.find(alt);
		} else if (relpath.size() >= 5 && relpath.substr(relpath.size()-5) == ".yaml") {
			std::string alt = relpath;
			alt.replace(relpath.size()-5, 5, ".yml");
			it = _files.find(alt);
		}
	}
	if (it == _files.end()) {
		// Case-insensitive search
		for (auto fit = _files.begin(); fit != _files.end(); ++fit) {
			if (ciCompare(fit->first, relpath) == 0) {
				it = fit;
				break;
			}
		}
	}
	if (it == _files.end()) {
		// Extra diagnostics: log nearby keys to aid debugging
		std::string dirPrefix = relpath;
		size_t slashPos = dirPrefix.find_last_of('/');
		if (slashPos != std::string::npos) {
			dirPrefix = dirPrefix.substr(0, slashPos + 1);
		} else {
			dirPrefix.clear();
		}
		int logged = 0;
		for (const auto& kv : _files) {
			if (!dirPrefix.empty() && kv.first.size() >= dirPrefix.size() && kv.first.compare(0, dirPrefix.size(), dirPrefix) == 0) {
				if (logged < 10) {
					Log(LOG_DEBUG) << "OXC nearby key: " << kv.first;
					++logged;
				}
			}
			else if (logged < 5) {
				// Also log a few candidates ending with metadata file names
				if (kv.first.size() >= 13 && (kv.first.substr(kv.first.size()-13) == "metadata.yml" || kv.first.substr(kv.first.size()-14) == "metadata.yaml")) {
					Log(LOG_DEBUG) << "OXC candidate key: " << kv.first;
					++logged;
				}
			}
			if (logged >= 15) break;
		}
		Log(LOG_ERROR) << "File not found in OXC container (after diagnostics): " << relpath;
		throw Exception("File not found in OXC container: " + relpath);
	}
	if (it == _files.end()) {
		throw Exception("File not found in OXC container: " + relpath);
	}

	const FileEntry& entry = it->second;

	// Read encrypted blob (ciphertext + MAC)
	if (entry.length < 32) {
		throw Exception("Invalid encrypted blob size in OXC container");
	}

	std::vector<uint8_t> encBlob = _readFixed(_dataSectionStart + entry.offset, entry.length);
	std::vector<uint8_t> cipher(encBlob.begin(), encBlob.end() - 32);
	std::vector<uint8_t> mac(encBlob.end() - 32, encBlob.end());

	// Decode IV and MAC from base64
	std::vector<uint8_t> iv = _base64Decode(entry.iv);
	std::vector<uint8_t> expectedMac = _base64Decode(entry.mac);

	if (iv.size() != 16 || expectedMac.size() != 32) {
		throw Exception("Invalid IV or MAC in OXC container file entry");
	}

	// Decrypt
	std::vector<uint8_t> encKey(32);
	std::vector<uint8_t> hmacKey(32);
	std::copy(_key.begin(), _key.begin() + 32, encKey.begin());
	std::copy(_key.begin() + 32, _key.end(), hmacKey.begin());

	std::vector<uint8_t> plain;
	try {
		plain = OXCCrypto::decryptCbcHmac(encKey, hmacKey, iv, cipher, mac);
	} catch (const Exception& e) {
		Log(LOG_ERROR) << "Failed to decrypt file from OXC container: " << relpath;
		throw;
	}

	// Decompress if needed
	if (entry.compressed) {
		uLongf decompressedSize = plain.size() * 10; // Initial guess
		std::vector<uint8_t> decompressed(decompressedSize);

		int result = Z_BUF_ERROR;
		while (result == Z_BUF_ERROR) {
			decompressedSize = decompressed.size();
			result = uncompress(decompressed.data(), &decompressedSize,
			                   plain.data(), plain.size());

			if (result == Z_BUF_ERROR) {
				decompressed.resize(decompressed.size() * 2);
			}
		}

		if (result != Z_OK) {
			throw Exception("Failed to decompress file from OXC container: " + relpath);
		}

		decompressed.resize(decompressedSize);
		return decompressed;
	}

	return plain;
}

SDL_RWops* OXCContainer::extractFileToRWops(const std::string& relpath) {
	std::vector<uint8_t> data = extractFile(relpath);

	// Allocate memory that SDL will own
	void* mem = SDL_malloc(data.size());
	if (!mem) {
		throw Exception("Failed to allocate memory for OXC file extraction");
	}

	memcpy(mem, data.data(), data.size());

	SDL_RWops* rwops = SDL_RWFromConstMem(mem, data.size());
	if (!rwops) {
		SDL_free(mem);
		throw Exception("Failed to create SDL_RWops for OXC file");
	}

	// Set up close callback to free memory
	rwops->close = [](struct SDL_RWops* context) -> int {
		if (context) {
			if (context->hidden.mem.base) {
				SDL_free(context->hidden.mem.base);
			}
			SDL_FreeRW(context);
		}
		return 0;
	};

	return rwops;
}

} // namespace OpenXcom
