#include "monster.h"

extern "C" {
#include <logg.h>
}

#include <string>

static BITMAP* heartIcon;
static BITMAP* clockIcon;
static BITMAP* manaIcon;

Battle* activeBattle;

int Battle::FRAME_COLOR;

static int ATTACK_EFFECT_DURATION;
static int ATTACK_FLASH_DURATION;
const int ATTACK_EFFECT_LINE_LENGTH = 20;
const int ATTACK_EFFECT_NUM_LINES = 2;
const int ATTACK_EFFECT_SEPARATION = 10;
static long lastAttackEffectUpdate;
static int attackEffectCount;

const int MAX_COMBATANTS = 10;

static char result[MAX_COMBATANTS][20];
static Point resultPosition[MAX_COMBATANTS];
static int numResults;
static int resultCount;
static bool isDamage[MAX_COMBATANTS];
static float resultAlpha;

static std::vector<int> deathList;
static float deathAlpha;
static long lastDeathUpdate;

static bool inSummary = false;
static std::vector<char*> summaryLines;
static std::vector<MWgtSpeech*> summaryWidgets;
static MWgtFrame* summaryFrame;
static MWgtAdvancer* advancer;

static bool runSuccess;

static long lastItemEffectUpdate = 0;

bool impatientMode = false;

static void createWidgetsFromSummaryLines()
{
	summaryFrame = new MWgtFrame(0, 0, Screen::BUFFER_WIDTH-50, 55,
			Battle::FRAME_COLOR, 200);

	for (unsigned int i = 0; i < summaryLines.size(); i++) {
		MWgtSpeech* s = new MWgtSpeech(10, 10, Screen::BUFFER_WIDTH-70,
				summaryLines[i]);
		while (!s->atStop())
			s->advance();
		summaryWidgets.push_back(s);
	}

	tguiSetParent(0);
	tguiAddWidget(summaryFrame);
	tguiSetParent(summaryFrame);
	tguiAddWidget(summaryWidgets[0]);
	advancer = new MWgtAdvancer(Screen::BUFFER_WIDTH-70, 50-15, summaryWidgets[0]);
	tguiAddWidget(advancer);
	tguiTranslateWidget(summaryFrame, 25, 10);
	tguiSetFocus(advancer);
}

void Battle::summarizeWin()
{
	callLua(luaState, "player_win", ">");

	callLua(luaState, "get_item", ">ii");
	int quantity = (int)lua_tonumber(luaState, -2);
	int number = (int)lua_tonumber(luaState, -1)-1;
	lua_pop(luaState, 2);
	if (quantity > 0) {
		char* line = new char[100];
		sprintf(line, "You found a %s!$", getItemName(number));
		summaryLines.push_back(line);
		int slot = findInventoryItem(number);
		if (slot < 0) {
			slot = firstInventorySlotAvailable();
			if (slot < 0) {
				char* line = new char[100];
				sprintf(line, "Your inventory is full, item dropped...");
				summaryLines.push_back(line);
			}
			else {
				stats.inventory[slot].id = number;
				stats.inventory[slot].quantity = 1;
			}
		}
		else {
			stats.inventory[slot].quantity++;
			if (stats.inventory[slot].quantity > 99)
				stats.inventory[slot].quantity = 99;
		}
	}


	callLua(luaState, "get_gold", ">i");
	int gold = (int)lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	if (gold > 0) {
		char *line = new char[100];
		sprintf(line, "You found %d gold!$", gold);
		summaryLines.push_back(line);
	}
	
	callLua(luaState, "get_experience", ">i");
	int experience = (int)lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);
	if (experience > 0) {
		char* line = new char[100];
		sprintf(line, "You gained %d experience!$", experience);
		summaryLines.push_back(line);
	}

	int currLevel = getLevel(stats.experience);
	int newLevel = getLevel(stats.experience+experience);

	Combatant* c = getCombatant(0);
	BattleStats* s = c->getModifiedStats();

	int extraHP = stats.maxHP - getMaxHP(getLevel(stats.experience));

	stats.gold += gold;
	stats.hp = s->hp;
	stats.mana = s->mana;
	stats.experience += experience;
	if (stats.experience > ExperienceForLevel[NUM_LEVELS-1]) {
		stats.experience = ExperienceForLevel[NUM_LEVELS-1];
	}

	if (newLevel > currLevel) {
		char* line = new char[100];
		sprintf(line, "%s advanced a level!$", playerName);
		summaryLines.push_back(line);
		stats.maxHP = getMaxHP(getLevel(stats.experience)) + extraHP;
		stats.hp = stats.maxHP;
		stats.mana = getMaxMana(getLevel(stats.experience));
		stats.maxMana = stats.mana;
	}

	if (partner >= 0) {
		c = getCombatant(1);
		s = c->getModifiedStats();

		int currLevel = getLevel(partnerStats.experience);
		int newLevel = getLevel(partnerStats.experience+experience);

		partnerStats.hp = s->hp;
		partnerStats.mana = s->mana;
		partnerStats.experience += experience;
		if (partnerStats.experience > ExperienceForLevel[NUM_LEVELS-1]) {
			partnerStats.experience = ExperienceForLevel[NUM_LEVELS-1];
		}

		if (newLevel > currLevel) {
			char* line = new char[100];
			sprintf(line, "%s advanced a level!$", partnerNames[partner]);
			summaryLines.push_back(line);
			partnerStats.maxHP = getMaxHP(getLevel(partnerStats.experience));
			partnerStats.hp = partnerStats.maxHP;
			partnerStats.mana = getMaxMana(getLevel(partnerStats.experience));
			partnerStats.maxMana = getMaxMana(getLevel(partnerStats.experience));
		}
	}

	if (summaryLines.size() <= 0) {
		char *line = new char[100];
		sprintf(line, "You were victorious!$");
		summaryLines.push_back(line);
	}

	createWidgetsFromSummaryLines();


	inSummary = true;
	summaryEndsBattle = true;

	clear_keybuf();
}

