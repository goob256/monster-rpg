#include "monster.h"

void EnemyCombatant::init()
{
	luaState = lua_open();

	openLuaLibs(luaState);

	registerCFunctions(luaState);

	debug_message("Loading global script...\n");
	if (luaL_loadfile(luaState, getResource("scripts/global.%s", getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running global script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Loading global enemy script...\n");
	if (luaL_loadfile(luaState, getResource("enemy_scripts/global.%s", getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running global enemy script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Loading enemy script...\n");
	if (luaL_loadfile(luaState, getResource("enemy_scripts/%s.%s", name, getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running enemy script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}
	
	callLua(luaState, "start", "i>", number+1);

	animSet = new AnimationSet(getResource("enemy_animations/%s.anims", name));
	
	callLua(luaState, "get_size", ">iiii");

	width = (int)lua_tonumber(luaState, -4);
	height = (int)lua_tonumber(luaState, -3);
	shadowOffset = (int)lua_tonumber(luaState, -2);
	shadowHeight = (int)lua_tonumber(luaState, -1);

	lua_pop(luaState, 4);

	lua_getglobal(luaState, "hide_status");
	if (lua_isboolean(luaState, -1)) {
	    if (lua_toboolean(luaState, -1)) {
		statusDetails = false;
	   }
	}
}

void EnemyCombatant::update(int step)
{
	callLua(luaState, "update", "i>", step);
}

Animation* EnemyCombatant::getCurrentAnimation()
{
	return animSet->getCurrentAnimation();
}

TurnAction* EnemyCombatant::getAction()
{
	static char name[500];

	callLua(luaState, "get_action", ">iisi");

	turnAction.type = (TurnType)lua_tonumber(luaState, -4);
	turnAction.action.index = (int)lua_tonumber(luaState, -3);
	strcpy(name, lua_tostring(luaState, -2));
	turnAction.action.name = name;
	turnAction.target = (int)lua_tonumber(luaState, -1)-1;

	lua_pop(luaState, 4);

	return &turnAction;
}

bool EnemyCombatant::isPlayer()
{
	return false;
}

int EnemyCombatant::getPower()
{
	callLua(luaState, "get_power", ">i");

	int power = (int)lua_tonumber(luaState, -1);

	lua_pop(luaState, 1);

	return power;
}

int EnemyCombatant::getWeaponDefense(int weapon)
{
	callLua(luaState, "get_weapon_defense", "i>i", weapon+1);

	int defense = (int)lua_tonumber(luaState, -1);

	lua_pop(luaState, 1);

	return defense;
}

int EnemyCombatant::getSpellDefense(char *spell_name)
{
	callLua(luaState, "get_spell_defense", "s>i", spell_name);

	int defense = (int)lua_tonumber(luaState, -1);

	lua_pop(luaState, 1);

	return defense;
}

EnemyCombatant::EnemyCombatant(const char* name, int x, int y)
	: Combatant(name, x, y)
{
}

EnemyCombatant::~EnemyCombatant()
{
	callLua(luaState, "stop", ">");
	lua_close(luaState);
}
