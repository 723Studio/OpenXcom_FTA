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
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <SDL_types.h>
#include "../Engine/RNG.h"

namespace OpenXcom
{

class Mod;
class Armor;
class RuleItem;
class RuleSoldier;
enum SoldierRole : int;
class RulePrisoner;
class ModScript;
class ScriptParserBase;

enum SpecialAbility { SPECAB_NONE, SPECAB_EXPLODEONDEATH, SPECAB_BURNFLOOR, SPECAB_BURN_AND_EXPLODE };
enum SpecialObjective { SPECOBJ_NONE, SPECOBJ_FRIENDLY_VIP, SPECOBJ_ENEMY_VIP };
/**
 * This struct holds some plain unit attribute data together.
 */
struct UnitStats
{
	using Type = Sint16;
	using Ptr = Type UnitStats::*;

	enum StatStringType {STATSTR_UC, STATSTR_LC, STATSTR_ABBREV, STATSTR_SHORT};

	/// Max value that is allowed to set to stat, less that max value allowed by type.
	constexpr static int BaseStatLimit = 8000;

	/// How much more stun can be than health.
	constexpr static int StunMultipler = 4;
	/// Max value allowed for stun value.
	constexpr static int StunStatLimit = BaseStatLimit * StunMultipler;

	/// How much more over kill can go to negative than health.
	constexpr static int OverkillMultipler = 4;

	Type tu, stamina, health, bravery, reactions, firing, throwing, strength, psiStrength, psiSkill, melee, mana, //soldiers
		maneuvering, missiles, dogfight, tracking, cooperation, beams, synaptic, gravity, //pilot
		physics, chemistry, biology, insight, data, computers, tactics, materials, designing, psionics, xenolinguistics, //scientist
		weaponry, explosives, efficiency, microelectronics, metallurgy, processing, hacking, construction, diligence, alienTech, reverseEngineering, //engineers
		stealth, perseption, charisma, investigation, deception, interrogation; //agents

