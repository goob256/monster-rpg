#include <math.h>

#include "monster.h"

int SpellTarget::getX()
{
	return x;
}

int SpellTarget::getY()
{
	return y;
}

int SpellTarget::getWidth()
{
	return w;
}

int SpellTarget::getHeight()
{
	return h;
}

bool SpellTarget::getTowardsLeft()
{
	return towardsLeft;
}

SpellTarget::SpellTarget(int x, int y, int w, int h, bool towardsLeft) :
	x(x),
	y(y),
	w(w),
	h(h),
	towardsLeft(towardsLeft)
{
}

char Spell::resultsBuffer[32];

char* Spell::getName()
{
	return name;
}

bool Spell::isForBattleOnly()
{
	return battleOnly;
}

int Spell::getCost()
{
	return cost;
}

bool Spell::affectsAll()
{
	return _affectsAll;
}

bool Spell::noResults()
{
	return _noResults;
}

Spell::Spell(char* name)
{
	this->name = strdup(name);
}

Spell::~Spell()
{
	free(name);
}

class CureSpell : public Spell
{
public:
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	CureSpell(char* name, int hpGained, int cost);
	~CureSpell();
private:
	int hpGained;
};

char* CureSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool &isDamage, bool targetIsPlayer)
{
	playOgg("dust.ogg");
	if (ps) {
		ps->hp = MIN(ps->maxHP, ps->hp+hpGained);
	}
	if (bs) {
		bs->hp = MIN(bs->maxHP, bs->hp+hpGained);
	}

	sprintf(resultsBuffer, "%d", hpGained);

	isDamage = false;

	return resultsBuffer;
}

bool CureSpell::update(int step)
{
	return updateDustEffect(step);
}

void CureSpell::draw(BITMAP* buffer)
{
	drawDustEffect(buffer);
}

void CureSpell::init(std::vector<SpellTarget *>& targets)
{
	SpellTarget* target = targets[0];
	initDustEffect(target->getX(), target->getY(), makecol(200, 200, 200));
}

CureSpell::CureSpell(char* name, int hpGained, int cost) :
	Spell(name)
{
	this->hpGained = hpGained;
	this->cost = cost;
	battleOnly = false;
	_noResults = false;
	_affectsAll = false;
}

CureSpell::~CureSpell()
{
}

class RainSpell : public Spell
{
public:
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool &isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	RainSpell(char* name, int time, int numParticles, int trailLength, int width, int height,
		int xOffset, float dx, float dy, int damage, int randDamage, int cost, char* sound);
	~RainSpell();
private:
	int time;
	int numParticles;
	int trailLength;
	int width;
	int height;
	int xOffset;
	float dx;
	float dy;
	int damage;
	int randDamage;
	char* sound;
	Particle* particles;
	float da;
};

char* RainSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer)
{
	int thisDamage;
	
	thisDamage = damage + ((rand() % 2 == 1) ? (rand() % randDamage) : (-(rand() % randDamage)));
	thisDamage -= defense;

	if (targetIsPlayer && thisDamage < 0)
		thisDamage = 1;

	if (bs) {
		bs->hp = MAX(0, bs->hp-thisDamage);
	}

	isDamage = thisDamage > 0;

	if (thisDamage < 0)
		thisDamage = -thisDamage;

	sprintf(resultsBuffer, "%d", thisDamage);

	return resultsBuffer;
}

bool RainSpell::update(int step)
{
	if (updateParticles(particles, numParticles, false, true,
			0.0f, 0.0f, 0.0f, da, step, time) <= 0) {
		delete[] particles;
		return true;
	}
	return false;
}

void RainSpell::draw(BITMAP* buffer)
{
	drawParticles(buffer, particles, numParticles);
}

void RainSpell::init(std::vector<SpellTarget *>& targets)
{
	SpellTarget* target = targets[0];

	particles = new Particle[numParticles];
	da = -(dy / (float)height * 255.0f);
	
	for (int i = 0; i < numParticles; i++) {
		particles[i].trail_size = trailLength;
		particles[i].trail_used = 0;
		particles[i].trail[0].x = target->getX() + ((rand() % width) - (width/2)) +
			(target->getTowardsLeft() ? xOffset : -xOffset);
		int y = (rand() % height);
		float yPercent = (float)y / (float)height;
		particles[i].trail[0].y = target->getY() + (y - (height/2));
		particles[i].trail[0].alpha = 255 - (255.0f * yPercent);
		particles[i].start_x = particles[i].trail[0].x;
		particles[i].start_y = target->getY() - (height / 2);
		particles[i].start_alpha = 255;
		particles[i].dx = target->getTowardsLeft() ? dx : -dx;
		particles[i].dy = dy;
		particles[i].r = 0;
		particles[i].g = rand() % 100 + 100;
		particles[i].b = 255;
		particles[i].lifetime = 0;
	}

	playOgg(sound);
}

