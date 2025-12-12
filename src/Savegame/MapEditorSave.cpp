/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include "MapEditorSave.h"
#include <algorithm>
#include "../Engine/Yaml.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/Exception.h"
#include "../Engine/Options.h"

namespace OpenXcom
{

const std::string MapEditorSave::AUTOSAVE_MAPEDITOR = "_auto_mapeditor_.asav",
				  MapEditorSave::MAINSAVE_MAPEDITOR = "mapeditor.sav";

const std::string MapEditorSave::MAP_DIRECTORY = "/MAPS",
				  MapEditorSave::RMP_DIRECTORY = "/ROUTES";

/**
 * Initializes the class for storing information for saving or editing maps
 */
MapEditorSave::MapEditorSave()
{
    _savedMapFiles.clear();
    _matchedFiles.clear();
}

/**
 * Deletes data on edited map files from memory
 */
MapEditorSave::~MapEditorSave()
{

}

/**
 * Loads the data on edited map files from the user directory
 */
void MapEditorSave::load()
{
    std::string filename = MAINSAVE_MAPEDITOR;
	std::string filepath = Options::getMasterUserFolder() + filename;

    _savedMapFiles.clear();
    _matchedFiles.clear();

    if (!CrossPlatform::fileExists(filepath))
    {
        return;
    }

    YAML::YamlRootNodeReader reader(filepath);
    if (const auto& savedFiles = reader["savedMapFiles"])
    {
        for (const auto& node : savedFiles.children())
        {
            MapFileInfo mapFile;
            node.readNode("name", mapFile.name, std::string());
            node.readNode("baseDirectory", mapFile.baseDirectory, std::string());
            node.readNode("mods", mapFile.mods, std::vector<std::string>());
            node.readNode("terrain", mapFile.terrain, std::string());
            node.readNode("mcds", mapFile.mcds, std::vector<std::string>());

            _savedMapFiles.push_back(mapFile);
        }
    }

}

/**
 * Saves the data on edited map files to the user directory
 */
void MapEditorSave::save()
{
    YAML::YamlRootNodeWriter writer;
    writer.setAsMap();

    writer.write("savedMapFiles", _savedMapFiles, [](YAML::YamlNodeWriter& savedFiles, const MapFileInfo& mapFile)
    {
        auto fileData = savedFiles.write();
        fileData.setAsMap();
        fileData.write("name", mapFile.name);
        fileData.write("baseDirectory", mapFile.baseDirectory);
        fileData.write("mods", mapFile.mods);
        fileData.write("terrain", mapFile.terrain);
        fileData.write("mcds", mapFile.mcds);
    });

    std::string filename = MAINSAVE_MAPEDITOR;
	std::string filepath = Options::getMasterUserFolder() + filename;
	if (!CrossPlatform::writeFile(filepath, writer.emit().yaml))
	{
		throw Exception("Failed to save " + filepath);
	}
}

/**
 * Gets the data for the map file we want to load
 * @return pointer to the map info
 */
MapFileInfo *MapEditorSave::getMapFileToLoad()
{
    return &_mapFileToLoad;
}

/**
 * Clears the MapFileInfo for the file we want to load
 */
void MapEditorSave::clearMapFileToLoad()
{
    MapFileInfo fileInfo;
    _mapFileToLoad = fileInfo;
}

/**
 * Gets the data for the current map being edited
 * @return pointer to the current map info
 */
MapFileInfo *MapEditorSave::getCurrentMapFile()
{
    return &_currentMapFile;
}

/**
 * Adds the data on a newly saved map file to the list
 * @param fileInfo information on the new edited map file
 */
void MapEditorSave::addMap(MapFileInfo fileInfo)
{
    // Only add a new entry if one doesn't already exist
    if(std::find_if(_savedMapFiles.begin(), _savedMapFiles.end(), [&](const MapFileInfo& file) { return file == fileInfo; }) == _savedMapFiles.end())
    {
        _savedMapFiles.push_back(fileInfo);
    }

    _currentMapFile = fileInfo;
}

/**
 * Search for entries matching the given directory + map name
 * Populates the list of matched files returned by getMatchedFiles
 * @param fileInfo pointer to the information on the map file we're looking for
 * @return number of matching entries found in the saved map file data
 */
size_t MapEditorSave::findMatchingFiles(MapFileInfo *fileInfo)
{
    _mapFileToLoad = *fileInfo;
    _matchedFiles.clear();

    if (fileInfo->baseDirectory.empty() || fileInfo->name.empty())
        return 0;

    size_t numFound = 0;

    for (auto i : _savedMapFiles)
    {
        if (i.baseDirectory == fileInfo->baseDirectory && i.name == fileInfo->name)
        {
            _matchedFiles.push_back(i);
            ++numFound;
        }
    }

    if (numFound == 1)
    {
        _mapFileToLoad = _matchedFiles.front();
    }

    return numFound;
}

/**
 * Gets the list of entries found by the search
 * @return pointer to the list of MapFileInfo
 */
std::vector<MapFileInfo> *MapEditorSave::getMatchedFiles()
{
    return &_matchedFiles;
}

}