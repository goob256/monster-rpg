#include <cstdarg>
#include <cstring>

#include <TGUI/tgui.h>

#include "monster.h"

/*
 * Distance walked without a battle in the previous area
 */
int accumulated_distance = 0;

/*
 * Player is controlled by a script.
 */
bool playerScripted = false;

bool battlesDisabled = false;


/*
 * Call a Lua function, leaving the results on the stack.
 */
void callLua(lua_State* luaState, const char *func, const char *sig, ...)
{
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(luaState, func);  /* get function */

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {
			case 'd':  /* double argument */
				lua_pushnumber(luaState, va_arg(vl, double));
				break;

			case 'i':  /* int argument */
				lua_pushnumber(luaState, va_arg(vl, int));
				break;

			case 's':  /* string argument */
				lua_pushstring(luaState, va_arg(vl, char *));
				break;
			case '>':
				goto endwhile;
			default:
				break;
		}
		narg++;
		luaL_checkstack(luaState, 1, "too many arguments");
	}
endwhile:

	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(luaState, narg, nres, 0) != 0) {
		debug_message("error running function `f': %s", lua_tostring(luaState, -1));
	}

	va_end(vl);
}

static const luaL_Reg lualibs[] = {
	{ "", luaopen_base },
	{ LUA_LOADLIBNAME, luaopen_package },
	{ LUA_TABLIBNAME, luaopen_table },
//	{ LUA_IOLIBNAME, luaopen_io },
	{ LUA_OSLIBNAME, luaopen_os },
	{ LUA_STRLIBNAME, luaopen_string },
	{ LUA_MATHLIBNAME, luaopen_math },
	{ LUA_DBLIBNAME, luaopen_debug },
	{ 0, 0 }
};

void openLuaLibs(lua_State *L) 
{
	const luaL_Reg *lib = lualibs;
	for (; lib->func; lib++) {
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}
}

void dumpLuaStack(lua_State *l) 
{
        int i;
        int top = lua_gettop(l);

        printf ("--- stack ---\n");
        printf ("top=%u   ...   ", top);

        for (i = 1; i <= top; i++) {  /* repeat for each level */
                int t = lua_type(l, i);
                switch (t) {

                case LUA_TSTRING:  /* strings */
                        printf("`%s'", lua_tostring(l, i));
                        break;

                case LUA_TBOOLEAN:  /* booleans */
                        printf(lua_toboolean(l, i) ? "true" : "false");
                        break;

                case LUA_TNUMBER:  /* numbers */
                        printf("%g", lua_tonumber(l, i));
                        break;

                case LUA_TTABLE:   /* table */
                        printf("table");
                        break;

                default:  /* other values */
                        printf("%s", lua_typename(l, t));
                        break;

                }
                printf("  ");  /* put a separator */
        }
        printf("\n");  /* end the listing */

        printf ("-------------\n");
}

char* getItemName(int id)
{
	static char name[100];
	if (id < 0) {
		strcpy(name, "");
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_item_name", "i>s", id+1);
		snprintf(name, (sizeof(name)/sizeof(*name))-1, "%s", lua_tostring(stack, -1));
		lua_pop(stack, 1);
	}
	return name;
}

int getItemType(int id)
{
	if (id < 0) {
		return -1;
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_item_type", "i>i", id+1);
		int itemNum = (int)lua_tonumber(stack, -1)-1;
		lua_pop(stack, 1);
		return itemNum;
	}
}

int getWeaponPower(int id)
{
	if (id < 0) {
		return -1;
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_weapon_power", "i>i", id+1);
		int power = (int)lua_tonumber(stack, -1)-1;
		lua_pop(stack, 1);
		return power;
	}
}

int getArmorDefense(int id)
{
	if (id < 0) {
		return -1;
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_armor_defense", "i>i", id+1);
		int defense = (int)lua_tonumber(stack, -1)-1;
		lua_pop(stack, 1);
		return defense;
	}
}

ItemEffects* getItemEffects(int itemNum)
{
	lua_State* luaState = lua_open();

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

	callLua(luaState, "get_item_effects", "i>iiiiis", itemNum+1);

	ItemEffects* ie = new ItemEffects();

	ie->status = (int)lua_tonumber(luaState, -6);
	ie->hp = (int)lua_tonumber(luaState, -5);
	ie->maxHP = (int)lua_tonumber(luaState, -4);
	ie->latency = (int)lua_tonumber(luaState, -3);
	ie->mana = (int)lua_tonumber(luaState, -2);
	ie->spell = strdup(lua_tostring(luaState, -1));

	lua_pop(luaState, 6);
	
	lua_close(luaState);

	return ie;
}

bool itemAffectsAll(int itemID)
{
	lua_State* luaState = lua_open();

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

	callLua(luaState, "item_affects_all", "i>i", itemID+1);

	bool result = (bool)lua_toboolean(luaState, -1);

	lua_pop(luaState, 1);
	
	lua_close(luaState);

	return result;
}

char* getItemSound(int id)
{
	static char name[100];

	if (id < 0) {
		return 0;
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_item_sound", "i>s", id+1);
		snprintf(name, (sizeof(name)/sizeof(*name))-1, "%s", lua_tostring(stack, -1));
		lua_pop(stack, 1);
		return name;
	}
}

void deleteItemEffects(ItemEffects* ie)
{
	free(ie->spell);
	delete ie;
}

void getItemDescription(int id, char *s1, char *s2, char *s3, int bufsize)
{
	if (id < 0) {
		strcpy(s1, "Unknown.");
		strcpy(s2, "");
		strcpy(s3, "");
		return;
	}
	else {
		lua_State* stack = currArea->getLuaState();
		callLua(stack, "get_item_description", "i>sss", id+1);
		snprintf(s1, bufsize-1, "%s", lua_tostring(stack, -3));
		snprintf(s2, bufsize-1, "%s", lua_tostring(stack, -2));
		snprintf(s3, bufsize-1, "%s", lua_tostring(stack, -1));
		lua_pop(stack, 3);
	}
}

static int CStartArea(lua_State* stack)
{
	const char* name = lua_tostring(stack, 1);

	startArea(name);

	accumulated_distance = 0;

	return 0;
}

static int CSetObjectPosition(lua_State* stack)
{
	int i = (int)lua_tonumber(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);

	currArea->getObject(i-1)->setPosition(x, y);

	return 0;
}

static int CFadeIn(lua_State* stack)
{
	BITMAP* backBuffer = scr->getBackBuffer();
	currArea->draw(backBuffer);
	scr->fadeIn();
	return 0;
}

