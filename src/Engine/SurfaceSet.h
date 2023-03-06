#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
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

#include <vector>
#include <string>
#include <SDL.h>

namespace OpenXcom
{

class Surface;

/**
 * Container of a set of surfaces.
 * Used to manage single images that contain series of
 * frames inside, like animated sprites, making them easier
 * to access without constant cropping.
 */
class SurfaceSet
{
private:
	std::vector<Surface> _frames;
	int _width, _height;
	int _sharedFrames;

public:
	/// Crates a surface set with frames of the specified size.
	SurfaceSet(int width, int height);
	/// Creates a surface set from an existing one.
	SurfaceSet(const SurfaceSet& other) = default;
	/// Creates a surface set from an existing one.
	SurfaceSet(SurfaceSet&& other) = default;
	/// Cleans up the surface set.
	~SurfaceSet();
	/// Assignment operator.
	SurfaceSet& operator=(const SurfaceSet& other) = default;
	/// Assignment operator.
	SurfaceSet& operator=(SurfaceSet&& other) = default;

	/// Loads an X-Com set of PCK/TAB image files.
	void loadPck(const std::string &pck, const std::string &tab = "");
	/// Loads an X-Com DAT image file.
	void loadDat(const std::string &filename);
	/// Gets a particular frame from the set.
	Surface *getFrame(int i);
	/// Gets a particular frame from the set.
	const Surface *getFrame(int i) const;
	/// Creates a new surface and returns a pointer to it.
	Surface *addFrame(int i);
	/// Gets the width of all frames.
	int getWidth() const;
	/// Gets the height of all frames.
	int getHeight() const;

	/// Set number of shared frame indexes that are accessible for all mods.
	void setMaxSharedFrames(int i);
	/// Gets number of shared frame indexes that are accessible for all mods.
	int getMaxSharedFrames() const;

	/// Gets the total frames in the set.
	size_t getTotalFrames() const;
	/// Sets the surface set's palette.
	void setPalette(const SDL_Color *colors, int firstcolor = 0, int ncolors = 256);
};

}
