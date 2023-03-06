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
#include <algorithm>
#include <vector>
#include <random>
#include <string>
#define __STDC_LIMIT_MACROS
#include <stdint.h>

namespace OpenXcom
{

/**
 * Random Number Generator used throughout the game
 * for all your randomness needs. Uses a 64-bit xorshift
 * pseudorandom number generator.
 */
namespace RNG
{
	class RandomState
	{
		 uint64_t _seedState;

	public:
		/// Default constructor initializing the seed by time and this type address.
		RandomState();
		/// Constructor from predefined seed.
		explicit RandomState(uint64_t seed);
		/// Get current seed.
		uint64_t getSeed() const;

		/// Get next random number.
		uint64_t next();
		/// Generates a random integer number, inclusive.
		int generate(int min, int max);
		///  Generates a percentage chance.
		bool percent(int value)
		{
			return generate(0, 99) < value;
		}
		/// Get new random-sub-sequence, that depends on current seed but each time it creates a different sequence.
		RandomState subSequence()
		{
			return RandomState{ next() ^ 0x055e3ac3461280cful}; //random value to have different new seed but still deterministic values when game run again.
		}
	};

	/// Gets the seed in use.
	uint64_t getSeed();
	/// Sets the seed in use.
	void setSeed(uint64_t n);
	/// Get state.
	RandomState& globalRandomState();
	/// Generates a random integer number, inclusive.
	int generate(int min, int max);
	/// Generates a random floating-point number.
	double generate(double min, double max);
	/// Generates a random integer number, inclusive (non-seed version).
	int seedless(int min, int max);
	/// Generates a percentage chance.
	bool percent(int value);
	///Generates a random string of specified lenght.
	std::string randomString(std::string::size_type length);
	/// Shuffles a list randomly.
	/**
	 * Randomly changes the orders of the elements in a list.
	 * @param list The container to randomize.
	 */
	template <typename T>
	void shuffle(std::vector<T> &list)
	{
		if (list.empty())
			return;
		for (size_t i = list.size() - 1; i > 0; --i)
			std::swap(list[i], list[generate(0, i)]);
	}
}

}