static int CFadeOut(lua_State* stack)
{
	BITMAP* backBuffer = scr->getBackBuffer();
	currArea->draw(backBuffer);
	scr->fadeOut();
	return 0;
}

static int CSetObjectDirection(lua_State* stack)
{
	int i = (int)lua_tonumber(stack, 1);
	Direction d = (Direction)(lua_tonumber(stack, 2)-1);

	currArea->getObject(i-1)->setDirection(d);
	
	return 0;
}

static int CSetObjectTotalHeight(lua_State* stack)
{
	int i = (int)lua_tonumber(stack, 1);
	int h = (int)lua_tonumber(stack, 2);

	currArea->getObject(i-1)->setTotalHeight(h);
	
	return 0;
}

static int CAddObject(lua_State* stack)
{
	int x = (int)lua_tonumber(stack, 1);
	int y = (int)lua_tonumber(stack, 2);
	int w = (int)lua_tonumber(stack, 3);
	int base_h = (int)lua_tonumber(stack, 4);
	int total_h = (int)lua_tonumber(stack, 5);

	Object* o = 0;

	try {
		o = new Object(x, y, w, base_h, total_h);
		currArea->addObject(o);
	}
	catch (std::bad_alloc e) {
		debug_message("%s", "Error adding object at %d,%d\n", x, y);
		if (o)
			delete o;
	}

	lua_pop(stack, 4);

	return 0;
}

static int CSetObjectAnimationSet(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	const char* name = lua_tostring(stack, 2);

	debug_message("Setting object %d animation set to %s\n", n-1, name);

	AnimationSet* a;
	try {
		a = new AnimationSet(getResource("objects/%s.anims", name));
	}
	catch (std::bad_alloc e) {
		debug_message("Error loading animation set %s\n", name);
		lua_pop(stack, 2);
		return 0;
	}

	Object* o = currArea->getObject(n-1);
	o->setAnimationSet(a);
	
	lua_pop(stack, 2);

	return 0;
}

static int CSetObjectInputToScriptControlled(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	if (n == 1) {
		playerScripted = true;
	}

	Object* o = currArea->getObject(n-1);
	if (o->getInput())
	   o->deleteInput();
	Input* i = new ScriptInput;
	o->setInput(i);

	lua_pop(stack, 1);

	return 0;
}

static void realSetObjectInputToPlayerControlled(int n)
{
	Object* o = currArea->getObject(n);
	if (o->getInput())
	    o->deleteInput();
	Input* i = createInput(config.getInputType());
	o->setInput(i);
}

static int CSetObjectInputToPlayerControlled(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	if (n == 1) {
		playerScripted = false;
	}

	realSetObjectInputToPlayerControlled(n-1);

	lua_pop(stack, 1);

	return 0;
}

static int CMoveObject(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	bool l = (bool)lua_tonumber(stack, 2);
	bool r = (bool)lua_tonumber(stack, 3);
	bool u = (bool)lua_tonumber(stack, 4);
	bool d = (bool)lua_tonumber(stack, 5);

	Object* o = currArea->getObject(n-1);
	Input* i = o->getInput();

	i->set(l, r, u, d);

	lua_pop(stack, 5);

	return 0;
}

static int CGetObjectPosition(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	float x = o->getFX();
	float y = o->getFY();

	lua_pushnumber(stack, x);
	lua_pushnumber(stack, y);

	return 2;
}

static int CMakeCharacter(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	o->setType(OBJECT_CHARACTER);

	return 0;
}

static void doDialogue(const char* text, int color)
{
	tguiSetParent(0);
	MWgtSpeech* speech = new MWgtSpeech(12, 12, 276, text);
	int maxLines = speech->getMaxLines();
	int height = 40 + (maxLines * (mWgtFontHeight()+3));
	MWgtFrame* frame = new MWgtFrame(0, 0, 300, height,
			color, 200);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(speech);
	MWgtAdvancer* advancer = new MWgtAdvancer(275, height-20, speech);
	tguiAddWidget(advancer);
	tguiSetFocus(advancer);

	bool top = player->getY() < Screen::BUFFER_HEIGHT/2 ?
		true : false;

	if (top) {
		tguiTranslateWidget(frame, 10, 240-height-10);
	}
	else {
		tguiTranslateWidget(frame, 10, 10);
	}

	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();
	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			currArea->draw(bb);
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == speech) {
				break;
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	tguiDeleteWidget(frame);

	delete frame;
	delete speech;
	delete advancer;

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	currArea->draw(bb);
	scr->draw();
}

/*
 * Display a speech dialog.
 */
int CDoDialogue(lua_State* stack)
{
	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	const char* text = lua_tostring(stack, 1);

	doDialogue(text, makecol(0, 96, 160));

	return 0;
}

int CDoMessage(lua_State* stack)
{
	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	const char* text = lua_tostring(stack, 1);

	doDialogue(text, makecol(160, 0, 0));

	return 0;
}

int CDoPickupMessage(lua_State* stack)
{
	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	const char* text = lua_tostring(stack, 1);

	doDialogue(text, makecol(160, 160, 0));

	return 0;
}

int CCheckMilestone(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	lua_pushboolean(stack, (int)stats.milestones[n-1]);

	return 1;
}

int CSetMilestone(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	bool value = (bool)lua_toboolean(stack, 2);

	stats.milestones[n-1] = value;

	return 0;
}

int CSetObjectSolid(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	bool value = (bool)lua_toboolean(stack, 2);

	Object* o = currArea->getObject(n-1);
	o->setSolid(value);

	return 0;
}

int CObjectIsSolid(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	lua_pushboolean(stack, (int)o->isSolid());

	return 1;
}

int CSetObjectHigh(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	bool value = (bool)lua_toboolean(stack, 2);

	Object* o = currArea->getObject(n-1);
	o->setHigh(value);

	return 0;
}

int CObjectIsHigh(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	lua_pushboolean(stack, (int)o->isHigh());

	return 1;
}

int CAdvanceObjectAnimation(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	o->advanceAnimation();

	return 0;
}

int CSetObjectAnimated(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	bool value = (bool)lua_toboolean(stack, 2);

	Object* o = currArea->getObject(n-1);
	o->setAnimated(value);

	return 0;
}

int CObjectIsColliding(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Object* o = currArea->getObject(n-1);
	bool coll = currArea->checkCollision(o, o->getX(), o->getY(),
			o->getBoundingBox());

	if (coll)
		lua_pushboolean(stack, 1);
	else
		lua_pushboolean(stack, 0);

	return 1;
}