	UnitStats() : tu(0), stamina(0), health(0), bravery(0), reactions(0), firing(0), throwing(0),
		strength(0), psiStrength(0), psiSkill(0), melee(0), mana(0),
		maneuvering(0), missiles(0), dogfight(0), tracking(0), cooperation(0), beams(0), synaptic(0), gravity(0),
		physics(0), chemistry(0), biology(0), insight(0), data(0), computers(0), tactics(0), materials(0),
		designing(0), psionics(0), xenolinguistics(0),
		weaponry(0), explosives(0), efficiency(0), microelectronics(0), metallurgy(0), processing(0), hacking(0), construction(0),
		diligence(0), alienTech(0), reverseEngineering(0),
		stealth(0), perseption(0), charisma(0), investigation(0), deception(0), interrogation(0){}
	UnitStats(int tu_, int stamina_, int health_, int bravery_, int reactions_, int firing_, int throwing_,
		int strength_, int psiStrength_, int psiSkill_, int melee_, int mana_,
		int maneuvering_, int missiles_, int dogfight_, int tracking_, int cooperation_, int beams_, int synaptic_, int gravity_,
		int physics_, int chemistry_, int biology_, int insight_, int data_, int computers_, int tactics_, int materials_,
		int designing_, int psionics_, int xenolinguistics_,
		int weaponry_, int explosives_, int efficiency_, int microelectronics_, int metallurgy_, int processing_, int hacking_,
		int construction_, int diligence_, int alienTech_, int reverseEngineering_,
		int stealth_, int perseption_, int charisma_, int investigation_, int deception_, int interrogation_) :
		tu(tu_), stamina(stamina_), health(health_), bravery(bravery_), reactions(reactions_), firing(firing_), throwing(throwing_),
		strength(strength_), psiStrength(psiStrength_), psiSkill(psiSkill_), melee(melee_), mana(mana_),
		maneuvering(maneuvering_), missiles(missiles_), dogfight(dogfight_), tracking(tracking_), cooperation(cooperation_), beams(beams_), synaptic(synaptic_), gravity(gravity_),
		physics(physics_), chemistry(chemistry_), biology(biology_), insight(insight_), data(data_), computers(computers_), tactics(tactics_), materials(materials_),
		designing(designing_), psionics(psionics_), xenolinguistics(xenolinguistics_),
		weaponry(weaponry_), explosives(explosives_), efficiency(efficiency_), microelectronics(microelectronics_), metallurgy(metallurgy_), processing(processing_),
		hacking(hacking_), construction(construction_), diligence(diligence_), alienTech(alienTech_), reverseEngineering(reverseEngineering_),
		stealth(stealth_), perseption(perseption_), charisma(charisma_), investigation(investigation_), deception(deception_), interrogation(interrogation_) {}
	UnitStats& operator+=(const UnitStats& stats) {
		tu += stats.tu;
		stamina += stats.stamina;
		health += stats.health;
		bravery += stats.bravery;
		reactions += stats.reactions;
		firing += stats.firing;
		throwing += stats.throwing;
		strength += stats.strength;
		psiStrength += stats.psiStrength;
		psiSkill += stats.psiSkill;
		melee += stats.melee;
		mana += stats.mana;
		maneuvering += stats.maneuvering;
		missiles += stats.missiles;
		dogfight += stats.dogfight;
		tracking += stats.tracking;
		cooperation += stats.cooperation;
		beams += stats.beams;
		synaptic += stats.synaptic;
		gravity += stats.gravity;
		physics += stats.physics;
		chemistry += stats.chemistry;
		biology += stats.biology;
		insight += stats.insight;
		data += stats.data;
		computers += stats.computers;
		tactics += stats.tactics;
		materials += stats.materials;
		designing += stats.designing;
		psionics += stats.psionics;
		xenolinguistics += stats.xenolinguistics;
		weaponry += stats.weaponry;
		explosives += stats.explosives;
		efficiency += stats.efficiency;
		microelectronics += stats.microelectronics;
		metallurgy += stats.metallurgy;
		processing += stats.processing;
		hacking += stats.hacking;
		construction += stats.construction;
		diligence += stats.diligence;
		alienTech += stats.alienTech;
		reverseEngineering += stats.reverseEngineering;
		stealth += stats.stealth;
		perseption += stats.perseption;
		charisma += stats.charisma;
		investigation += stats.investigation;
		deception += stats.deception;
		interrogation += stats.interrogation;
		return *this; }
	UnitStats operator+(const UnitStats& stats) const { return UnitStats(
		tu + stats.tu,
		stamina + stats.stamina,
		health + stats.health,
		bravery + stats.bravery,
		reactions + stats.reactions,
		firing + stats.firing,
		throwing + stats.throwing,
		strength + stats.strength,
		psiStrength + stats.psiStrength,
		psiSkill + stats.psiSkill,
		melee + stats.melee,
		mana + stats.mana,
		maneuvering + stats.maneuvering,
		missiles + stats.missiles,
		dogfight + stats.dogfight,
		tracking + stats.tracking,
		cooperation + stats.cooperation,
		beams + stats.beams,
		synaptic + stats.synaptic,
		gravity + stats.gravity,
		physics + stats.physics,
		chemistry + stats.chemistry,
		biology + stats.biology,
		insight + stats.insight,
		data + stats.data,
		computers + stats.computers,
		tactics + stats.tactics,
		materials + stats.materials,
		designing + stats.designing,
		psionics + stats.psionics,
		xenolinguistics + stats.xenolinguistics,
		weaponry + stats.weaponry,
		explosives + stats.explosives,
		efficiency + stats.efficiency,
		microelectronics + stats.microelectronics,
		metallurgy + stats.metallurgy,
		processing + stats.processing,
		hacking + stats.hacking,
		construction + stats.construction,
		diligence + stats.diligence,
		alienTech + stats.alienTech,
		reverseEngineering + stats.reverseEngineering,
		stealth + stats.stealth,
		perseption + stats.perseption,
		charisma + stats.charisma,
		investigation + stats.investigation,
		deception + stats.deception,
		interrogation + stats.interrogation); }
	UnitStats& operator-=(const UnitStats& stats) {
		tu -= stats.tu;
		stamina -= stats.stamina;
		health -= stats.health;
		bravery -= stats.bravery;
		reactions -= stats.reactions;
		firing -= stats.firing;
		throwing -= stats.throwing;
		strength -= stats.strength;
		psiStrength -= stats.psiStrength;
		psiSkill -= stats.psiSkill;
		melee -= stats.melee;
		mana -= stats.mana;
		maneuvering -= stats.maneuvering;
		missiles -= stats.missiles;
		dogfight -= stats.dogfight;
		tracking -= stats.tracking;
		cooperation -= stats.cooperation;
		beams -= stats.beams;
		synaptic -= stats.synaptic;
		gravity -= stats.gravity;
		physics -= stats.physics;
		chemistry -= stats.chemistry;
		biology -= stats.biology;
		insight -= stats.insight;
		data -= stats.data;
		materials -= stats.materials;
		designing -= stats.designing;
		psionics -= stats.psionics;
		xenolinguistics -= stats.xenolinguistics;
		weaponry -= stats.weaponry;
		explosives -= stats.explosives;
		efficiency -= stats.efficiency;
		microelectronics -= stats.microelectronics;
		metallurgy -= stats.metallurgy;
		processing -= stats.processing;
		hacking -= stats.hacking;
		construction -= stats.construction;
		diligence -= stats.diligence;
		alienTech -= stats.alienTech;
		reverseEngineering -= stats.reverseEngineering;
		stealth -= stats.stealth;
		perseption -= stats.perseption;
		charisma -= stats.charisma;
		investigation -= stats.investigation;
		deception -= stats.deception;
		interrogation -= stats.interrogation;
		return *this;}
	UnitStats operator-(const UnitStats& stats) const { return UnitStats(
		tu - stats.tu,
		stamina - stats.stamina,
		health - stats.health,
		bravery - stats.bravery,
		reactions - stats.reactions,
		firing - stats.firing,
		throwing - stats.throwing,
		strength - stats.strength,
		psiStrength - stats.psiStrength,
		psiSkill - stats.psiSkill,
		melee - stats.melee,
		mana - stats.mana,
		maneuvering - stats.maneuvering,
		missiles - stats.missiles,
		dogfight - stats.dogfight,
		tracking - stats.tracking,
		cooperation - stats.cooperation,
		beams - stats.beams,
		synaptic - stats.synaptic,
		gravity - stats.gravity,
		physics - stats.physics,
		chemistry - stats.chemistry,
		biology - stats.biology,
		insight - stats.insight,
		data - stats.data,
		computers - stats.computers,
		tactics - stats.tactics,
		materials - stats.materials,
		designing - stats.designing,
		psionics - stats.psionics,
		xenolinguistics - stats.xenolinguistics,
		weaponry - stats.weaponry,
		explosives - stats.explosives,
		efficiency - stats.efficiency,
		microelectronics - stats.microelectronics,
		metallurgy - stats.metallurgy,
		processing - stats.processing,
		hacking - stats.hacking,
		construction - stats.construction,
		diligence - stats.diligence,
		alienTech - stats.alienTech,
		reverseEngineering - stats.reverseEngineering,
		stealth - stats.stealth,
		perseption - stats.perseption,
		charisma - stats.charisma,
		investigation - stats.investigation,
		deception - stats.deception,
		interrogation - stats.interrogation); }
	UnitStats operator-() const { return UnitStats(
		-tu, -stamina, -health, -bravery, -reactions, -firing, -throwing, -strength, -psiStrength, -psiSkill, -melee, -mana,
		-maneuvering, -missiles, -dogfight, -tracking, -cooperation, -beams, -synaptic, -gravity,
		-physics, -chemistry, -biology, -insight, -data, -computers, -tactics, -materials, -designing, -psionics, -xenolinguistics,
		-weaponry, -explosives, -efficiency, -microelectronics, -metallurgy, -processing, -hacking, -construction, -diligence, -alienTech, -reverseEngineering,
		-stealth, -perseption, -charisma, -investigation, -deception, -interrogation); }
	void merge(const UnitStats& stats) {
		tu = (stats.tu ? stats.tu : tu);
		stamina = (stats.stamina ? stats.stamina : stamina);
		health = (stats.health ? stats.health : health);
		bravery = (stats.bravery ? stats.bravery : bravery);
		reactions = (stats.reactions ? stats.reactions : reactions);
		firing = (stats.firing ? stats.firing : firing);
		throwing = (stats.throwing ? stats.throwing : throwing);
		strength = (stats.strength ? stats.strength : strength);
		psiStrength = (stats.psiStrength ? stats.psiStrength : psiStrength);
		psiSkill = (stats.psiSkill ? stats.psiSkill : psiSkill);
		melee = (stats.melee ? stats.melee : melee);
		mana = (stats.mana ? stats.mana : mana);
		maneuvering = (stats.maneuvering ? stats.maneuvering : maneuvering);
		missiles = (stats.missiles ? stats.missiles : missiles);
		dogfight = (stats.dogfight ? stats.dogfight : dogfight);
		tracking = (stats.tracking ? stats.tracking : tracking);
		cooperation = (stats.cooperation ? stats.cooperation : cooperation);
		beams = (stats.beams ? stats.beams : beams);
		synaptic = (stats.synaptic ? stats.synaptic : synaptic);
		gravity = (stats.gravity ? stats.gravity : gravity);
		physics = (stats.physics ? stats.physics : physics);
		chemistry = (stats.chemistry ? stats.chemistry : chemistry);
		biology = (stats.biology ? stats.biology : biology);
		insight = (stats.insight ? stats.insight : insight);
		data = (stats.data ? stats.data : data);
		computers = (stats.computers ? stats.computers : computers);
		tactics = (stats.tactics ? stats.tactics : tactics);
		materials = (stats.materials ? stats.materials : materials);
		designing = (stats.designing ? stats.designing : designing);
		psionics = (stats.psionics ? stats.psionics : psionics);
		xenolinguistics = (stats.xenolinguistics ? stats.xenolinguistics : xenolinguistics);
		weaponry = (stats.weaponry ? stats.weaponry : weaponry);
		explosives = (stats.explosives ? stats.explosives : explosives);
		efficiency = (stats.efficiency ? stats.efficiency : efficiency);
		microelectronics = (stats.microelectronics ? stats.microelectronics : microelectronics);
		metallurgy = (stats.metallurgy ? stats.metallurgy : metallurgy);
		processing = (stats.processing ? stats.processing : processing);
		hacking = (stats.hacking ? stats.hacking : hacking);
		construction = (stats.construction ? stats.construction : construction);
		diligence = (stats.diligence ? stats.diligence : diligence);
		alienTech = (stats.alienTech ? stats.alienTech : alienTech);
		reverseEngineering = (stats.reverseEngineering ? stats.reverseEngineering : reverseEngineering);
		stealth = (stats.stealth ? stats.stealth : stealth);
		perseption = (stats.perseption ? stats.perseption : perseption);
		charisma = (stats.charisma ? stats.charisma : charisma);
		investigation = (stats.investigation ? stats.investigation : investigation);
		deception = (stats.deception ? stats.deception : deception);
		interrogation = (stats.interrogation ? stats.interrogation : interrogation);
	};

