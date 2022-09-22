#include <allegro.h>
#include <sys/stat.h>

#ifdef ALLEGRO_WINDOWS
#define mkdir(a, b) mkdir(a)
#endif


#include "monster.h"

#include "menudat.h"

static DATAFILE* datafile;
static BITMAP* face_bg;
static BITMAP*coro_face;
static BITMAP** partner_faces;

int pickItemSmall()
{
	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();

	int bw = Screen::BUFFER_WIDTH;
	int bh = Screen::BUFFER_HEIGHT;

	const int NUM_ROWS = 3;

	int ih = mWgtFontHeight() * NUM_ROWS;	// inventory height
	int fh = ih + 20;			// frame height

	MWgtFrame* frame = new MWgtFrame(0, bh-fh, bw, fh, makecol(0, 96, 160),
			255);
	MWgtInventory* inventory = new MWgtInventory(10, 240-(ih+10), bw-20,
		2, 3, false, 0, 0, stats.inventory);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(inventory);
	tguiSetFocus(inventory);
	
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
			if (widget == inventory) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int p1, p2;
	inventory->getPressed(&p1, &p2);

	tguiDeleteWidget(frame);

	delete frame;
	delete inventory;

	return p1;
}

int pickItemBig(void (*callback)(int* pressed), int* selected, int* head)
{
	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();

	int bw = Screen::BUFFER_WIDTH;
	int bh = Screen::BUFFER_HEIGHT;

	const int NUM_ROWS = (bh - 20) / mWgtFontHeight();

	int ih = mWgtFontHeight() * NUM_ROWS;	// inventory height
	int fh = bh;				// frame height

	MWgtFrame* frame = new MWgtFrame(0, bh-fh, bw, fh, makecol(0, 96, 160),
			255);
	MWgtInventory* inventory = new MWgtInventory(10, 10, bw-20,
		2, NUM_ROWS, false, *selected, *head, stats.inventory);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(inventory);
	tguiSetFocus(inventory);

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
			if (widget == inventory) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int p1, p2;
	inventory->getPressed(&p1, &p2);

	if (callback)
		(*callback)(&p1);

	*selected = inventory->getSelected();
	*head = inventory->getHead();

	tguiDeleteWidget(frame);

	delete frame;
	delete inventory;

	return p1;
}

int pickSpellBig(void (*callback)(int* pressed, PlayerStats* st), int* selected, int* head, PlayerStats* st)
{
	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();

	int bw = Screen::BUFFER_WIDTH;
	int bh = Screen::BUFFER_HEIGHT;

	const int NUM_ROWS = (bh - 20) / mWgtFontHeight();

	int ih = mWgtFontHeight() * NUM_ROWS;	// inventory height
	int fh = bh;				// frame height

	MWgtFrame* frame = new MWgtFrame(0, bh-fh, bw, fh, makecol(0, 96, 160),
			255);
	MWgtSpellSelector* spells = new MWgtSpellSelector(10, 10, bw-20,
		2, NUM_ROWS, false, *selected, *head, st->spells);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(spells);
	tguiSetFocus(spells);

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
			if (widget == spells) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int p1, p2;
	spells->getPressed(&p1, &p2);

	*selected = spells->getSelected();
	*head = spells->getHead();

	if (callback)
		(*callback)(&p1, st);

	tguiDeleteWidget(frame);

	delete frame;
	delete spells;

	return p1;
}

void pick2Items(int* selected1, int* selected2, int* selected, int* head)
{
	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();

	int bw = Screen::BUFFER_WIDTH;
	int bh = Screen::BUFFER_HEIGHT;

	const int NUM_ROWS = (bh - 20) / mWgtFontHeight();

	int ih = mWgtFontHeight() * NUM_ROWS;	// inventory height
	int fh = bh;				// frame height

	MWgtFrame* frame = new MWgtFrame(0, bh-fh, bw, fh, makecol(0, 96, 160),
			255);
	MWgtInventory* inventory = new MWgtInventory(10, 10, bw-20,
		2, NUM_ROWS, true, *selected, *head, stats.inventory);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(inventory);
	tguiSetFocus(inventory);

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
			if (widget == inventory) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	inventory->getPressed(selected1, selected2);

	*selected = inventory->getSelected();
	*head = inventory->getHead();

	tguiDeleteWidget(frame);

	delete frame;
	delete inventory;
}

void pick2Spells(int* selected1, int* selected2, int* selected, int* head, char** spellNames)
{
	BITMAP* bb = scr->getBackBuffer();

	clear_keybuf();

	int bw = Screen::BUFFER_WIDTH;
	int bh = Screen::BUFFER_HEIGHT;

	const int NUM_ROWS = (bh - 20) / mWgtFontHeight();

	int ih = mWgtFontHeight() * NUM_ROWS;	// inventory height
	int fh = bh;				// frame height

	MWgtFrame* frame = new MWgtFrame(0, bh-fh, bw, fh, makecol(0, 96, 160),
			255);
	MWgtSpellSelector* spells = new MWgtSpellSelector(10, 10, bw-20,
		2, NUM_ROWS, true, *selected, *head, spellNames);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(spells);
	tguiSetFocus(spells);

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
			if (widget == spells) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	spells->getPressed(selected1, selected2);

	*selected = spells->getSelected();
	*head = spells->getHead();

	tguiDeleteWidget(frame);

	delete frame;
	delete spells;
}

void loadMenuData() throw (ReadError)
{
	datafile = load_datafile(getResource("menu.dat"));
	if (!datafile)
		throw ReadError();

	coro_face = (BITMAP*)datafile[CORO_FACE_PCX].dat;
	face_bg = (BITMAP*)datafile[FACE_BG_PCX].dat;

	partner_faces = new BITMAP*[NUM_PARTNERS];

	for (int i = 0; i < NUM_PARTNERS; i++) {
		partner_faces[i] = (BITMAP*)datafile[FACE_BG_PCX+i+1].dat;
	}
}