void Battle::checkForDeadPlayers()
{
	if (players.size() == 2) {
		Combatant* c1 = getCombatant(0);
		Combatant* c2 = getCombatant(1);
		BattleStats* s1 = c1->getModifiedStats();
		BattleStats* s2 = c2->getModifiedStats();
		if (s1->hp <= 0 && s2->hp <= 0) {
			playersDead = true;
		}
	}
	else {
		Combatant* c1 = getCombatant(0);
		BattleStats* s1 = c1->getModifiedStats();
		if (s1->hp <= 0) {
			playersDead = true;
		}
	}

	if (playersDead) {
		callLua(luaState, "player_loss", ">");

		char* line = new char[100];
		sprintf(line, "You were defeated in battle...$");
		summaryLines.push_back(line);
	
		summaryFrame = new MWgtFrame(0, 0, Screen::BUFFER_WIDTH-50, 55,
			Battle::FRAME_COLOR, 200);

		for (unsigned int i = 0; i < summaryLines.size(); i++) {
			MWgtSpeech* s = new MWgtSpeech(10, 10, Screen::BUFFER_WIDTH-70,
					summaryLines[i]);
			while (!s->atStop())
				s->advance();
			summaryWidgets.push_back(s);
		}

		tguiSetParent(0);
		tguiAddWidget(summaryFrame);
		tguiSetParent(summaryFrame);
		tguiAddWidget(summaryWidgets[0]);
		advancer = new MWgtAdvancer(Screen::BUFFER_WIDTH-70, 50-15, summaryWidgets[0]);
		tguiAddWidget(advancer);
		tguiTranslateWidget(summaryFrame, 25, 10);
		tguiSetFocus(advancer);

		inSummary = true;
		summaryEndsBattle = true;

		clear_keybuf();
	}
}

/*
 * Returns true if player runs successfully.
 * First checks if experience between current and next level
 * is 100x the experience gained from battle. If so, run is
 * always successful. Then checks if a random number is less
 * than the percentage from the battle script, and decides
 * success based on that.
 */
bool Battle::checkForRunSuccess()
{
	callLua(luaState, "get_run_success_percentage", ">i");
	int successPercentage = (int)lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	if (successPercentage == 0) {
		return false;
	}

	int currLevel = getLevel(stats.experience);
	int currLevelExp = ExperienceForLevel[currLevel];
	int nextLevelExp;
	if (currLevel == NUM_LEVELS-1) {
		nextLevelExp = currLevelExp*2;
	}
	else {
		nextLevelExp = ExperienceForLevel[currLevel+1];
	}

	int battleExp;

	callLua(luaState, "get_experience", ">i");

	battleExp = (int)lua_tonumber(luaState, -1);
	lua_pop(luaState, 1);

	if (battleExp*1000 < nextLevelExp-currLevelExp) {
		return true;
	}

	int i = rand() % 100;

	if (i < successPercentage) {
		return true;
	}

	return false;
}

