#ifndef BATTLE_H
#define BATTLE_H

#include <list>

#include "mwidgets.h"
#include "spell.h"

extern "C" {
#include <logg.h>
}

/*
 * The size of pixelation for the focus effect
 * at the start of a battle
 */
const int FOCUS_MAX_SIZE = 16;

const int BATTLE_ONE_PLAYER_X = 280;
const int BATTLE_ONE_PLAYER_Y = 100;

const int BATTLE_TWO_PLAYERS_PLAYER_ONE_X = 280;
const int BATTLE_TWO_PLAYERS_PLAYER_ONE_Y = 80;
const int BATTLE_TWO_PLAYERS_PLAYER_TWO_X = 280;
const int BATTLE_TWO_PLAYERS_PLAYER_TWO_Y = 120;

struct BattleStats;

class BattleAffect {
public:
	char* getName() {
		return name;
	}
	bool isGood() {
		return good;
	}
	/*
	 * Decrease timeRemaining by step, return true when time to
	 * be removed
	 */
	virtual bool update(int step) = 0;
	virtual void apply(BattleStats* target) = 0;

	BattleAffect(char* name)
	{
		this->name = strdup(name);
	}

	virtual ~BattleAffect() {
		free(name);
	}
protected:
	char* name;
	bool good;
};

struct BattleStats {
	int hp;
	int maxHP;
	int latency;		// lower is faster
	int turnReadyCount;	// 0-100%
	int turnCount;		// increase turnReadyCount when this reaches latency
	int experience;
	int mana;
	int maxMana;
	std::list<BattleAffect*> affects;
};

enum TurnType {
	TURN_DONE = 0,
	TURN_RUN,
	TURN_ATTACK,
	TURN_SPELL,
	TURN_ITEM
};

struct TurnAction {
	TurnType type;
	union {
		int index;
		char* name;
	} action;
	int target; // -1 == all players, -2 == all enemies, else combatant index
};

class Battle;

class Combatant {
public:
	BattleStats* getBaseStats();		// before affects
	BattleStats* getModifiedStats();	// after affects
	char* getName();
	/*
	 * Unlock battle when finished turn
	 */
	virtual void update(int step);
	virtual Animation* getCurrentAnimation() = 0;
	void setNumber(int number);
	int getNumber();
	int getX();
	int getY();
	bool showStatusDetails();
	void setShowStatusDetails(bool details);
	void setSubAnimation(char* name);
	int getWidth();
	int getHeight();
	int getShadowOffset();
	int getShadowHeight();
	virtual int getPower() = 0;
	virtual int getWeaponDefense(int weapon) = 0;
	virtual int getSpellDefense(char *spell_name) = 0;
	virtual void drawSpecial(BITMAP* buffer) {};
	/*
	 * lock battle
	 * Attack, spell, etc
	 */
	virtual TurnAction* getAction() = 0;
	virtual bool isPlayer() = 0;
	Combatant(const char* name, int x, int y);
	virtual ~Combatant();
protected:
	int number;
	char* name;
	BattleStats baseStats;
	BattleStats modifiedStats;
	int x;
	int y;
	int width;
	int height;
	int shadowOffset;
	int shadowHeight;
	TurnAction turnAction;
	AnimationSet* animSet;
	bool statusDetails;
	int currentCombatant;
};

class PlayerCombatant : public Combatant {
public:
	void update(int step);
	TurnAction* getAction();
	Animation* getCurrentAnimation();
	bool isPlayer();
	void drawSpecial(BITMAP* backBuffer);
	int getPower();
	int getWeaponDefense(int weapon);
	int getSpellDefense(char *spell);
	bool isPartner();
	PlayerCombatant(const char* name, bool isPartner, int x, int y);
	~PlayerCombatant();
private:
	void createActionGUI();
	void destroyActionGUI();
	void createEnemySelectionGUI();
	void destroyEnemySelectionGUI();
	void createSpellGUI();
	void destroySpellGUI();
	void createItemGUI();
	void destroyItemGUI();
	void createPlayerSelectionGUI();
	void destroyPlayerSelectionGUI();

	int findEnemy(int n);
	int findPlayer(int n);

	bool imaPartner;