void destroyMenuData()
{
	unload_datafile(datafile);
	delete[] partner_faces;
}


void drawStats(BITMAP* bmp, char* name, BITMAP* face, 
		PlayerStats* stats, int x, int y, int bgColor, bool hasMana)
{
	int rectwidth = 180;
	int rectheight = 12*6+10;

	int rx = x - 2;
	int ry = y - 2;

	rect(bmp, rx, ry, rx+rectwidth, ry+rectheight, makecol(255, 255, 255));

	draw_sprite(bmp, face_bg, x+3, y+5);

	int offset = (face_bg->w - face->w) / 2;

	draw_sprite(bmp, face, x+3+offset, y+5+offset);

	x += face_bg->w + 8;

	char text[100];

	sprintf(text, "%s", name);

	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	sprintf(text, "LV %2d", getLevel(stats->experience)+1);

	mPrintf(bmp, x+85, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	y += 12;

	sprintf(text, "EXP");
	
	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	mPrintf(bmp, x+40, y, makecol(255, 255, 255), bgColor, false, true,
			"%d", stats->experience);

	y += 12;

	sprintf(text, "HP");
	
	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	mPrintf(bmp, x+40, y, makecol(255, 255, 255), bgColor, false, true,
			"%d/%d", stats->hp, stats->maxHP);

	y += 12;

	sprintf(text, "MP");
	
	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	if (hasMana) {
		mPrintf(bmp, x+40, y, makecol(255, 255, 255), bgColor, false, true,
				"%d/%d", stats->mana, stats->maxMana);
	}
	else {
		mPrintf(bmp, x+40, y, makecol(255, 255, 255), bgColor, false, true,
				"%d/%d", 0, 0);
	}

	y += 12;

	sprintf(text, "%s", getItemName(stats->weapon));
	
	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);

	y += 12;

	sprintf(text, "%s", getItemName(stats->armor));
	
	mPrintf(bmp, x, y, makecol(255, 255, 255), bgColor, false, true,
			text);
}

void drawMenu(BITMAP* bmp)
{
	int bg = makecol(0, 96, 160);

	drawFrame(bmp, 0, 0, Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT,
		bg, 200);

	int x = 15;
	int y = 15;

	drawStats(bmp, "Coro", coro_face, &stats, x, y, bg, true);

	if (partner >= 0) {
		if (partnerHasMana[partner])
			drawStats(bmp, partnerNames[partner], partner_faces[partner],
				&partnerStats, x, y+92, bg, true);
		else
			drawStats(bmp, partnerNames[partner], partner_faces[partner],
				&partnerStats, x, y+92, bg, false);
	}

	mPrintf(bmp, 15, Screen::BUFFER_HEIGHT-15-mWgtFontHeight(),
		makecol(255, 255, 255), bg, false,
		true, "%d gold", stats.gold);
}

void redraw()
{
	BITMAP* fxBuffer = scr->getFXBuffer();
	currArea->draw(fxBuffer);
	drawMenu(fxBuffer);
}

int inventoryPopup()
{
	// 225,15
	// 200, 28
	int ret = -1;
	
	MWgtFrame* frame = new MWgtFrame(0, 0, 70, mWgtFontHeight()*4+20,
			makecol(0, 96, 160), 255);
	MWgtTextButton* use = new MWgtTextButton(10, 10, "Use");
	MWgtTextButton* arrange = new MWgtTextButton(10, 10+mWgtFontHeight(),
			"Arrange");
	MWgtTextButton* drop = new MWgtTextButton(10, 10+mWgtFontHeight()*2,
			"Drop");
	MWgtTextButton* examine = new MWgtTextButton(10, 10+mWgtFontHeight()*3,
			"Examine");

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(use);
	tguiAddWidget(arrange);
	tguiAddWidget(drop);
	tguiAddWidget(examine);

	tguiTranslateWidget(frame, 221, 32);
	tguiSetFocus(use);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == use) {
				ret = 0;
				break;
			}
			else if (widget == arrange) {
				ret = 1;
				break;
			}
			else if (widget == drop) {
				ret = 2;
				break;
			}
			else if (widget == examine) {
				ret = 3;
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			playOgg("button.ogg");
			break;
		}
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);


	delete frame;
	delete use;
	delete arrange;
	delete drop;

	return ret;
}

int magicPopup()
{
	// 225,15
	// 200, 28
	int ret = -1;
	
	MWgtFrame* frame = new MWgtFrame(0, 0, 70, mWgtFontHeight()*2+20,
			makecol(0, 96, 160), 255);
	MWgtTextButton* use = new MWgtTextButton(10, 10, "Cast");
	MWgtTextButton* arrange = new MWgtTextButton(10, 10+mWgtFontHeight(),
			"Arrange");

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(use);
	tguiAddWidget(arrange);

	tguiTranslateWidget(frame, 221, 47);
	tguiSetFocus(use);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == use) {
				ret = 0;
				break;
			}
			else if (widget == arrange) {
				ret = 1;
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			playOgg("button.ogg");
			break;
		}
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);


	delete frame;
	delete use;
	delete arrange;

	return ret;
}

int unequipPopup()
{
	// 225,15
	// 200, 28
	int ret = -1;
	
	MWgtFrame* frame = new MWgtFrame(0, 0, 70, mWgtFontHeight()*2+20,
			makecol(0, 96, 160), 255);
	MWgtTextButton* weapon = new MWgtTextButton(10, 10, "Weapon");
	MWgtTextButton* armor = new MWgtTextButton(10, 10+mWgtFontHeight(),
			"Armor");

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(weapon);
	tguiAddWidget(armor);

	tguiTranslateWidget(frame, 221, 32+45);
	tguiSetFocus(weapon);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == weapon) {
				ret = 0;
				break;
			}
			else if (widget == armor) {
				ret = 1;
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			playOgg("button.ogg");
			break;
		}
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);


	delete frame;
	delete weapon; 
	delete armor;

	return ret;
}