void Battle::init()
{
	numPlayers = 1;

	if (partner >= 0) {
		if (stats.hp > 0 && partnerStats.hp > 0) {
			Combatant* p1 = new PlayerCombatant(playerName, false, BATTLE_TWO_PLAYERS_PLAYER_ONE_X, BATTLE_TWO_PLAYERS_PLAYER_ONE_Y);
			Combatant* p2 = new PlayerCombatant(partnerNames[partner], true, BATTLE_TWO_PLAYERS_PLAYER_TWO_X, BATTLE_TWO_PLAYERS_PLAYER_TWO_Y);
			p1->setNumber(0);
			p2->setNumber(1);
			players.push_back(0);
			players.push_back(1);
			combatants.push_back(p1);
			combatants.push_back(p2);
			numPlayers = 2;
		}
		else if (stats.hp > 0) {
			Combatant* p = new PlayerCombatant(playerName, false, BATTLE_ONE_PLAYER_X, BATTLE_ONE_PLAYER_Y);
			p->setNumber(0);
			players.push_back(0);
			combatants.push_back(p);
		}
		else {
			Combatant* p = new PlayerCombatant(partnerNames[partner], true, BATTLE_ONE_PLAYER_X, BATTLE_ONE_PLAYER_Y);
			p->setNumber(0);
			players.push_back(0);
			combatants.push_back(p);
		}
	}
	else {
		Combatant* p = new PlayerCombatant(playerName, false, BATTLE_ONE_PLAYER_X, BATTLE_ONE_PLAYER_Y);
		p->setNumber(0);
		players.push_back(0);
		combatants.push_back(p);
	}

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

	debug_message("Loading global battle script...\n");
	if (luaL_loadfile(luaState, getResource("battle_scripts/global.%s", getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running global battle script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Loading battle script...\n");
	if (luaL_loadfile(luaState, getResource("battle_scripts/%s.%s", id, getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running battle script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}
	
	debug_message("Calling 'start'...\n");

	callLua(luaState, "start", ">");
}

/*
void Battle::lock()
{
	locked = true;
}

void Battle::unlock()
{
	locked = false;
}
*/

void Battle::drawSummary()
{
	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH, 70);
	tguiClearDirtyRectangle();
}

void Battle::drawPlayerStatus(PlayerCombatant* c)
{
	int x = Screen::BUFFER_WIDTH/2+7;
	int y;

	if (!strcmp(c->getName(), playerName)) {
		y = Screen::BUFFER_HEIGHT-STATUS_H+7;
	}
	else {
		y = Screen::BUFFER_HEIGHT-STATUS_H/2+7;
	}

	int color = c->getNumber() == currentCombatant ? makecol(255, 255, 0) : makecol(255, 255, 255);

	mPrintf(backBuffer, x, y, color, FRAME_COLOR, false,
		true, c->getName());

	BattleStats* modStats = c->getModifiedStats();

	if (c->showStatusDetails()) {
		draw_sprite(backBuffer, clockIcon,
				x+Screen::BUFFER_WIDTH/4,
				y);
		draw_sprite(backBuffer, heartIcon,
				x+Screen::BUFFER_WIDTH/4,
				y+8);
		draw_sprite(backBuffer, manaIcon,
				x+Screen::BUFFER_WIDTH/4,
				y+16);
		drawMeter(backBuffer, x+10+Screen::BUFFER_WIDTH/4,
			y+1, METER_WIDTH, 6, (float)modStats->turnReadyCount / 100.0f);
		drawMeter(backBuffer, x+10+Screen::BUFFER_WIDTH/4,
			y+9, METER_WIDTH, 6, (float)modStats->hp / modStats->maxHP);
		if (c->isPartner() && !partnerHasMana[partner]) {
			drawMeter(backBuffer, x+10+Screen::BUFFER_WIDTH/4,
				y+17, METER_WIDTH, 6, 0);
		}
		else {
			drawMeter(backBuffer, x+10+Screen::BUFFER_WIDTH/4,
				y+17, METER_WIDTH, 6, (float)modStats->mana / modStats->maxMana);
		}
	}
	else {
		draw_sprite(backBuffer, clockIcon,
				x+Screen::BUFFER_WIDTH/4,
				y);
		drawMeter(backBuffer, x+Screen::BUFFER_WIDTH/4,
			y+1, METER_WIDTH, 6, (float)modStats->turnReadyCount / 100.0f);
	}
}

void Battle::drawStatus()
{
	drawFrame(backBuffer, 0, Screen::BUFFER_HEIGHT-STATUS_H,
		Screen::BUFFER_WIDTH/2, STATUS_H,
		FRAME_COLOR, 255);
	drawFrame(backBuffer, Screen::BUFFER_WIDTH/2, Screen::BUFFER_HEIGHT-STATUS_H,
		Screen::BUFFER_WIDTH/2, STATUS_H/2,
		FRAME_COLOR, 255);
	drawFrame(backBuffer, Screen::BUFFER_WIDTH/2, Screen::BUFFER_HEIGHT-STATUS_H/2,
		Screen::BUFFER_WIDTH/2, STATUS_H/2,
		FRAME_COLOR, 255);

	/* Draw enemy status */

	std::list<Combatant*>::iterator it = combatants.begin();

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->isPlayer() == true) {
			drawPlayerStatus(dynamic_cast<PlayerCombatant *>(c));
			it++;
			continue;
		}
		int y = (Screen::BUFFER_HEIGHT - STATUS_H) + ((c->getNumber() - numPlayers) * 18 + 7);
		int color = c->getNumber() == currentCombatant ? makecol(255, 255, 0) : makecol(255, 255, 255);
		mPrintf(backBuffer, 7, y, color, FRAME_COLOR, false,
			true, c->getName());
		BattleStats* modStats = c->getModifiedStats();
		if (c->showStatusDetails()) {
			draw_sprite(backBuffer, clockIcon,
					Screen::BUFFER_WIDTH/4,
					y);
			draw_sprite(backBuffer, heartIcon,
					Screen::BUFFER_WIDTH/4,
					y+8);
			drawMeter(backBuffer, Screen::BUFFER_WIDTH/4+10,
				y+1, METER_WIDTH, 6, (float)modStats->turnReadyCount / 100.0f);
			drawMeter(backBuffer, Screen::BUFFER_WIDTH/4+10,
				y+9, METER_WIDTH, 6, (float)modStats->hp / modStats->maxHP);
		}
		else {
			draw_sprite(backBuffer, clockIcon,
					Screen::BUFFER_WIDTH/4,
					y);
			drawMeter(backBuffer, Screen::BUFFER_WIDTH/4+10,
				y+1, METER_WIDTH, 6, (float)modStats->turnReadyCount / 100.0f);
		}
		it++;
	}

	if (inSummary) {
		drawSummary();
	}
}

void Battle::drawPlayerInAction(Combatant* c)
{
	static bool started = false;
	static long lastUpdate;
	static int count;
	static bool walkLeftDone;
	static bool motionDone;
	static int xOffset;
	static int walkDelay;
	const int WALK_DISTANCE = 20;
	static bool walkRightStarted = false;
	
	if (started == false) {
		if (currentAction->type == TURN_RUN && runSuccess) {
			if ((partner < 0) || (partner >= 0 && c->getNumber() == 1)) {
				started = true;
			}
			c->setSubAnimation("walk_e");
			lastUpdate = tguiCurrentTimeMillis();
			count = 0;
			walkLeftDone = false;
			xOffset = 0;
			if (config.getBattleSpeed() == 0) {
				walkDelay = 25;
			}
			else {
				walkDelay = 15;
			}
		}
		else if (currentAction->type == TURN_RUN) {
			deathsAnimated = true;
			return;
		}
		else {
			started = true;
			c->setSubAnimation("walk_w");
			lastUpdate = tguiCurrentTimeMillis();
			count = 0;
			walkLeftDone = false;
			motionDone = false;
			xOffset = 0;
			if (config.getBattleSpeed() == 0) {
				walkDelay = 25;
			}
			else {
				walkDelay = 15;
			}
		}
	}

	long now = tguiCurrentTimeMillis();
	if (walkRightStarted == true) {
		lastUpdate = now;
		walkRightStarted = false;
	}
	int duration = now - lastUpdate;
	lastUpdate = now;
	count += duration;

	Animation* a = c->getCurrentAnimation();

	if (currentAction->type == TURN_RUN && runSuccess) {
		a->update(duration);
		while (count >= walkDelay) {
			count -= walkDelay;
			xOffset++;
			if (xOffset > Screen::BUFFER_WIDTH-BATTLE_ONE_PLAYER_X) {
				walkLeftDone = true;
				a->reset();
				count = 0;
				started = false;
				deathsAnimated = true;
				char* line = new char[100];
				strcpy(line, "You escaped from the battle...$");
				summaryLines.push_back(line);
				createWidgetsFromSummaryLines();
				inSummary = true;
				summaryEndsBattle = true;
				return;
			}
		}
	}
	else {
		if (walkLeftDone && motionDone) {
			a->update(duration);
			while (count >= walkDelay) {
				count -= walkDelay;
				xOffset++;
				if (xOffset >= 0) {
					started = false;
					turnAnimated = true;
					initAction();
					c->setSubAnimation("stand_w");
					return;
				}
			}
		}
		else if (walkLeftDone) {
			if (a->update(duration)) {
				motionDone = true;
				c->setSubAnimation("walk_e");
				walkRightStarted = true;
				count = 0;
			}
		}
		else {
			a->update(duration);
			while (count >= walkDelay) {
				count -= walkDelay;
				xOffset--;
				if (xOffset < -WALK_DISTANCE) {
					walkLeftDone = true;
					c->setSubAnimation("attack");
					a = c->getCurrentAnimation();
					a->reset();
					count = 0;
				}
			}
		}
	}

	a->draw(backBuffer, c->getX()+xOffset, c->getY());
}

void Battle::drawEnemyInAction(Combatant* c)
{
	static bool started = false;
	static int count;
	static int lastUpdate;
	static BITMAP* bmp;
	int DURATION = config.getBattleSpeed() == 0 ? 1000 : 500;
	
	if (started == false) {
		started = true;
		count = 0;
		lastUpdate = tguiCurrentTimeMillis();
		Animation* a = c->getCurrentAnimation();
		BITMAP* currFrame = a->getCurrentBitmap();
		bmp = create_bitmap(currFrame->w, currFrame->h);
		blit(currFrame, bmp, 0, 0, 0, 0, currFrame->w, currFrame->h);
		makeBlackAndWhite(bmp);
		brighten(bmp, 100);
	}

	draw_sprite(backBuffer, bmp, c->getX(), c->getY());

	long now = tguiCurrentTimeMillis();
	count += now - lastUpdate;
	lastUpdate = now;
	if (count >= DURATION) {
		started = false;
		turnAnimated = true;
		initAction();
		destroy_bitmap(bmp);
	}
}

void Battle::drawCombatantInAction(Combatant* c)
{
	_showStatus = true;

	if (c->isPlayer()) {
		if (!inSummary || !runSuccess) {
			drawPlayerInAction(c);
		}
	}
	else {
		drawEnemyInAction(c);
	}
}

void Battle::drawCombatants()
{
	std::list<Combatant*>::iterator it = combatants.begin();

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->getNumber() == currentCombatant && (!currentAction || !c->isPlayer())) {
			int x = c->getX();
			int y = c->getY();
			int cx = x + c->getWidth() / 2;
			int cy = y + c->getShadowOffset();
			drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
			set_trans_blender(0, 0, 0, 128);
			ellipsefill(backBuffer, cx, cy, c->getWidth(), c->getShadowHeight(), makecol(255, 255, 0));
			ellipse(backBuffer, cx, cy, c->getWidth(), c->getShadowHeight(), makecol(0, 0, 0));
			drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
		}
		if ((c->getNumber() == currentCombatant && currentAction != 0 && !turnAnimated)
			|| (currentAction != 0 && c->isPlayer() && runSuccess)) {
			drawCombatantInAction(c);
		}
		else {
			int x = c->getX();
			int y = c->getY();
			Animation* a = c->getCurrentAnimation();
			BattleStats* s = c->getModifiedStats();
			if (!c->isPlayer() && s->hp <= 0) {
				drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);
				set_trans_blender(0, 0, 0, (int)deathAlpha);
				a->drawTrans(backBuffer, x, y);
				drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
			}
			else if (c->isPlayer() && runSuccess) {
				// do nothing
			}
			else {
				a->draw(backBuffer, x, y);
			}
		}
		
		if (turnAnimated && actionAnimated && resultAnimated) {
			drawDeaths();
		}
		else if (turnAnimated && actionAnimated) {
			drawResult();
			if (resultAnimated) {
				initDeaths();
			}
		}
		else if (turnAnimated) {
			drawAction();
			if (actionAnimated) {
				initResult();
			}
			if (resultAnimated) {
				initDeaths();
			}
		}
		it++;
	}

	for (it = combatants.begin(); it != combatants.end(); it++) {
		Combatant* c = *it;
		c->drawSpecial(backBuffer);
	}
}

