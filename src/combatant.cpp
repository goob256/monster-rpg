#include "monster.h"

BattleStats* Combatant::getBaseStats()
{
	return &baseStats;
}


BattleStats* Combatant::getModifiedStats()
{
	modifiedStats.hp = baseStats.hp;
	modifiedStats.maxHP = baseStats.maxHP;
	modifiedStats.latency = baseStats.latency;
	modifiedStats.turnReadyCount = baseStats.turnReadyCount;
	modifiedStats.turnCount = baseStats.turnCount;
	modifiedStats.experience = baseStats.experience;
	modifiedStats.mana = baseStats.mana;
	modifiedStats.maxMana = baseStats.maxMana;

	std::list<BattleAffect*>::iterator it = baseStats.affects.begin();

	while (it != baseStats.affects.end()) {
		BattleAffect* ba = *it;
		ba->apply(&modifiedStats);
		it++;
	}

	return &modifiedStats;
}

char* Combatant::getName()
{
	return name;
}

void Combatant::setNumber(int number)
{
	this->number = number;
}

int Combatant::getNumber()
{
	return number;
}

int Combatant::getX()
{
	return x;
}

int Combatant::getY()
{
	return y;
}

bool Combatant::showStatusDetails()
{
	return statusDetails;
}

void Combatant::setShowStatusDetails(bool details)
{
	statusDetails = details;
}

void Combatant::setSubAnimation(char* name)
{
	animSet->setSubAnimation(name);
}

int Combatant::getWidth()
{
	return width;
}

int Combatant::getHeight()
{
	return height;
}

int Combatant::getShadowOffset()
{
	return shadowOffset;
}

int Combatant::getShadowHeight()
{
	return shadowHeight;
}

void Combatant::update(int step)
{
	std::list<BattleAffect*>::iterator it = baseStats.affects.begin();

	while (it != baseStats.affects.end()) {
		BattleAffect* ba = *it;
		ba->update(step);
		it++;
	}
}

Combatant::Combatant(const char* name, int x, int y) :
	name(strdup(name)),
	x(x),
	y(y),
	statusDetails(true)
{
	baseStats.turnReadyCount = 0;
	baseStats.turnCount = 0;
}

Combatant::~Combatant()
{
	free(name);
	delete animSet;
}
