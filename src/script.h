#ifndef SCRIPTING_H
#define SCRIPTING_H

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "player.h"

const int ITEM_TYPE_ITEM = 0;
const int ITEM_TYPE_WEAPON = 1;
const int ITEM_TYPE_ARMOR = 2;
const int ITEM_TYPE_SPECIAL = 3;

extern void openLuaLibs(lua_State* state);
extern void initGame() throw (ReadError, ScriptError);
extern void callLua(lua_State* luaState, const char *func, const char *sig, ...);
extern void dumpLuaStack(lua_State* stack);
extern void registerCFunctions(lua_State* luaState);
extern char* getItemName(int id);
extern int getItemType(int id);
extern int getWeaponPower(int id);
extern int getArmorDefense(int id);
extern ItemEffects* getItemEffects(int itemNum);
extern char* getItemSound(int id);
extern void deleteItemEffects(ItemEffects* ie);
extern bool itemAffectsAll(int itemID);
void getItemDescription(int id, char *s1, char *s2, char *s3, int bufsize);

extern int accumulated_distance;
extern bool playerScripted;
extern bool battlesDisabled;

#endif