void dropPromptCallback(int* pressed)
{
	if (stats.inventory[*pressed].id < 0) {
		notify("", "You can't drop that...", "");
		*pressed = -1;
	}
	else if (getItemType(stats.inventory[*pressed].id) == ITEM_TYPE_SPECIAL) {
		notify("", "You better hold on to that...", "");
		*pressed = -1;
	}
	else if (*pressed >= 0 && !prompt(false, "Do you really want to", "drop this item?", ""))
		*pressed = -1;
}

void pickEquipCallback(int* pressed)
{
	if (*pressed < 0)
		; // do nothing
	else if (stats.inventory[*pressed].id < 0) {
		notify("", "You can't equip nothing...", "");
		*pressed = -1;
	}
	else if (stats.inventory[*pressed].quantity > 1 && firstInventorySlotAvailable() < 0) {
		notify("", "You must drop something first...", "");
		*pressed = -1;
	}
	else {
		int type = getItemType(stats.inventory[*pressed].id);
		if (type != ITEM_TYPE_WEAPON && type != ITEM_TYPE_ARMOR) {
			notify("That item is not", "a weapon or armor...", "");
			*pressed = -1;
		}
	}
}

void useItemCallback(int* pressed)
{
	if (*pressed < 0) {
			return;
	}
	if (getItemType(stats.inventory[*pressed].id) == ITEM_TYPE_SPECIAL) {
		notify("", "You can't use that now...", "");
		*pressed = -1;
	}
	else if (*pressed >= 0 && getItemType(stats.inventory[*pressed].id) != ITEM_TYPE_ITEM) {
		notify("", "You can't use that...", "");
		*pressed = -1;
	}
	// FIXME: is it's a tent, increase all stats and play sample
}

void examineItemCallback(int* pressed)
{
	/* Do nothing */
}

void castSpellCallback(int* pressed, PlayerStats* st)
{
	if (*pressed < 0) {
			return;
	}
	if (st->spells[*pressed] == 0) {
			*pressed = -1;
	}
	else if (spellIsForBattleOnly(st->spells[*pressed])) {
		notify("", "You can't cast that now...", "");
		*pressed = -1;
	}
	else if (getSpellCost(st->spells[*pressed]) > st->mana) {
		notify("You don't have enough mana", "to cast that...", "");
		*pressed = -1;
	}
	// FIXME: is it's a tent, increase all stats and play sample
}