void Battle::initAttackEffect()
{
	lastAttackEffectUpdate = tguiCurrentTimeMillis();
	attackEffectCount = 0;
	if (config.getBattleSpeed() == 0) {
		ATTACK_EFFECT_DURATION = 1200;
		ATTACK_FLASH_DURATION = 200;
	}
	else {
		ATTACK_EFFECT_DURATION = 600;
		ATTACK_FLASH_DURATION = 100;
	}

	playOgg("attack.ogg");
}

void Battle::drawAttackEffect()
{
	long now = tguiCurrentTimeMillis();
	int step = now - lastAttackEffectUpdate;
	lastAttackEffectUpdate = now;
	attackEffectCount += step;
	if (attackEffectCount >= ATTACK_EFFECT_DURATION) {
		actionAnimated = true;
		return;
	}

	if (attackEffectCount < ATTACK_FLASH_DURATION) {
		brighten(backBuffer, 100);
	}

	Combatant* c = getCombatant(currentAction->target);
	int x = c->getX() + c->getWidth() / 2 - ((ATTACK_EFFECT_NUM_LINES-1) * ATTACK_EFFECT_SEPARATION) / 2;
	int y = c->getY() + c->getHeight() / 2;
	int len = ATTACK_EFFECT_LINE_LENGTH;

	for (int i = 0; i < ATTACK_EFFECT_NUM_LINES; i++) {
		if (getCombatant(currentAction->target)->isPlayer()) {
			line(backBuffer, x-len/2, y-len/2, x+len/2, y+len/2, 0);
		}
		else {
			line(backBuffer, x-len/2, y+len/2, x+len/2, y-len/2, 0);
		}
		x += ATTACK_EFFECT_SEPARATION;
	}
}

void Battle::initItemEffect()
{
	if (!strcmp(getItemName(currentAction->action.index), "Cure") ||
			!strcmp(getItemName(currentAction->action.index), "Cure 2") ||
			!strcmp(getItemName(currentAction->action.index), "Cure 3")) {
		initCureEffect();
	}
	/*
	else if (!strcmp(getItemName(currentAction->action.index), "Heal")) {
		initHealEffect();
	}
	*/
	else if (!strcmp(getItemName(currentAction->action.index), "Revitalize")) {
		initRevitalizeEffect();
	}
	playOgg(getItemSound(currentAction->action.index));
	lastItemEffectUpdate = currentTimeMillis();
}

void Battle::drawItemEffect()
{
	long now = currentTimeMillis();
	int step = now - lastItemEffectUpdate;
	lastItemEffectUpdate = now;

	if (!strcmp(getItemName(currentAction->action.index), "Cure")
		|| !strcmp(getItemName(currentAction->action.index), "Heal")
		|| !strcmp(getItemName(currentAction->action.index), "Cure 2")
		|| !strcmp(getItemName(currentAction->action.index), "Cure 3")) {
		actionAnimated = updateDustEffect(step);
		drawDustEffect(scr->getBackBuffer());
	}
	else if (!strcmp(getItemName(currentAction->action.index), "Revitalize")) {
		actionAnimated = updateFireworkEffect(step);
		drawFireworkEffect(scr->getBackBuffer());
	}
	else {
		actionAnimated = true;
	}
}