int CObjectsAreColliding(lua_State* stack)
{
	int n1 = (int)lua_tonumber(stack, 1);
	int n2 = (int)lua_tonumber(stack, 2);

	Object* o1 = currArea->getObject(n1-1);
	Object* o2 = currArea->getObject(n2-1);

	int x1 = o1->getX();
	int x2 = x1 + o1->getWidth()-1;
	int y1 = o1->getY();
	int y2 = y1 + o1->getHeight()-1;
	int x3 = o2->getX();
	int x4 = x3 + o2->getWidth()-1;
	int y3 = o2->getY();
	int y4 = y3 + o2->getHeight()-1;

	bool coll = checkBoxCollision(x1, y1, x2, y2, x3, y3, x4, y4);

	lua_pushboolean(stack, (int)coll);

	return 1;
}

int CPlaySound(lua_State* stack)
{
	const char* base = lua_tostring(stack, 1);

	char name[1000];

	sprintf(name, "%s.ogg", base);

	lua_pushnumber(stack, playOgg(name));

	return 1;
}

int CGetStart(lua_State* stack)
{
	//FIXME: use save state
	lua_pushstring(stack, "heroshome");
	lua_pushnumber(stack, 256);
	lua_pushnumber(stack, 64);
	lua_pushnumber(stack, (int)DIRECTION_SOUTH+1);
	return 4;
}

int CGetPreviousAreaName(lua_State* stack)
{
	if (prevAreaName)
		lua_pushstring(stack, prevAreaName);
	else
		lua_pushnil(stack);
	return 1;
}

int CFirstInventorySlotAvailable(lua_State* stack)
{
	lua_pushnumber(stack, firstInventorySlotAvailable());
	return 1;
}

int CSetInventory(lua_State* stack)
{
	int slot = (int)lua_tonumber(stack, 1);
	int id = (int)lua_tonumber(stack, 2);
	int quantity = (int)lua_tonumber(stack, 3);

	stats.inventory[slot].id = id-1;
	stats.inventory[slot].quantity = quantity;

	return 0;
}

int CFindInventoryItem(lua_State* stack)
{
	int id = (int)lua_tonumber(stack, 1);

	lua_pushnumber(stack, findInventoryItem(id-1));

	return 1;
}

int CAddToInventory(lua_State* stack)
{
	int slot = (int)lua_tonumber(stack, 1);
	int quantity = (int)lua_tonumber(stack, 2);

	stats.inventory[slot].quantity += quantity;
	if (stats.inventory[slot].quantity > MAX_QUANTITY)
		stats.inventory[slot].quantity = MAX_QUANTITY;

	return 0;
}

int CGiveGold(lua_State* stack)
{
	unsigned int quantity = (unsigned int)lua_tonumber(stack, 1);

	stats.gold = MIN(stats.gold+quantity, MAX_GOLD);

	return 0;
}

int CTakeGold(lua_State* stack)
{
	int quantity = (int)lua_tonumber(stack, 1);

	stats.gold -= quantity;

	return 0;
}

int CCheckGold(lua_State* stack)
{
	lua_pushnumber(stack, stats.gold);

	return 1;
}

int CSetObjectAnimationFrame(lua_State* stack)
{
	int obj = (int)lua_tonumber(stack, 1);
	int frame = (int)lua_tonumber(stack, 2);

	Object* o = currArea->getObject(obj-1);
	o->setAnimationFrame(frame-1);

	return 0;
}

int CSetObjectSubAnimation(lua_State* stack)
{
	int obj = (int)lua_tonumber(stack, 1);
	int sub = (int)lua_tonumber(stack, 2);

	Object* o = currArea->getObject(obj-1);
	o->setSubAnimation(sub-1);

	return 0;
}

int CSetObjectSubAnimationNamed(lua_State* stack)
{
	int obj = (int)lua_tonumber(stack, 1);
	const char* name = lua_tostring(stack, 2);

	Object* o = currArea->getObject(obj-1);
	if (!o->setSubAnimation(name)) {
	    debug_message("Trying to set object %d sub animation to %s failed.\n", obj-1, name);
	}

	return 0;
}

int CSetObjectHidden(lua_State* stack)
{
	int objnum = (int)lua_tonumber(stack, 1);
	bool value = (bool)lua_toboolean(stack, 2);

	Object* o = currArea->getObject(objnum-1);
	o->setHidden(value);
	
	return 0;
}

int CGetHP(lua_State* stack)
{
	lua_pushnumber(stack, stats.hp);
	return 1;
}

int CGetMaxHP(lua_State* stack)
{
	lua_pushnumber(stack, stats.maxHP);
	return 1;
}

int CSetMaxHP(lua_State* stack)
{
	int maxHP = (int)lua_tonumber(stack, 1);

	stats.maxHP = MIN(MAX_MAXHP, maxHP);

	return 0;
}

int CSetHP(lua_State* stack)
{
	int hp = (int)lua_tonumber(stack, 1);
	stats.hp = hp;
	return 0;
}

int CAddToHP(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	stats.hp += n;
	stats.hp = MIN(stats.hp, stats.maxHP);
	return 0;
}

int CTileIsSolid(lua_State* stack)
{
	int x = (int)lua_tonumber(stack, 1);
	int y = (int)lua_tonumber(stack, 2);

	Tile* t = currArea->getTile(x, y);
	lua_pushboolean(stack, (int)t->solid);

	return 1;
}