bool pauseGame()
{
    bool ret = true;
    int selectedItem=0;
    int selectedSpell=0;
    enum SelectionReason {
		NONE=0,
		EQUIP,
		USE,
		UNEQUIP_WEAPON,
		UNEQUIP_ARMOR,
		CASTER,
		SPELL,
		STATS
    };
    SelectionReason selectionReason = NONE;
    PlayerStats* caster=0;

    BITMAP* fxBuffer = scr->getFXBuffer();
    redraw();

    BITMAP* bb = scr->getBackBuffer();

/*
    for (int i = 0; i < 250; i++) {
	    updateMusic(); rest(1);
    }
    */
    waitForRelease();
    clear_keybuf();

    MWgtTextButton* magic = new MWgtTextButton(225, 15, "Magic");
    MWgtTextButton* inventory = new MWgtTextButton(225, 30, "Inventory");
    MWgtTextButton* equip = new MWgtTextButton(225, 45, "Equip");
    MWgtTextButton* unequip = new MWgtTextButton(225, 60, "Un-Equip");
    MWgtTextButton* playerstats = new MWgtTextButton(225, 75, "Stats");
    MWgtTextButton* save = new MWgtTextButton(225, 90, "Save");
    MWgtTextButton* resume = new MWgtTextButton(225, 105, "Resume");
    MWgtTextButton* quit = new MWgtTextButton(225, 120, "Quit");

    std::vector<Point*> positions;

    Point pos1 = { 200, 55 };
    Point pos2 = { 200, 145 };

    positions.push_back(&pos1);

    if (partner >= 0)
	    positions.push_back(&pos2);

    MWgtSelector* selector = new MWgtSelector(&positions, true, false);

	tguiSetParent(0);
	tguiAddWidget(magic);
	tguiAddWidget(inventory);
	tguiAddWidget(equip);
	tguiAddWidget(unequip);
	tguiAddWidget(playerstats);
	tguiAddWidget(save);
	tguiAddWidget(resume);
	tguiAddWidget(quit);
	tguiSetFocus(magic);

    bool done = false;
    long start = tguiCurrentTimeMillis();

    while (!done) {
	    updateMusic();

	    bool update;
	    if (tguiScreenIsDirty()) {
		    tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
		    blit(fxBuffer, bb, 0, 0, 0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
		    tguiClearDirtyRectangle();
		    scr->draw();
		    update = true;
	    }
	    else
		    update = false;
	    if (keypressed() || update) {
		    TGUIWidget* widget = tguiUpdate();
		    if (widget == selector) {
		    		waitForRelease();
				/*
				    rest(250);
				    */
				    clear_keybuf();
		    }
		    if (widget == quit) {
			    if (prompt(false, "Do you really want to quit?", "Unsaved data will be lost...", "")) {
				    ret = false;
				    break;
			    }
			    else {
				    redraw();
				    tguiSetFocus(quit);
			    }
		    }
		    else if (widget == inventory) {
			    int n = inventoryPopup();
			    if (n == 0) {
				    int selected = 0;
				    int head = 0;
				    selectedItem = pickItemBig(useItemCallback, &selected, &head);
				    /*
				    if (selectedItem >= 0 && getItemType(stats.inventory[selectedItem].id) != ITEM_TYPE_ITEM) {
					    playOgg("error.ogg");
					    notify("You can't use that...", "", "");
				    }
				    */
				    if (selectedItem >= 0 && stats.inventory[selectedItem].id >= 0) {
					    if (itemAffectsAll(stats.inventory[selectedItem].id)) {
						    playOgg(getItemSound(stats.inventory[selectedItem].id));
						    ItemEffects* ie = getItemEffects(stats.inventory[selectedItem].id);
						    applyItemEffects(ie, &stats, 0);
						    if (partner >= 0) {
							    applyItemEffects(ie, &partnerStats, 0);
						    }
						    deleteItemEffects(ie);
						    stats.inventory[selectedItem].quantity--;
						    if (stats.inventory[selectedItem].quantity <= 0) {
							    stats.inventory[selectedItem].id = -1;
							    stats.inventory[selectedItem].quantity = 0;
						    }
						    tguiSetFocus(inventory);
						    redraw();
					    }
					    else {
						    selectionReason = USE;
						    selector->setPosition(0);
						    tguiSetParent(0);
						    tguiAddWidget(selector);
						    tguiSetFocus(selector);
					    }
					    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
				    }
				    else if (selectedItem >= 0 && stats.inventory[selectedItem].id < 0) {
					    playOgg("error.ogg");
				    }
				    else {
					    tguiSetFocus(inventory);
					    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
				    }
			    }
			    else if (n == 3) {
				int selected = 0;
				int head = 0;
				selectedItem = pickItemBig(examineItemCallback, &selected, &head);
				if (selectedItem >= 0 && stats.inventory[selectedItem].id >= 0) {
					showItemDescription(stats.inventory[selectedItem].id);
				        tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
				}
			        else if (selectedItem >= 0 && stats.inventory[selectedItem].id < 0) {
		   		    playOgg("error.ogg");
				    tguiSetFocus(inventory);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			        }
			        else {
				    tguiSetFocus(inventory);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			        }
			    }
			    else {
				    if (n == 1) {
					    int i1;
					    int i2;
					    int selected = 0;
					    int head = 0;
					    for (;;) {
						    pick2Items(&i1, &i2, &head, &selected);
						    if (i1 < 0)
							    break;
						    Item tmp;
						    tmp.id = stats.inventory[i1].id;
						    tmp.quantity = stats.inventory[i1].quantity;
						    stats.inventory[i1].id = stats.inventory[i2].id;
						    stats.inventory[i1].quantity = stats.inventory[i2].quantity;
						    stats.inventory[i2].id = tmp.id;
						    stats.inventory[i2].quantity = tmp.quantity;
					    }
				    }
				    else if (n == 2) {
					    int selected = 0;
					    int head = 0;
					    for (;;) {
						    n = pickItemBig(dropPromptCallback, &selected, &head);
						    if (n >= 0) {
							    stats.inventory[n].quantity--;
							    if (stats.inventory[n].quantity <= 0) {
								    stats.inventory[n].id = -1;
								    stats.inventory[n].quantity = 0;
							    }
						    }
						    else
							    break;
					    }
				    }
				    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
				    tguiSetFocus(inventory);
			    }
		    }
		    else if (widget == magic) {
			    selectionReason = CASTER;
			    selector->setPosition(0);
			    tguiSetParent(0);
			    tguiAddWidget(selector);
			    tguiSetFocus(selector);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
		    }
		    else if (widget == equip) {
			    int selected = 0;
			    int head = 0;
			    selectedItem = pickItemBig(pickEquipCallback, &selected, &head);
			    if (selectedItem >= 0) {
				    selectionReason = EQUIP;
				    selector->setPosition(0);
				    tguiSetParent(0);
				    tguiAddWidget(selector);
				    tguiSetFocus(selector);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			    }
			    else {
				    tguiSetFocus(equip);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			    }
		    }
		    else if (widget == playerstats) {
			    selectionReason = STATS;
			    selector->setPosition(0);
			    tguiSetParent(0);
			    tguiAddWidget(selector);
			    tguiSetFocus(selector);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
		    }
		    else if (widget == unequip) {
			    int n = unequipPopup();
			    if (n < 0) {
				    tguiSetFocus(unequip);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			    }
			    else if (n == 0) {
				    selectionReason = UNEQUIP_WEAPON;
				    selector->setPosition(0);
				    tguiSetParent(0);
				    tguiAddWidget(selector);
				    tguiSetFocus(selector);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			    }
			    else {
				    selectionReason = UNEQUIP_ARMOR;
				    selector->setPosition(0);
				    tguiSetParent(0);
				    tguiAddWidget(selector);
				    tguiSetFocus(selector);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
			    }
		    }
		    else if (widget == selector && selectionReason == EQUIP) {
			    while (keypressed()) readkey();
			    tguiDeleteWidget(selector);
			    int type = getItemType(stats.inventory[selectedItem].id);
			    Item tmp;
			    int pos = selector->getPosition();
			    if (pos >= 0) {
					    PlayerStats* s;
					    if (pos == 0)
						    s = &stats;
					    else
						    s = &partnerStats;
					    int* weapon_or_armor;
					    if (type == ITEM_TYPE_WEAPON) {
						    weapon_or_armor = &s->weapon;
					    }
					    else {
						    weapon_or_armor = &s->armor;
					    }
					    tmp.id = *weapon_or_armor;
					    tmp.quantity = 1;
					    *weapon_or_armor = stats.inventory[selectedItem].id;
					    /*
					    if (stats.inventory[selectedItem].quantity > 1 && firstInventorySlotAvailable() < 0) {
					    }
					    */
					    stats.inventory[selectedItem].quantity--;
					    if (stats.inventory[selectedItem].quantity <= 0) {
						    stats.inventory[selectedItem].id = -1;
						    stats.inventory[selectedItem].quantity = 0;
					    }
					    if (tmp.id < 0) {
						    // do nothing
					    }
					    else if (stats.inventory[selectedItem].quantity <= 0) {
						    int slot = firstInventorySlotAvailable();
						    stats.inventory[slot].id = tmp.id;
						    stats.inventory[slot].quantity = tmp.quantity;
					    }
					    else {
						    stats.inventory[selectedItem].id = tmp.id;
						    stats.inventory[selectedItem].quantity = tmp.quantity;
					    }
			    }
			    tguiSetFocus(equip);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			    redraw();
		    }
		    else if (widget == selector && selectionReason == USE) {
			    tguiDeleteWidget(selector);
			    int pos = selector->getPosition();
			    if (pos >= 0) {
					    playOgg(getItemSound(stats.inventory[selectedItem].id));
					    PlayerStats* s;
					    if (pos == 0)
						    s = &stats;
					    else
						    s = &partnerStats;
					    if (!strcmp(getItemName(stats.inventory[selectedItem].id), "Scroll") && pos != 0) {
						    char msg[100];
						    sprintf(msg, "%s can't use that...", partnerNames[partner]);
						    notify("", msg, "");
					    }
					    else {
						    ItemEffects* ie = getItemEffects(stats.inventory[selectedItem].id);
						    applyItemEffects(ie, s, 0);
						    deleteItemEffects(ie);
						    stats.inventory[selectedItem].quantity--;
						    if (stats.inventory[selectedItem].quantity <= 0) {
							    stats.inventory[selectedItem].id = -1;
							    stats.inventory[selectedItem].quantity = 0;
						    }
					    }
			    }
			    tguiSetFocus(inventory);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			    redraw();
		    }
		    else if (widget == selector && selectionReason == UNEQUIP_WEAPON) {
			    tguiDeleteWidget(selector);
			    int pos = selector->getPosition();
			    if (pos >= 0) {
				    int slot = firstInventorySlotAvailable();
				    if (slot < 0) {
					    notify("You inventory is full...", "Drop or use an item", "to proceed");
				    }
				    else {
					    PlayerStats* s;
					    if (pos == 0)
						    s = &stats;
					    else
						    s = &partnerStats;
					    stats.inventory[slot].id = s->weapon;
					    stats.inventory[slot].quantity = 1;
					    s->weapon = -1;
				    }
			    }
			    tguiSetFocus(unequip);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			    redraw();
		    }
		    else if (widget == selector && selectionReason == UNEQUIP_ARMOR) {
			    tguiDeleteWidget(selector);
			    int pos = selector->getPosition();
			    if (pos >= 0) {
				    int slot = firstInventorySlotAvailable();
				    if (slot < 0) {
					    notify("You inventory is full...", "Drop or use an item", "to proceed");
				    }
				    else {
					    PlayerStats* s;
					    if (pos == 0)
						    s = &stats;
					    else
						    s = &partnerStats;
					    stats.inventory[slot].id = s->armor;
					    stats.inventory[slot].quantity = 1;
					    s->armor = -1;
				    }
			    }
			    tguiSetFocus(unequip);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			    redraw();
		    }
		    else if (widget == selector && selectionReason == CASTER) {
			    int p = selector->getPosition();
			    tguiDeleteWidget(selector);
			    if (p >= 0) {
					    char** spellNames;
					    bool hasMana = true;
					    if (p == 0) {
							    caster = &stats;
							    spellNames = stats.spells;
					    }
					    else {
					    		if (!partnerHasMana[partner]) {
								hasMana = false;
							}
							else {
								caster = &partnerStats;
								spellNames = partnerStats.spells;
							}
					    }
					    if (hasMana) {
						    int n = magicPopup();
						    if (n == 0) {
							    int selected = 0;
							    int head = 0;
							    selectedSpell = pickSpellBig(castSpellCallback, &selected, &head, caster);
							    if (selectedSpell >= 0 && getSpellCost(spellNames[selectedSpell]) <= caster->mana) {
								    selectionReason = SPELL;
								    selector->setPosition(0);
								    tguiSetParent(0);
								    tguiAddWidget(selector);
								    tguiSetFocus(selector);
								    tguiUpdateDirtyRectangle(0, 0,
									    Screen::BUFFER_WIDTH,
									    Screen::BUFFER_HEIGHT);
							    }
							    else {
								    tguiSetFocus(magic);
								    tguiUpdateDirtyRectangle(0, 0,
									    Screen::BUFFER_WIDTH,
									    Screen::BUFFER_HEIGHT);
							    }
						    }
						    else if (n == 1) {
								    int i1;
								    int i2;
								    int selected = 0;
								    int head = 0;
								    for (;;) {
									    pick2Spells(&i1, &i2, &head, &selected, spellNames);
									    if (i1 < 0) {
										    tguiUpdateDirtyRectangle(0, 0,
											    Screen::BUFFER_WIDTH,
											    Screen::BUFFER_HEIGHT);
											    tguiSetFocus(magic);
											    break;
									    }
									    char* tmp;
									    tmp = caster->spells[i1];
									    caster->spells[i1] = caster->spells[i2];
									    caster->spells[i2] = tmp;
								    }
						    }
						    else {
								    tguiUpdateDirtyRectangle(0, 0,
									    Screen::BUFFER_WIDTH,
									    Screen::BUFFER_HEIGHT);
									    tguiSetFocus(magic);
						    }
					}
					else {
						playOgg("error.ogg");
						 tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
						    tguiSetFocus(magic);
					}
						    /*
						    else if (n == 2) {
							    int selected = 0;
							    int head = 0;
							    for (;;) {
								    n = pickItemBig(dropPromptCallback, &selected, &head);
								    if (n >= 0) {
									    stats.inventory[n].quantity--;
									    if (stats.inventory[n].quantity <= 0) {
										    stats.inventory[n].id = -1;
										    stats.inventory[n].quantity = 0;
									    }
								    }
								    else
									    break;
							    }
						    }
						    */
			    }
			    else {
					    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
						    tguiSetFocus(magic);
			    }
		    }
		    else if (widget == selector && selectionReason == SPELL) {
				    int pos = selector->getPosition();
				    tguiDeleteWidget(selector);
				    if (pos >= 0) {
						    PlayerStats* st;
						    if (pos == 0) {
								    st = &stats;
						    }
						    else {
								    st = &partnerStats;
						    }
						    Spell* s = findSpell(caster->spells[selectedSpell]);
						    if (s) {
						    	bool isDamage;
								    s->apply(st, 0, 0, isDamage, true);
									caster->mana -= s->getCost();
						    }
				    }
				    tguiSetFocus(magic);
				    tguiUpdateDirtyRectangle(0, 0,
					    Screen::BUFFER_WIDTH,
					    Screen::BUFFER_HEIGHT);
				    redraw();
		    }
		    else if (widget == selector && selectionReason == STATS) {
		    	    waitForRelease();
			    tguiDeleteWidget(selector);
			    int pos = selector->getPosition();
				PlayerStats* s;
				char *lines[16];
				if (pos == 0) {
					s = &stats;
				}
				else if (pos == 1) {
					s = &partnerStats;
				}
				else
					goto nostats;
				for (int i = 0; i < 16; i++) {
					if (i < 6) {
						lines[i] = new char[100];
					}
					else {
						lines[i] = NULL;
					}
				}
				snprintf(lines[0], 99, "HP: %d/%d", s->hp, s->maxHP);
				if (partnerHasMana[partner] || pos == 0)
					snprintf(lines[1], 99, "Mana: %d/%d", s->mana, s->maxMana);
				else
					strcpy(lines[1], "Mana: 0/0");
				snprintf(lines[2], 99, "Power: %d", getPower(getLevel(s->experience)));
				snprintf(lines[3], 99, "Level: %d", getLevel(s->experience)+1);
				snprintf(lines[4], 99, "Experience: %d", s->experience);
				if (s->experience >= 99999)
					snprintf(lines[5], 99, "-");
				else
					snprintf(lines[5], 99, "Next Level: %d", (ExperienceForLevel[getLevel(s->experience)+1]-ExperienceForLevel[getLevel(s->experience)])-((s->experience)-(ExperienceForLevel[getLevel(s->experience)])));
				jumboNotify(lines);
				for (int i = 0; i < 16; i++) {
					if (lines[i])
						delete[] lines[i];
				}
			    nostats:
			    tguiSetFocus(playerstats);
			    tguiUpdateDirtyRectangle(0, 0,
				    Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			    redraw();
		    }
		    else if (widget == save) {
			    lua_State* luaState = currArea->getLuaState();
			    lua_getglobal(luaState, "can_save");
			    if (lua_isboolean(luaState, -1) && lua_toboolean(luaState, -1)) {
				    scr->fadeOut();
				    int n = selectSaveState("Save Game", "Are you sure you want to overwrite", "this save state?", "", true);
				    if (n >= 0) {
					    saveGame(getUserResource("save/%d.save", n+1));
				    }
				    scr->fadeOut();
				    redraw();
				    tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
				    blit(fxBuffer, bb, 0, 0, 0, 0, Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
				    tguiClearDirtyRectangle();
				    blit(bb, fxBuffer, 0, 0, 0, 0, Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT);
				    scr->fadeIn();
				    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
			    }
			    else {
				    TGUIWidget* focus = tguiGetFocus();
				    notify("", "You can't save here...", "");
				    tguiSetFocus(focus);
				    tguiUpdateDirtyRectangle(0, 0,
						    Screen::BUFFER_WIDTH,
						    Screen::BUFFER_HEIGHT);
			    }
		    }
		    else if (widget == resume || (tguiGetFocus() != selector && escapePressed())) {
			    done = true;
		    }
		    while (keypressed())
			    readkey();
	    }
	    long end = tguiCurrentTimeMillis();
	    long duration = end - start;
	    start = end;
	    int timeStep = 80;
	    gamepad2Keypresses(duration);
	    if (duration < timeStep)
		    rest(timeStep-duration);
    }

	/*
    for (int i = 0; i < 250; i++) {
	    updateMusic(); rest(1);
    }
    */
    waitForRelease();
    clear_keybuf();

	tguiDeleteWidget(inventory);
	tguiDeleteWidget(magic);
	tguiDeleteWidget(equip);
	tguiDeleteWidget(unequip);
	tguiDeleteWidget(playerstats);
	tguiDeleteWidget(save);
	tguiDeleteWidget(resume);
	tguiDeleteWidget(quit);

	delete inventory;
	delete equip;
	delete unequip;
	delete magic;
	delete save;
	delete resume;
	delete quit;

	positions.clear();
	delete selector;

	currArea->draw(bb);
	scr->draw();

	return ret;
}

void notify(char* s1, char* s2, char* s3)
{
	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int color = makecol(255, 255, 255);

	int frameHeight = 20 + (mWgtFontHeight()+3)*3 + 20;

	MWgtFrame* frame = new MWgtFrame(0, 0, 300, frameHeight,
			makecol(96, 0, 160), 255);
	MWgtText* line1 = new MWgtText(0, 0, s1, color);
	MWgtText* line2 = new MWgtText(0, 0, s2, color);
	MWgtText* line3 = new MWgtText(0, 0, s3, color);
	MWgtButton* button = new MWgtButton(0, 0, -1, "OK");
	
	int halfFontHeight = mWgtFontHeight() / 2;

	tguiCenterWidget(line1, 150, 10+halfFontHeight/2);
	tguiCenterWidget(line2, 150, 10+mWgtFontHeight()+3+halfFontHeight/2);
	tguiCenterWidget(line3, 150, 10+(mWgtFontHeight()+3)*2+halfFontHeight/2);
	tguiCenterWidget(button, 150, 12+(mWgtFontHeight()+3)*3+button->getHeight()/2);

	tguiPush();

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(line1);
	tguiAddWidget(line2);
	tguiAddWidget(line3);
	tguiAddWidget(button);
	tguiSetFocus(button);

	tguiTranslateWidget(frame, (Screen::BUFFER_WIDTH-300)/2,
			(Screen::BUFFER_HEIGHT-85)/2);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == button) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed())
			break;
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);

	delete frame;
	delete line1;
	delete line2;
	delete line3;
	delete button;
	
	tguiPop();
}

