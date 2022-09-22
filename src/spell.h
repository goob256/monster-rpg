#ifndef _SPELL_H
#define _SPELL_H

struct BattleStats;

const int MAX_SPELLS = 64;

class SpellTarget
{
public:
	int getX();
	int getY();
	int getWidth();
	int getHeight();
	bool getTowardsLeft();
	SpellTarget(int x, int y, int w, int h, bool towardsLeft);
private:
	/* Where the effect is targetting */
	int x; /* center */
	int y; /* center */
	int w;
	int h;
	bool towardsLeft;
};

class Spell
{
public:
	char* getName();
	bool isForBattleOnly();
	int getCost();
	bool affectsAll();
	bool noResults();
	/*
	 * A spell that produces no "results" must not use anything
	 * passed in to apply, they will all be NULL.
	 */
	virtual char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer) = 0;
	/*
	 * Update animation
	 * Return true when finished
	 */
	virtual bool update(int step) = 0;
	virtual void draw(BITMAP* buffer) = 0;
	virtual void init(std::vector<SpellTarget*>& targets) = 0;
	Spell(char* name);
	virtual ~Spell();
protected:
	static char resultsBuffer[32];
	char* name;
	bool battleOnly;
	int cost;
	bool _affectsAll;
	bool _noResults;
};

extern void initSpells();
extern void destroySpells();
extern Spell* findSpell(char* name);
extern bool spellIsForBattleOnly(char* name);
extern int getSpellCost(char *name);
extern void setSpellMilestone(char *spellName);

#endif
