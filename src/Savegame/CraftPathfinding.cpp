#include "CraftPathfinding.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>

#include "../fmath.h"
#include "../Mod/RuleGlobe.h"
#include "../Mod/Polygon.h"
#include "../Mod/Texture.h"
#include "../Mod/RuleCraft.h"

namespace OpenXcom
{
namespace
{
struct Vec3
{
	double x;
	double y;
	double z;
};

static Vec3 lonLatToVec(double lon, double lat)
{
	const double clat = cos(lat);
	return { clat * cos(lon), clat * sin(lon), sin(lat) };
}

static void vecToLonLat(const Vec3& v, double& lon, double& lat)
{
	lat = asin(std::clamp(v.z, -1.0, 1.0));
	lon = atan2(v.y, v.x);
	while (lon < 0) lon += 2 * M_PI;
	while (lon >= 2 * M_PI) lon -= 2 * M_PI;
}

static Vec3 slerp(const Vec3& a, const Vec3& b, double t)
{
	const double dotab = std::clamp(a.x*b.x + a.y*b.y + a.z*b.z, -1.0, 1.0);
	const double omega = acos(dotab);
	if (AreSame(omega, 0.0))
	{
		return a;
	}
	const double sinOmega = sin(omega);
	const double w1 = sin((1.0 - t) * omega) / sinOmega;
	const double w2 = sin(t * omega) / sinOmega;
	return { a.x*w1 + b.x*w2, a.y*w1 + b.y*w2, a.z*w1 + b.z*w2 };
}

static double angularDistance(double lon1, double lat1, double lon2, double lat2)
{
	if (AreSame(lon1, lon2) && AreSame(lat1, lat2))
		return 0.0;
	return acos(cos(lat1) * cos(lat2) * cos(lon2 - lon1) + sin(lat1) * sin(lat2));
}

static bool isPolarCapLand(double lat)
{
	return std::abs(lat) >= (M_PI / 2.0 - CraftPathfinding::PolarCapRadian);
}

static bool isLandForPathfinding(const RuleGlobe* globe, int textureId)
{
	(void)globe;
	(void)textureId;
	// RuleGlobe polygons represent textured land portions of the globe.
	// Treat all polygons as land to avoid creating coastal "holes" when texture
	// flags are incomplete or repurposed by mods.
	return true;
}

struct LandWaterMask
{
	std::vector<uint8_t> fineLand;   // 1 = land, 0 = water
	std::vector<uint8_t> coarseLand; // 1 = land, 0 = water (majority, kept for legacy/debug)
	std::vector<uint8_t> coarseLandCount; // 0..16 land samples per coarse cell
	const RuleGlobe* rules = nullptr;

	LandWaterMask() = default;

	static int wrapX(int x, int w)
	{
		x %= w;
		if (x < 0) x += w;
		return x;
	}

	bool fineIsLand(int x, int y) const
	{
		return fineLand[(size_t)y * CraftPathfinding::FineWidth + (size_t)x] != 0;
	}

	bool fineIsLandLonLat(double lon, double lat) const
	{
		if (isPolarCapLand(lat))
			return true;
		while (lon < 0) lon += 2 * M_PI;
		while (lon >= 2 * M_PI) lon -= 2 * M_PI;
		const double fx = (lon / (2.0 * M_PI)) * CraftPathfinding::FineWidth;
		const double fy = ((M_PI / 2.0 - lat) / M_PI) * CraftPathfinding::FineHeight;
		int x = (int)floor(fx);
		int y = (int)floor(fy);
		x = wrapX(x, CraftPathfinding::FineWidth);
		y = std::clamp(y, 0, CraftPathfinding::FineHeight - 1);
		return fineIsLand(x, y);
	}

	bool fineIsLandLonLatFuzzy(double lon, double lat, int radius) const
	{
		if (radius <= 0)
			return fineIsLandLonLat(lon, lat);
		if (isPolarCapLand(lat))
			return true;
		while (lon < 0) lon += 2 * M_PI;
		while (lon >= 2 * M_PI) lon -= 2 * M_PI;
		const double fx = (lon / (2.0 * M_PI)) * CraftPathfinding::FineWidth;
		const double fy = ((M_PI / 2.0 - lat) / M_PI) * CraftPathfinding::FineHeight;
		int x0 = (int)floor(fx);
		int y0 = (int)floor(fy);
		for (int dy = -radius; dy <= radius; ++dy)
		{
			int y = std::clamp(y0 + dy, 0, CraftPathfinding::FineHeight - 1);
			for (int dx = -radius; dx <= radius; ++dx)
			{
				int x = wrapX(x0 + dx, CraftPathfinding::FineWidth);
				if (fineIsLand(x, y))
					return true;
			}
		}
		return false;
	}

