#ifndef PLAYER_H
#define PLAYER_H

#include <vector>

#include "battle.h"
#include "spell.h"

const int MAX_MILESTONES = 8000;
const int MAX_INVENTORY = 64;
const unsigned int MAX_GOLD = 9999;
const int MAX_EXPERIENCE = 99999;
const int NUM_LEVELS = 32;
const int MAX_QUANTITY = 99;
const int NUM_PARTNERS = 1;
const int MAX_MAXHP = 9999;

const int ExperienceForLevel[NUM_LEVELS] = {
	0,
	10,
	25,
	50,
	100,
	150,
	200,
	300,
	450,
	600,
	800,
	1000,
	1250,
	1500,
	1750,
	2000,
	2500,
	3000,
	4000,
	5000,
	6000,
	7500,
	10000,
	12500,
	15000,
	20000,
	25000,
	30000,
	40000,
	50000,
	70000,
	99999
};

const int PowerForLevel[NUM_LEVELS] = {
	1,
	2,
	3,
	5,
	8,
	12,
	18,
	25,
	37,
	50,
	65,
	85,
	105,
	125,
	160,
	200,
	250,
	280,
	310,
	375,
	430,
	500,
	560,
	625,
	675,
	715,
	750,
	810,
	875,
	930,
	965,
	1000
};

const int MaxHP[NUM_LEVELS] = {
	5,
	7,
	10,
	14,
	18,
	24,
	30,
	38,
	46,
	56,
	66,
	78,
	88,
	100,
	115,
	150,
	200,
	300,
	400,
	600,
	1000,
	1500,
	2500,
	3000,
	4000,
	5000,
	6000,
	7000,
	8000,
	9500,
};

const int MaxMana[NUM_LEVELS] = {
	0,
	0,
	0,
	0,
	5,
	10,
	15,
	18,
	22,
	26,
	33,
	37,
	44,
	50,
	57,
	75,
	100,
	150,
	200,
	300,
	500,
	750,
	1250,
	1500,
	2000,
	2500,
	3000,
	3500,
	4000,
	5000,
};

struct Item {
	int id;
	int quantity;
};

struct ItemEffects {
	int status;
	int hp;
	int maxHP;
	int latency;
	int mana;
	char* spell;
};

struct PlayerStats {
	int saveState;
	bool milestones[MAX_MILESTONES];
	int experience;
	int weapon;
	int armor;
	Item inventory[MAX_INVENTORY];
	int mana;
	int maxMana;
	int hp;
	int maxHP;
	unsigned int gold;
	char* spells[MAX_SPELLS];
};

extern PlayerStats stats;
extern int partner;
extern PlayerStats partnerStats;
extern char* playerName;
extern char* partnerNames[];
extern bool playersDead;
extern PlayerStats initialPartnerStats[NUM_PARTNERS];
extern bool partnerHasMana[];

extern void clearMilestones(bool* ms);
extern int firstInventorySlotAvailable();
extern int findInventoryItem(int id);
extern int getLevel(int experience);
extern int getMaxHP(int level);
extern int getMaxMana(int level);
extern int getPower(int level);
extern void applyItemEffects(ItemEffects* ie, PlayerStats* ps, BattleStats* bs);
extern void getItemResults(ItemEffects* ie, char* result);
extern void increaseMana(int step, BattleStats* p1bs, BattleStats* p2bs);

#endif
