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
#include <unordered_map>
#include <cstdint>
#include <SDL_rwops.h>

namespace OpenXcom
{

/**
 * Represents an encrypted OXC container file (.oxc format).
 *
 * Container format:
 * - Magic: "OXFTA1\0\0" (8 bytes)
 * - Version: 3 (1 byte)
 * - Salt: 16 bytes
 * - Manifest IV: 16 bytes
 * - Manifest encrypted length: 4 bytes (big-endian)
 * - Encrypted manifest + HMAC (32 bytes)
 * - File data blobs (encrypted + HMAC)
 *
 * The manifest is a YAML document containing file metadata.
 */
class OXCContainer
{
public:
	/**
	 * File entry metadata from the manifest.
	 */
	struct FileEntry {
		std::string path;        // Relative path within container
		size_t offset;           // Offset in data section
		size_t length;           // Length of encrypted blob (includes MAC)
		bool compressed;         // Whether data is zlib-compressed
		std::string iv;          // Base64-encoded IV
		std::string mac;         // Base64-encoded MAC
	};

private:
	std::string _fullpath;
	SDL_RWops* _rwops;
	bool _ownsRwops;
	std::vector<uint8_t> _key;
	std::unordered_map<std::string, FileEntry> _files;
	size_t _dataSectionStart;
	bool _loaded;

	void _parseContainer(const std::string& passphrase);
	std::vector<uint8_t> _readFixed(size_t pos, size_t len);
	std::vector<uint8_t> _base64Decode(const std::string& encoded);

public:
	/**
	 * Create an OXC container from a file path.
	 * @param fullpath Path to the .oxc file
	 * @param passphrase Encryption passphrase
	 */
	OXCContainer(const std::string& fullpath, const std::string& passphrase);

	/**
	 * Create an OXC container from SDL_RWops.
	 * @param rwops SDL_RWops to read from
	 * @param fullpath Virtual path for logging
	 * @param passphrase Encryption passphrase
	 */
	OXCContainer(SDL_RWops* rwops, const std::string& fullpath, const std::string& passphrase, bool takeOwnership = false);

	~OXCContainer();

	/**
	 * Check if a file exists in the container.
	 * @param relpath Relative path
	 * @return True if file exists
	 */
	bool hasFile(const std::string& relpath) const;

	/**
	 * Get list of all file paths in container.
	 * @return Vector of relative paths
	 */
	std::vector<std::string> getFileList() const;

	/**
	 * Extract a file to memory.
	 * @param relpath Relative path
	 * @return Decrypted and decompressed file data
	 */
	std::vector<uint8_t> extractFile(const std::string& relpath);

	/**
	 * Extract a file to SDL_RWops.
	 * @param relpath Relative path
	 * @return SDL_RWops with file data (caller must free)
	 */
	SDL_RWops* extractFileToRWops(const std::string& relpath);

	/**
	 * Get the full path of the container.
	 */
	const std::string& getFullPath() const { return _fullpath; }
};

} // namespace OpenXcom