RainSpell::RainSpell(char* name, int time, int numParticles, int trailLength, int width, int height,
	int xOffset, float dx, float dy, int damage, int randDamage, int cost, char* sound) :
	Spell(name)
{
	this->time = time;
	this->numParticles = numParticles;
	this->trailLength = trailLength;
	this->width = width;
	this->height = height;
	this->xOffset = xOffset;
	this->dx = dx;
	this->dy = dy;
	this->damage = damage;
	this->randDamage = randDamage;
	this->cost = cost;
	this->sound = sound;
	battleOnly = true;
	_affectsAll = false;
	_noResults = false;
}

RainSpell::~RainSpell()
{
}

struct DarknessLine
{
	float length;
	float speed;
};

class DarknessSpell : public Spell
{
public:
	static const int X_DIST = 80;
	static const int NUM_LINES = Screen::BUFFER_WIDTH+X_DIST;
	static const int SPEED_DELTA_RAND = 750;
	static const int MIN_SPEED_RAND = 250;
	static constexpr float MAX_SPEED = 0.075f;
	static const int DURATION = 1800;
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	DarknessSpell(void);
	~DarknessSpell();
private:
	int damage;
	int randDamage;
	DarknessLine lines[NUM_LINES];
	bool towardsLeft;
	int count;
};

char* DarknessSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer)
{
	int thisDamage;
	
	thisDamage = damage + ((rand() % 2 == 1) ? (rand() % randDamage) : (-(rand() % randDamage)));
	thisDamage -= defense;

	if (targetIsPlayer && thisDamage < 0)
		thisDamage = 1;

	if (bs) {
		bs->hp = MAX(0, bs->hp-thisDamage);
	}

	isDamage = thisDamage > 0;

	if (thisDamage < 0)
		thisDamage = -thisDamage;

	sprintf(resultsBuffer, "%d", thisDamage);

	return resultsBuffer;
}

bool DarknessSpell::update(int step)
{
	for (int i = 0; i < NUM_LINES; i++) {
		lines[i].length += (float)step * lines[i].speed;
	}
	count += step;
	if (count > DURATION)
		return true;
	else
		return false;
}

void DarknessSpell::draw(BITMAP* buffer)
{
	for (int i = 0; i < NUM_LINES; i++) {
		int x1, y1, x2, y2;
		float root;
		x1 = Screen::BUFFER_WIDTH;
		y1 = i - X_DIST;
		root = sqrt((lines[i].length*lines[i].length)/2);
		x2 = x1 - (int)root;
		y2 = y1 + (int)root;
		line(buffer, x1, y1, x2, y2, makecol(0, 0, 0));
	}
}

void DarknessSpell::init(std::vector<SpellTarget *>& targets)
{
	SpellTarget* target = targets[0];

	damage = 80;
	randDamage = 40;

	for (int i = 0; i < NUM_LINES; i++) {
		lines[i].length = 0;
		lines[i].speed =
			((float)(rand() % SPEED_DELTA_RAND + MIN_SPEED_RAND) /
			(SPEED_DELTA_RAND+MIN_SPEED_RAND)) * MAX_SPEED;
	}

	towardsLeft = target->getTowardsLeft();

	count = 0;

	playOgg("darkness.ogg");
}

DarknessSpell::DarknessSpell() :
	Spell("Darkness")
{
	battleOnly = true;
	_affectsAll = true;
	_noResults = false;
}

DarknessSpell::~DarknessSpell()
{
}

struct Flame {
	Point start_pos;
	Point curr_pos;
	bool y_moves;
};

class FireSpell : public Spell
{
public:
	static const int MOVE_DURATION = 75;
	static const int TOTAL_MOVES = (2500/MOVE_DURATION);
	static const int NUM_FLAMES = 50;
	static const int MAX_DEVIATION = 8;
	static const int MAX_INIT_X_DEV = 12; /* Maximum initial x deviation */
	static const int MAX_INIT_Y_DEV = 8;
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	FireSpell(char *name, bool affects_all, int dmg, int randDmg, int cost);
	~FireSpell();
private:
	Flame *flames;
	int count;
	Animation *anim;
	int num_targets;
	int anim_w;
	int anim_h;
	int damage;
	int randDamage;
	int numMoves;
};

char* FireSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer)
{
	int thisDamage;
	
	thisDamage = damage + ((rand() % 2 == 1) ? (rand() % randDamage) : (-(rand() % randDamage)));
	thisDamage -= defense;

	if (targetIsPlayer && thisDamage < 0)
		thisDamage = 1;

	if (bs) {
		bs->hp = MAX(0, bs->hp-thisDamage);
	}

	isDamage = thisDamage > 0;

	if (thisDamage < 0)
		thisDamage = -thisDamage;

	sprintf(resultsBuffer, "%d", thisDamage);

	return resultsBuffer;
}

bool FireSpell::update(int step)
{
	count += step;
	anim->update(step);

	if (numMoves > TOTAL_MOVES) {
		delete[] flames;
		return true;
	}

	if (count < MOVE_DURATION) return false;

	numMoves++;

	count -= MOVE_DURATION;

	for (int i = 0; i < NUM_FLAMES*num_targets; i++) {
		Point& curr = flames[i].curr_pos;
		Point& start = flames[i].start_pos;
		if (curr.x > start.x+MAX_DEVIATION) {
			curr.x--;
		}
		else if (curr.x < start.x-MAX_DEVIATION) {
			curr.x++;
		}
		else {
			int r = rand() % 3;
			if (r == 0)
				curr.x++;
			else if (r == 1)
				curr.x--;
			else {
				if (flames[i].y_moves)
					curr.y--;
			}
		}
	}

	return false;
}

void FireSpell::draw(BITMAP* buffer)
{
	for (int i = 0; i < NUM_FLAMES*num_targets; i++) {
		anim->draw(buffer, flames[i].curr_pos.x,
			flames[i].curr_pos.y);
	}
}

void FireSpell::init(std::vector<SpellTarget *>& targets)
{
	numMoves = 0;

	flames = new Flame[targets.size()*NUM_FLAMES];

	for (int i = 0; i < targets.size(); i++) {
		for (int j = 0; j < NUM_FLAMES; j++) {
			Flame& f = flames[(i*NUM_FLAMES)+j];
			int x = rand() % (MAX_INIT_X_DEV*2);
			x -= MAX_INIT_X_DEV;
			int y = rand() % (MAX_INIT_Y_DEV);
			f.start_pos.x = targets[i]->getX() + x - (anim_w/2);
			f.start_pos.y = targets[i]->getY() - y - (anim_h/2)
				+ (targets[i]->getHeight()/2);
			f.curr_pos.x = f.start_pos.x;
			f.curr_pos.y = f.start_pos.y;
			f.y_moves = rand() % 2;
		}
	}

	count = 0;

	num_targets = targets.size();

	playOgg("fire.ogg");
}

FireSpell::FireSpell(char *name, bool affects_all, int dmg, int randDmg,
	int cost) :
	Spell(name),
	damage(dmg),
	randDamage(randDmg)
{
	battleOnly = true;
	_affectsAll = affects_all;
	anim = new Animation();
	anim->load(getResource("spell_gfx/Fire.anim"));
	BITMAP *bmp = anim->getBitmap(0);
	anim_w = bmp->w;
	anim_h = bmp->h;
	this->cost = cost;
	_noResults = false;
}

FireSpell::~FireSpell()
{
	delete anim;
}

class SummonSpell : public Spell
{
public:
	static const int MINION1_X = 60;
	static const int MINION1_Y = 35;
	static const int MINION2_X = 80;
	static const int MINION2_Y = 70;
	static const int MINION_W = 27;
	static const int MINION_H = 51;
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	SummonSpell();
	~SummonSpell();
private:
	int hpGained;
};

char* SummonSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool &isDamage, bool targetIsPlayer)
{
	playOgg("dust.ogg");
	activeBattle->addEnemy("Minion", MINION1_X, MINION1_Y, false);
	activeBattle->addEnemy("Minion", MINION2_X, MINION2_Y, false);

	return resultsBuffer;
}

bool SummonSpell::update(int step)
{
	return updateFireworkEffect(step);
}

void SummonSpell::draw(BITMAP* buffer)
{
	drawFireworkEffect(buffer);
}