void jumboNotify(char *s[16])
{
	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int color = makecol(255, 255, 255);

	int fw = Screen::BUFFER_WIDTH;
	int fh = Screen::BUFFER_HEIGHT;

	MWgtFrame* frame = new MWgtFrame(0, 0, fw, fh,
			makecol(0, 96, 160), 255);
	MWgtText* lines[16];

	for (int i = 0; i < 16; i++) {
		lines[i] = NULL;
	}
	for (int i = 0; i < 16; i++) {
		if (!s[i])
			break;
		lines[i] = new MWgtText(0, 0, s[i], color);
		tguiCenterWidget(lines[i], fw/2, 15+(i*12)+(mWgtFontHeight()/2));
	}
	
	MWgtButton* button = new MWgtButton(0, 0, -1, "OK");
	tguiCenterWidget(button, fw/2, fh-20);

	tguiPush();

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);

	for (int i = 0; i < 16; i++) {
		if (lines[i]) {
			tguiAddWidget(lines[i]);
		}
		else	
			break;
	}

	tguiAddWidget(button);
	tguiSetFocus(button);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == button) {
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed())
			break;
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);

	delete frame;

	for (int i = 0; i < 16; i++) {
		if (lines[i])
			delete lines[i];
		else
			break;
	}
	delete button;
	
	tguiPop();
}

