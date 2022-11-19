#pragma once
/*
 * Copyright 2010-2021 OpenXcom Developers.
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
#include <map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "../Savegame/WeightedOptions.h"

namespace OpenXcom
{

/**
 * Represents a custom Geoscape event.
 * Events are spawned using Event Script ruleset.
 */
class RuleDiplomacyFaction
{
private:
	std::string _name, _description, _background, _cardBackground, _discoverResearch, _discoverEvent, _startingResearch;
	int _genMissionFrequency, _helpTreatyGap;
	int _sellPriceFactor, _buyPriceFactor, _repPriceFactor, _stockMod, _powerHungry, _scienceBaseCost;
	int _startingReputation, _startingFunds, _startingPower;
	std::map<std::string, int> _startingItems, _startingStaff, _staffWeights;
	std::map<std::string, int> _wishList;
	std::vector<std::string> _helpTreatyMissions, _helpTreatyEventScripts, _usualEventsScripts, _happyEvents, _angryEvents, _startingResearches, _factionalEvents;
public:
	/// Creates a blank RuleDiplomacyFaction.
	RuleDiplomacyFaction(const std::string &name);
	/// Cleans up the Faction ruleset.
	~RuleDiplomacyFaction() = default;
	/// Loads the Faction definition from YAML.
	void load(const YAML::Node &node);
	/// Gets the Faction's name.
	const std::string &getName() const { return _name; }
	/// Gets the Faction's description.
	const std::string &getDescription() const { return _description; }
	/// Gets the Faction's background sprite name.
	const std::string &getBackground() const { return _background; }
	/// Gets the sprite name, that uses to render diplomacy card.
	const std::string &getCardBackground() const { return _cardBackground; }

	/// Gets the reseach name, that opens the Faction.
	const std::string& getDiscoverResearch() const { return _discoverResearch; }
	/// Gets the event name, that will spawn when Faction was discovered.
	const std::string& getDiscoverEvent() const { return _discoverEvent; }
	/// Gets the Faction's starting reputation.
	const int& getStartingReputation() const { return _startingReputation; }
	/// Gets the Faction's starting funds.
	int getStartingFunds() const { return _startingFunds; }
	/// Gets the Faction's starting power.
	int getStartingPower() const { return _startingPower; }
	/// Gets the set of Faction's starting items.
	const std::map<std::string, int>& getStartingItems() const { return _startingItems; }
	/// Gets the set of Faction's starting non-item property and personell.
	const std::map<std::string, int>& getStartingStaff() const { return _startingStaff; }
	/// Gets the research project names that are discovered by the faction from very start.
	const std::vector<std::string>& getStartingResearches() const { return _startingResearches; }


	/// Gets help treaty mission scripts for the faction.
	const std::vector<std::string>& getHelpTreatyMissions() const { return _helpTreatyMissions; }
	/// Gets mission frequency for generating.
	int getGenMissionFrequency() const { return _genMissionFrequency; }
	/// Gets time gap for help treaty missions.
	int getHelpTreatyGap() const { return _helpTreatyGap; }
	/// Gets geoscape event scripts to run as help treaty list.
	const std::vector<std::string>& getHelpTreatyEventScripts() const { return _helpTreatyEventScripts; }
	/// Gets geoscape event scripts to run without any factional's condition.
	const std::vector<std::string>& getUsualEventScripts() const { return _usualEventsScripts; }
	/// Gets geoscape events to spawn when faction is super happy with player's actions.
	const std::vector<std::string>& getHappyEvents() const { return _happyEvents; }
	/// Gets geoscape events to spawn when faction is super angry on player.
	const std::vector<std::string>& getAngryEvents() const { return _angryEvents; }
	/// Gets internal factional events to process.
	const std::vector<std::string>& getFactionalEvents() const { return _factionalEvents; }
	/// Gets power hungry value.
	int getPowerHungry() const { return _powerHungry; }
	/// Gets base cost to do science.
	int getScienceBaseCost() const { return _scienceBaseCost; }

	/// Getters for faction market data.
	int getSellPriceFactor() const { return _sellPriceFactor; }
	int getBuyPriceFactor() const { return _buyPriceFactor; }
	int getRepPriceFactor() const { return _repPriceFactor; }
	int getStockMod() const { return _stockMod; }
	const std::map<std::string, int>& getWishList() const { return _wishList; }
	const std::map<std::string, int>& getStaffWeights() const { return _staffWeights; }




};
}