void SummonSpell::init(std::vector<SpellTarget *>& targets)
{
	initFireworkEffect(MINION1_X+(MINION_W/2), MINION1_Y+(MINION_H/2), makecol(255, 50, 50));
	initFireworkEffect(MINION2_X+(MINION_W/2), MINION2_Y+(MINION_H/2), makecol(255, 50, 50));
}

SummonSpell::SummonSpell() :
	Spell("Summon")
{
	battleOnly = true;
	_noResults = true;
	_affectsAll = false;
}

SummonSpell::~SummonSpell()
{
}

class DemonSpell : public Spell
{
public:
	static const int MOVE_DURATION = 3;
	char* apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer);
	bool update(int step);
	void draw(BITMAP* buffer);
	void init(std::vector<SpellTarget *>& targets);
	DemonSpell(char *name, bool affects_all, int dmg, int randDmg, int cost);
	~DemonSpell();
private:
	Point *demons;
	int count;
	Animation *anim;
	int num_targets;
	int anim_w;
	int anim_h;
	int damage;
	int randDamage;
};

char* DemonSpell::apply(PlayerStats* ps, BattleStats* bs, int defense, bool& isDamage, bool targetIsPlayer)
{
	int thisDamage;
	
	thisDamage = damage + ((rand() % 2 == 1) ? (rand() % randDamage) : (-(rand() % randDamage)));
	thisDamage -= defense;

	if (targetIsPlayer && thisDamage < 0)
		thisDamage = 1;

	if (bs) {
		bs->hp = MAX(0, bs->hp-thisDamage);
	}

	isDamage = thisDamage > 0;

	if (thisDamage < 0)
		thisDamage = -thisDamage;

	sprintf(resultsBuffer, "%d", thisDamage);

	return resultsBuffer;
}

bool DemonSpell::update(int step)
{
	count += step;
	anim->update(step);

	if (demons[0].x >= Screen::BUFFER_WIDTH) {
		delete[] demons;
		return true;
	}

	if (count < MOVE_DURATION) return false;

	count -= MOVE_DURATION;

	for (int i = 0; i < num_targets; i++) {
		demons[i].x++;
	}

	return false;
}

void DemonSpell::draw(BITMAP* buffer)
{
	for (int i = 0; i < num_targets; i++) {
		anim->draw(buffer, demons[i].x,
			demons[i].y);
	}
}

void DemonSpell::init(std::vector<SpellTarget *>& targets)
{
	demons = new Point[targets.size()];

	for (int i = 0; i < targets.size(); i++) {
		demons[i].x = 0 - anim_w;
		demons[i].y = targets[i]->getY() - (anim_h/2);
	}

	count = 0;

	num_targets = targets.size();

	playOgg("demons.ogg");
}

DemonSpell::DemonSpell(char *name, bool affects_all, int dmg, int randDmg,
	int cost) :
	Spell(name),
	damage(dmg),
	randDamage(randDmg)
{
	battleOnly = true;
	_affectsAll = affects_all;
	anim = new Animation();
	anim->load(getResource("spell_gfx/Demon.anim"));
	BITMAP *bmp = anim->getBitmap(0);
	anim_w = bmp->w;
	anim_h = bmp->h;
	this->cost = cost;
	_noResults = false;
}

DemonSpell::~DemonSpell()
{
	delete anim;
}

static std::vector<Spell*> spells;

void initSpells()
{
	spells.push_back(new CureSpell("Cure", 50, 5));
	spells.push_back(new RainSpell("Rain", 2500, 30, 5, 32, 64, 0, 0.0f, 0.06f, 200, 32, 9, "rain.ogg"));
	spells.push_back(new DarknessSpell());
	spells.push_back(new FireSpell("Fire", false, 200, 80, 10));
	spells.push_back(new SummonSpell());
	spells.push_back(new DemonSpell("Demons", true, 150, 75, 8));
}

void destroySpells()
{
	for (unsigned int i = 0; i < spells.size(); i++) {
		delete spells[i];
	}
	spells.clear();
}

Spell* findSpell(char* name)
{
	for (unsigned int i = 0; i < spells.size(); i++) {
		Spell* s = spells[i];
		if (!strcmp(s->getName(), name)) {
			return s;
		}
	}
	return 0;
}

bool spellIsForBattleOnly(char* name)
{
	Spell* s = findSpell(name);
	return s->isForBattleOnly();
}

int getSpellCost(char* name)
{
	Spell* s = findSpell(name);
	return s->getCost();
}