	bool empty()
	{
		return bravery || reactions || firing || psiSkill || psiStrength || melee || throwing || mana ||
			maneuvering || missiles || dogfight || cooperation || tracking || beams ||
			synaptic || gravity || physics || chemistry || biology || insight || data || computers || tactics ||
			materials || designing || psionics || xenolinguistics ||
			weaponry || explosives || efficiency || microelectronics || metallurgy || processing ||
			hacking || construction || diligence || alienTech || reverseEngineering ||
			stealth || perseption || charisma || investigation || deception || interrogation;
	}
	template<typename Func>
	static void fieldLoop(Func f)
	{
		constexpr static Ptr allFields[] =
		{
			&UnitStats::tu, &UnitStats::stamina, &UnitStats::health, &UnitStats::bravery,
			&UnitStats::reactions, &UnitStats::firing, &UnitStats::throwing, &UnitStats::strength,
			&UnitStats::psiStrength, &UnitStats::psiSkill, &UnitStats::melee, &UnitStats::mana,
			&UnitStats::maneuvering, &UnitStats::missiles, &UnitStats::dogfight, &UnitStats::tracking,
			&UnitStats::cooperation, &UnitStats::beams, &UnitStats::synaptic, &UnitStats::gravity,
			&UnitStats::physics, &UnitStats::chemistry, &UnitStats::biology, &UnitStats::insight, &UnitStats::data, &UnitStats::computers,& UnitStats::tactics,
			&UnitStats::materials, &UnitStats::designing, &UnitStats::psionics, &UnitStats::xenolinguistics,
			&UnitStats::weaponry, &UnitStats::explosives, &UnitStats::efficiency, &UnitStats::microelectronics, &UnitStats::metallurgy, &UnitStats::processing,
			&UnitStats::hacking, &UnitStats::construction, &UnitStats::diligence, &UnitStats::alienTech, &UnitStats::reverseEngineering,
			&UnitStats::stealth, &UnitStats::perseption, &UnitStats::charisma, &UnitStats::investigation, &UnitStats::deception, &UnitStats::interrogation
		};

		for (Ptr p : allFields)
		{
			f(p);
		}
	}

