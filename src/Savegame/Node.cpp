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
#include "Node.h"

namespace OpenXcom
{


Node::Node() : _id(0), _segment(0), _type(0), _rank(0), _flags(0), _reserved(0), _priority(0), _allocated(false), _dummy(false)
{

}

/**
 * Initializes a Node.
 * @param id
 * @param pos
 * @param segment
 * @param type
 * @param rank
 * @param flags
 * @param reserved
 * @param priority
 */
Node::Node(int id, Position pos, int segment, int type, int rank, int flags, int reserved, int priority) : _id(id), _pos(pos), _segment(segment), _type(type), _rank(rank), _flags(flags), _reserved(reserved), _priority(priority), _allocated(false), _dummy(false)
{
}

/**
 * clean up node.
 */
Node::~Node()
{
}




/* following data is the order in which certain alien ranks spawn on certain node ranks */
/* note that they all can fall back to rank 0 nodes - which is scout (outside ufo) */
const int Node::nodeRank[8][7] = {
	{ 4, 3, 5, 8, 7, 2, 0 }, //commander
	{ 4, 3, 5, 8, 7, 2, 0 }, //leader
	{ 5, 4, 3, 2, 7, 8, 0 }, //engineer
	{ 7, 6, 2, 8, 3, 4, 0 }, //medic
	{ 3, 4, 5, 2, 7, 8, 0 }, //navigator
	{ 2, 5, 3, 4, 6, 8, 0 }, //soldier
	{ 6, 2, 5, 3, 4, 8, 0 }, //terrorist
	{ 8, 2, 5, 3, 4, 6, 0 }  }; //also terrorist



/**
 * Loads the UFO from a YAML file.
 * @param node YAML node.
 */
void Node::load(const YAML::YamlNodeReader& reader)
{
	reader.tryRead("id", _id);
	reader.tryRead("position", _pos);
	//reader.tryRead("segment", _segment);
	reader.tryRead("type", _type);
	reader.tryRead("rank", _rank);
	reader.tryRead("flags", _flags);
	reader.tryRead("reserved", _reserved);
	reader.tryRead("priority", _priority);
	reader.tryRead("allocated", _allocated);
	reader.tryRead("links", _nodeLinks);
	reader.tryRead("linkTypes", _linkTypes);
	reader.tryRead("dummy", _dummy);
}

/**
 * Saves the UFO to a YAML file.
 * @return YAML node.
 */
void Node::save(YAML::YamlNodeWriter writer) const
{
	writer.setAsMap();
	writer.setFlowStyle();
	writer.write("id", _id);
	writer.write("position", _pos);
	//writer.write("segment", _segment);
	writer.write("type", _type);
	writer.write("rank", _rank);
	writer.write("flags", _flags);
	writer.write("reserved", _reserved);
	writer.write("priority", _priority);
	if (_allocated)
		writer.write("allocated", _allocated);
	writer.write("links", _nodeLinks);
	writer.write("linkTypes", _linkTypes);
	if (_dummy)
		writer.write("dummy", _dummy);
}

/**
 * Get the node's id
 * @return unique id
 */
int Node::getID() const
{
	return _id;
}

/**
 * Get the rank of units that can spawn on this node.
 * @return node rank
 */
NodeRank Node::getRank() const
{
	return (NodeRank)_rank;
}

/**
 * Set the rank of units that can spawn on this node.
 * @param rank the new node rank
 */
void Node::setRank(int rank)
{
	_rank = rank;
}

/**
 * Get the priority of this spawnpoint.
 * @return priority
 */
int Node::getPriority() const
{
	return _priority;
}

/**
 * Sets the node's priority
 * @param priority the priority of the spawn point
 */
void Node::setPriority(int priority)
{
	_priority = priority;
}

/**
 * Gets the Node's position.
 * @return position
 */
Position Node::getPosition() const
{
	return _pos;
}

/**
 * Sets the Node's position.
 * @param pos the new position
 */
void Node::setPosition(Position pos)
{
	_pos = pos;
}

/**
 * Gets the Node's segment.
 * @return segment
 */
int Node::getSegment() const
{
	return _segment;
}

/// get the node's paths
std::vector<int> *Node::getNodeLinks()
{
	return &_nodeLinks;
}

/**
 * get the unit types that can use the links
 * note that this is only used to not lose information in RMP files for the OXCE map editor, it's not actually used anywhere in the game
 * @return the types of units that can use the links
 */
std::vector<int> *Node::getLinkTypes()
{
	return &_linkTypes;
}

/**
 * Gets the Node's type.
 * @return type
 */
int Node::getType() const
{
	return _type;
}

bool Node::isAllocated() const
{
	return _allocated;
}

void Node::allocateNode()
{
	_allocated = true;
}

void Node::freeNode()
{
	_allocated = false;
}

bool Node::isTarget() const
{
	return _reserved == 5;
}

/**
 * Sets the node as a target or not
 * @param reserved is a target if == 5
 */
void Node::setReserved(int reserved)
{
	_reserved = reserved;
}

void Node::setType(int type)
{
	_type = type;
}

/**
 * Set's the nodes "flags" variable
 * @param flags the patrol desirability value
 */
void Node::setFlags(int flags)
{
	_flags = flags;
}

void Node::setDummy(bool dummy)
{
	_dummy = dummy;
}

bool Node::isDummy() const
{
	return _dummy;
}

}