void Battle::initSpellEffect()
{
	currentSpell = findSpell(currentAction->action.name);
	if (!currentSpell) {
		actionAnimated = true;
		return;
	}

	std::vector<SpellTarget*> targets;
	Combatant *c;

	if (currentAction->target == -1) {
		std::list<Combatant *>::iterator it;
		for (it = combatants.begin(); it != combatants.end(); it++) {
			c = *it;
			if (c->isPlayer() && c->getModifiedStats()->hp > 0) {
				targets.push_back(new SpellTarget(c->getX() + c->getWidth()/2,
					c->getY() + c->getHeight()/2,
					c->getWidth(),
					c->getHeight(),
					false));
			}
		}
	}
	else if (currentAction->target == -2) {
		std::list<Combatant *>::iterator it;
		for (it = combatants.begin(); it != combatants.end(); it++) {
			c = *it;
			if (!c->isPlayer() && c->getModifiedStats()->hp > 0) {
				targets.push_back(new SpellTarget(c->getX() + c->getWidth()/2,
					c->getY() + c->getHeight()/2,
					c->getWidth(),
					c->getHeight(),
					false));
			}
		}
	}
	else {
		c = getCombatant(currentAction->target);
		if (c->isPlayer()) {
			targets.push_back(new SpellTarget(c->getX() + c->getWidth()/2,
				c->getY() + c->getHeight()/2,
				c->getWidth(),
				c->getHeight(),
				false));
		}
		else {
			targets.push_back(new SpellTarget(c->getX() + c->getWidth()/2,
				c->getY() + c->getHeight()/2,
				c->getWidth(),
				c->getHeight(),
				true));
		}
	}
	
	currentSpell->init(targets);

	for (int i = 0; i < targets.size(); i++) {
		delete targets[i];
	}
	targets.clear();

	lastSpellUpdate = currentTimeMillis();
}

void Battle::drawSpellEffect()
{
		long now = currentTimeMillis();
		int step = now - lastSpellUpdate;
		lastSpellUpdate = now;
		
		actionAnimated = currentSpell->update(step);

		if (!actionAnimated) {
			currentSpell->draw(backBuffer);
		}
}

void Battle::initCureEffect()
{
	int dustColor = makecol(200, 200, 200);
	Combatant* c = getCombatant(currentAction->target);
	int x = c->getX() + (c->getWidth()/2);
	int y = c->getY();
	initDustEffect(x, y, dustColor);
}

/*
void Battle::initHealEffect()
{
	int dustColor = makecol(100, 255, 100);
	Combatant* c = getCombatant(currentAction->target);
	int x = c->getX() + (c->getWidth()/2);
	int y = c->getY();
	initDustEffect(x, y, dustColor);
}
*/

void Battle::initRevitalizeEffect()
{
	int dustColor = makecol(150, 255, 255);
	if (currentAction->target == -1) {
		if (partner >= 0) {
			Combatant* c = getCombatant(0);
			int x = c->getX() + (c->getWidth()/2);
			int y = c->getY();
			initFireworkEffect(x, y, dustColor);
			c = getCombatant(1);
			x = c->getX() + (c->getWidth()/2);
			y = c->getY();
			initFireworkEffect(x, y, dustColor);
		}
		else {
			Combatant* c = getCombatant(0);
			int x = c->getX() + (c->getWidth()/2);
			int y = c->getY();
			initFireworkEffect(x, y, dustColor);
		}
	}
	else {
		std::list<Combatant*>::iterator it;
		for (it = combatants.begin(); it != combatants.end(); it++) {
			Combatant* c = *it;
			if (c->isPlayer()) {
				continue;
			}
			int x = c->getX() + (c->getWidth()/2);
			int y = c->getY();
			initFireworkEffect(x, y, dustColor);
		}
	}
}

void Battle::initAction()
{
	if (currentAction->type == TURN_ATTACK) {
		initAttackEffect();
	}
	else if (currentAction->type == TURN_ITEM) {
		initItemEffect();
	}
	else if (currentAction->type == TURN_SPELL) {
		initSpellEffect();
	}
}

void Battle::drawAction()
{
	if (currentAction->type == TURN_ATTACK) {
		drawAttackEffect();
	}
	else if (currentAction->type == TURN_ITEM) {
		drawItemEffect();
	}
	else if (currentAction->type == TURN_SPELL) {
		drawSpellEffect();
	}
	else {
		actionAnimated = true;
	}
}

void Battle::initResult()
{
	if (currentAction->type == TURN_RUN) {
		resultAnimated = true;
		return;
	}

	deathAlpha = 255.0f;

	bool playerDead;
	bool partnerDead = false;
	playerDead = getCombatant(0)->getModifiedStats()->hp <= 0;
	if (partner >= 0)
		partnerDead = getCombatant(1)->getModifiedStats()->hp <= 0;

	applyTurn();

	if (numResults <= 0) {
		resultAnimated = true;
		return;
	}

	if (currentAction->target >= 0) {
		Combatant* c = getCombatant(currentAction->target);

		if (c->isPlayer()) {
			resultPosition[0].x = c->getX() - text_length(tinyfont, result[0]);
		}
		else {
			resultPosition[0].x = c->getX() + c->getWidth();
		}
		resultPosition[0].y = c->getY();
	}
	else {
		int i = 0;
		if (currentAction->target == -1) {
			if (numResults == 2) {
				Combatant *c;
				if (!playerDead) {
					c = getCombatant(0);
					resultPosition[i].x = c->getX() - text_length(tinyfont, result[i]);
					resultPosition[i].y = c->getY();
					i++;
				}
				if (!partnerDead) {
					c = getCombatant(1);
					resultPosition[i].x = c->getX() - text_length(tinyfont, result[i]);
					resultPosition[i].y = c->getY();
					i++;
				}
			}
			else {
				Combatant* c = getCombatant(0);
				resultPosition[0].x = c->getX() - text_length(tinyfont, result[0]);
				resultPosition[0].y = c->getY();
			}
		}
		else { // -2
			std::list<Combatant*>::iterator it;
			for (it = combatants.begin(); it != combatants.end(); it++) {
				Combatant* c = *it;
				if (c->isPlayer()) {
					continue;
				}
				resultPosition[i].x = c->getX() + c->getWidth();
				resultPosition[i].y = c->getY();
				i++;
			}
		}
	}

	resultAlpha = 255.0f;
	resultCount = 0;
}