	static std::string getStatString(Type UnitStats::* t, StatStringType type = STATSTR_UC)
	{
		constexpr static std::pair<Ptr, const char*> statStings[] =
		{
			{&UnitStats::tu, "STR_TIME_UNITS"},
			{&UnitStats::stamina, "STR_STAMINA"},
			{&UnitStats::health, "STR_HEALTH"},
			{&UnitStats::bravery, "STR_BRAVERY"},
			{&UnitStats::reactions, "STR_REACTIONS"},
			{&UnitStats::firing, "STR_FIRING_ACCURACY"},
			{&UnitStats::throwing, "STR_THROWING_ACCURACY"},
			{&UnitStats::strength, "STR_STRENGTH"},
			{&UnitStats::psiStrength, "STR_PSIONIC_STRENGTH"},
			{&UnitStats::psiSkill, "STR_PSIONIC_SKILL"},
			{&UnitStats::melee, "STR_MELEE_ACCURACY"},
			{&UnitStats::mana, "STR_MANA_POOL"},
			{&UnitStats::maneuvering, "STR_MANEUVERING"},
			{&UnitStats::missiles, "STR_MISSILE_OPERATION"},
			{&UnitStats::dogfight, "STR_DOGFIGHT"},
			{&UnitStats::tracking, "STR_TRACKING"},
			{&UnitStats::cooperation, "STR_COOPERATION"},
			{&UnitStats::beams, "STR_BEAMS_OPERATION"},
			{&UnitStats::synaptic, "STR_SYNAPTIC_CONNECTIVITY"},
			{&UnitStats::gravity, "STR_GRAVITY_MANIPULATION"},
			{&UnitStats::physics, "STR_PHYSICS"},
			{&UnitStats::chemistry, "STR_CHEMISTRY"},
			{&UnitStats::biology, "STR_BIOLOGY"},
			{&UnitStats::insight, "STR_INSIGHT"},
			{&UnitStats::data, "STR_DATA_ANALISIS"},
			{&UnitStats::computers, "STR_COMPUTER_SCIENCE"},
			{&UnitStats::tactics, "STR_TACTICS"},
			{&UnitStats::materials, "STR_MATERIAL_SCIENCE"},
			{&UnitStats::designing, "STR_DESIGNING"},
			{&UnitStats::psionics, "STR_PSIONICS"},
			{&UnitStats::xenolinguistics, "STR_XENOLINGUISTICS"},
			{&UnitStats::weaponry, "STR_WEAPONRY"},
			{&UnitStats::explosives, "STR_EXPLOSIVES"},
			{&UnitStats::efficiency, "STR_EFFICIENCY"},
			{&UnitStats::microelectronics, "STR_MICROELECTRONICS"},
			{&UnitStats::metallurgy, "STR_METALLURGY"},
			{&UnitStats::processing, "STR_PROCESSING"},
			{&UnitStats::hacking, "STR_HACKING"},
			{&UnitStats::construction, "STR_CONSTRUCTION"},
			{&UnitStats::diligence, "STR_DILIGENCE"},
			{&UnitStats::alienTech, "STR_ALIEN_TECH"},
			{&UnitStats::reverseEngineering, "STR_REVERSE_ENGINEERING"},
			{&UnitStats::stealth, "STR_STEALTH"},
			{&UnitStats::perseption, "STR_PERSEPTION"},
			{&UnitStats::charisma, "STR_CHARISMA"},
			{&UnitStats::investigation, "STR_INVESTIGATION"},
			{&UnitStats::deception, "STR_DECEPTION"},
			{&UnitStats::interrogation, "STR_INTERROGATION"}
		};

		for (auto& p : statStings)
		{
			if (t == p.first)
			{
				std::string suffix;
				switch (type)
				{
				case STATSTR_UC:
					suffix = "_UC";
					break;
				case STATSTR_LC:
					suffix = "_LC";
					break;
				case STATSTR_SHORT:
					suffix = "_SHORT";
					break;
				case STATSTR_ABBREV:
					suffix = "_ABBREV";
					break;
				}

				return p.second + suffix;
			}
		}
		return "NONE";
	}