int CDoShop(lua_State* stack)
{
	const char* shopkeeperName = lua_tostring(stack, 1);
	int numItems = (int)lua_tonumber(stack, 2);
	int* itemNums = new int[numItems];
	unsigned int* itemCosts = new unsigned int[numItems];
	int itemsBought = 0;

	for (int i = 0; i < numItems; i++) {
		itemNums[i] = (int)lua_tonumber(stack, (i*2)+3)-1;
		itemCosts[i] = (unsigned int)lua_tonumber(stack, (i*2)+4);
	}

	int frameHeight = (3 + numItems) * mWgtFontHeight() + 40;
	int frameWidth = 256;

	tguiSetParent(0);
	MWgtFrame* mainFrame = new MWgtFrame(0, 0, frameWidth, frameHeight,
			makecol(0, 160, 0), 200);
	tguiAddWidget(mainFrame);
	tguiSetParent(mainFrame);
	char mainText[20];
	sprintf(mainText, "You have %d gold", stats.gold);
	MWgtText* mainTextWidget = new MWgtText(10, 10, mainText,
			makecol(255, 255, 255));
	tguiAddWidget(mainTextWidget);
	int y = 10 + mWgtFontHeight()*2;
	MWgtTextButton** textButtons = new MWgtTextButton*[numItems];;
	MWgtText** costTextWidgets = new MWgtText*[numItems];
	char** buttonText = new char*[numItems];
	char** costTexts = new char*[numItems];
	for (int i = 0; i < numItems; i++) {
		callLua(stack, "get_item_name", "i>s", itemNums[i]+1);
		const int MAX_ITEM_NAME = 100;
		buttonText[i] = new char[MAX_ITEM_NAME];
		snprintf(buttonText[i], MAX_ITEM_NAME-1, "%s", lua_tostring(stack, -1));
		lua_pop(stack, 1);
		textButtons[i] = new MWgtTextButton(10, y, buttonText[i]);
		tguiAddWidget(textButtons[i]);
		costTexts[i] = new char[5];
		sprintf(costTexts[i], "%d", itemCosts[i]);
		costTextWidgets[i] = new MWgtText(206, y, costTexts[i], makecol(255, 255, 255));
		tguiAddWidget(costTextWidgets[i]);
		y += mWgtFontHeight();
	}
	y += mWgtFontHeight();
	MWgtButton* doneButton = new MWgtButton(frameWidth/2-32, y, 64, "Done");
	tguiAddWidget(doneButton);
	tguiSetFocus(textButtons[0]);
	tguiTranslateWidget(mainFrame, Screen::BUFFER_WIDTH/2-frameWidth/2,
			Screen::BUFFER_HEIGHT/2-frameHeight/2);
	costTextWidgets[0]->setColor(makecol(255, 255, 0));

	int confirmWidth = 20 + mWgtTextWidth("Are you sure?");
	int confirmHeight = 50 + mWgtFontHeight()*3;
	MWgtFrame* confirmFrame = new MWgtFrame(0, 0, confirmWidth,
			confirmHeight, makecol(0, 160, 0), 200);
	int by = 10+mWgtFontHeight()*2;
	MWgtText* confirmText = new MWgtText(10, 10, "Are you sure?", makecol(255, 255, 255));
	MWgtButton* yes = new MWgtButton(confirmWidth/4-15, by, 30, "Yes");
	MWgtButton* no = new MWgtButton(confirmWidth*3/4-15, by, 30, "No");
	MWgtButton *examine = new MWgtButton(confirmWidth/2-30, by+23, 60, "Examine");

	BITMAP* bb = scr->getBackBuffer();

	bool confirming = false;
	int itemSelected = -1;
	bool translatedConfirm = false;
	bool boughtSpecial = false;

	//rest(250);
	waitForRelease();
	clear_keybuf();

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			currArea->draw(bb);
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			TGUIWidget* activeWidget = tguiGetActiveWidget();
			for (int i = 0; i < numItems; i++) {
				if (textButtons[i] == activeWidget) {
					costTextWidgets[i]->setColor(makecol(255, 255, 0));
				}
				else {
					costTextWidgets[i]->setColor(makecol(255, 255, 255));
				}
			}
			if (confirming) {
				if (widget == yes || widget == no || widget == examine || escapePressed()) {
					if (widget == yes) {
						/* Check if you have enough money/space etc */
						int slot = findInventoryItem(itemNums[itemSelected]);
						if (slot < 0) {
							slot = firstInventorySlotAvailable();
						}
						if (slot < 0 || stats.inventory[slot].quantity > 98) {
							playOgg("error.ogg");
							doDialogue("Coro: Hmm.. I can't carry this...$", makecol(0, 96, 16));
						}
						else if (stats.gold < itemCosts[itemSelected]) {
							playOgg("error.ogg");
							char msg[100];
							sprintf(msg, "%s: You don't have enough for that...$", shopkeeperName);
							doDialogue(msg, makecol(0, 96, 160));
						}
						else {
							playOgg("give.ogg");
							itemsBought++;
							stats.gold -= itemCosts[itemSelected];
							sprintf(mainText, "You have %d gold", stats.gold);
							if (getItemType(itemNums[itemSelected]) == ITEM_TYPE_SPECIAL) {
							    boughtSpecial = true;
							}
							else {
							    int slot = findInventoryItem(itemNums[itemSelected]);
							    if (slot < 0) {
								    slot = firstInventorySlotAvailable();
								    stats.inventory[slot].id = itemNums[itemSelected];
								    stats.inventory[slot].quantity = 1;
							    }
							    else {
								    stats.inventory[slot].quantity++;
								    if (stats.inventory[slot].quantity > 99) {
									    stats.inventory[slot].quantity = 99;
								    }
							    }
							}
						}
					}
					else if (widget == examine) {
						showItemDescription(itemNums[itemSelected]);
					}
					/*
					for (int i = 0; i < 250; i++) {
						updateMusic();
						rest(1);
					}
					*/
					waitForRelease();
					clear_keybuf();
					tguiDeleteWidget(confirmFrame);
					tguiSetFocus(textButtons[0]);
					costTextWidgets[0]->setColor(makecol(255, 255, 0));
					tguiMakeDirty(mainFrame);
					confirming = false;
					if (boughtSpecial) {
					    break;
					}
				}
			}
			else {
				itemSelected = -1;
				int i;
				for (i = 0; i < numItems; i++) {
					if (widget == textButtons[i]) {
						itemSelected = i;
						break;
					}
				}
				if (itemSelected >= 0) {
					tguiSetParent(0);
					tguiAddWidget(confirmFrame);
					tguiSetParent(confirmFrame);
					tguiAddWidget(confirmText);
					tguiAddWidget(yes);
					tguiAddWidget(no);
					tguiAddWidget(examine);
					if (!translatedConfirm) {
						tguiTranslateWidget(confirmFrame, Screen::BUFFER_WIDTH/2-confirmWidth/2,
							Screen::BUFFER_HEIGHT/2-confirmHeight/2);
						translatedConfirm = true;
					}
					costTextWidgets[itemSelected]->setColor(makecol(255, 255, 255));
					tguiSetFocus(yes);
					confirming = true;
				}
				else if (widget == doneButton || escapePressed()) {
					break;
				}
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	delete[] itemNums;
	delete[] itemCosts;

	tguiDeleteWidget(mainFrame);

	delete mainFrame;
	delete mainTextWidget;

	for (int i = 0; i < numItems; i++) {
		delete textButtons[i];
		delete costTextWidgets[i];
		delete[] buttonText[i];
		delete[] costTexts[i];
	}

	delete[] textButtons;
	delete[] costTextWidgets;
	delete[] buttonText;
	delete[] costTexts;

	delete doneButton;
	delete confirmFrame;
	delete confirmText;
	delete yes;
	delete no;
	delete examine;

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	currArea->draw(bb);
	scr->draw();

	lua_pushnumber(stack, itemsBought);

	return 1;
}

int CDoQuestion(lua_State* stack)
{
	const char* text1 = lua_tostring(stack, 1);
	const char* text2 = lua_tostring(stack, 2);

	int frameHeight = 35 + mWgtFontHeight() * 3;
	int frameWidth = 300;

	tguiSetParent(0);

	MWgtFrame* mainFrame = new MWgtFrame(0, 0, frameWidth, frameHeight,
			makecol(0, 96, 160), 200);
	tguiAddWidget(mainFrame);
	tguiSetParent(mainFrame);

	MWgtText* text1Widget = new MWgtText(10, 10, text1,
			makecol(255, 255, 255));
	tguiAddWidget(text1Widget);

	MWgtText* text2Widget = new MWgtText(10, 10+mWgtFontHeight(), text2,
			makecol(255, 255, 255));
	tguiAddWidget(text2Widget);

	int by = 10 + mWgtFontHeight()*2 + 5;

	MWgtButton* yes = new MWgtButton(frameWidth/4-15, by, 30, "Yes");
	MWgtButton* no = new MWgtButton(frameWidth*3/4-15, by, 30, "No");

	tguiAddWidget(yes);
	tguiAddWidget(no);

	tguiTranslateWidget(mainFrame, Screen::BUFFER_WIDTH/2-frameWidth/2,
			Screen::BUFFER_HEIGHT/2-frameHeight/2);

	tguiSetFocus(yes);

	BITMAP* bb = scr->getBackBuffer();

	//rest(250);
	waitForRelease();
	clear_keybuf();

	long start = tguiCurrentTimeMillis();
	int ret = 0;

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			currArea->draw(bb);
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == yes) {
			    ret = 1;
			    break;
			}
			else if (widget == no) {
			    ret = 0;
			    break;
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	tguiDeleteWidget(mainFrame);

	delete mainFrame;
	delete text1Widget;
	delete text2Widget;
	delete yes;
	delete no;

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	currArea->draw(bb);
	scr->draw();

	lua_pushboolean(stack, ret);

	return 1;
}

int CRemoveObject(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	currArea->removeObject(n-1);

	return 0;
}

static bool closeTo(float f1, float f2)
{
	float ACCEPTABLE_DISTANCE = 0.9f;
	if (f1 >= (f2-ACCEPTABLE_DISTANCE) && f1 <= (f2+ACCEPTABLE_DISTANCE))
		return true;
	else
		return false;
}

static int CUpdateUntilArrival(lua_State* stack)
{
	int objNum = (int)lua_tonumber(stack, 1)-1;
	float destX = lua_tonumber(stack, 2);
	float destY = lua_tonumber(stack, 3);
	Object* obj = currArea->getObject(objNum);

	BITMAP* backBuffer = scr->getBackBuffer();

	Input* input = player->getInput();
	if (input->isPlayerControlled())
		player->setInput(0);

	unsigned long startRun = currentTimeMillis();
	unsigned long start = currentTimeMillis();
	unsigned int frameSkip = 0;

	unsigned int maxUpdateStep = 40;

	for (;;) {
		updateMusic();

		if (closeTo(obj->getFX(), destX) && closeTo(obj->getFY(), destY)) {
			debug_message("Reached destination.\n");
			break;
		}
		if (frameSkip++ >= config.getFrameSkip()) {
			frameSkip = 0;
			if (config.useDirtyRectangles() && !config.useSmoothScaling()) {
				currArea->drawDirty(backBuffer);
				std::vector<Object*>* objects = currArea->getObjects();
				scr->drawDirty(objects);
			}
			else {
				currArea->draw(backBuffer);
				scr->draw();
			}
		}

		rest(1);

		unsigned long now = currentTimeMillis();
		unsigned long duration = MIN(now - start, maxUpdateStep);

		currArea->update(duration);

		start = currentTimeMillis();
	}

	if (input->isPlayerControlled())
	 	player->setInput(input);

	return 0;
}

int CGetObjectDirection(lua_State* stack)
{
	int objNum = (int)lua_tonumber(stack, 1)-1;

	Input* input = currArea->getObject(objNum)->getInput();
	if (input) {
		InputDescriptor* id = input->getDescriptor();
		lua_pushnumber(stack, (int)id->direction+1);
	}
	else {
		lua_pushnumber(stack, (int)DIRECTION_SOUTH+1);
	}

	return 1;
}

int CStopObject(lua_State* stack)
{
	int objNum = (int)lua_tonumber(stack, 1)-1;

	Input* input = currArea->getObject(objNum)->getInput();

	if (input) {
		input->set(false, false, false, false, false, false);
	}

	return 0;
}

int CGetTileIndices(lua_State* stack)
{
	int x = (int)lua_tonumber(stack, 1)-1;
	int y = (int)lua_tonumber(stack, 2)-1;

	Tile* tile = currArea->getTile(x, y);

	for (int i = 0; i < TILE_LAYERS; i++) {
		lua_pushnumber(stack, tile->anims[i]+1);
	}

	return TILE_LAYERS;
}

int CAddEnemyToBattle(lua_State* stack)
{
	const char *name = lua_tostring(stack, 1);
	int x = (int)lua_tonumber(stack, 2);
	int y = (int)lua_tonumber(stack, 3);

	activeBattle->addEnemy(name, x, y, true);

	return 0;
}

/*
 * Pass in OGG filename with extension (no path)
 */
int CStartBattle(lua_State* stack)
{
	if (battlesDisabled)
		return 0;

	const char *id = lua_tostring(stack, 1);
	const char *terrain = lua_tostring(stack, 2);

	playOgg("enter_battle.ogg");

	BITMAP* backBuf = scr->getBackBuffer();
	BITMAP* fxBuf = scr->getFXBuffer();
	blit(backBuf, fxBuf, 0, 0, 0, 0, backBuf->w, backBuf->h);
	drawFocusEffect(false, FOCUS_MAX_SIZE);

	activeBattle = new Battle(id, terrain);
	activeBattle->start();
	delete activeBattle;
	activeBattle = NULL;

	if (!playersDead) {
		if (stats.hp <= 0) {
			stats.hp = 1;
		}
		if (partner >= 0 && partnerStats.hp <= 0) {
			partnerStats.hp = 1;
		}
	}
	else {
		if (playerScripted) {
			realSetObjectInputToPlayerControlled(0);
			playerScripted = false;
		}
	}

	return 0;
}

int CGetCombatantHP(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int hp = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		hp = modStats->hp;
	}

	lua_pushnumber(stack, hp);

	return 1;
}

