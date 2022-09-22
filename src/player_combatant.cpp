#include "monster.h"

void PlayerCombatant::update(int step)
{
}

Animation* PlayerCombatant::getCurrentAnimation()
{
	return animSet->getCurrentAnimation();
}

void PlayerCombatant::drawSpecial(BITMAP* backBuffer)
{
	if (firstFrame != 0) {
		// FIXME: add other widgets too
		tguiMakeDirty(firstFrame);

		if (itemFrame) {
			tguiMakeDirty(itemFrame);
		}
		else if (spellFrame) {
			tguiMakeDirty(spellFrame);
		}

		if (enemySelector) {
			tguiMakeDirty(enemySelector);
		}
		else if (playerSelector) {
			tguiMakeDirty(playerSelector);
		}

		tguiClearDirtyRectangle();
	}
}

TurnAction* PlayerCombatant::getAction()
{
	static long start;

	if (enemySelector || playerSelector) {
		activeBattle->showStatus();
	}
	else {
		activeBattle->hideStatus();
	}

	if (!firstFrame) {
		createActionGUI();
		start = tguiCurrentTimeMillis();
	}

	TGUIWidget* widget = tguiUpdate();

	if (widget == txtAttack) {
		selectionReason = ATTACK;
		createEnemySelectionGUI();
	}
	else if (widget == txtSpell) {
		if (!isPartner() || partnerHasMana[partner])
			createSpellGUI();
		else
			playOgg("error.ogg");
	}
	else if (widget == txtItem) {
		createItemGUI();
	}
	else if (widget == txtSkip) {
		turnAction.type = TURN_DONE;
		turnAction.action.index = 0;
		turnAction.target = 0;
		destroyActionGUI();
		firstFrame = 0;
		return &turnAction;
	}
	else if (widget == txtRun) {
		turnAction.type = TURN_RUN;
		turnAction.action.index = 0;
		turnAction.target = 0;
		destroyActionGUI();
		firstFrame = 0;
		return &turnAction;
	}
	else if (enemySelector != 0 && selectionReason == ATTACK && widget == enemySelector) {
		if (enemySelector->getPosition() >= 0) {
			selected = dynamic_cast<MWgtSelector*>(widget)->getPosition();
			destroyEnemySelectionGUI();
			enemySelector = 0;
			if (selected < 0) {
				tguiSetFocus(txtAttack);
			}
			else {
				turnAction.type = TURN_ATTACK;
				turnAction.action.index = 0;
				turnAction.target = findEnemy(selected);
				destroyActionGUI();
				firstFrame = 0;
				return &turnAction;
			}
		}
		else {
			destroyEnemySelectionGUI();
			enemySelector = NULL;
			tguiSetFocus(txtSpell);
		}
	}
	else if (enemySelector != 0 && selectionReason == SPELL && widget == enemySelector) {
		if (enemySelector->getPosition() >= 0) {
			selected = dynamic_cast<MWgtSelector*>(widget)->getPosition();
			destroyEnemySelectionGUI();
			enemySelector = 0;
			if (selected < 0) {
				tguiSetFocus(txtAttack);
			}
			else {
				turnAction.type = TURN_SPELL;
				turnAction.action.name = spellName;
				turnAction.target = findEnemy(selected);
				Spell* spell = findSpell(spellName);
				destroyActionGUI();
				firstFrame = 0;
				return &turnAction;
			}
		}
		else {
			destroyEnemySelectionGUI();
			enemySelector = NULL;
			tguiSetFocus(txtSpell);
		}
	}
	else if (spells != 0 && widget == spells) {
		spells->getPressed(&selected, 0);
		destroySpellGUI();
		spellFrame = 0;
		spells = 0;
		PlayerStats* st = imaPartner ? &partnerStats : &stats;
		Spell* spell = (selected >= 0 && st->spells[selected]) ? findSpell(st->spells[selected]) : 0;
		if (selected < 0) {
			tguiSetFocus(txtAttack);
		}
		else if (spell == 0) {
		    playOgg("error.ogg");
		    tguiSetFocus(txtAttack);
		}
		else if (baseStats.mana < spell->getCost()) {
		    playOgg("error.ogg");
		    tguiSetFocus(txtAttack);
		    rest(500);
		}
		else {
			spellName = spell->getName();
			selectionReason = SPELL;
			if (spell->isForBattleOnly()) {
				createEnemySelectionGUI();
			}
			else {
				createPlayerSelectionGUI();
			}
		}
	}
	else if (inventory != 0 && widget == inventory) {
		inventory->getPressed(&selected, 0);
		destroyItemGUI();
		itemFrame = 0;
		inventory = 0;
		if (selected < 0) {
			tguiSetFocus(txtAttack);
		}
		else if (getItemType(stats.inventory[selected].id) != ITEM_TYPE_ITEM) {
			playOgg("error.ogg");
			tguiSetFocus(txtAttack);
		}
		else {
			if (itemAffectsAll(stats.inventory[selected].id)) {
				selectedPartyItem = true;
			}
			else {
				selectionReason = ITEM;
				createPlayerSelectionGUI();
			}
		}
	}
	else if ((playerSelector != 0 && selectionReason == ITEM && widget == playerSelector) || selectedPartyItem) {
		int sel = 0;
		if (!selectedPartyItem) {
			sel = dynamic_cast<MWgtSelector*>(widget)->getPosition();
			if (sel >= 0) {
				while (activeBattle->getCombatant(sel)->getModifiedStats()->hp <= 0)
					sel++;
			}
			destroyPlayerSelectionGUI();
			playerSelector = 0;
		}
		if (sel < 0 && !selectedPartyItem) {
			tguiSetFocus(txtAttack);
		}
		else {
			if (selectedPartyItem) {
				debug_message("Using party item...\n");
			}
			selectedPartyItem = false;
			turnAction.type = TURN_ITEM;
			turnAction.action.index = stats.inventory[selected].id;
			if (itemAffectsAll(stats.inventory[selected].id)) {
				turnAction.target = -1;
			}
			else {
				turnAction.target = findPlayer(sel);
			}
			stats.inventory[selected].quantity--;
			if (stats.inventory[selected].quantity <= 0) {
				stats.inventory[selected].quantity = 0;
				stats.inventory[selected].id = -1;
			}
			destroyActionGUI();
			firstFrame = 0;
			return &turnAction;
		}
	}
	else if (playerSelector != 0 && selectionReason == SPELL && widget == playerSelector) {
		if (playerSelector->getPosition() >= 0) {
			int sel = dynamic_cast<MWgtSelector*>(widget)->getPosition();
			while (activeBattle->getCombatant(sel)->getModifiedStats()->hp <= 0)
				sel++;
			destroyPlayerSelectionGUI();
			playerSelector = 0;
			turnAction.type = TURN_SPELL;
			turnAction.action.name = spellName;
			turnAction.target = findPlayer(sel);
			Spell* spell = findSpell(spellName);
			destroyActionGUI();
			firstFrame = 0;
			return &turnAction;
		}
		else {
			destroyPlayerSelectionGUI();
			playerSelector = NULL;
			tguiSetFocus(txtAttack);
		}
	}

	//gamepad2Keypresses(tguiCurrentTimeMillis() - start);

	start = tguiCurrentTimeMillis();

	return 0;
}