	static UnitStats templateMerge(const UnitStats& origStats, const UnitStats& fixedStats)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				if ((fixedStats.*p) == -1)
				{
					(r.*p) = 0;
				}
				else if ((fixedStats.*p) != 0)
				{
					(r.*p) = (fixedStats.*p);
				}
				else
				{
					(r.*p) = (origStats.*p);
				}
			}
		);
		return r;
	}

	/*
	 * Soft limit definition:
	 * 1. if the statChange is zero or negative, keep statChange as it is (i.e. don't apply any limits)
	 * 2. if the statChange is positive and currentStats <= upperBound, consider upperBound   (i.e. set result to min(statChange, upperBound-currentStats))
	 * 3. if the statChange is positive and currentStats >  upperBound, keep the currentStats (i.e. set result to 0)
	 */
	static UnitStats softLimit(const UnitStats& statChange, const UnitStats& currentStats, const UnitStats& upperBound)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				if ((statChange.*p) <= 0)
				{
					// 1. keep statChange
					(r.*p) = (statChange.*p);
				}
				else
				{
					if ((currentStats.*p) <= (upperBound.*p))
					{
						// 2. consider upperBound
						Sint16 tmp = (upperBound.*p) - (currentStats.*p);
						(r.*p) = std::min((statChange.*p), tmp);
					}
					else
					{
						// 3. keep currentStats
						(r.*p) = 0;
					}
				}
			}
		);
		return r;
	}

	static UnitStats combine(const UnitStats &mask, const UnitStats &keep, const UnitStats &reroll)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				if ((mask.*p))
				{
					(r.*p) = (reroll.*p);
				}
				else
				{
					(r.*p) = (keep.*p);
				}
			}
		);
		return r;
	}

	static UnitStats random(const UnitStats &a, const UnitStats &b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				Sint16 min = std::min((a.*p), (b.*p));
				Sint16 max = std::max((a.*p), (b.*p));
				if (min == max)
				{
					(r.*p) = max;
				}
				else
				{
					Sint16 rnd = RNG::generate(min, max);
					(r.*p) = rnd;
				}
			}
		);
		return r;
	}

	static UnitStats isRandom(const UnitStats &a, const UnitStats &b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				if ( ((a.*p) != 0 || (b.*p) != 0) && (a.*p) != (b.*p))
				{
					(r.*p) = 1;
				}
				else
				{
					(r.*p) = 0;
				}
			}
		);
		return r;
	}

	static UnitStats percent(const UnitStats& base, const UnitStats& percent, int multipler = 1)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = (base.*p) * (percent.*p) * multipler / 100;
			}
		);
		return r;
	}

	static UnitStats scalar(int i)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = i;
			}
		);
		return r;
	}

	static UnitStats obeyFixedMinimum(const UnitStats &a)
	{
		// minimum 1 for health, minimum 0 for other stats (note to self: it might be worth considering minimum 10 for bravery in the future)
		static const UnitStats fixedMinimum = UnitStats(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		return max(a, fixedMinimum);
	}

	static UnitStats max(const UnitStats& a, const UnitStats& b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = std::max((a.*p), (b.*p));
			}
		);
		return r;
	}

	static UnitStats min(const UnitStats& a, const UnitStats& b)
	{
		UnitStats r;
		fieldLoop(
			[&](Ptr p)
			{
				(r.*p) = std::min((a.*p), (b.*p));
			}
		);
		return r;
	}

	template<typename T, UnitStats T::*Stat, Ptr StatMax>
	static void getMaxStatScript(const T *t, int& val)
	{
		if (t)
		{
			val = ((t->*Stat).*StatMax);
		}
		else
		{
			val = 0;
		}
	}

	template<typename T, UnitStats T::*Stat, Ptr StatMax>
	static void setMaxStatScript(T *t, int val)
	{
		if (t)
		{
			val = std::min(std::max(val, 1), BaseStatLimit);
			((t->*Stat).*StatMax) = val;
		}
	}

	template<typename T, UnitStats T::*Stat, Ptr StatMax>
	static void addMaxStatScript(T *t, int val)
	{
		if (t)
		{
			//limit range to prevent overflow
			val = std::min(std::max(val, -BaseStatLimit), BaseStatLimit);
			setMaxStatScript<T, Stat, StatMax>(t, val + ((t->*Stat).*StatMax));
		}
	}

	template<typename T, UnitStats T::*Stat, int T::*Curr, Ptr StatMax>
	static void setMaxAndCurrStatScript(T *t, int val)
	{
		if (t)
		{
			val = std::min(std::max(val, 1), BaseStatLimit);
			((t->*Stat).*StatMax) = val;

			//update current value
			if ((t->*Curr) > val)
			{
				(t->*Curr) = val;
			}
		}
	}

	template<typename T, UnitStats T::*Stat, int T::*Curr, Ptr StatMax>
	static void addMaxAndCurrStatScript(T *t, int val)
	{
		if (t)
		{
			//limit range to prevent overflow
			val = std::min(std::max(val, -BaseStatLimit), BaseStatLimit);
			setMaxAndCurrStatScript<T, Stat, Curr, StatMax>(t, val + ((t->*Stat).*StatMax));
		}
	}

	template<auto Stat, typename TBind>
	static void addGetStatsScript(TBind& b, std::string prefix, bool skipResorcesStats = false)
	{
		if (!skipResorcesStats)
		{
			b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::tu>>(prefix + "getTimeUnits");
			b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::stamina>>(prefix + "getStamina");
			b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::health>>(prefix + "getHealth");
			b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::mana>>(prefix + "getManaPool");
		}

		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::bravery>>(prefix + "getBravery");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::reactions>>(prefix + "getReactions");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::firing>>(prefix + "getFiring");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::throwing>>(prefix + "getThrowing");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::strength>>(prefix + "getStrength");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiStrength>>(prefix + "getPsiStrength");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiSkill>>(prefix + "getPsiSkill");
		b.template add<&getMaxStatScript<typename TBind::Type, Stat, &UnitStats::melee>>(prefix + "getMelee");
	}

	template<auto Stat, typename TBind>
	static void addSetStatsScript(TBind& b, std::string prefix, bool skipResorcesStats = false)
	{
		if (!skipResorcesStats)
		{
			b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::tu>>(prefix + "setTimeUnits");
			b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::stamina>>(prefix + "setStamina");
			b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::health>>(prefix + "setHealth");
			b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::mana>>(prefix + "setManaPool");

			b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::tu>>(prefix + "addTimeUnits");
			b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::stamina>>(prefix + "addStamina");
			b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::health>>(prefix + "addHealth");
			b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::mana>>(prefix + "addManaPool");
		}

		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::bravery>>(prefix + "setBravery");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::reactions>>(prefix + "setReactions");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::firing>>(prefix + "setFiring");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::throwing>>(prefix + "setThrowing");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::strength>>(prefix + "setStrength");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiStrength>>(prefix + "setPsiStrength");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiSkill>>(prefix + "setPsiSkill");
		b.template add<&setMaxStatScript<typename TBind::Type, Stat, &UnitStats::melee>>(prefix + "setMelee");

		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::bravery>>(prefix + "addBravery");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::reactions>>(prefix + "addReactions");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::firing>>(prefix + "addFiring");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::throwing>>(prefix + "addThrowing");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::strength>>(prefix + "addStrength");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiStrength>>(prefix + "addPsiStrength");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::psiSkill>>(prefix + "addPsiSkill");
		b.template add<&addMaxStatScript<typename TBind::Type, Stat, &UnitStats::melee>>(prefix + "addMelee");
	}

	template<auto Stat, auto CurrTu, auto CurrEnergy, auto CurrHealth, auto CurrMana, typename TBind>
	static void addSetStatsWithCurrScript(TBind& b, std::string prefix)
	{
		// when we change stats of BattleUnit its resources should be adjust.
		b.template add<&setMaxAndCurrStatScript<typename TBind::Type, Stat, CurrTu, &UnitStats::tu>>(prefix + "setTimeUnits");
		b.template add<&setMaxAndCurrStatScript<typename TBind::Type, Stat, CurrEnergy, &UnitStats::stamina>>(prefix + "setStamina");
		b.template add<&setMaxAndCurrStatScript<typename TBind::Type, Stat, CurrHealth, &UnitStats::health>>(prefix + "setHealth");
		b.template add<&setMaxAndCurrStatScript<typename TBind::Type, Stat, CurrMana, &UnitStats::mana>>(prefix + "setManaPool");

		b.template add<&addMaxAndCurrStatScript<typename TBind::Type, Stat, CurrTu, &UnitStats::tu>>(prefix + "addTimeUnits");
		b.template add<&addMaxAndCurrStatScript<typename TBind::Type, Stat, CurrEnergy, &UnitStats::stamina>>(prefix + "addStamina");
		b.template add<&addMaxAndCurrStatScript<typename TBind::Type, Stat, CurrHealth, &UnitStats::health>>(prefix + "addHealth");
		b.template add<&addMaxAndCurrStatScript<typename TBind::Type, Stat, CurrMana, &UnitStats::mana>>(prefix + "addManaPool");

		addSetStatsScript<Stat>(b, prefix, true);
	}
};