bool prompt(bool dfault, char* s1, char* s2, char* s3)
{
	bool ret = false;

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int color = makecol(255, 255, 255);

	int frameHeight = 20 + (mWgtFontHeight()+3)*3 + 20;

	MWgtFrame* frame = new MWgtFrame(0, 0, 300, frameHeight,
			makecol(96, 0, 160), 255);
	MWgtText* line1 = new MWgtText(0, 0, s1, color);
	MWgtText* line2 = new MWgtText(0, 0, s2, color);
	MWgtText* line3 = new MWgtText(0, 0, s3, color);
	MWgtButton* button1 = new MWgtButton(0, 0, -1, "Yes");
	MWgtButton* button2 = new MWgtButton(0, 0, -1, "No");
	
	int halfFontHeight = mWgtFontHeight() / 2;

	tguiCenterWidget(line1, 150, 10+halfFontHeight/2);
	tguiCenterWidget(line2, 150, 10+mWgtFontHeight()+3+halfFontHeight/2);
	tguiCenterWidget(line3, 150, 10+(mWgtFontHeight()+3)*2+halfFontHeight/2);
	tguiCenterWidget(button1, 75, 12+(mWgtFontHeight()+3)*3+button1->getHeight()/2);
	tguiCenterWidget(button2, 225, 12+(mWgtFontHeight()+3)*3+button2->getHeight()/2);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	tguiAddWidget(line1);
	tguiAddWidget(line2);
	tguiAddWidget(line3);
	tguiAddWidget(button1);
	tguiAddWidget(button2);

	if (dfault)
		tguiSetFocus(button1);
	else
		tguiSetFocus(button2);

	tguiTranslateWidget(frame, (Screen::BUFFER_WIDTH-300)/2,
			(Screen::BUFFER_HEIGHT-85)/2);

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		bool update;
		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == button1) {
				ret = true;
				break;
			}
			else if (widget == button2) {
				ret = false;
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed())
			break;
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(frame);

	delete frame;
	delete line1;
	delete line2;
	delete line3;
	delete button1;
	delete button2;

	return ret;
}