int CSetCombatantHP(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int hp = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->hp = hp;
	}

	return 0;
}

int CGetCombatantMaxHP(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int maxHP = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		maxHP = modStats->maxHP;
	}

	lua_pushnumber(stack, maxHP);

	return 1;
}

int CSetCombatantMaxHP(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int maxHP = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->maxHP = maxHP;
	}

	return 0;
}

int CGetCombatantLatency(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int latency = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		latency = modStats->latency;
	}

	lua_pushnumber(stack, latency);

	return 1;
}

int CSetCombatantLatency(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int latency = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->latency = latency;
	}

	return 0;
}

int CGetCombatantExperience(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int experience = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		experience = modStats->experience;
	}

	lua_pushnumber(stack, experience);

	return 1;
}

int CSetCombatantExperience(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int experience = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->experience = experience;
	}

	return 0;
}

int CGetCombatantMana(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int mana = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		mana = modStats->mana;
	}

	lua_pushnumber(stack, mana);

	return 1;
}

int CSetCombatantMana(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int mana = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->mana = mana;
	}

	return 0;
}

int CGetCombatantMaxMana(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	int maxMana = -1;

	if (c) {
		BattleStats* modStats = c->getModifiedStats();
		maxMana = modStats->maxMana;
	}

	lua_pushnumber(stack, maxMana);

	return 1;
}