struct StatAdjustment
{
	/// Name of class used in script.
	static constexpr const char *ScriptName = "StatAdjustment";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);

	UnitStats statGrowth;
	int growthMultiplier;
	double aimMultiplier;
	double armorMultiplier;
};

/**
 * Represents the static data for a unit that is generated on the battlescape, this includes: HWPs, aliens and civilians.
 * @sa Soldier BattleUnit
 */
class Unit
{
private:
	std::string _type;
	std::string _civilianRecoveryType, _spawnedPersonName, _liveAlienName;
	YAML::Node _spawnedSoldier;
	std::string _race;
	int _showFullNameInAlienInventory;
	std::string _rank;
	UnitStats _stats, _statsRandom;
	std::vector<SoldierRole> _roles;
	std::string _prisonerName;
	const RulePrisoner* _prisoner = nullptr;
	std::string _armorName;
	const Armor* _armor;
	int _standHeight, _kneelHeight, _floatHeight;
	std::vector<int> _deathSound, _panicSound, _berserkSound, _aggroSound;
	std::vector<int> _selectUnitSound, _startMovingSound, _selectWeaponSound, _annoyedSound, _spawnedUnitSound;
	int _value, _moraleLossWhenKilled, _moveSound;
	int _intelligence, _aggression, _spotter, _sniper, _energyRecovery;
	SpecialAbility _specab;
	SpecialObjective _specialObjective;
	const RuleItem* _liveAlien = nullptr;
	const Unit *_spawnUnit = nullptr;
	const Unit* _altUnit = nullptr;
	std::string _spawnUnitName;
	bool _livingWeapon;
	std::string _meleeWeapon, _psiWeapon, _altRecoveredUnit;
	std::vector<std::vector<std::string> > _builtInWeaponsNames;
	std::vector<std::vector<const RuleItem*> > _builtInWeapons;
	bool _capturable;
	bool _canSurrender, _autoSurrender;
	bool _isLeeroyJenkins;
	bool _waitIfOutsideWeaponRange;
	int _pickUpWeaponsMoreActively;
	bool _vip;
	bool _cosmetic, _ignoredByAI, _treatedByAI;
	bool _canPanic;
	bool _canBeMindControlled;
	int _berserkChance;

