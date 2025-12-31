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
#include "../Battlescape/Position.h"
#include "../Engine/Yaml.h"
#include <vector>

namespace OpenXcom
{

enum NodeRank{
	NR_SCOUT = 0,
	NR_XCOM = 1,
	NR_SOLDIER = 2,
	NR_NAVIGATOR = 3,
	NR_LEADER = 4,
	NR_ENGINEER = 5,
	NR_MISC1 = 6,
	NR_MEDIC = 7,
	NR_MISC2 = 8,
	// extended node ranks to target new alien ranks explicitly
	NR_COMMANDER = 9,
	NR_SPECIAL_GUEST1 = 10,
	NR_HEAVY_TROOPER = 11,
	NR_ASSAULT_TROOPER = 12,
	NR_SPECIAL_GUEST2 = 13,
	NR_NON_COMBATANT = 14,
	NR_SPECIAL_GUEST3 = 15
};

/**
 * Represents a node/spawnpoint in the battlescape, loaded from RMP files.
 * @sa http://www.ufopaedia.org/index.php?title=ROUTES
 */
class Node
{
private:
	int _id;
	Position _pos;
	int _segment;
	std::vector<int> _nodeLinks;
	int _type;      // 19 = Unit type in Mapview 2
	int _rank;      // 20 = Node rank in Mapview 2
	int _flags;     // 21 = Patrol priority in Mapview 2
	int _reserved;  // 22 = Base Attack in Mapview 2
	int _priority;  // 23 = Spawn weight in Mapview 2
	bool _allocated;
	bool _dummy;
public:
	static const int CRAFTSEGMENT = 1000;
	static const int UFOSEGMENT = 2000;
	static const int TYPE_FLYING = 0x01; // non-flying unit can not spawn here when this bit is set
	static const int TYPE_SMALL = 0x02; // large unit can not spawn here when this bit is set
	static const int TYPE_DANGEROUS = 0x04; // an alien was shot here, stop patrolling to it like an idiot with a death wish
	// Preferred node ranks per alien rank index (from AlienRace.members). Backward compatible: the first
	// 8 entries mirror the old 8x7 table; extra entries can be appended without code changes.
	static const std::vector<std::vector<int>> nodeRankMap;
	static int getPrimaryNodeRankForAlien(int alienRank);
	/// Creates a Node.
	Node();
	Node(int id, Position pos, int segment, int type, int rank, int flags, int reserved, int priority);
	/// Cleans up the Node.
	~Node();
	/// Loads the node from YAML.
	void load(const YAML::YamlNodeReader& reader);
	/// Saves the node to YAML.
	void save(YAML::YamlNodeWriter writer) const;
	/// get the node's id
	int getID() const;
	/// get the node's paths
	std::vector<int> *getNodeLinks();
	/// Gets node's rank.
	NodeRank getRank() const;
	/// Gets node's priority.
	int getPriority() const;
	/// Gets the node's position.
	Position getPosition() const;
	/// Gets the node's segment.
	int getSegment() const;
	/// Gets the node's type.
	int getType() const;
	/// Sets the node's type, surprisingly
	void setType(int type);
	/// gets "flags" variable, which is really the patrolling desirability value
	int getFlags() const { return _flags; }
	/// compares the _flags variables of the nodes (for the purpose of patrol decisions!)
	bool operator<(Node &b) const { return _flags < b.getFlags(); };
	bool isAllocated() const;
	void allocateNode();
	void freeNode();
	bool isTarget() const;
	void setDummy(bool dummy);
	bool isDummy() const;

};

}
