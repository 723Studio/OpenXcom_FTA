#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace OpenXcom
{

class RuleGlobe;

enum class CraftPathfindingMode : uint8_t;

struct CraftPlannedRoute
{
	std::vector<std::pair<double, double>> waypointsLonLat;
	double lengthRadian = 0.0;
};

class CraftPathfinding
{
public:
	static constexpr int FineWidth = 4096;
	static constexpr int FineHeight = 2048;
	static constexpr int CoarseWidth = 1024;
	static constexpr int CoarseHeight = 512;
	static constexpr double SegmentCheckStepRadian = 0.002;
	static constexpr double PolarCapRadian = 0.01;

	// Computes a surface-constrained route from start to end.
	// Returns false if start/end are not on the required surface or no route exists.
	static bool planRoute(const RuleGlobe* globe,
		double startLon, double startLat,
		double endLon, double endLat,
		CraftPathfindingMode mode,
		CraftPlannedRoute& out);

	// Helper for quick validation of a single great-circle segment.
	static bool isSegmentAllowed(const RuleGlobe* globe,
		double lon1, double lat1,
		double lon2, double lat2,
		CraftPathfindingMode mode);
};

}