void Battle::drawResult()
{
	static bool started = false;
	static long lastUpdate;
	float ALPHA_STEP = config.getBattleSpeed() == 0 ? 0.2f : 0.4f;
	const int RISE_DELAY = config.getBattleSpeed() == 0 ? 100 : 50;

	if (!started) {
		started = true;
		lastUpdate = tguiCurrentTimeMillis();
		resultCount = 0;
	}

	long now = tguiCurrentTimeMillis();
	int step = now - lastUpdate;
	lastUpdate = now;

	resultAlpha -= step * ALPHA_STEP;
	if (resultAlpha < 0) {
		started = false;
		resultAnimated = true;
		return;
	}

	resultCount += step;

	while (resultCount >= RISE_DELAY) {
		for (int i = 0; i < numResults; i++) {
			resultPosition[i].y--;
		}
		resultCount -= RISE_DELAY;
	}

	int color;

	for (int i = 0; i < numResults; i++) {
		if (isDamage[i]) {
			color = makecol(255, 150, 150);
		}
		else {
			color = makecol(50, 255, 50);
		}
		textout_ex(backBuffer, tinyfont, result[i], resultPosition[i].x-1,
				resultPosition[i].y, makecol(0, 0, 0), -1);
		textout_ex(backBuffer, tinyfont, result[i], resultPosition[i].x+1,
				resultPosition[i].y, makecol(0, 0, 0), -1);
		textout_ex(backBuffer, tinyfont, result[i], resultPosition[i].x,
				resultPosition[i].y-1, makecol(0, 0, 0), -1);
		textout_ex(backBuffer, tinyfont, result[i], resultPosition[i].x,
				resultPosition[i].y+1, makecol(0, 0, 0), -1);
		textout_ex(backBuffer, tinyfont, result[i], resultPosition[i].x,
				resultPosition[i].y, color, -1);
	}
}

void Battle::initDeaths()
{
	bool sound_played = false;

	std::list<Combatant*>::iterator it = combatants.begin();

	while (it != combatants.end()) {
		Combatant* c = *it;
		BattleStats* s = c->getModifiedStats();
		if (s->hp <= 0) {
			deathList.push_back(c->getNumber());
			if (c->isPlayer()) {
				c->setSubAnimation("dead");
			}
			else {
				if (!sound_played) {
					playOgg("enemy_death.ogg");
					sound_played = true;
				}
				Animation* a = c->getCurrentAnimation();
				BITMAP* b = a->getCurrentBitmap();
				brighten(b, 50);
				makeRed(b);
			}
		}
		it++;
	}

	debug_message("Death list size=%d\n", deathList.size());

	if (deathList.size() <= 0) {
		deathsAnimated = true;
	}
	else {
		deathAlpha = 255.0f;
		lastDeathUpdate = tguiCurrentTimeMillis();
	}
}

void Battle::drawDeaths()
{
	if (deathsAnimated) {
		return;
	}

	long now = tguiCurrentTimeMillis();
	int step = now - lastDeathUpdate;
	lastDeathUpdate = now;

	const float ALPHA_STEP = config.getBattleSpeed() == 0 ? 0.2f : 0.4f;

	deathAlpha -= ALPHA_STEP * step;
	if (deathAlpha < 0) {
		deathsAnimated = true;
	}
}

void Battle::draw()
{
	blit(background, backBuffer, 0, 0, 0, 0, background->w, background->h);
	drawCombatants();
	if (currentCombatant < 0 || _showStatus) {
		drawStatus();
	}
	scr->draw();
}

void Battle::applyTurn()
{
	debug_message("in apply turn\n");

	Combatant* curr = getCombatant(currentCombatant);
	Combatant* target = getCombatant(currentAction->target);

	int i = 0;

	switch (currentAction->type) {
	case TURN_ATTACK:
		if (rand() % 32 == 0) {
			sprintf(result[0], "MISS!");
			isDamage[0] = false;
		}
		else {
			int power = curr->getPower();
			int armor;
			if (curr->isPlayer()) {
				PlayerCombatant* pc = dynamic_cast<PlayerCombatant*>(curr);
				if (!pc->isPartner()) {
					armor = target->getWeaponDefense(stats.weapon);
				}
				else {
					armor = target->getWeaponDefense(partnerStats.weapon);
				}
			}
			else {
		       		armor = target->getWeaponDefense(-1);
			}
			int damage;
			if (armor >= 99999) {
				damage = 0;
			}
			else {
				damage = getDamage(power, armor);
			}
			BattleStats* s = target->getBaseStats();
			s->hp -= damage;
			sprintf(result[0], "%d", damage);
			isDamage[0] = true;
		}
		numResults = 1;
		break;
	case TURN_ITEM:
		{
		ItemEffects* ie = getItemEffects(currentAction->action.index);
		std::list<Combatant*>::iterator it;
		if (itemAffectsAll(currentAction->action.index)) {
			if (currentAction->target == -1) {
				if (partner >= 0) {
					Combatant* c = getCombatant(0);
					BattleStats* bs = c->getBaseStats();
					if (bs->hp > 0) {
						applyItemEffects(ie, 0, bs);
						getItemResults(ie, result[i]);
						isDamage[i] = false;
						i++;
					}
					c = getCombatant(1);
					bs = c->getBaseStats();
					if (bs->hp > 0) {
						applyItemEffects(ie, 0, bs);
						getItemResults(ie, result[i]);
						isDamage[i] = false;
						i++;
					}
				}
				else {
					Combatant* c = getCombatant(0);
					BattleStats* bs = c->getBaseStats();
					if (bs->hp > 0) {
						applyItemEffects(ie, 0, bs);
						getItemResults(ie, result[0]);
						isDamage[0] = false;
					}
				}
			}
			else {
				std::list<Combatant*>::iterator it;
				for (it = combatants.begin(); it != combatants.end(); it++) {
					Combatant* c = *it;
					if (c->isPlayer()) {
						continue;
					}
					BattleStats* bs = c->getBaseStats();
					if (bs->hp > 0) {
						applyItemEffects(ie, 0, bs);
						getItemResults(ie, result[i]);
						isDamage[i] = false;
						i++;
					}
				}
			}
		}
		else {
			Combatant* c = getCombatant(currentAction->target);
			BattleStats* bs = c->getBaseStats();
			applyItemEffects(ie, 0, bs);
			getItemResults(ie, result[i]);
			isDamage[i] = false;
			i++;
		}
		deleteItemEffects(ie);
		numResults = i;
		}
		break;
	case TURN_SPELL:
		{
		debug_message("applying spell\n");
		if (currentSpell->noResults()) {
			debug_message("spell has no \"results\"");
			numResults = 0;
			bool tmp;
			currentSpell->apply(NULL, NULL, 0, tmp, false);
		}
		else {
			Combatant *c;
			BattleStats *bs;
			int defense;
			bool is_damage;
			if (currentSpell->affectsAll()) {
				debug_message("spell affects all\n");
				bool affects_players = currentAction->target == -1;
				std::list<Combatant *>::iterator it;
				for (it = combatants.begin(); it != combatants.end(); it++) {
					c = *it;
					if (c->isPlayer() != affects_players) {
						continue;
					}
					bs = c->getBaseStats();
					if (bs->hp > 0) {
						defense = c->getSpellDefense(currentSpell->getName());
						if (defense >= 99999) {
							is_damage = true;
							sprintf(result[i], "0");
						}
						else {
							sprintf(result[i], "%s", currentSpell->apply(0, bs, defense, is_damage, c->isPlayer()));
						}
						isDamage[i] = is_damage;
						i++;
					}
				}
				c = getCombatant(currentCombatant);
				bs = c->getBaseStats();
				bs->mana -= currentSpell->getCost();
				numResults = i;
			}
			else {
				debug_message("applying spell to target %d\n", currentAction->target);
				c = getCombatant(currentAction->target);
				bs = c->getBaseStats();
				defense = c->getSpellDefense(currentSpell->getName());
				if (defense >= 99999) {
					is_damage = true;
					sprintf(result[0], "0");
				}
				else {
					sprintf(result[0], "%s", currentSpell->apply(0, bs, defense, is_damage, c->isPlayer()));
				}
				isDamage[0] = is_damage;
				c = getCombatant(currentCombatant);
				bs = c->getBaseStats();
				bs->mana -= currentSpell->getCost();
				numResults = 1;
			}
		}
		}
		break;
	default:
		break;
	}
}