	void loadRoles(const std::vector<int>& r);

public:
	/// Creates a blank unit ruleset.
	Unit(const std::string &type);
	/// Cleans up the unit ruleset.
	~Unit();
	/// Loads the unit data from YAML.
	void load(const YAML::Node& node, Mod *mod);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);

	/// Gets the unit's type.
	const std::string& getType() const;
	/// Gets the type of staff (soldier/engineer/scientists) or type of item to be recovered when a civilian is saved.
	const std::string &getCivilianRecoveryType() const { return _civilianRecoveryType; }
	/// Gets the custom name of the "spawned person".
	const std::string &getSpawnedPersonName() const { return _spawnedPersonName; }
	/// Gets the spawned soldier template.
	const YAML::Node &getSpawnedSoldierTemplate() const { return _spawnedSoldier; }
	/// Gets the unit's stats.
	UnitStats *getStats();
	/// Gets the unit's random part of stats.
	UnitStats* getRandomStats();
	/// Gets the unit's height when standing.
	int getStandHeight() const;
	/// Gets the unit's height when kneeling.
	int getKneelHeight() const;
	/// Gets the unit's float elevation.
	int getFloatHeight() const;
	/// Gets the armor type.
	Armor* getArmor() const;
	/// Gets the alien race type.
	std::string getRace() const;
	/// Gets the alien rank.
	std::string getRank() const;
	/// Gets the unit's role.
	std::vector<SoldierRole> getRoles() const { return _roles; }
	/// Gets the prisoner type.
	const RulePrisoner* getPrisoner() const { return _prisoner; }
	/// Gets the value - for score calculation.
	int getValue() const;
	/// Percentage modifier for morale loss when this unit is killed.
	int getMoraleLossWhenKilled() { return _moraleLossWhenKilled; };
	/// Gets the death sound id.
	const std::vector<int> &getDeathSounds() const;
	/// Gets the unit's panic sounds.
	const std::vector<int> &getPanicSounds() const;
	/// Gets the unit's berserk sounds.
	const std::vector<int> &getBerserkSounds() const;
	/// Gets the unit's "select unit" sounds.
	const std::vector<int> &getSelectUnitSounds() const { return _selectUnitSound; }
	/// Gets the unit's "start moving" sounds.
	const std::vector<int> &getStartMovingSounds() const { return _startMovingSound; }
	/// Gets the unit's "select weapon" sounds.
	const std::vector<int> &getSelectWeaponSounds() const { return _selectWeaponSound; }
	/// Gets the unit's "annoyed" sounds.
	const std::vector<int> &getAnnoyedSounds() const { return _annoyedSound; }
	/// Gets the unit's "spawned unit" sounds.
	const std::vector<int>& getSpawnedUnitSounds() const { return _spawnedUnitSound; }
	/// Gets the move sound id.
	int getMoveSound() const;
	/// Gets the intelligence. This is the number of turns AI remembers your troop positions.
	int getIntelligence() const;
	/// Gets the aggression. Determines the chance of revenge and taking cover.
	int getAggression() const;
	/// Gets the spotter score. This is the number of turns sniper AI units can use spotting info from this unit.
	int getSpotterDuration() const;
	/// Gets the sniper score. Determines chance of acting on information gained by spotter units.
	int getSniperPercentage() const;
	/// Gets the alien's special ability.
	int getSpecialAbility() const;
	/// Gets the Geoscape live alien item.
	const RuleItem* getLiveAlienGeoscape() const { return _liveAlien; }
	/// Gets the unit's spawn unit.
	const Unit *getSpawnUnit() const;
	/// Gets the unit's warcries.
	const std::vector<int> &getAggroSounds() const;
	/// Gets how much energy this unit recovers per turn.
	int getEnergyRecovery() const;
	/// Checks if this unit has a built in weapon.
	bool isLivingWeapon() const;
	/// Gets the name of any melee weapon that may be built in to this unit.
	const std::string &getMeleeWeapon() const;
	/// Gets the name of any psi weapon that may be built in to this unit.
	const std::string &getPsiWeapon() const;
	/// Gets a vector of integrated items this unit has available.
	const std::vector<std::vector<const RuleItem*> > &getBuiltInWeapons() const;
	/// Gets whether the alien can be captured alive.
	bool getCapturable() const;
	/// Checks if this unit can surrender.
	bool canSurrender() const;
	/// Gets special objective of a unit.
	SpecialObjective getSpecialObjective() const { return _specialObjective; }
	/// Gets alternative unit for recovery.
	const Unit* getAltUnit() const { return _altUnit; };
	/// Checks if this unit surrenders automatically, if all other units surrendered too.
	bool autoSurrender() const;
	bool isLeeroyJenkins() const { return _isLeeroyJenkins; };
	/// Should the unit get "stuck" trying to fire from outside of weapon range? Vanilla bug, that may serve as "feature" in rare cases.
	bool waitIfOutsideWeaponRange() { return _waitIfOutsideWeaponRange; };
	/// Should the unit try to pick up weapons more actively?
	int getPickUpWeaponsMoreActively() const { return _pickUpWeaponsMoreActively; }
	/// Should alien inventory show full name (e.g. Sectoid Leader) or just the race (e.g. Sectoid)?
	bool getShowFullNameInAlienInventory(Mod *mod) const;
	/// Is this a VIP unit?
	bool isVIP() const { return _vip; }
	/// Is this only a cosmetic unit?
	bool isCosmetic() const { return _cosmetic; }
	/// Should this unit be ignored by the AI?
	bool isIgnoredByAI() const { return _ignoredByAI; }
	/// Should this unit be treated by the AI as an enemy no matter side?
	bool isTreatedByAI() const { return _treatedByAI; }
	/// Checks if this unit can panic.
	bool canPanic() const { return _canPanic; }
	/// Checks if this unit can be mind controlled.
	bool canBeMindControlled() const { return _canBeMindControlled; }
	/// Gets the probability of unit berserking when panicked.
	int getBerserkChance() const { return _berserkChance; }

	/// Name of class used in script.
	static constexpr const char *ScriptName = "RuleUnit";
	/// Register all useful function used by script.
	static void ScriptRegister(ScriptParserBase* parser);
};

}