	bool coarseIsLand(int x, int y) const
	{
		return coarseLand[(size_t)y * CraftPathfinding::CoarseWidth + (size_t)x] != 0;
	}

	uint8_t coarseGetLandCount(int x, int y) const
	{
		return coarseLandCount[(size_t)y * CraftPathfinding::CoarseWidth + (size_t)x];
	}

	static void lonLatToFineXY(double lon, double lat, int& x, int& y)
	{
		while (lon < 0) lon += 2 * M_PI;
		while (lon >= 2 * M_PI) lon -= 2 * M_PI;
		x = (int)floor((lon / (2.0 * M_PI)) * CraftPathfinding::FineWidth);
		y = (int)floor(((M_PI / 2.0 - lat) / M_PI) * CraftPathfinding::FineHeight);
		x = wrapX(x, CraftPathfinding::FineWidth);
		y = std::clamp(y, 0, CraftPathfinding::FineHeight - 1);
	}

	static void lonLatToCoarseXY(double lon, double lat, int& x, int& y)
	{
		while (lon < 0) lon += 2 * M_PI;
		while (lon >= 2 * M_PI) lon -= 2 * M_PI;
		x = (int)floor((lon / (2.0 * M_PI)) * CraftPathfinding::CoarseWidth);
		y = (int)floor(((M_PI / 2.0 - lat) / M_PI) * CraftPathfinding::CoarseHeight);
		x = wrapX(x, CraftPathfinding::CoarseWidth);
		y = std::clamp(y, 0, CraftPathfinding::CoarseHeight - 1);
	}

	static void coarseCellCenterLonLat(int x, int y, double& lon, double& lat)
	{
		lon = ((x + 0.5) / (double)CraftPathfinding::CoarseWidth) * 2.0 * M_PI;
		lat = (M_PI / 2.0) - ((y + 0.5) / (double)CraftPathfinding::CoarseHeight) * M_PI;
	}

