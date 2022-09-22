#include "monster.h"

Object* player = 0;
PlayerStats stats;
int partner = -1;
PlayerStats partnerStats;
BITMAP* face_bg;
BITMAP* coros_face;
BITMAP** partners_faces;
bool playersDead = false;

char* playerName = "Coro";
char* partnerNames[] = {
	"Moryt"
};

bool partnerHasMana[] = {
	false
};

const int MORYT_INITIAL_EXPERIENCE = ExperienceForLevel[14];

PlayerStats initialPartnerStats[NUM_PARTNERS] = {
	{
		0,
		{ 0, },
		MORYT_INITIAL_EXPERIENCE,
		-1,
		-1,
		{ { 0, }, },
		0,
		0,
		getMaxHP(getLevel(MORYT_INITIAL_EXPERIENCE)),
		getMaxHP(getLevel(MORYT_INITIAL_EXPERIENCE)),
		0,
		{ 0, }
	}
};

void clearMilestones(bool* ms)
{
	for (int i = 0; i < MAX_MILESTONES; i++)
		ms[i] = false;
}

int firstInventorySlotAvailable()
{
	for (int i = 0; i < MAX_INVENTORY; i++)
		if (stats.inventory[i].id < 0)
			return i;
	return -1;
}

int findInventoryItem(int id)
{
	for (int i = 0; i < MAX_INVENTORY; i++)
		if (stats.inventory[i].id == id)
			return i;
	return -1;
}

int getLevel(int experience)
{
	int level;
	for (level = 0; level < NUM_LEVELS; level++) {
		if (experience < ExperienceForLevel[level])
			break;
	}
	return level-1;
}

int getMaxHP(int level)
{
	return MaxHP[level];
}

int getMaxMana(int level)
{
	return MaxMana[level];
}

int getPower(int level)
{
	return PowerForLevel[level];
}

void applyItemEffects(ItemEffects* ie, PlayerStats* ps, BattleStats* bs)
{
	// FIXME:
	debug_message("status->%d\n", ie->status);
	debug_message("hp->%d\n", ie->hp);
	debug_message("maxHP->%d\n", ie->maxHP);
	debug_message("mana->%d\n", ie->mana);

	// Clear status

	std::vector<BattleAffect*> affectsToDelete;

	if (ie->status != 0 && bs) {
		std::list<BattleAffect*>::iterator it = bs->affects.begin();
		while (it != bs->affects.end()) {
			BattleAffect* ba = *it;
			if (!ba->isGood()) {
				affectsToDelete.push_back(ba);
			}
			it++;
		}
		for (unsigned int i = 0; i < affectsToDelete.size(); i++) {
			bs->affects.remove(affectsToDelete[i]);
			delete affectsToDelete[i];
		}
		affectsToDelete.clear();
	}

	// Adjust hp

	if (ps) {	
		ps->hp = MIN(ps->maxHP, MAX(0, ps->hp + ie->hp));
	}
	if (bs) {
		bs->hp = MIN(bs->maxHP, MAX(0, bs->hp + ie->hp));
	}

	// Adjust maxHP

	if (ps) {	
		ps->maxHP += ie->maxHP;
	}
	if (bs) {
		bs->maxHP += ie->maxHP;
	}

	// Adjust latency
	
	if (bs) {
		bs->latency += ie->latency;
		if (bs->latency < 10) {
			bs->latency = 10;
		}
		else if (bs->latency > 100) {
			bs->latency = 100;
		}
	}

	// Adjust mana
	
	if (ps) {
		ps->mana = MIN(ps->maxMana, MAX(0, ps->mana + ie->mana));
	}
	if (bs) {
		bs->mana = MIN(bs->maxMana, MAX(0, bs->mana + ie->mana));
	}

	// Adjust spells
	
	if (strcmp(ie->spell, "") && ps) {
		for (int i = 0; i < MAX_SPELLS; i++) {
			if (ps->spells[i] == 0) {
				ps->spells[i] = strdup(ie->spell);
				setSpellMilestone(ie->spell);
				break;
			}
		}
	}
}

void getItemResults(ItemEffects* ie, char* result)
{
	if (ie->status && ie->hp && ie->mana) {
		strcpy(result, "!!!");
	}
	else if (ie->status) {
		strcpy(result, "HEAL!");
	}
	else if (ie->hp != 0) {
		sprintf(result, "%d", ie->hp);
	}
	else if (ie->maxHP != 0) {
		sprintf(result, "%d", ie->maxHP);
	}
	else if (ie->latency != 0) {
		sprintf(result, "FAST!");
	}
	else if (ie->mana != 0) {
		sprintf(result, "%d", ie->mana);
	}
	else {
		strcpy(result, "???");
	}
}

void increaseMana(int step, BattleStats* bs1, BattleStats* bs2)
{
    static int coroCount = 0;

    coroCount += step;

    if (coroCount > 200*(NUM_LEVELS-getLevel(stats.experience))) {
	coroCount = 0;
	if (bs1) {
	    bs1->mana = MIN(bs1->maxMana, stats.mana+1);
	}
	else {
	    stats.mana = MIN(stats.maxMana, stats.mana+1);
	}
    }

    static int partnerCount = 0;

    if (partner < 0) {
	return;
    }

    partnerCount += step;

    if (partnerCount > 200*(NUM_LEVELS-getLevel(partnerStats.experience))) {
	partnerCount = 0;
	if (bs2) {
	    bs2->mana = MIN(bs2->maxMana, partnerStats.mana+1);
	}
	else {
	    partnerStats.mana = MIN(partnerStats.maxMana, partnerStats.mana+1);
	}
    }
}