	MWgtFrame* firstFrame;
	MWgtTextButton* txtAttack;
	MWgtTextButton* txtSpell;
	MWgtTextButton* txtItem;
	MWgtTextButton* txtSkip;
	MWgtTextButton* txtRun;

	MWgtFrame* itemFrame;
	MWgtInventory* inventory;

	MWgtFrame* spellFrame;
	MWgtSpellSelector* spells;

	int selected;
	bool selectionIsItem;

	std::vector<Point*> enemyPoints;
	MWgtSelector* enemySelector;
	
	std::vector<Point*> playerPoints;
	MWgtSelector* playerSelector;

	static const int SPELL = 0;
	static const int ATTACK = 1;
	static const int ITEM = 2;
	int selectionReason;
	char* spellName;
	bool selectedPartyItem;
};

class EnemyCombatant : public Combatant {
public:
	void init();
	void update(int step);
	TurnAction* getAction();
	Animation* getCurrentAnimation();
	bool isPlayer();
	int getPower();
	int getWeaponDefense(int weapon);
	int getSpellDefense(char *spell_name);
	EnemyCombatant(const char* name, int x, int y);
	~EnemyCombatant();
private:
	lua_State* luaState;
};

class Battle {
public:
	static int FRAME_COLOR;
	void start();
	void addEnemy(const char* name, int x, int y, bool can_suprise);
	Combatant* getCombatant(int number);
	int getNumberOfPlayerCombatants();
	std::vector<int>* getPlayerCombatants();
	bool combatantIsDead(int number);
	std::list<Combatant*>& getCombatants();
	void showStatus();
	void hideStatus();
	/*
	 * battle_scripts/id.lua is run to setup the enemies
	 */
	Battle(const char* id, const char* terrain);
	~Battle();
private:
	static const int STATUS_H = 80;
	static const int METER_WIDTH = 50;
	void loadBackground(const char* name);
	/*
	 * Call once at start
	 */
	void init();
	/*
	void lock();
	void unlock();
	*/
	void drawPlayerStatus(PlayerCombatant* c);
	void drawStatus();
	void drawPlayerInAction(Combatant* c);
	void drawEnemyInAction(Combatant* c);
	void drawCombatantInAction(Combatant* c);
	void drawCombatants();
	void initAttackEffect();
	void drawAttackEffect();
	void initItemEffect();
	void drawItemEffect();
	void initCureEffect();
	//void initHealEffect();
	void initRevitalizeEffect();
	void initSpellEffect();
	void drawSpellEffect();
	void initAction();
	void drawAction();
	void initResult();
	void drawResult();
	void initDeaths();
	void drawDeaths();
	void draw();
	void applyTurn();
	/*
	 * Call each Combatants update method
	 * if Combatant is ready for turn, add to ready list
	 * Draws background and combatants
	 * if !locked draw status and update turns
	 */
	void update(int step);
	void removeDeadEnemies();
	void updateSummary(int step);
	void drawSummary();
	void summarizeWin();
	void checkForDeadPlayers();
	bool checkForRunSuccess();
	char* id;
	/*
	 * If battle is locked by a Combatant, update() will still be
	 * called on all Combatants, but getAction etc will never be
	 * called. Used e.g. during animations after an attack/spell.
	 */
	//bool locked;
	/*
	 * Player should always be first in the list
	 */
	std::list<Combatant*> combatants;
	/*
	 * The first number in the list is the number of the
	 * combatant who's turn it is, second is who's turn is next,
	 * etc.
	 */
	std::list<int> readyList;
	std::vector<int> players;
	BITMAP* background;
	BITMAP* backBuffer;
	lua_State* luaState;
	int numPlayers;
	int currentCombatant;
	TurnAction* currentAction;
	bool turnAnimated; 	// combatant motion
	bool actionAnimated;	// action special effect
	bool resultAnimated;	// damage
	bool deathsAnimated;
	bool actionGotten;
	bool battleDone;
	bool _showStatus;
	Spell* currentSpell;
	long lastSpellUpdate;
	bool summaryEndsBattle;
	bool runSuccess;
	bool suprised;
};

extern Battle* activeBattle;
extern bool impatientMode;

extern BattleAffect* createBattleAffect(char* name);
extern int getPlayerLatency();
extern void initBattleSystem();
extern void destroyBattleSystem();
extern int getDamage(int power, int armor);

#endif
