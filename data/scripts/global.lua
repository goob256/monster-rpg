--
-- Copyright (c) 2007, Trent Gamblin
-- All rights reserved.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions are met:
--    -- Redistributions of source code must retain the above copyright
--       notice, this list of conditions and the following disclaimer.
--    -- Redistributions in binary form must reproduce the above copyright
--       notice, this list of conditions and the following disclaimer in the
--       documentation and/or other materials provided with the distribution.
--    -- Neither the name of the <organization> nor the
--       names of its contributors may be used to endorse or promote products
--       derived from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY <copyright holder> ``AS IS'' AND ANY
-- EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
-- DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
-- DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
-- LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
-- ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
-- (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
-- SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--

MS_CHECKED_WELL = 1
MS_OLDMANS_GOLD = 3
MS_OLDMANS_SWORD = 4
MS_GARDEN_GOLD = 5
MS_GUARDS_SHIELD = 6
MS_MILDREDS_CURE = 7
MS_GAVE_LOOKING_SCOPE = 8
MS_SHOPKEEPERS_GOLD = 9
MS_OVERCHARGED = 10
MS_BOUGHT_LOOKING_SCOPE = 11
MS_CAVE_CURES = 12
MS_CAVE_SCROLL = 13
MS_CAVE_GOLD = 14
MS_DEFEATED_CYCLOPS = 15
MS_INN_HEARD_SONG = 16
MS_BOUGHT_PENDANT = 17
MS_GAVE_BACK_PENDANT = 18
MS_CRYSTAL_BLADE_CHEST = 19
MS_THIEF_CHEST1 = 20
MS_THIEF_CHEST2 = 21
MS_THIEF_CHEST3 = 22
MS_THIEF_CHEST4 = 23
MS_SCROLL_RAIN = 24
MS_GAINED_MORYT = 25
MS_LOST_MORYT = 26
MS_SPOKE_TO_MYSTIC = 27
MS_GOT_MILK = 28
MS_SCROLL_FIRE = 29
MS_LEARNED_CURE = 30
MS_LEARNED_RAIN = 31
MS_LEARNED_FIRE = 32
MS_DEFEATED_WARLOCK = 33
MS_BURNED_LOG = 34

ITEM_ITEM = -1
ITEM_GOLD = 0
ITEM_OLDMANS_SWORD = 1
ITEM_CURE = 2
ITEM_GUARDS_SHIELD = 3
ITEM_HEAL = 4
ITEM_LOOKING_SCOPE = 5
ITEM_WILD_MAN_SWORD = 6
ITEM_WILD_MAN_ARMOR = 7
ITEM_CURE2 = 8
ITEM_SCROLL_CURE = 9
ITEM_CRYSTAL_BLADE = 10
ITEM_REVITALIZE = 11
ITEM_SCROLL_RAIN = 12
ITEM_CURE3 = 13
ITEM_BONE_BLADE = 14
ITEM_BONEMAIL = 15
ITEM_HERB = 16
ITEM_BAIT = 17
ITEM_SCROLL_FIRE = 18

ITEM_TYPE_ITEM = 1
ITEM_TYPE_WEAPON = 2
ITEM_TYPE_ARMOR = 3
ITEM_TYPE_SPECIAL = 4

item_names = {}
item_names[1] = "Old Man's Sword"
item_names[2] = "Cure"
item_names[3] = "Guard's Shield"
item_names[4] = "Heal"
item_names[5] = "Looking Scope"
item_names[6] = "Wild Man Sword"
item_names[7] = "Wild Man Armor"
item_names[8] = "Cure 2"
item_names[9] = "Scroll"
item_names[10] = "Crystal Blade"
item_names[11] = "Revitalize"
item_names[12] = "Scroll"
item_names[13] = "Cure 3"
item_names[14] = "Bone Blade"
item_names[15] = "Bonemail"
item_names[16] = "Herb"
item_names[17] = "Bait"
item_names[18] = "Scroll"

item_types = {}
item_types[1] = ITEM_TYPE_WEAPON
item_types[2] = ITEM_TYPE_ITEM
item_types[3] = ITEM_TYPE_ARMOR
item_types[4] = ITEM_TYPE_ITEM
item_types[5] = ITEM_TYPE_SPECIAL
item_types[6] = ITEM_TYPE_WEAPON
item_types[7] = ITEM_TYPE_ARMOR
item_types[8] = ITEM_TYPE_ITEM
item_types[9] = ITEM_TYPE_ITEM
item_types[10] = ITEM_TYPE_WEAPON
item_types[11] = ITEM_TYPE_ITEM
item_types[12] = ITEM_TYPE_ITEM
item_types[13] = ITEM_TYPE_ITEM
item_types[14] = ITEM_TYPE_WEAPON
item_types[15] = ITEM_TYPE_ARMOR
item_types[16] = ITEM_TYPE_ITEM
item_types[17] = ITEM_TYPE_ITEM
item_types[18] = ITEM_TYPE_ITEM

PARTNER_MORYT = 0

function randint(max)
	return getRandomNumber(math.floor(max));
end

function get_item_name(n)
	return item_names[n]
end

function get_item_type(n)
	return item_types[n]
end

-- Returns status (bad), hp, maxHP, latency, mana, spell (effect on BattleStats)
function get_item_effects(n)
	if (n == ITEM_CURE) then
		return 0, 10, 0, 0, 0, ""
	elseif (n == ITEM_HEAL) then
		return 1, 0, 0, 0, 0, ""
	elseif (n == ITEM_CURE2) then
		return 0, 50, 0, 0, 0, ""
	elseif (n == ITEM_SCROLL_CURE) then
		return 0, 0, 0, 0, 0, "Cure"
	elseif (n == ITEM_REVITALIZE) then
		return 1, 99999, 0, 0, 99999, 0
	elseif (n == ITEM_SCROLL_RAIN) then
		return 0, 0, 0, 0, 0, "Rain"
	elseif (n == ITEM_CURE3) then
		return 0, 250, 0, 0, 0, ""
	elseif (n == ITEM_HERB) then
		return 0, 0, 0, 0, 100, ""
	elseif (n == ITEM_BAIT) then
		return 0, 1, 0, 0, 0, ""
	elseif (n == ITEM_SCROLL_FIRE) then
		return 0, 0, 0, 0, 0, "Fire"
	else
		return 0, 0, 0, 0, 0, ""
	end
end

function get_weapon_power(n)
	if (n == ITEM_OLDMANS_SWORD) then
		return 2
	elseif (n == ITEM_WILD_MAN_SWORD) then
		return 40
	elseif (n == ITEM_CRYSTAL_BLADE) then
		return 75
	elseif (n == ITEM_BONE_BLADE) then
		return 150
	else
		return 0
	end
end

function get_armor_defense(n)
	if (n == ITEM_GUARDS_SHIELD) then
		return 1
	elseif (n == ITEM_WILD_MAN_ARMOR) then
		return 40
	elseif (n == ITEM_BONEMAIL) then
		return 100
	else
		return 0
	end
end

function get_item_sound(n)
	if (n == ITEM_CURE or n == ITEM_HEAL or n == ITEM_CURE2
			or n == ITEM_SCROLL_CURE
			or n == ITEM_SCROLL_RAIN
			or n == ITEM_CURE3
			or n == ITEM_HERB
			or n == ITEM_SCROLL_FIRE) then
		return "dust.ogg"
	elseif (n == ITEM_REVITALIZE) then
		return "revitalize.ogg"
	else
		return "nil"
	end
end

function item_affects_all(n)
	if (n == ITEM_REVITALIZE) then
		return true
	else
		return false
	end
end

function get_item_description(number)
	if (number == 1) then -- Old man's sword
		return
			"A rusty old sword.",
			"+" .. get_weapon_power(number) .. " power.",
			""
	elseif (number == 2) then -- Cure
		return
			"A curing potion.",
			"",
			""
	elseif (number == 3) then -- Guard's Shield
		return
			"A rusty old shield.",
			"+" .. get_armor_defense(number) .. " defense.",
			""
	elseif (number == 4) then -- Heal
		return
			"A healing herb.",
			"",
			""
	elseif (number == 5) then -- Looking Scope
		return
			"A small looking scope.",
			"",
			""
	elseif (number == 6) then -- Wild Man Sword
		return
			"A well used sword.",
			"+" .. get_weapon_power(number) .. " power.",
			""
	elseif (number == 7) then -- Wild Man Armor
		return
			"Well used armor.",
			"+" .. get_armor_defense(number) .. " defense.",
			""
	elseif (number == 8) then -- Cure 2
		return
			"A curing potion.",
			"",
			""
	elseif (number == 9) then -- Scroll
		return
			"A Cure scroll.",
			"",
			""
	elseif (number == 10) then -- Crystal Blade
		return
			"A shiny crystal blade.",
			"+" .. get_weapon_power(number) .. " power.",
			""
	elseif (number == 11) then -- Revitalize
		return
			"A powerful potion.",
			"",
			""
	elseif (number == 12) then -- Scroll
		return
			"A Rain scroll.",
			"",
			""
	elseif (number == 13) then -- Cure 3
		return
			"A powerful curing potion.",
			"",
			""
	elseif (number == 14) then -- Bone Blade
		return
			"A blade made of whale bone.",
			"+" .. get_weapon_power(number) .. " power.",
			""
	elseif (number == 15) then -- Bonemail
		return
			"Mail made from whale bones.",
			"+" .. get_armor_defense(number) .. " defense.",
			""
	elseif (number == 16) then -- Herb
		return
			"A mystic herb.",
			"",
			""
	elseif (number == 17) then -- Bait
		return
			"Fishing bait.",
			"",
			""
	elseif (number == 18) then -- Scroll
		return
			"A Fire scroll.",
			"",
			""
	end
end

function set_spell_milestone(name)
	if (name == "Cure") then
		setMilestone(MS_LEARNED_CURE, true)
	elseif (name == "Rain") then
		setMilestone(MS_LEARNED_RAIN, true)
	elseif (name == "Fire") then
		setMilestone(MS_LEARNED_FIRE, true)
	end
end

