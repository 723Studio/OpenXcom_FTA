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
#include "Transfer.h"
#include "Base.h"
#include "Soldier.h"
#include "Craft.h"
#include "ItemContainer.h"
#include "BasePrisoner.h"
#include "../Engine/Language.h"
#include "../Mod/Mod.h"
#include "../Engine/Logger.h"

namespace OpenXcom
{

/**
 * Initializes a transfer.
 * @param hours Hours in-transit.
 */
Transfer::Transfer(int hours) : _hours(hours), _soldier(0), _craft(0), _prisoner(0), _itemQty(0), _delivered(false)
{
}

/**
 * Cleans up undelivered transfers.
 */
Transfer::~Transfer()
{
	if (!_delivered)
	{
		delete _soldier;
		delete _craft;
		delete _prisoner;
	}
}

/**
 * Loads the transfer from a YAML file.
 * @param node YAML node.
 * @param base Destination base.
 * @param rule Game mod.
 * @param save Pointer to savegame.
 * @return Was the transfer content valid?
 */
bool Transfer::load(const YAML::YamlNodeReader& reader, Base *base, const Mod *mod, SavedGame *save)
{
	reader.tryRead("hours", _hours);

	if (const auto& soldier = reader["soldier"])
	{
		std::string type = soldier["type"].readVal(mod->getSoldiersList().front());
		if (mod->getSoldier(type))
		{
			_soldier = new Soldier(mod->getSoldier(type), nullptr, 0 /*nationality*/);
			_soldier->load(soldier, mod, save, mod->getScriptGlobal());
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load soldier " << type;
			delete this;
			return false;
		}
	}
	if (const auto& craft = reader["craft"])
	{
		std::string type = craft["type"].readVal<std::string>();
		if (mod->getCraft(type))
		{
			_craft = new Craft(mod->getCraft(type), base);
			_craft->load(craft, mod->getScriptGlobal(), mod, 0);
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load craft " << type;
			delete this;
			return false;
		}

	}
	if (const auto& item = reader["itemId"])
	{
		std::string name = item.readVal<std::string>();
		_itemId = mod->getItem(name);
		if (!_itemId)
		{
			Log(LOG_ERROR) << "Failed to load item " << name;
			delete this;
			return false;
		}
	}
	reader.tryRead("itemQty", _itemQty);
	int legacyStaff = 0;
	if (reader.tryRead("scientists", legacyStaff) || reader.tryRead("engineers", legacyStaff))
	{
		Log(LOG_WARNING) << "Ignoring legacy scientist/engineer transfer from unsupported OXCE staff-counter save data.";
		delete this;
		return false;
	}
	if (const auto& prisoner = reader["prisoner"])
	{
		std::string id = prisoner["id"].readVal<std::string>();
		std::string type = prisoner["type"].readVal<std::string>();
		if (mod->getPrisonerRules(type) != nullptr)
		{
			_prisoner = new BasePrisoner(mod->getPrisonerRules(type), 0, type, id);
			_prisoner->load(prisoner, mod);
		}
		else
		{
			Log(LOG_ERROR) << "Failed to load prisoner " << type;
			delete this;
			return false;
		}
	}
	reader.tryRead("delivered", _delivered);
	return true;
}

/**
 * Saves the transfer to a YAML file.
 * @param writer YAML writer.
 * @param b Base the transfer belongs to.
 * @param mod Mod for the transfer.
 */
void Transfer::save(YAML::YamlNodeWriter writer, const Base* b, const Mod* mod) const
{
	writer.setAsMap();
	writer.write("hours", _hours);
	if (_soldier != 0)
	{
		_soldier->save(writer["soldier"], mod->getScriptGlobal());
	}
	else if (_craft != 0)
	{
		_craft->save(writer["craft"], mod->getScriptGlobal());
	}
	else if (_itemQty != 0)
	{
		writer.write("itemId", _itemId->getType());
		writer.write("itemQty", _itemQty);
	}
	else if (_prisoner != 0)
	{
		_prisoner->save(writer["prisoner"]);
	}

	if (_delivered)
		writer.write("delivered", _delivered);
}

/**
 * Changes the soldier being transferred.
 * @param soldier Pointer to soldier.
 */
void Transfer::setSoldier(Soldier *soldier)
{
	_soldier = soldier;
}

/**
 * Changes the craft being transferred.
 * @param craft Pointer to craft.
 */
void Transfer::setCraft(Craft *craft)
{
	_craft = craft;
}

/**
 * Gets the craft being transferred.
 * @return a Pointer to craft.
 */
Craft *Transfer::getCraft()
{
	return _craft;
}

/**
 * Returns the items being transferred.
 * @return Item ID.
 */
const RuleItem* Transfer::getItems() const
{
	return _itemId;
}

/**
 * Changes the items being transferred.
 * @param id Item identifier.
 * @param qty Item quantity.
 */
void Transfer::setItems(const RuleItem* rule, int qty)
{
	_itemId = rule;
	_itemQty = qty;
}

/**
 * Returns the name of the contents of the transfer.
 * @param lang Language to get strings from.
 * @return Name string.
 */
std::string Transfer::getName(Language *lang) const
{
	if (_soldier != 0)
	{
		return _soldier->getName();
	}
	else if (_craft != 0)
	{
		return _craft->getName(lang);
	}
	else if (_prisoner != 0)
	{
		return _prisoner->getId();
	}
	return lang->getString(_itemId->getType());
}

/**
 * Returns the time remaining until the
 * transfer arrives at its destination.
 * @return Amount of hours.
 */
int Transfer::getHours() const
{
	return _hours;
}

/**
 * Returns the quantity of items in the transfer.
 * @return Amount of items.
 */
int Transfer::getQuantity() const
{
	if (_itemQty != 0)
	{
		return _itemQty;
	}
	return 1;
}

/**
 * Returns the type of the contents of the transfer.
 * @return TransferType.
 */
TransferType Transfer::getType() const
{
	if (_soldier != 0)
	{
		return TRANSFER_SOLDIER;
	}
	else if (_craft != 0)
	{
		return TRANSFER_CRAFT;
	}
	else if (_prisoner != 0)
	{
		return TRANSFER_PRISONER;
	}
	return TRANSFER_ITEM;
}

/**
 * Advances the transfer and takes care of
 * the delivery once it's arrived.
 * @param base Pointer to destination base.
 */
void Transfer::advance(Base *base)
{
	_hours--;
	if (_hours <= 0)
	{
		if (_soldier != 0)
		{
			base->getSoldiers()->push_back(_soldier);
		}
		else if (_craft != 0)
		{
			base->getCrafts()->push_back(_craft);
			_craft->setBase(base);
			_craft->checkup();
		}
		else if (_itemQty != 0)
		{
			base->getStorageItems()->addItem(_itemId, _itemQty);
		}
		else if (_prisoner != 0)
		{
			base->addPrisoner(_prisoner);
		}
		_delivered = true;
	}
}

/**
 * Get a pointer to the soldier being transferred.
 * @return a pointer to the soldier being moved.
 */
Soldier *Transfer::getSoldier() const
{
	return _soldier;
}

}