	static void fineCellCenterLonLat(int x, int y, double& lon, double& lat)
	{
		lon = ((x + 0.5) / (double)CraftPathfinding::FineWidth) * 2.0 * M_PI;
		lat = (M_PI / 2.0) - ((y + 0.5) / (double)CraftPathfinding::FineHeight) * M_PI;
	}
};

static LandWaterMask& getMask(const RuleGlobe* globe)
{
	static LandWaterMask mask;
	if (mask.rules == globe && !mask.fineLand.empty() && !mask.coarseLand.empty() && !mask.coarseLandCount.empty())
		return mask;

	mask = LandWaterMask{};
	mask.rules = globe;
	mask.fineLand.assign((size_t)CraftPathfinding::FineWidth * CraftPathfinding::FineHeight, 0);

	// Rasterize land polygons into equirectangular grid.
	for (Polygon* poly : *globe->getPolygons())
	{
		if (!poly)
			continue;
		if (!isLandForPathfinding(globe, poly->getTexture()))
			continue;

		const int n = poly->getPoints();
		if (n < 3)
			continue;

		std::vector<double> xs;
		std::vector<double> ys;
		xs.reserve((size_t)n);
		ys.reserve((size_t)n);

		for (int i = 0; i < n; ++i)
		{
			double lon = poly->getLongitude(i);
			double lat = poly->getLatitude(i);
			while (lon < 0) lon += 2 * M_PI;
			while (lon >= 2 * M_PI) lon -= 2 * M_PI;
			double x = (lon / (2.0 * M_PI)) * CraftPathfinding::FineWidth;
			double y = ((M_PI / 2.0 - lat) / M_PI) * CraftPathfinding::FineHeight;
			xs.push_back(x);
			ys.push_back(y);
		}

		// Unwrap X to avoid dateline jump.
		for (int i = 1; i < n; ++i)
		{
			double dx = xs[i] - xs[i - 1];
			if (dx > CraftPathfinding::FineWidth / 2.0)
				xs[i] -= CraftPathfinding::FineWidth;
			else if (dx < -CraftPathfinding::FineWidth / 2.0)
				xs[i] += CraftPathfinding::FineWidth;
		}

		double minYd = ys[0], maxYd = ys[0];
		for (int i = 1; i < n; ++i)
		{
			minYd = std::min(minYd, ys[i]);
			maxYd = std::max(maxYd, ys[i]);
		}
		int minY = std::clamp((int)floor(minYd), 0, CraftPathfinding::FineHeight - 1);
		int maxY = std::clamp((int)ceil(maxYd), 0, CraftPathfinding::FineHeight - 1);

		for (int y = minY; y <= maxY; ++y)
		{
			const double scanY = y + 0.5;
			std::vector<double> inter;
			inter.reserve((size_t)n);
			for (int i = 0; i < n; ++i)
			{
				int j = (i + 1) % n;
				double y1 = ys[i];
				double y2 = ys[j];
				double x1 = xs[i];
				double x2 = xs[j];

				// Skip horizontal edges.
				if (AreSame(y1, y2))
					continue;
				// Check if scanline intersects edge (half-open interval).
				const bool cond = (scanY >= std::min(y1, y2)) && (scanY < std::max(y1, y2));
				if (!cond)
					continue;
				double t = (scanY - y1) / (y2 - y1);
				double x = x1 + t * (x2 - x1);
				inter.push_back(x);
			}

			if (inter.size() < 2)
				continue;
			std::sort(inter.begin(), inter.end());
			for (size_t k = 0; k + 1 < inter.size(); k += 2)
			{
				int xStart = (int)floor(inter[k]);
				int xEnd = (int)ceil(inter[k + 1]);
				for (int x = xStart; x <= xEnd; ++x)
				{
					int wx = LandWaterMask::wrapX(x, CraftPathfinding::FineWidth);
					mask.fineLand[(size_t)y * CraftPathfinding::FineWidth + (size_t)wx] = 1;
				}
			}
		}
	}

	// Polar caps treated as land.
	const int capRows = (int)ceil((CraftPathfinding::PolarCapRadian / M_PI) * CraftPathfinding::FineHeight);
	for (int y = 0; y < capRows; ++y)
	{
		for (int x = 0; x < CraftPathfinding::FineWidth; ++x)
		{
			mask.fineLand[(size_t)y * CraftPathfinding::FineWidth + (size_t)x] = 1;
			mask.fineLand[(size_t)(CraftPathfinding::FineHeight - 1 - y) * CraftPathfinding::FineWidth + (size_t)x] = 1;
		}
	}

	// Build coarse mask by majority downsampling (4x4 blocks).
	mask.coarseLand.assign((size_t)CraftPathfinding::CoarseWidth * CraftPathfinding::CoarseHeight, 0);
	mask.coarseLandCount.assign((size_t)CraftPathfinding::CoarseWidth * CraftPathfinding::CoarseHeight, 0);
	for (int cy = 0; cy < CraftPathfinding::CoarseHeight; ++cy)
	{
		for (int cx = 0; cx < CraftPathfinding::CoarseWidth; ++cx)
		{
			int landCount = 0;
			for (int oy = 0; oy < 4; ++oy)
			{
				int fy = cy * 4 + oy;
				for (int ox = 0; ox < 4; ++ox)
				{
					int fx = cx * 4 + ox;
					landCount += mask.fineIsLand(fx, fy) ? 1 : 0;
				}
			}
			const size_t idx = (size_t)cy * CraftPathfinding::CoarseWidth + (size_t)cx;
			mask.coarseLandCount[idx] = (uint8_t)std::clamp(landCount, 0, 16);
			mask.coarseLand[idx] = (landCount >= 8) ? 1 : 0;
		}
	}

	return mask;
}

static bool isAllowedSurface(const LandWaterMask& mask, double lon, double lat, CraftPathfindingMode mode)
{
	if (mode == CraftPathfindingMode::BOTH)
		return true;
	const bool land = mask.fineIsLandLonLat(lon, lat);
	return (mode == CraftPathfindingMode::ONLY_LAND) ? land : !land;
}

static bool isAllowedSurfaceEndpoint(const LandWaterMask& mask, double lon, double lat, CraftPathfindingMode mode)
{
	if (mode == CraftPathfindingMode::BOTH)
		return true;
	// Fuzzy endpoint check to tolerate clicks near coastlines.
	const bool land = mask.fineIsLandLonLatFuzzy(lon, lat, 1);
	return (mode == CraftPathfindingMode::ONLY_LAND) ? land : !land;
}

static bool isAllowedCoarseCell(const LandWaterMask& mask, int x, int y, CraftPathfindingMode mode)
{
	if (mode == CraftPathfindingMode::BOTH)
		return true;
	// Coarse cells are only an A* guide. To avoid false negatives near coasts,
	// consider a coarse cell passable if it contains at least one fine sample
	// of the required surface.
	const uint8_t landCount = mask.coarseGetLandCount(x, y);
	if (mode == CraftPathfindingMode::ONLY_LAND)
		return landCount != 0;
	return landCount != 16;
}

static bool isSegmentAllowedFine(const LandWaterMask& mask,
	double lon1, double lat1,
	double lon2, double lat2,
	CraftPathfindingMode mode);

static bool pickAllowedPointInCoarseCell(const LandWaterMask& mask, int cx, int cy, CraftPathfindingMode mode, double& outLon, double& outLat)
{
	if (mode == CraftPathfindingMode::BOTH)
	{
		LandWaterMask::coarseCellCenterLonLat(cx, cy, outLon, outLat);
		return true;
	}

	double bestLon = 0.0;
	double bestLat = 0.0;
	double bestScore = std::numeric_limits<double>::infinity();

	double centerLon, centerLat;
	LandWaterMask::coarseCellCenterLonLat(cx, cy, centerLon, centerLat);

	bool found = false;
	for (int oy = 0; oy < 4; ++oy)
	{
		const int fy = cy * 4 + oy;
		for (int ox = 0; ox < 4; ++ox)
		{
			const int fx = cx * 4 + ox;
			double lon, lat;
			LandWaterMask::fineCellCenterLonLat(fx, fy, lon, lat);
			if (!isAllowedSurface(mask, lon, lat, mode))
				continue;
			const double score = angularDistance(lon, lat, centerLon, centerLat);
			if (score < bestScore)
			{
				bestScore = score;
				bestLon = lon;
				bestLat = lat;
				found = true;
			}
		}
	}

	if (!found)
		return false;
	outLon = bestLon;
	outLat = bestLat;
	return true;
}

static void collectAllowedPointsInCoarseCell(const LandWaterMask& mask, int cx, int cy, CraftPathfindingMode mode, std::vector<std::pair<double, double>>& out)
{
	out.clear();
	out.reserve(16);
	for (int oy = 0; oy < 4; ++oy)
	{
		const int fy = cy * 4 + oy;
		for (int ox = 0; ox < 4; ++ox)
		{
			const int fx = cx * 4 + ox;
			double lon, lat;
			LandWaterMask::fineCellCenterLonLat(fx, fy, lon, lat);
			if (isAllowedSurface(mask, lon, lat, mode))
				out.push_back({ lon, lat });
		}
	}
}

static bool refineCoarsePathToWaypoints(const LandWaterMask& mask,
	CraftPathfindingMode mode,
	double startLon, double startLat,
	double endLon, double endLat,
	const std::vector<int>& coarsePath,
	std::vector<std::pair<double, double>>& outPts)
{
	outPts.clear();
	if (coarsePath.size() < 2)
		return false;

	const int w = CraftPathfinding::CoarseWidth;

	int curCx, curCy;
	LandWaterMask::lonLatToCoarseXY(startLon, startLat, curCx, curCy);

	std::pair<double, double> current = { startLon, startLat };
	outPts.push_back(current);

	std::vector<std::pair<double, double>> ptsA;
	std::vector<std::pair<double, double>> ptsB;

	auto pushPointIfNew = [&](const std::pair<double, double>& p)
	{
		if (outPts.empty())
		{
			outPts.push_back(p);
			return;
		}
		if (!AreSame(outPts.back().first, p.first) || !AreSame(outPts.back().second, p.second))
			outPts.push_back(p);
	};

	// Walk intermediate coarse cells (excluding the final one, handled separately).
	for (size_t i = 1; i + 1 < coarsePath.size(); ++i)
	{
		const int idx = coarsePath[i];
		const int nextCx = idx % w;
		const int nextCy = idx / w;

		collectAllowedPointsInCoarseCell(mask, nextCx, nextCy, mode, ptsB);
		if (ptsB.empty())
			return false;

		// Prefer a point that is directly connectable from current.
		double nextCenterLon, nextCenterLat;
		LandWaterMask::coarseCellCenterLonLat(nextCx, nextCy, nextCenterLon, nextCenterLat);
		bool foundDirect = false;
		std::pair<double, double> bestDirect;
		double bestDirectScore = std::numeric_limits<double>::infinity();
		for (const auto& b : ptsB)
		{
			if (!isSegmentAllowedFine(mask, current.first, current.second, b.first, b.second, mode))
				continue;
			const double score = angularDistance(b.first, b.second, nextCenterLon, nextCenterLat);
			if (score < bestDirectScore)
			{
				bestDirectScore = score;
				bestDirect = b;
				foundDirect = true;
			}
		}
		if (foundDirect)
		{
			pushPointIfNew(bestDirect);
			current = bestDirect;
			curCx = nextCx;
			curCy = nextCy;
			continue;
		}

		// Fallback: allow a reposition point inside current coarse cell, then jump to next.
		collectAllowedPointsInCoarseCell(mask, curCx, curCy, mode, ptsA);
		if (ptsA.empty())
			return false;

		bool foundTwoStep = false;
		std::pair<double, double> bestA;
		std::pair<double, double> bestB;
		double bestScore = std::numeric_limits<double>::infinity();

		double curCenterLon, curCenterLat;
		LandWaterMask::coarseCellCenterLonLat(curCx, curCy, curCenterLon, curCenterLat);

		for (const auto& a : ptsA)
		{
			if (!isSegmentAllowedFine(mask, current.first, current.second, a.first, a.second, mode))
				continue;
			for (const auto& b : ptsB)
			{
				if (!isSegmentAllowedFine(mask, a.first, a.second, b.first, b.second, mode))
					continue;
				const double score = angularDistance(a.first, a.second, curCenterLon, curCenterLat)
					+ angularDistance(b.first, b.second, nextCenterLon, nextCenterLat);
				if (score < bestScore)
				{
					bestScore = score;
					bestA = a;
					bestB = b;
					foundTwoStep = true;
				}
			}
		}
		if (!foundTwoStep)
			return false;

		pushPointIfNew(bestA);
		pushPointIfNew(bestB);
		current = bestB;
		curCx = nextCx;
		curCy = nextCy;
	}

	// Handle end: pick a fine point in end cell that connects from current, then connect to the exact end.
	int endCx, endCy;
	LandWaterMask::lonLatToCoarseXY(endLon, endLat, endCx, endCy);
	collectAllowedPointsInCoarseCell(mask, endCx, endCy, mode, ptsB);
	if (ptsB.empty())
		return false;

	bool foundEnd = false;
	std::pair<double, double> bestEnd;
	const std::pair<double, double> endExact = { endLon, endLat };
	for (const auto& b : ptsB)
	{
		if (!isSegmentAllowedFine(mask, current.first, current.second, b.first, b.second, mode))
			continue;
		if (!isSegmentAllowedFine(mask, b.first, b.second, endExact.first, endExact.second, mode))
			continue;
		bestEnd = b;
		foundEnd = true;
		break;
	}
	if (!foundEnd)
	{
		// Allow a reposition in current cell as a last resort.
		collectAllowedPointsInCoarseCell(mask, curCx, curCy, mode, ptsA);
		for (const auto& a : ptsA)
		{
			if (!isSegmentAllowedFine(mask, current.first, current.second, a.first, a.second, mode))
				continue;
			for (const auto& b : ptsB)
			{
				if (!isSegmentAllowedFine(mask, a.first, a.second, b.first, b.second, mode))
					continue;
				if (!isSegmentAllowedFine(mask, b.first, b.second, endExact.first, endExact.second, mode))
					continue;
				pushPointIfNew(a);
				bestEnd = b;
				foundEnd = true;
				break;
			}
			if (foundEnd)
				break;
		}
	}
	if (!foundEnd)
		return false;

	pushPointIfNew(bestEnd);
	pushPointIfNew(endExact);
	return true;
}

struct FineBandNode
{
	int fx = 0;
	int fy = 0;
	double lon = 0.0;
	double lat = 0.0;
};

static uint32_t fineKey(int fx, int fy)
{
	return ((uint32_t)fy << 12) | (uint32_t)(fx & 0xFFF);
}

static bool pickNearestFineBandNode(const LandWaterMask& mask,
	const std::unordered_map<uint32_t, int>& indexByKey,
	const std::vector<FineBandNode>& nodes,
	double lon, double lat,
	int& outIndex)
{
	int x0, y0;
	LandWaterMask::lonLatToFineXY(lon, lat, x0, y0);

	int best = -1;
	double bestDist = std::numeric_limits<double>::infinity();

	for (int r = 0; r <= 2; ++r)
	{
		for (int dy = -r; dy <= r; ++dy)
		{
			int y = std::clamp(y0 + dy, 0, CraftPathfinding::FineHeight - 1);
			for (int dx = -r; dx <= r; ++dx)
			{
				int x = LandWaterMask::wrapX(x0 + dx, CraftPathfinding::FineWidth);
				auto it = indexByKey.find(fineKey(x, y));
				if (it == indexByKey.end())
					continue;
				const FineBandNode& n = nodes[(size_t)it->second];
				const double d = angularDistance(lon, lat, n.lon, n.lat);
				if (d < bestDist)
				{
					bestDist = d;
					best = it->second;
				}
			}
		}
		if (best != -1)
			break;
	}

	if (best == -1)
		return false;
	(void)mask;
	outIndex = best;
	return true;
}

static bool aStarFineBand(const LandWaterMask& mask,
	CraftPathfindingMode mode,
	double startLon, double startLat,
	double endLon, double endLat,
	const std::vector<int>& coarsePath,
	std::vector<std::pair<double, double>>& outPts)
{
	if (coarsePath.size() < 2)
		return false;

	std::unordered_map<uint32_t, int> indexByKey;
	indexByKey.reserve(coarsePath.size() * 200);
	std::vector<FineBandNode> nodes;
	nodes.reserve(coarsePath.size() * 200);

	const int cw = CraftPathfinding::CoarseWidth;
	const int ch = CraftPathfinding::CoarseHeight;

	auto tryAddFine = [&](int fx, int fy)
	{
		fx = LandWaterMask::wrapX(fx, CraftPathfinding::FineWidth);
		fy = std::clamp(fy, 0, CraftPathfinding::FineHeight - 1);
		uint32_t k = fineKey(fx, fy);
		if (indexByKey.find(k) != indexByKey.end())
			return;
		double lon, lat;
		LandWaterMask::fineCellCenterLonLat(fx, fy, lon, lat);
		if (!isAllowedSurface(mask, lon, lat, mode))
			return;
		int idx = (int)nodes.size();
		indexByKey.emplace(k, idx);
		nodes.push_back({ fx, fy, lon, lat });
	};

	// Build a band of fine nodes from coarsePath cells plus a 1-cell coarse neighborhood.
	for (int coarseIdx : coarsePath)
	{
		int cx = coarseIdx % cw;
		int cy = coarseIdx / cw;
		for (int oy = -1; oy <= 1; ++oy)
		{
			int ncy = cy + oy;
			if (ncy < 0 || ncy >= ch)
				continue;
			for (int ox = -1; ox <= 1; ++ox)
			{
				int ncx = LandWaterMask::wrapX(cx + ox, cw);
				for (int fy = ncy * 4; fy < ncy * 4 + 4; ++fy)
				{
					for (int fx = ncx * 4; fx < ncx * 4 + 4; ++fx)
					{
						tryAddFine(fx, fy);
					}
				}
			}
		}
	}

	if (nodes.size() < 2)
		return false;

	int startIdx = -1;
	int goalIdx = -1;
	if (!pickNearestFineBandNode(mask, indexByKey, nodes, startLon, startLat, startIdx))
		return false;
	if (!pickNearestFineBandNode(mask, indexByKey, nodes, endLon, endLat, goalIdx))
		return false;

	struct PQ
	{
		int idx;
		double f;
		double g;
	};
	struct PQGreater
	{
		bool operator()(const PQ& a, const PQ& b) const { return a.f > b.f; }
	};

	std::priority_queue<PQ, std::vector<PQ>, PQGreater> open;
	std::vector<double> gScore(nodes.size(), std::numeric_limits<double>::infinity());
	std::vector<int> parent(nodes.size(), -1);

	auto heuristic = [&](int idx)
	{
		return angularDistance(nodes[(size_t)idx].lon, nodes[(size_t)idx].lat, nodes[(size_t)goalIdx].lon, nodes[(size_t)goalIdx].lat);
	};

	gScore[(size_t)startIdx] = 0.0;
	open.push({ startIdx, heuristic(startIdx), 0.0 });

	static const int dx8[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
	static const int dy8[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	while (!open.empty())
	{
		PQ cur = open.top();
		open.pop();
		if (cur.idx == goalIdx)
			break;
		if (cur.g > gScore[(size_t)cur.idx])
			continue;

		const FineBandNode& c = nodes[(size_t)cur.idx];
		for (int k = 0; k < 8; ++k)
		{
			int nfx = LandWaterMask::wrapX(c.fx + dx8[k], CraftPathfinding::FineWidth);
			int nfy = c.fy + dy8[k];
			if (nfy < 0 || nfy >= CraftPathfinding::FineHeight)
				continue;
			auto it = indexByKey.find(fineKey(nfx, nfy));
			if (it == indexByKey.end())
				continue;
			int nidx = it->second;
			const FineBandNode& n = nodes[(size_t)nidx];

			// Very local step; still validate surface constraint on the great-circle segment.
			if (!isSegmentAllowedFine(mask, c.lon, c.lat, n.lon, n.lat, mode))
				continue;

			double step = angularDistance(c.lon, c.lat, n.lon, n.lat);
			double tentative = gScore[(size_t)cur.idx] + step;
			if (tentative < gScore[(size_t)nidx])
			{
				gScore[(size_t)nidx] = tentative;
				parent[(size_t)nidx] = cur.idx;
				open.push({ nidx, tentative + heuristic(nidx), tentative });
			}
		}
	}

	if (goalIdx != startIdx && parent[(size_t)goalIdx] == -1)
		return false;

	std::vector<int> path;
	int cur = goalIdx;
	while (cur != -1)
	{
		path.push_back(cur);
		if (cur == startIdx)
			break;
		cur = parent[(size_t)cur];
	}
	if (path.empty() || path.back() != startIdx)
		return false;
	std::reverse(path.begin(), path.end());

	std::vector<std::pair<double, double>> pts;
	pts.reserve(path.size() + 2);
	pts.push_back({ startLon, startLat });
	for (size_t i = 1; i + 1 < path.size(); ++i)
	{
		const FineBandNode& n = nodes[(size_t)path[i]];
		pts.push_back({ n.lon, n.lat });
	}
	pts.push_back({ endLon, endLat });

	outPts.swap(pts);
	return true;
}

struct OpenNode
{
	int idx;
	double f;
	double g;
};

struct OpenNodeGreater
{
	bool operator()(const OpenNode& a, const OpenNode& b) const
	{
		return a.f > b.f;
	}
};

static bool aStarCoarse(const LandWaterMask& mask,
	int sx, int sy,
	int gx, int gy,
	CraftPathfindingMode mode,
	std::vector<int>& outPath)
{
	const int w = CraftPathfinding::CoarseWidth;
	const int h = CraftPathfinding::CoarseHeight;
	const int start = sy * w + sx;
	const int goal = gy * w + gx;

	static std::vector<double> gScore;
	static std::vector<int> parent;
	static std::vector<uint32_t> stamp;
	static uint32_t curStamp = 1;

	const int n = w * h;
	if ((int)gScore.size() != n)
	{
		gScore.assign(n, std::numeric_limits<double>::infinity());
		parent.assign(n, -1);
		stamp.assign(n, 0);
		curStamp = 1;
	}
	if (++curStamp == 0)
	{
		std::fill(stamp.begin(), stamp.end(), 0);
		curStamp = 1;
	}

	auto touch = [&](int idx)
	{
		if (stamp[idx] != curStamp)
		{
			stamp[idx] = curStamp;
			gScore[idx] = std::numeric_limits<double>::infinity();
			parent[idx] = -1;
		}
	};

	std::priority_queue<OpenNode, std::vector<OpenNode>, OpenNodeGreater> open;

	touch(start);
	gScore[start] = 0.0;

	double goalLon, goalLat;
	LandWaterMask::coarseCellCenterLonLat(gx, gy, goalLon, goalLat);

	auto heuristic = [&](int x, int y)
	{
		double lon, lat;
		LandWaterMask::coarseCellCenterLonLat(x, y, lon, lat);
		return angularDistance(lon, lat, goalLon, goalLat);
	};

	open.push({ start, heuristic(sx, sy), 0.0 });

	static const int dx8[8] = { -1, 1, 0, 0, -1, -1, 1, 1 };
	static const int dy8[8] = { 0, 0, -1, 1, -1, 1, -1, 1 };

	while (!open.empty())
	{
		OpenNode cur = open.top();
		open.pop();

		if (cur.idx == goal)
			break;

		int cx = cur.idx % w;
		int cy = cur.idx / w;

		// Outdated entry.
		touch(cur.idx);
		if (cur.g > gScore[cur.idx])
			continue;

		double curLon, curLat;
		LandWaterMask::coarseCellCenterLonLat(cx, cy, curLon, curLat);

		for (int k = 0; k < 8; ++k)
		{
			int nx = cx + dx8[k];
			int ny = cy + dy8[k];
			nx = LandWaterMask::wrapX(nx, w);
			if (ny < 0 || ny >= h)
				continue;
			if (!isAllowedCoarseCell(mask, nx, ny, mode))
				continue;

			int nidx = ny * w + nx;
			touch(nidx);

			double nLon, nLat;
			LandWaterMask::coarseCellCenterLonLat(nx, ny, nLon, nLat);
			double step = angularDistance(curLon, curLat, nLon, nLat);
			double tentative = gScore[cur.idx] + step;
			if (tentative < gScore[nidx])
			{
				gScore[nidx] = tentative;
				parent[nidx] = cur.idx;
				open.push({ nidx, tentative + heuristic(nx, ny), tentative });
			}
		}
	}

	// Reconstruct.
	touch(goal);
	if (parent[goal] == -1 && goal != start)
		return false;

	outPath.clear();
	int cur = goal;
	while (cur != -1)
	{
		outPath.push_back(cur);
		if (cur == start)
			break;
		cur = parent[cur];
	}
	if (outPath.empty() || outPath.back() != start)
		return false;
	std::reverse(outPath.begin(), outPath.end());
	return true;
}

static bool isSegmentAllowedFine(const LandWaterMask& mask,
	double lon1, double lat1,
	double lon2, double lat2,
	CraftPathfindingMode mode)
{
	if (mode == CraftPathfindingMode::BOTH)
		return true;

	const double dist = angularDistance(lon1, lat1, lon2, lat2);
	if (AreSame(dist, 0.0))
		return isAllowedSurface(mask, lon1, lat1, mode);

	const int steps = std::max(1, (int)ceil(dist / CraftPathfinding::SegmentCheckStepRadian));
	const Vec3 a = lonLatToVec(lon1, lat1);
	const Vec3 b = lonLatToVec(lon2, lat2);

	for (int i = 0; i <= steps; ++i)
	{
		double t = (double)i / (double)steps;
		Vec3 p = slerp(a, b, t);
		double lon, lat;
		vecToLonLat(p, lon, lat);
		if (!isAllowedSurface(mask, lon, lat, mode))
			return false;
	}
	return true;
}

static void shortcutPath(const LandWaterMask& mask, CraftPathfindingMode mode, std::vector<std::pair<double,double>>& pts)
{
	if (pts.size() <= 2)
		return;

	std::vector<std::pair<double,double>> out;
	out.reserve(pts.size());

	size_t i = 0;
	while (i + 1 < pts.size())
	{
		out.push_back(pts[i]);
		size_t best = i + 1;
		for (size_t j = pts.size() - 1; j > i + 1; --j)
		{
			if (isSegmentAllowedFine(mask, pts[i].first, pts[i].second, pts[j].first, pts[j].second, mode))
			{
				best = j;
				break;
			}
		}
		i = best;
	}
	out.push_back(pts.back());
	pts.swap(out);
}

static double sumPathLengthRadian(const std::vector<std::pair<double,double>>& pts)
{
	double sum = 0.0;
	for (size_t i = 1; i < pts.size(); ++i)
	{
		sum += angularDistance(pts[i-1].first, pts[i-1].second, pts[i].first, pts[i].second);
	}
	return sum;
}

}

bool CraftPathfinding::isSegmentAllowed(const RuleGlobe* globe,
	double lon1, double lat1,
	double lon2, double lat2,
	CraftPathfindingMode mode)
{
	const LandWaterMask& mask = getMask(globe);
	return isSegmentAllowedFine(mask, lon1, lat1, lon2, lat2, mode);
}

bool CraftPathfinding::planRoute(const RuleGlobe* globe,
	double startLon, double startLat,
	double endLon, double endLat,
	CraftPathfindingMode mode,
	CraftPlannedRoute& out)
{
	out = CraftPlannedRoute{};
	if (!globe)
		return false;
	if (mode == CraftPathfindingMode::BOTH)
	{
		out.waypointsLonLat = { { endLon, endLat } };
		out.lengthRadian = angularDistance(startLon, startLat, endLon, endLat);
		return true;
	}

	const LandWaterMask& mask = getMask(globe);
	if (!isAllowedSurfaceEndpoint(mask, startLon, startLat, mode))
		return false;
	if (!isAllowedSurfaceEndpoint(mask, endLon, endLat, mode))
		return false;

	int sx, sy, gx, gy;
	LandWaterMask::lonLatToCoarseXY(startLon, startLat, sx, sy);
	LandWaterMask::lonLatToCoarseXY(endLon, endLat, gx, gy);

	if (!isAllowedCoarseCell(mask, sx, sy, mode))
		return false;
	if (!isAllowedCoarseCell(mask, gx, gy, mode))
		return false;

	std::vector<int> coarsePath;
	if (!aStarCoarse(mask, sx, sy, gx, gy, mode, coarsePath))
		return false;

	// Convert coarse path to lon/lat points.
	std::vector<std::pair<double,double>> pts;
	if (!refineCoarsePathToWaypoints(mask, mode, startLon, startLat, endLon, endLat, coarsePath, pts))
	{
		// Fallback for tricky coastal geometry: do a fine-grid A* in a narrow band around the coarse path.
		if (!aStarFineBand(mask, mode, startLon, startLat, endLon, endLat, coarsePath, pts))
			return false;
	}

	// Validate and shortcut.
	shortcutPath(mask, mode, pts);

	// Final validation on each segment.
	for (size_t i = 1; i < pts.size(); ++i)
	{
		if (!isSegmentAllowedFine(mask, pts[i-1].first, pts[i-1].second, pts[i].first, pts[i].second, mode))
			return false;
	}

	out.waypointsLonLat = std::move(pts);
	out.lengthRadian = sumPathLengthRadian(out.waypointsLonLat);
	return true;
}

}