bool PlayerCombatant::isPlayer()
{
	return true;
}

void PlayerCombatant::createActionGUI()
{
	clear_keybuf();

	tguiSetParent(0);

	const int FIRST_FRAME_W = 65;
	const int FIRST_FRAME_H = 80;

	firstFrame = new MWgtFrame(0, 0, FIRST_FRAME_W, FIRST_FRAME_H, Battle::FRAME_COLOR, 255);
	txtAttack = new MWgtTextButton(10, 8, "Attack");
	txtSpell = new MWgtTextButton(10, 20, "Magic");
	txtItem = new MWgtTextButton(10, 32, "Item");
	txtSkip = new MWgtTextButton(10, 44, "Skip");
	txtRun = new MWgtTextButton(10, 56, "Run");

	tguiAddWidget(firstFrame);
	tguiSetParent(firstFrame);
	tguiAddWidget(txtAttack);
	tguiAddWidget(txtSpell);
	tguiAddWidget(txtItem);
	tguiAddWidget(txtSkip);
	tguiAddWidget(txtRun);

	tguiSetFocus(txtAttack);

	tguiTranslateWidget(firstFrame, 0, Screen::BUFFER_HEIGHT-FIRST_FRAME_H);
}

void PlayerCombatant::destroyActionGUI()
{
	tguiDeleteWidget(firstFrame);

	delete firstFrame;
	delete txtAttack;
	delete txtSpell;
	delete txtItem;
	delete txtSkip;
	delete txtRun;
}