void Battle::updateSummary(int step)
{
	TGUIWidget* widget = tguiUpdate();

	if (widget == summaryWidgets[summaryWidgets.size()-1]) {
		tguiDeleteWidget(summaryFrame);
		if (summaryEndsBattle) {
			battleDone = true;
		}

		inSummary = false;

		delete summaryFrame;
		delete advancer;
		for (unsigned int i = 0; i < summaryLines.size(); i++) {
			delete[] summaryLines[i];
		}
		summaryLines.clear();
		for (unsigned int i = 0; i < summaryWidgets.size(); i++) {
			delete summaryWidgets[i];
		}
		summaryWidgets.clear();

		return;
	}
	else {
		for (unsigned int i = 0; i < summaryWidgets.size()-1; i++) {
			if (widget == summaryWidgets[i]) {
				TGUIWidget* nextWidget = summaryWidgets[i+1];
				tguiTranslateWidget(summaryFrame, -25, -10);
				tguiDeleteWidget(summaryWidgets[i]);
				tguiAddWidget(nextWidget);
				tguiDeleteWidget(advancer);
				delete advancer;
				advancer = new MWgtAdvancer(Screen::BUFFER_WIDTH-70, 50-15, (MWgtSpeech*)nextWidget);
				tguiAddWidget(advancer);
				tguiSetFocus(advancer);
				tguiTranslateWidget(summaryFrame, 25, 10);
			}
		}
	}
}

/*
* Call each Combatants update method
* if Combatant is ready for turn, add to ready list
* Draws background and combatants
* if !locked draw status and update turns
*/
void Battle::update(int step)
{
	callLua(luaState, "update", "i>", step);

	std::list<Combatant*>::iterator it = combatants.begin();

	while (it != combatants.end()) {
		Combatant* c = *it;
		c->update(step);
		int num = c->getNumber();
		std::list<int>::iterator readyIt = readyList.begin();
		bool found = false;
		while (readyIt != readyList.end()) {
			if (num == *readyIt) {
				found = true;
				break;
			}
			readyIt++;
		}
		if (found) {
			it++;
			continue;
		}
		BattleStats* baseStats = c->getBaseStats();
		BattleStats* modStats = c->getModifiedStats();
		if (modStats->hp <= 0) {
			debug_message("Skipping turn hp <= 0\n");
			baseStats->turnReadyCount = 0;
		}
		else {
			if (config.getBattleSpeed() == 0) {
				baseStats->turnCount += step;
			}
			else {
				baseStats->turnCount += step * 2;
			}
			while (baseStats->turnCount >= modStats->latency) {
				baseStats->turnCount -= modStats->latency;
				baseStats->turnReadyCount++;
				if (baseStats->turnReadyCount >= 100) {
					baseStats->turnCount = 0;
					baseStats->turnReadyCount = 0;
					readyList.push_back(num);
					break;
				}
			}
		}
		it++;
	}

	for (;;) {
		if (readyList.size() > 0) {
			currentCombatant = readyList.front();
			/* This should never happen */
			if ((!getCombatant(currentCombatant)) || (getCombatant(currentCombatant)->getModifiedStats()->hp <= 0)) {
				debug_message("Skipping combatant ... hp <= 0\n");
				currentCombatant = -1;
			}
			readyList.pop_front();
			if (currentCombatant != -1 ||
					readyList.size() <= 0) {
				break;
			}
			else if (readyList.size() <= 0) {
				currentCombatant = -1;
				break;
			}
		}
		else {
			break;
		}
	}
}

void Battle::removeDeadEnemies()
{
	std::vector<Combatant*> removeList;

	std::list<Combatant*>::iterator it = combatants.begin();
	while (it != combatants.end()) {
		Combatant* c = *it;
		if (!c->isPlayer()) {
			BattleStats* s = c->getModifiedStats();
			if (s->hp <= 0) {
				removeList.push_back(c);
			}
		}
		it++;
	}

	for (unsigned int i = 0; i < removeList.size(); i++) {
		int num = removeList[i]->getNumber();
		readyList.remove(num);
		combatants.remove(removeList[i]);
		delete removeList[i];
	}

	removeList.clear();

	if (combatants.size() <= players.size()) {
		summarizeWin();
	}
}

void Battle::addEnemy(const char* name, int x, int y, bool can_suprise)
{
	EnemyCombatant* ec = new EnemyCombatant(name, x, y);
	/* Suprised? */
	if (can_suprise && suprised) {
		debug_message("suprised\n");
		BattleStats *baseStats = ec->getBaseStats();
		baseStats->turnReadyCount = 100;
	}
	combatants.push_back(ec);
	ec->setNumber(combatants.size()-1);
	ec->init();
}

Combatant* Battle::getCombatant(int number)
{
	std::list<Combatant*>::iterator it = combatants.begin();

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->getNumber() == number) {
			return c;
		}
		it++;
	}

	return 0;
}

int Battle::getNumberOfPlayerCombatants()
{
	return players.size();
}

std::vector<int>* Battle::getPlayerCombatants()
{
	return &players;
}

