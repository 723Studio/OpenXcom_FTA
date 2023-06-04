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
#include <utility>
#include <algorithm>

namespace OpenXcom
{

/**
 * Generic class that represents some subset of 2d space
 */
template<typename Tag, typename DataType>
struct AreaSubset
{

	//define part of surface
	DataType beg_x, end_x;
	DataType beg_y, end_y;

	AreaSubset():
			beg_x(0), end_x(0),
			beg_y(0), end_y(0)
	{

	}

	AreaSubset(int max_x, int max_y):
			beg_x(0), end_x(max_x),
			beg_y(0), end_y(max_y)
	{

	}


	AreaSubset(std::pair<int, int> range_x, std::pair<int, int> range_y):
			beg_x(range_x.first), end_x(range_x.second),
			beg_y(range_y.first), end_y(range_y.second)
	{

	}

	AreaSubset(const AreaSubset& r) = default;

	AreaSubset& operator=(const AreaSubset& r) = default;

	inline AreaSubset offset(int x, int y) const
	{
		AreaSubset ret = *this;
		ret.beg_x += x;
		ret.end_x += x;
		ret.beg_y += y;
		ret.end_y += y;
		return ret;
	}

	inline int size_x() const
	{
		return end_x - beg_x;
	}

	inline int size_y() const
	{
		return end_y - beg_y;
	}

	/**
	 * Check if area is empty.
	 * @return True if have non-zero area.
	 */
	explicit operator bool() const
	{
		return size_x() && size_y();
	}

	/**
	 * Check if two areas are same.
	 */
	bool operator==(const AreaSubset& other) const
	{
		return
			beg_x == other.beg_x &&
			end_x == other.end_x &&

			beg_y == other.beg_y &&
			end_y == other.end_y &&

			true;
	}

	/**
	 * Check if two areas are not same.
	 */
	bool operator!=(const AreaSubset& other) const
	{
		return !(*this == other);
	}

	static inline void intersectionRange(DataType& begin_a, DataType& end_a, const DataType& begin_b, const DataType& end_b)
	{
		if (begin_a >= end_b || begin_b >= end_a)
		{
			//intersection is empty
			end_a = begin_a;
		}
		else
		{
			begin_a = std::max(begin_a, begin_b);
			end_a = std::min(end_a, end_b);
		}
	}
	static inline void boundBoxRange(DataType& begin_a, DataType& end_a, const DataType& begin_b, const DataType& end_b)
	{
		if (begin_b < end_b && begin_a < end_a)
		{
			begin_a = std::min(begin_a, begin_b);
			end_a = std::max(end_a, end_b);
		}
		else if (begin_b < end_b)
		{
			begin_a = begin_b;
			end_a = end_b;
		}
		else
		{
			//keep first range same
		}
	}

	static inline AreaSubset intersection(const AreaSubset& a, const AreaSubset& b)
	{
		AreaSubset ret = a;
		intersectionRange(ret.beg_x, ret.end_x, b.beg_x, b.end_x);
		intersectionRange(ret.beg_y, ret.end_y, b.beg_y, b.end_y);
		return ret;
	}
	static inline AreaSubset intersection(const AreaSubset& a, const AreaSubset& b, const AreaSubset& c)
	{
		AreaSubset ret = intersection(a, b);
		intersectionRange(ret.beg_x, ret.end_x, c.beg_x, c.end_x);
		intersectionRange(ret.beg_y, ret.end_y, c.beg_y, c.end_y);
		return ret;
	}
	static inline AreaSubset intersection(const AreaSubset& a, const AreaSubset& b, const AreaSubset& c, const AreaSubset& d)
	{
		AreaSubset ret = intersection(a, b, c);
		intersectionRange(ret.beg_x, ret.end_x, d.beg_x, d.end_x);
		intersectionRange(ret.beg_y, ret.end_y, d.beg_y, d.end_y);
		return ret;
	}
	static inline AreaSubset boundBox(const AreaSubset& a, const AreaSubset& b)
	{
		AreaSubset ret = a;
		boundBoxRange(ret.beg_x, ret.end_x, b.beg_x, b.end_x);
		boundBoxRange(ret.beg_y, ret.end_y, b.beg_y, b.end_y);
		return ret;
	}
	static inline AreaSubset boundBox(const AreaSubset& a, const AreaSubset& b, const AreaSubset& c)
	{
		AreaSubset ret = boundBox(a, b);
		boundBoxRange(ret.beg_x, ret.end_x, c.beg_x, c.end_x);
		boundBoxRange(ret.beg_y, ret.end_y, c.beg_y, c.end_y);
		return ret;
	}



	static inline AreaSubset fromPoint(int x, int y)
	{
		return AreaSubset{ 1, 1 }.offset(x, y);
	}
};

/**
 * Subset of graphic surface
 */
using GraphSubset = AreaSubset<void, int>;

}//namespace OpenXcom