void PlayerCombatant::createEnemySelectionGUI()
{
	clear_keybuf();

	tguiSetParent(0);

	std::list<Combatant*>& combatants = activeBattle->getCombatants();

	int numEnemies = combatants.size() - activeBattle->getNumberOfPlayerCombatants();

	std::list<Combatant*>::iterator it = combatants.begin();
	int current = 0;

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->isPlayer()) {
			it++;
			continue;
		}
		Point* p = new Point();
		p->x = c->getX() + c->getWidth();
		p->y = c->getY();
		enemyPoints.push_back(p);
		it++;
	}

	enemySelector = new MWgtSelector(&enemyPoints, true, true);

	tguiAddWidget(enemySelector);

	tguiSetFocus(enemySelector);
}

void PlayerCombatant::destroyEnemySelectionGUI()
{
	for (unsigned int i = 0; i < enemyPoints.size(); i++) {
		delete enemyPoints[i];
	}
	enemyPoints.clear();

	tguiDeleteWidget(enemySelector);

	delete enemySelector;
}

void PlayerCombatant::createSpellGUI()
{
	clear_keybuf();

	tguiSetParent(0);

	const int SPELL_FRAME_W = Screen::BUFFER_WIDTH;
	const int SPELL_FRAME_H = 80;

	spellFrame = new MWgtFrame(0, 0, SPELL_FRAME_W, SPELL_FRAME_H, Battle::FRAME_COLOR, 255);
	spells = new MWgtSpellSelector(10, 10, SPELL_FRAME_W-20, 2,
			4, false, 0, 0,
			imaPartner ? partnerStats.spells : stats.spells);

	tguiAddWidget(spellFrame);
	tguiSetParent(spellFrame);
	tguiAddWidget(spells);

	tguiSetFocus(spells);
	
	tguiTranslateWidget(spellFrame, 0, Screen::BUFFER_HEIGHT-SPELL_FRAME_H);
}

void PlayerCombatant::destroySpellGUI()
{
	tguiDeleteWidget(spellFrame);

	delete spellFrame;
	delete spells;
}

void PlayerCombatant::createItemGUI()
{
	clear_keybuf();

	tguiSetParent(0);

	const int ITEM_FRAME_W = Screen::BUFFER_WIDTH;
	const int ITEM_FRAME_H = 80;

	itemFrame = new MWgtFrame(0, 0, ITEM_FRAME_W, ITEM_FRAME_H, Battle::FRAME_COLOR, 255);
	inventory = new MWgtInventory(10, 10, ITEM_FRAME_W-20, 2,
			4, false, 0, 0, stats.inventory);

	tguiAddWidget(itemFrame);
	tguiSetParent(itemFrame);
	tguiAddWidget(inventory);

	tguiSetFocus(inventory);
	
	tguiTranslateWidget(itemFrame, 0, Screen::BUFFER_HEIGHT-ITEM_FRAME_H);
}

void PlayerCombatant::destroyItemGUI()
{
	tguiDeleteWidget(itemFrame);

	delete itemFrame;
	delete inventory;
}