int CSetCombatantMaxMana(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int maxMana = (int)lua_tonumber(stack, 2);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		baseStats->maxMana = maxMana;
	}

	return 0;
}

int CGetNumberOfPlayerCombatants(lua_State* stack)
{
	int n = activeBattle->getNumberOfPlayerCombatants();

	lua_pushnumber(stack, n);

	return 1;
}

int CGetPlayerCombatants(lua_State* stack)
{
	int n = activeBattle->getNumberOfPlayerCombatants();
	std::vector<int>* players = activeBattle->getPlayerCombatants();

	if (n == 2) {
		lua_pushnumber(stack, (*players)[0]+1);
		lua_pushnumber(stack, (*players)[1]+1);
		return 2;
	}
	else if (n == 1) {
		lua_pushnumber(stack, (*players)[0]+1);
		return 1;
	}
	else {
		return 0;
	}
}

int CCombatantIsDead(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	if (activeBattle->combatantIsDead(n-1)) {
		lua_pushboolean(stack, 1);
	}
	else {
		lua_pushboolean(stack, 0);
	}

	return 1;
}

int CShowCombatantStatusDetails(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		c->setShowStatusDetails(true);
	}

	return 0;
}

int CHideCombatantStatusDetails(lua_State* stack)
{
	int n = (int)lua_tonumber(stack, 1);

	Combatant* c = activeBattle->getCombatant(n-1);

	if (c) {
		c->setShowStatusDetails(false);
	}

	return 0;
}

int CStartMusic(lua_State* stack)
{
	const char* name = lua_tostring(stack, 1);

	startMusic(name);

	return 0;
}

int CStopMusic(lua_State* stack)
{
	stopMusic();

	return 0;
}

int CPushMusic(lua_State* stack)
{
	pushMusic();

	return 0;
}

int CGetMusicVolume(lua_State* stack)
{
    lua_pushnumber(stack, config.getMusicVolume());
    return 1;
}

int CSetMusicVolume(lua_State* stack)
{
    int volume = (int)lua_tonumber(stack, 1);

    config.setMusicVolume(volume);
    restartMusic();

    return 0;
}

int CRevitalizePlayers(lua_State* stack)
{
    stats.hp = stats.maxHP;
    stats.mana = stats.maxMana;

    if (partner >= 0) {
	partnerStats.hp = partnerStats.maxHP;
	partnerStats.mana = partnerStats.maxMana;
    }

    return 0;
}

int CRedrawArea(lua_State* stack)
{
    currArea->draw(scr->getBackBuffer());

    return 0;
}

int CGetTime(lua_State* stack)
{
    lua_pushnumber(stack, currentTimeMillis());

    return 1;
}

int CStopSound(lua_State* stack)
{
    int num = (int)lua_tonumber(stack, 1);

    voice_stop(num);

    return 0;
}

int CSetPartner(lua_State* stack)
{
	int p = (int)lua_tonumber(stack, 1);

	partner = p;

	memcpy(&partnerStats, &initialPartnerStats[p], sizeof(PlayerStats));

	return 0;
}

int CGetPartner(lua_State* stack)
{
	lua_pushnumber(stack, partner+1);
	return 1;
}

int CGetAccumulatedDistance(lua_State *stack)
{
	lua_pushnumber(stack, accumulated_distance);
	return 1;
}

int CSetAccumulatedDistance(lua_State *stack)
{
	accumulated_distance = (int)lua_tonumber(stack, -1);
	return 0;
}

void setSpellMilestone(char *spellName)
{
	lua_State* stack = currArea->getLuaState();
	callLua(stack, "set_spell_milestone", "s>", spellName);
}

int CBurnStuff(lua_State *stack)
{
	int num_hotspots = (int)lua_tonumber(stack, 1);
	std::vector<SpellTarget*> targets;

	for (int i = 0; i < num_hotspots; i++) {
		int x = (int)lua_tonumber(stack, 2+(i*2));
		int y = (int)lua_tonumber(stack, 3+(i*2));
		SpellTarget *st = new SpellTarget(x, y, 1, 1, false);
		targets.push_back(st);
	}

	Spell* fire = findSpell("Fire");
	fire->init(targets);

	long start = currentTimeMillis();

	for (;;) {
		currArea->draw(scr->getBackBuffer());
		fire->draw(scr->getBackBuffer());
		scr->draw();
		int duration = currentTimeMillis() - start;
		start = currentTimeMillis();
		if (fire->update(duration))
			break;
		rest(5);
	}

	for (int i = 0; i < targets.size(); i++) {
		delete targets[i];
	}
	targets.clear();

	return 0;
}

int CRest(lua_State *stack)
{
	int millis = (int)lua_tonumber(stack, 1);
	rest(millis);
	return 0;
}