bool Battle::combatantIsDead(int number)
{
	Combatant* c = getCombatant(number);

	if (c) {
		BattleStats* baseStats = c->getBaseStats();
		if (baseStats->hp > 0) {
			return false;
		}
		else {
			return true;
		}
	}
	else {
		return false;
	}
}

std::list<Combatant*>& Battle::getCombatants()
{
	return combatants;
}

void Battle::loadBackground(const char* name)
{
	debug_message("loading background %s\n", name);
	DATAFILE* bg_datafile = load_datafile_object(getResource("battle_bgs/%s.dat", name), "BACKGROUND_PCX");
	BITMAP* bg_tile = (BITMAP*)bg_datafile->dat;

	background = create_bitmap(Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT);
	mode_7(background, bg_tile, itofix(0), itofix(0), itofix(0),
			mode7Params);
	
	unload_datafile_object(bg_datafile);
}

void Battle::start()
{
	init();
	
	backBuffer = scr->getBackBuffer();
	BITMAP* fxBuf = scr->getFXBuffer();
	blit(background, backBuffer, 0, 0, 0, 0, background->w, background->h);
	drawCombatants();
	if (currentCombatant < 0 || _showStatus) {
		drawStatus();
	}
	blit(backBuffer, fxBuf, 0, 0, 0, 0, backBuffer->w, backBuffer->h);
	drawFocusEffect(true, FOCUS_MAX_SIZE);

	unsigned long start = currentTimeMillis();
	unsigned int frameSkip = 0;

	unsigned int maxUpdateStep = 40;

	for (;;) {
		updateMusic();
	
		if (frameSkip++ >= config.getFrameSkip()) {
			frameSkip = 0;
			draw();
		}

		unsigned long now = currentTimeMillis();
		unsigned long duration = MIN(now - start, maxUpdateStep);
		start = now;

		gamepad2Keypresses(duration);

		/*
		int n = getNumberOfPlayerCombatants();
		if (n == 1) {
		    Combatant* c = getCombatant(0);
		    increaseMana(duration, c->getBaseStats(), 0);
		}
		else {
		    Combatant* p1 = getCombatant(0);
		    Combatant* p2 = getCombatant(1);
		    increaseMana(duration, p1->getBaseStats(), p2->getBaseStats());
		}
		*/

		rest(1);

		if (currentCombatant >= 0 && !actionGotten) {
			Combatant* c = getCombatant(currentCombatant);
			currentAction = c->getAction();
			if (currentAction != 0) {
				actionGotten = true;
				if (currentAction->type == TURN_DONE) {
					deathsAnimated = true;
				}
				else if (currentAction->type == TURN_RUN) {
					runSuccess = checkForRunSuccess();
					if (!runSuccess) {
						char* line = new char[100];
						strcpy(line, "You couldn't escape from the battle...$");
						summaryLines.push_back(line);
						createWidgetsFromSummaryLines();
						inSummary = true;
					}
					else {
						playOgg("run.ogg");
						Combatant* c = getCombatant(0);
						BattleStats* s = c->getModifiedStats();

						stats.hp = s->hp;
						stats.mana = s->mana;

						if (partner >= 0) {
							c = getCombatant(1);
							s = c->getModifiedStats();
							partnerStats.hp = s->hp;
							partnerStats.mana = s->mana;
						}
					}
				}
			}
		}
		else if (!actionGotten && !inSummary) {
			if (impatientMode) {
				while (currentCombatant < 0)
					update(1);
			}
			else {
				update(duration);
			}
		}
		else if (inSummary) {
			updateSummary(duration);
		}

		if (deathsAnimated) {
			_showStatus = false;
			Combatant* c = getCombatant(currentCombatant);
			BattleStats* baseStats = c->getBaseStats();
			baseStats->turnCount = 0;
			baseStats->turnReadyCount = 0;
			currentCombatant = -1;
			// after finished
			currentAction = 0;
			turnAnimated = false;
			actionAnimated = false;
			resultAnimated = false;
			deathsAnimated = false;
			actionGotten = false;
			checkForDeadPlayers();
			if (!playersDead) {
				removeDeadEnemies();
			}
		}

		if (battleDone) {
			break;
		}

		handleSpecialKeys();
	
		/*
		// FIXME:
		if (key[KEY_S]) {
			save_pcx("ss.pcx", screen, 0);
			rest(300);
		}
		*/
	}

	if (!playersDead) {
		currArea->draw(scr->getBackBuffer());
		scr->draw();
	}
}

void Battle::showStatus()
{
	_showStatus = true;
}

void Battle::hideStatus()
{
	_showStatus = false;
}

/*
 * battle_scripts/id.lua is run to setup the enemies
 */
Battle::Battle(const char* id, const char* terrain) :
	id(strdup(id)),
	currentCombatant(-1),
	currentAction(0),
	turnAnimated(false),
	actionAnimated(false),
	resultAnimated(false),
	deathsAnimated(false),
	actionGotten(false),
	battleDone(false),
	_showStatus(false),
	summaryEndsBattle(false),
	runSuccess(false)
{
	loadBackground(terrain);
	FRAME_COLOR = makecol(50, 50, 255);
	/* Suprised? */
	if ((rand() % 7) == 0) {
		suprised = true;
	}
	else {
		suprised = false;
	}
}

Battle::~Battle()
{
	free(id);

	destroy_bitmap(background);

	std::list<Combatant*>::iterator it = combatants.begin();
	while (it != combatants.end()) {
		Combatant* c = *it;
		delete c;
		it++;
	}

	combatants.clear();

	readyList.clear();

	callLua(luaState, "stop", ">");
	lua_close(luaState);
}

/* These numbers are arbitrary */
int getPlayerLatency()
{
	return 30;
}

void initBattleSystem()
{
	heartIcon = load_bitmap(getResource("battle_gfx/heart.pcx"), 0);
	clockIcon = load_bitmap(getResource("battle_gfx/clock.pcx"), 0);
	manaIcon = load_bitmap(getResource("battle_gfx/mana.pcx"), 0);
}

void destroyBattleSystem()
{
	destroy_bitmap(heartIcon);
	destroy_bitmap(clockIcon);
	destroy_bitmap(manaIcon);
}

int getDamage(int power, int armor)
{
	int pr = (int)(power * 0.2);
	if (pr <= 0) pr = 1;
	pr = (rand() % pr) - (pr / 2);
	
	int ar = (int)(armor * 0.2);
	if (ar <= 0) ar = 1;
	ar = (rand() % ar) - (ar / 2);

	return MIN(9999, MAX(1, (power + pr) - (armor + ar)));
}