namespace YAML
{
	template<>
	struct convert<OpenXcom::UnitStats>
	{
		static Node encode(const OpenXcom::UnitStats& rhs)
		{
			Node node;
			node["tu"] = rhs.tu;
			node["stamina"] = rhs.stamina;
			node["health"] = rhs.health;
			node["bravery"] = rhs.bravery;
			node["reactions"] = rhs.reactions;
			node["firing"] = rhs.firing;
			node["throwing"] = rhs.throwing;
			node["strength"] = rhs.strength;
			node["psiStrength"] = rhs.psiStrength;
			node["psiSkill"] = rhs.psiSkill;
			node["melee"] = rhs.melee;
			node["mana"] = rhs.mana;
			node["maneuvering"] = rhs.maneuvering;
			node["missiles"] = rhs.missiles;
			node["dogfight"] = rhs.dogfight;
			node["tracking"] = rhs.tracking;
			node["cooperation"] = rhs.cooperation;
			node["beams"] = rhs.beams;
			node["synaptic"] = rhs.synaptic;
			node["gravity"] = rhs.gravity;
			node["physics"] = rhs.physics;
			node["chemistry"] = rhs.chemistry;
			node["biology"] = rhs.biology;
			node["insight"] = rhs.insight;
			node["data"] = rhs.data;
			node["computers"] = rhs.computers;
			node["tactics"] = rhs.tactics;
			node["materials"] = rhs.materials;
			node["designing"] = rhs.designing;
			node["psionics"] = rhs.psionics;
			node["xenolinguistics"] = rhs.xenolinguistics;
			node["weaponry"] = rhs.weaponry;
			node["explosives"] = rhs.explosives;
			node["efficiency"] = rhs.efficiency;
			node["microelectronics"] = rhs.microelectronics;
			node["metallurgy"] = rhs.metallurgy;
			node["processing"] = rhs.processing;
			node["hacking"] = rhs.hacking;
			node["construction"] = rhs.construction;
			node["diligence"] = rhs.diligence;
			node["alienTech"] = rhs.alienTech;
			node["reverseEngineering"] = rhs.reverseEngineering;
			node["stealth"] = rhs.stealth;
			node["perseption"] = rhs.perseption;
			node["charisma"] = rhs.charisma;
			node["investigation"] = rhs.investigation;
			node["deception"] = rhs.deception;
			node["interrogation"] = rhs.interrogation;
			return node;
		}

		static bool decode(const Node& node, OpenXcom::UnitStats& rhs)
		{
			if (!node.IsMap())
				return false;

			rhs.tu = node["tu"].as<int>(rhs.tu);
			rhs.stamina = node["stamina"].as<int>(rhs.stamina);
			rhs.health = node["health"].as<int>(rhs.health);
			rhs.bravery = node["bravery"].as<int>(rhs.bravery);
			rhs.reactions = node["reactions"].as<int>(rhs.reactions);
			rhs.firing = node["firing"].as<int>(rhs.firing);
			rhs.throwing = node["throwing"].as<int>(rhs.throwing);
			rhs.strength = node["strength"].as<int>(rhs.strength);
			rhs.psiStrength = node["psiStrength"].as<int>(rhs.psiStrength);
			rhs.psiSkill = node["psiSkill"].as<int>(rhs.psiSkill);
			rhs.melee = node["melee"].as<int>(rhs.melee);
			rhs.mana = node["mana"].as<int>(rhs.mana);
			rhs.maneuvering = node["maneuvering"].as<int>(rhs.maneuvering);
			rhs.missiles = node["missiles"].as<int>(rhs.missiles);
			rhs.dogfight = node["dogfight"].as<int>(rhs.dogfight);
			rhs.tracking = node["tracking"].as<int>(rhs.tracking);
			rhs.cooperation = node["cooperation"].as<int>(rhs.cooperation);
			rhs.beams = node["beams"].as<int>(rhs.beams);
			rhs.synaptic = node["synaptic"].as<int>(rhs.synaptic);
			rhs.gravity = node["gravity"].as<int>(rhs.gravity);
			rhs.physics = node["physics"].as<int>(rhs.physics);
			rhs.chemistry = node["chemistry"].as<int>(rhs.chemistry);
			rhs.biology = node["biology"].as<int>(rhs.biology);
			rhs.insight = node["insight"].as<int>(rhs.insight);
			rhs.data = node["data"].as<int>(rhs.data);
			rhs.computers = node["computers"].as<int>(rhs.computers);
			rhs.tactics = node["tactics"].as<int>(rhs.tactics);
			rhs.materials = node["materials"].as<int>(rhs.materials);
			rhs.designing = node["designing"].as<int>(rhs.designing);
			rhs.psionics = node["psionics"].as<int>(rhs.psionics);
			rhs.xenolinguistics = node["xenolinguistics"].as<int>(rhs.xenolinguistics);
			rhs.weaponry = node["weaponry"].as<int>(rhs.weaponry);
			rhs.explosives = node["explosives"].as<int>(rhs.explosives);
			rhs.efficiency = node["efficiency"].as<int>(rhs.efficiency);
			rhs.microelectronics = node["microelectronics"].as<int>(rhs.microelectronics);
			rhs.metallurgy = node["metallurgy"].as<int>(rhs.metallurgy);
			rhs.processing = node["processing"].as<int>(rhs.processing);
			rhs.hacking = node["hacking"].as<int>(rhs.hacking);
			rhs.construction = node["construction"].as<int>(rhs.construction);
			rhs.diligence = node["diligence"].as<int>(rhs.diligence);
			rhs.alienTech = node["alienTech"].as<int>(rhs.alienTech);
			rhs.reverseEngineering = node["reverseEngineering"].as<int>(rhs.reverseEngineering);
			rhs.stealth = node["stealth"].as<int>(rhs.stealth);
			rhs.perseption = node["perseption"].as<int>(rhs.perseption);
			rhs.charisma = node["charisma"].as<int>(rhs.charisma);
			rhs.investigation = node["investigation"].as<int>(rhs.investigation);
			rhs.deception = node["deception"].as<int>(rhs.deception);
			rhs.interrogation = node["interrogation"].as<int>(rhs.interrogation);
			return true;
		}
	};
}