int CRedrawScreen(lua_State *stack)
{
	scr->draw();
	return 0;
}

int CGetNumberOfEnemyCombatants(lua_State* stack)
{
	int players = activeBattle->getNumberOfPlayerCombatants();
	std::list<Combatant*>& combatants = activeBattle->getCombatants();

	lua_pushnumber(stack, (combatants.size()-players));

	return 1;
}

int CSetPlayersDead(lua_State *stack)
{
	bool dead = (bool)lua_toboolean(stack, 1);
	playersDead = dead;
	return 0;
}

int CShowEnding(lua_State *stack)
{
	DATAFILE *dat = load_datafile_object(getResource("the_end.dat"), "THE_END_PCX");
	BITMAP *the_end = (BITMAP *)dat->dat;

	scr->fadeOut();
	rest(500);
	BITMAP *bb = scr->getBackBuffer();
	clear(bb);
	blit(the_end, bb, 0, 0, (bb->w-the_end->w)/2, (bb->h-the_end->h)/2,
		the_end->w, the_end->h);
	scr->fadeIn();
	rest(5000);

	unload_datafile_object(dat);
	
	return 0;
}

int CGetRandomNumber(lua_State* stack)
{
	int max = (int)lua_tonumber(stack, 1);

	int r = (rand() % max) + 1;
	lua_pushnumber(stack, r);

	return 1;
}

/*
 * This registers all the required C/C++ functions
 * with the Lua interpreter, so they can be called
 * from scripts.
 */