struct Preview {
	BITMAP* partnerFace;
	int coroExp;
	int partnerExp;
};

static Preview previews[3];

void readPreview(char* ss, Preview* preview)
{
	PACKFILE* f = pack_fopen(ss, "rp");
	if (!f)
		throw ReadError();

	// Skip unwanted data

	try {
		for (int i = 0; i < MAX_MILESTONES/8; i++)
			my_pack_getc(f); // milestones

		readString(f);		// area name

		igetl(f);		// x
		igetl(f);		// y
		igetl(f);		// direction

		PlayerStats coro_stats;

		readStats(&coro_stats, f); 

		for (int i = 0; i < MAX_INVENTORY; i++)
			readItem(0, f);

		int partner = igetl(f);

		if (partner == 9999) {
			char *s;
			for (int i = 0; i < MAX_SPELLS; i++) {
				s = readString(f);
			}
			partner = igetl(f);
		}

		PlayerStats partnerStats;

		readStats(&partnerStats, f);

		// FIXME: set partner icon
		
		preview->coroExp = coro_stats.experience;
		preview->partnerExp = partnerStats.experience;
		if (partner >= 0) {
			preview->partnerFace = partner_faces[partner];
		}
		else {
			preview->partnerFace = 0;
		}
	}
	catch (...) {
		pack_fclose(f);
		throw ReadError();
	}
		
	pack_fclose(f);
}