void PlayerCombatant::createPlayerSelectionGUI()
{
	clear_keybuf();

	tguiSetParent(0);

	std::list<Combatant*>& combatants = activeBattle->getCombatants();

	int numPlayers = activeBattle->getNumberOfPlayerCombatants();

	for (int i = 0; i < numPlayers; i++) {
		Combatant* c = activeBattle->getCombatant(i);
		if (c->getModifiedStats()->hp > 0) {
			Point* p = new Point();
			p->x = c->getX()-10;
			p->y = c->getY();
			playerPoints.push_back(p);
		}
	}

	playerSelector = new MWgtSelector(&playerPoints, false, true);

	tguiAddWidget(playerSelector);

	tguiSetFocus(playerSelector);
}

void PlayerCombatant::destroyPlayerSelectionGUI()
{
	for (unsigned int i = 0; i < playerPoints.size(); i++) {
		delete playerPoints[i];
	}
	playerPoints.clear();

	tguiDeleteWidget(playerSelector);

	delete playerSelector;
}

int PlayerCombatant::findPlayer(int n)
{
	/*
	if (n == 0) {
		if (activeBattle->getCombatant(0)->getBaseStats()->hp <= 0)
			return 1;
		else
			return 0;
	}
	else {
		return 1;
	}
	*/
	std::list<Combatant*>& combatants = activeBattle->getCombatants();

	std::list<Combatant*>::iterator it = combatants.begin();

	int found = -1;

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->isPlayer()) {
			if (n == 0) {
				found = c->getNumber();
				break;
			}
			n--;
		}
		it++;
	}

	return found;
}

int PlayerCombatant::findEnemy(int n)
{
	std::list<Combatant*>& combatants = activeBattle->getCombatants();

	std::list<Combatant*>::iterator it = combatants.begin();

	int found = -1;

	while (it != combatants.end()) {
		Combatant* c = *it;
		if (c->isPlayer()) {
			it++;
			continue;
		}
		if (n == 0) {
			found = c->getNumber();
		}
		n--;
		it++;
	}

	return found;
}

int PlayerCombatant::getPower()
{
	int power;
	if (!strcmp(name, playerName)) {
		power = ::getPower(getLevel(stats.experience));
		power += getWeaponPower(stats.weapon);
	}
	else {
		power = ::getPower(getLevel(partnerStats.experience));
		power += getWeaponPower(partnerStats.weapon);
	}
	return power;
}

int PlayerCombatant::getWeaponDefense(int weapon)
{
	if (!imaPartner) {
		return getArmorDefense(stats.armor);
	}
	else {
		return getArmorDefense(partnerStats.armor);
	}
}

int PlayerCombatant::getSpellDefense(char *spell_name)
{
	return 0;
}

bool PlayerCombatant::isPartner()
{
	return imaPartner;
}

PlayerCombatant::PlayerCombatant(const char* name, bool imaPartner, int x, int y)
	:
	Combatant(name, x, y),

	imaPartner(imaPartner),
	firstFrame(0),
	itemFrame(0),
	inventory(0),
	spellFrame(0),
	spells(0),
	enemySelector(0),
	playerSelector(0),
	selectedPartyItem(false)
{
	if (imaPartner) {
		baseStats.hp = partnerStats.hp;
		baseStats.maxHP = partnerStats.maxHP;
		baseStats.latency = getPlayerLatency();
		baseStats.turnReadyCount = 0;
		baseStats.turnCount = 0;
		baseStats.mana = partnerStats.mana;
		baseStats.maxMana = partnerStats.maxMana;
		animSet = new AnimationSet(getResource("objects/%s.anims", partnerNames[partner]));
	}
	else {
		baseStats.hp = stats.hp;
		baseStats.maxHP = stats.maxHP;
		baseStats.latency = getPlayerLatency();
		baseStats.turnReadyCount = 0;
		baseStats.turnCount = 0;
		baseStats.mana = stats.mana;
		baseStats.maxMana = stats.maxMana;
		animSet = new AnimationSet(getResource("objects/%s.anims", playerName));
	}

	width = 16;
	height = 16;
	shadowHeight = 8;
	shadowOffset = height;

	animSet->setSubAnimation("stand_w");
}

PlayerCombatant::~PlayerCombatant()
{
}