void registerCFunctions(lua_State* luaState)
{
	/* Area related functions */
	lua_pushcfunction(luaState, CStartArea);
	lua_setglobal(luaState, "startArea");
	
	lua_pushcfunction(luaState, CFadeIn);
	lua_setglobal(luaState, "fadeIn");
	
	lua_pushcfunction(luaState, CFadeOut);
	lua_setglobal(luaState, "fadeOut");

	lua_pushcfunction(luaState, CDoDialogue);
	lua_setglobal(luaState, "doDialogue");

	lua_pushcfunction(luaState, CDoMessage);
	lua_setglobal(luaState, "doMessage");

	lua_pushcfunction(luaState, CDoPickupMessage);
	lua_setglobal(luaState, "doPickupMessage");

	lua_pushcfunction(luaState, CGetPreviousAreaName);
	lua_setglobal(luaState, "getPreviousAreaName");

	lua_pushcfunction(luaState, CTileIsSolid);
	lua_setglobal(luaState, "tileIsSolid");

	lua_pushcfunction(luaState, CRedrawArea);
	lua_setglobal(luaState, "redrawArea");

	/* Object related functions */
	lua_pushcfunction(luaState, CSetObjectPosition);
	lua_setglobal(luaState, "setObjectPosition");
	
	lua_pushcfunction(luaState, CSetObjectDirection);
	lua_setglobal(luaState, "setObjectDirection");

	lua_pushcfunction(luaState, CAddObject);
	lua_setglobal(luaState, "addObject");
	
	lua_pushcfunction(luaState, CSetObjectAnimationSet);
	lua_setglobal(luaState, "setObjectAnimationSet");
	
	lua_pushcfunction(luaState, CSetObjectInputToScriptControlled);
	lua_setglobal(luaState, "setObjectInputToScriptControlled");

	lua_pushcfunction(luaState, CSetObjectInputToPlayerControlled);
	lua_setglobal(luaState, "setObjectInputToPlayerControlled");

	lua_pushcfunction(luaState, CMoveObject);
	lua_setglobal(luaState, "moveObject");

	lua_pushcfunction(luaState, CGetObjectPosition);
	lua_setglobal(luaState, "getObjectPosition");

	lua_pushcfunction(luaState, CMakeCharacter);
	lua_setglobal(luaState, "makeCharacter");

	lua_pushcfunction(luaState, CCheckMilestone);
	lua_setglobal(luaState, "checkMilestone");
	
	lua_pushcfunction(luaState, CSetMilestone);
	lua_setglobal(luaState, "setMilestone");
	
	lua_pushcfunction(luaState, CSetObjectSolid);
	lua_setglobal(luaState, "setObjectSolid");
	
	lua_pushcfunction(luaState, CObjectIsSolid);
	lua_setglobal(luaState, "objectIsSolid");
	
	lua_pushcfunction(luaState, CSetObjectHigh);
	lua_setglobal(luaState, "setObjectHigh");
	
	lua_pushcfunction(luaState, CObjectIsHigh);
	lua_setglobal(luaState, "objectIsHigh");
	
	lua_pushcfunction(luaState, CAdvanceObjectAnimation);
	lua_setglobal(luaState, "advanceObjectAnimation");
	
	lua_pushcfunction(luaState, CSetObjectAnimated);
	lua_setglobal(luaState, "setObjectAnimated");

	lua_pushcfunction(luaState, CObjectIsColliding);
	lua_setglobal(luaState, "objectIsColliding");

	lua_pushcfunction(luaState, CObjectsAreColliding);
	lua_setglobal(luaState, "objectsAreColliding");

	lua_pushcfunction(luaState, CRemoveObject);
	lua_setglobal(luaState, "removeObject");

	lua_pushcfunction(luaState, CGetObjectDirection);
	lua_setglobal(luaState, "getObjectDirection");

	lua_pushcfunction(luaState, CStopObject);
	lua_setglobal(luaState, "stopObject");

	lua_pushcfunction(luaState, CGetTileIndices);
	lua_setglobal(luaState, "getTileIndices");

	/* Miscellaneos */
	lua_pushcfunction(luaState, CPlaySound);
	lua_setglobal(luaState, "playSound");

	lua_pushcfunction(luaState, CGetStart);
	lua_setglobal(luaState, "getStart");

	lua_pushcfunction(luaState, CFirstInventorySlotAvailable);
	lua_setglobal(luaState, "findFirstEmptyInventorySlot");

	lua_pushcfunction(luaState, CSetInventory);
	lua_setglobal(luaState, "setInventory");

	lua_pushcfunction(luaState, CFindInventoryItem);
	lua_setglobal(luaState, "findInventoryItem");

	lua_pushcfunction(luaState, CAddToInventory);
	lua_setglobal(luaState, "addToInventory");

	lua_pushcfunction(luaState, CGiveGold);
	lua_setglobal(luaState, "giveGold");

	lua_pushcfunction(luaState, CTakeGold);
	lua_setglobal(luaState, "takeGold");

	lua_pushcfunction(luaState, CCheckGold);
	lua_setglobal(luaState, "checkGold");

	lua_pushcfunction(luaState, CSetObjectAnimationFrame);
	lua_setglobal(luaState, "setObjectAnimationFrame");
	
	lua_pushcfunction(luaState, CSetObjectSubAnimation);
	lua_setglobal(luaState, "setObjectSubAnimation");

	lua_pushcfunction(luaState, CSetObjectSubAnimationNamed);
	lua_setglobal(luaState, "setObjectSubAnimationNamed");

	lua_pushcfunction(luaState, CSetObjectHidden);
	lua_setglobal(luaState, "setObjectHidden");

	lua_pushcfunction(luaState, CGetHP);
	lua_setglobal(luaState, "getHP");

	lua_pushcfunction(luaState, CGetMaxHP);
	lua_setglobal(luaState, "getMaxHP");

	lua_pushcfunction(luaState, CSetMaxHP);
	lua_setglobal(luaState, "setMaxHP");

	lua_pushcfunction(luaState, CSetHP);
	lua_setglobal(luaState, "setHP");

	lua_pushcfunction(luaState, CAddToHP);
	lua_setglobal(luaState, "addToHP");
	
	lua_pushcfunction(luaState, CDoShop);
	lua_setglobal(luaState, "doShop");

	lua_pushcfunction(luaState, CDoQuestion);
	lua_setglobal(luaState, "doQuestion");

	lua_pushcfunction(luaState, CUpdateUntilArrival);
	lua_setglobal(luaState, "updateUntilArrival");

	lua_pushcfunction(luaState, CStartMusic);
	lua_setglobal(luaState, "startMusic");

	lua_pushcfunction(luaState, CStopMusic);
	lua_setglobal(luaState, "stopMusic");

	lua_pushcfunction(luaState, CPushMusic);
	lua_setglobal(luaState, "pushMusic");
	
	lua_pushcfunction(luaState, CSetObjectTotalHeight);
	lua_setglobal(luaState, "setObjectTotalHeight");
	
	lua_pushcfunction(luaState, CGetMusicVolume);
	lua_setglobal(luaState, "getMusicVolume");
	
	lua_pushcfunction(luaState, CSetMusicVolume);
	lua_setglobal(luaState, "setMusicVolume");

	lua_pushcfunction(luaState, CRevitalizePlayers);
	lua_setglobal(luaState, "revitalizePlayers");

	lua_pushcfunction(luaState, CGetTime);
	lua_setglobal(luaState, "getTime");

	lua_pushcfunction(luaState, CStopSound);
	lua_setglobal(luaState, "stopSound");

	lua_pushcfunction(luaState, CSetPartner);
	lua_setglobal(luaState, "setPartner");

	lua_pushcfunction(luaState, CGetPartner);
	lua_setglobal(luaState, "getPartner");

	lua_pushcfunction(luaState, CGetAccumulatedDistance);
	lua_setglobal(luaState, "getAccumulatedDistance");

	lua_pushcfunction(luaState, CSetAccumulatedDistance);
	lua_setglobal(luaState, "setAccumulatedDistance");

	lua_pushcfunction(luaState, CBurnStuff);
	lua_setglobal(luaState, "burnStuff");

	lua_pushcfunction(luaState, CRest);
	lua_setglobal(luaState, "rest");

	lua_pushcfunction(luaState, CRedrawScreen);
	lua_setglobal(luaState, "redrawScreen");

	lua_pushcfunction(luaState, CSetPlayersDead);
	lua_setglobal(luaState, "setPlayersDead");

	lua_pushcfunction(luaState, CShowEnding);
	lua_setglobal(luaState, "showEnding");

	lua_pushcfunction(luaState, CGetRandomNumber);
	lua_setglobal(luaState, "getRandomNumber");

	/* Battle stuff */
	
	lua_pushcfunction(luaState, CAddEnemyToBattle);
	lua_setglobal(luaState, "addEnemyToBattle");

	lua_pushcfunction(luaState, CStartBattle);
	lua_setglobal(luaState, "startBattle");

	lua_pushcfunction(luaState, CGetCombatantHP);
	lua_setglobal(luaState, "getCombatantHP");

	lua_pushcfunction(luaState, CSetCombatantHP);
	lua_setglobal(luaState, "setCombatantHP");

	lua_pushcfunction(luaState, CGetCombatantMaxHP);
	lua_setglobal(luaState, "getCombatantMaxHP");

	lua_pushcfunction(luaState, CSetCombatantMaxHP);
	lua_setglobal(luaState, "setCombatantMaxHP");

	lua_pushcfunction(luaState, CGetCombatantLatency);
	lua_setglobal(luaState, "getCombatantLatency");

	lua_pushcfunction(luaState, CSetCombatantLatency);
	lua_setglobal(luaState, "setCombatantLatency");

	lua_pushcfunction(luaState, CGetCombatantExperience);
	lua_setglobal(luaState, "getCombatantExperience");

	lua_pushcfunction(luaState, CSetCombatantExperience);
	lua_setglobal(luaState, "setCombatantExperience");

	lua_pushcfunction(luaState, CGetCombatantMana);
	lua_setglobal(luaState, "getCombatantMana");

	lua_pushcfunction(luaState, CSetCombatantMana);
	lua_setglobal(luaState, "setCombatantMana");

	lua_pushcfunction(luaState, CGetCombatantMaxMana);
	lua_setglobal(luaState, "getCombatantMaxMana");

	lua_pushcfunction(luaState, CSetCombatantMaxMana);
	lua_setglobal(luaState, "setCombatantMaxMana");

	lua_pushcfunction(luaState, CGetNumberOfPlayerCombatants);
	lua_setglobal(luaState, "getNumberOfPlayerCombatants");

	lua_pushcfunction(luaState, CGetPlayerCombatants);
	lua_setglobal(luaState, "getPlayerCombatants");

	lua_pushcfunction(luaState, CCombatantIsDead);
	lua_setglobal(luaState, "combatantIsDead");

	lua_pushcfunction(luaState, CShowCombatantStatusDetails);
	lua_setglobal(luaState, "showCombatantStatusDetails");

	lua_pushcfunction(luaState, CHideCombatantStatusDetails);
	lua_setglobal(luaState, "hideCombatantStatusDetails");

	lua_pushcfunction(luaState, CGetNumberOfEnemyCombatants);
	lua_setglobal(luaState, "getNumberOfEnemyCombatants");
}