bool createSaveState(char* filename)
{
	PACKFILE* f = pack_fopen(filename, "wp");
	if (!f)
		return false;

	try {
		for (int i = 0; i < MAX_MILESTONES/8; i++)
			my_pack_putc(0, f);

		writeString("start", f);

		iputl(0, f); // x
		iputl(0, f); // y
		iputl(0, f); // direction

		PlayerStats coroStats;
		PlayerStats partnerStats;

		clearStats(&coroStats);
		clearStats(&partnerStats);

		writeStats(&coroStats, f);

		for (int i = 0; i < MAX_INVENTORY; i++) {
			writeItem(&coroStats.inventory[i], f);
		}

		/* Hack that can be removed at some point */
		iputl(9999, f);
		for (int i = 0; i < MAX_SPELLS; i++) {
			writeString("", f);
		}
		iputl(-1, f);

		writeStats(&partnerStats, f);

		iputl(0, f); // accumulated distance
	}
	catch (...) {
		pack_fclose(f);
		return false;
	}

	pack_fclose(f);

	return true;
}

bool readPreviews()
{
	for (int i = 0; i < 3; i++) {
		if (exists(getUserResource("save/%d.save", i+1))) {
			try {
				readPreview(getUserResource("save/%d.save", i+1), &previews[i]);
			}
			catch (...) {
				return false;
			}
		}
		else {
			if (!createSaveState(getUserResource("save/%d.save", i+1))) {
				debug_message("Error creating empty save state.\n");
				return false;
			}
			previews[i].partnerFace = 0;
			previews[i].coroExp = 0;
			previews[i].partnerExp = -1;
		}
	}

	return true;
}

void drawPreview(BITMAP* bmp, Preview* preview, int x, int y)
{
	int bg = makecol(0, 96, 160);
	drawFrame(bmp, x, y, 256, 70, bg, 255);
	int offset = (face_bg->w-coro_face->w)/2;
	draw_sprite(bmp, face_bg, x+64-face_bg->w/2, y+10);
	draw_sprite(bmp, coro_face, x+64-face_bg->w/2+offset, y+10+offset);
	mPrintf(bmp, x+64, y+47, makecol(255, 255, 255), bg, true,
		true, "EXP %d", preview->coroExp);
	if (preview->partnerFace) {
		draw_sprite(bmp, face_bg, x+192-face_bg->w/2,
				y+10);
		draw_sprite(bmp, preview->partnerFace, x+192-face_bg->w/2+offset,
				y+10+offset);
		mPrintf(bmp, x+192, y+47, makecol(255, 255, 255), bg, true,
				true, "EXP %d", preview->partnerExp);
	}
}

void drawPreviews(BITMAP* bmp, int y1, int y2, int y3)
{
	drawPreview(bmp, &previews[0], (Screen::BUFFER_WIDTH-256)/2, y1);
	drawPreview(bmp, &previews[1], (Screen::BUFFER_WIDTH-256)/2, y2);
	drawPreview(bmp, &previews[2], (Screen::BUFFER_WIDTH-256)/2, y3);
}

int selectSaveState(char* caption, char* s1, char* s2, char* s3, bool def)
{
	int ret = -1;

	char userDir[1000];
	sprintf(userDir, "%s", getUserResource(""));

#if !defined __linux__ && !defined ALLEGRO_MACOSX
	userDir[strlen(userDir)-1] = 0;
#endif

	mkdir(userDir, 0770);
	mkdir(getUserResource("save"), 0770);

	if (!readPreviews()) {
		notify("", "Could not read save states", "");
		return -1;
	}

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	int y1 = Screen::BUFFER_HEIGHT-70*3-8;
	int y2 = y1 + 70;
	int y3 = y2 + 70;

	int sy1 = y1 + 70/2-8;
	int sy2 = y2 + 70/2-8;
	int sy3 = y3 + 70/2-8;

	Point point1 = { Screen::BUFFER_WIDTH/2-128-16, sy1 };
	Point point2 = { Screen::BUFFER_WIDTH/2-128-16, sy2 };
	Point point3 = { Screen::BUFFER_WIDTH/2-128-16, sy3 };
	
	std::vector<Point*> positions;

	positions.push_back(&point1);
	positions.push_back(&point2);
	positions.push_back(&point3);

	MWgtSelector* selector = new MWgtSelector(&positions, false, false);

	tguiPush();

	tguiSetParent(0);
	selector->setPosition(0);
	tguiAddWidget(selector);
	tguiSetFocus(selector);

	BITMAP* bb = scr->getBackBuffer();

	clear(bb);

	drawPreviews(bb, y1, y2, y3);
	mPrintf(bb, Screen::BUFFER_WIDTH/2, 5, makecol(255, 255, 255),
		makecol(0, 0, 0), true,
		false, caption);
	scr->fadeIn();

	long start = tguiCurrentTimeMillis();

	for (;;) {
		updateMusic();

		if (redrawEverything) {
		    	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			redrawEverything = false;
		}

		bool update;
		if (tguiScreenIsDirty()) {
			int rx = (Screen::BUFFER_WIDTH-256)/2;
			rectfill(bb, 0, 0, rx-1, Screen::BUFFER_HEIGHT, 0);
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == selector) {
				ret = selector->getPosition();
				break;
			}
			while (keypressed())
				readkey();
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
	}

	if (ret >= 0 && s1 && !prompt(def, s1, s2, s3))
		ret = -1;

	/*
	for (int i = 0; i < 250; i++) {
		updateMusic(); rest(1);
	}
	*/
	waitForRelease();
	clear_keybuf();

	tguiDeleteWidget(selector);

	delete selector;

	positions.clear();

	tguiPop();

	return ret;
}

void showItemDescription(int id)
{
	const int bufsize = 100;
	char s1[bufsize];
	char s2[bufsize];
	char s3[bufsize];
	getItemDescription(id, s1, s2, s3, bufsize);
	notify(s1, s2, s3);
}
