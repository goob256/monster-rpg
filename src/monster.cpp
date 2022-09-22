#include <allegro.h>

#include "monster.h"
#include "logos.h"

bool handleKeys()
{
	if (escapePressed() && !playerScripted) {
		if (!pauseGame())
			return false;
	}
	handleSpecialKeys();
	return true;
}

void handleScrolling()
{
	int x = player->getX();
	int y = player->getY();

	bool scroll = false;
	Direction scrollDirection;
	int xinc = 0;
	int yinc = 0;

	if (x < 0) {
		scroll = true;
		scrollDirection = DIRECTION_WEST;
		xinc = -1;
	}
	else if (x > (scr->BUFFER_WIDTH-player->getWidth()-1)) {
		scroll = true;
		scrollDirection = DIRECTION_EAST;
		xinc = 1;
	}
	else if (y < 0) {
		scroll = true;
		scrollDirection = DIRECTION_NORTH;
		yinc = -1;
	}
	else if (y > (scr->BUFFER_HEIGHT-player->getHeight()-1)) {
		scroll = true;
		scrollDirection = DIRECTION_SOUTH;
		yinc = 1;
	}

	if (scroll) {
		/*
		 * Extract the area section name and x,y
		 * coordinates of the area in the section from the
		 * area name, and create a new one from the
		 * direction the player is scrolling.
		 * Area names are in the form sectionXX-YY.
		 */
		char section[Area::MAX_AREA_NAME];
		char xx[10];
		char yy[10];
		char* name = currArea->getName();
		int i;
		for (i = 0; isalpha(name[i]); i++) {
			section[i] = name[i];
		}
		section[i] = 0;
		int j = 0;
		for (; isdigit(name[i]); i++) {
			xx[j++] = name[i];
		}
		i++;
		xx[j] = 0;
		j = 0;
		for (; isdigit(name[i]); i++) {
			yy[j++] = name[i];
		}
		yy[j] = 0;
		char newAreaName[Area::MAX_AREA_NAME];
		int xi = atoi(xx);
		int yi = atoi(yy);
		sprintf(newAreaName, "%s%d-%d", section, xi+xinc, yi+yinc);

		debug_message("New area name: %s\n", newAreaName);

		// Prepare for scrolling
		player->setHidden(true);
		BITMAP* fxBuffer2 = scr->getFXBuffer2();
		currArea->draw(fxBuffer2);

		if (prevAreaName)
			free(prevAreaName);

		prevAreaName = strdup(currArea->getName());

		delete currArea;
		char resName[1000];
		snprintf(resName, (sizeof(resName)/sizeof(*resName))-1, "areas/%s.area", newAreaName);
		currArea = new Area(getResource(resName));
		currArea->addObject(player);
		currArea->initLua();

		BITMAP* fxBuffer = scr->getFXBuffer();
		currArea->draw(fxBuffer);

		scr->scroll(scrollDirection);
	}
}

void run()
{
	scr->resetFrameCounter();

	BITMAP* backBuffer = scr->getBackBuffer();

	unsigned long startRun = currentTimeMillis();
	unsigned long start = currentTimeMillis();
	unsigned int frameSkip = 0;

	unsigned int maxUpdateStep = 40;

	for (;;) {
		if (redrawEverything) {
			redrawEverything = false;
			scr->draw();
		}

		updateMusic();

		if (frameSkip++ >= config.getFrameSkip()) {
			frameSkip = 0;
			if (config.useDirtyRectangles()) { // && (scr->getScale() % 2 == 0)) {// && !config.useSmoothScaling()) {
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

		if (!handleKeys()) {
			long runTime = currentTimeMillis() - startRun;
			int fps = scr->getFPS(runTime);
			debug_message("%d FPS.\n", fps);
			return;
		}

		unsigned long now = currentTimeMillis();
		unsigned long duration = MIN(now - start, maxUpdateStep);
		start = now;

		currArea->update(duration);
		//gamepad2Keypresses(duration);
		increaseMana(duration, 0, 0);

		handleScrolling();

		if (close_button_pressed) {
			close_button_pressed = false;
			if (prompt(true, "Really exit this game?", "Unsaved data will be lost!", ""))
				break;
		}

		if (playersDead) {
			playersDead = false;
			break;
		}
	}
}

static int check_arg(int argc, char **argv, char *arg)
{
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], arg))
			return i;
	}
	return -1;
}

int main(int argc, char** argv)
{
	DATAFILE *logos_datafile;
	BITMAP *nooskewl;
	BITMAP *bb;
	BITMAP *fx2;
	long start_time;
	const int xfade_time = 1500;

	if (check_arg(argc, argv, "-ttf") >= 0) {
		useBitmapFont = false;
	}

	if (check_arg(argc, argv, "-no-cfg") >= 0) {
		start(false);
	}
	else {
		start(true);
	}

	if (check_arg(argc, argv, "-impatient-mode") >= 0) {
		impatientMode = true;
	}

	int n = check_arg(argc, argv, "-audio-buffer");
	if (n >= 0) {
		int kb = atoi(argv[n+1]);
		logg_set_buffer_size(1024*kb);
	}

	/*
	if (config.getGamepadAvailable()) {
		if ((n = check_arg(argc, argv, "-joyb1")) >= 0) {
			int joyb1 = atoi(argv[n+1])-1;
			if (joyb1 < joy[0].num_buttons)
				config.setJoyButton1(joyb1);
		}

		if ((n = check_arg(argc, argv, "-joyb2")) >= 0) {
			int joyb2 = atoi(argv[n+1])-1;
			if (joyb2 < joy[0].num_buttons)
				config.setJoyButton2(joyb2);
		}

		if ((n = check_arg(argc, argv, "-joyb3")) >= 0) {
			int joyb3 = atoi(argv[n+1])-1;
			if (joyb3 < joy[0].num_buttons)
				config.setJoyButton3(joyb3);
		}
	}
	*/

	if (check_arg(argc, argv, "-disable-battles") >= 0)
		battlesDisabled = true;

	if (check_arg(argc, argv, "-no-intro") >= 0)
		goto end_intro;

	clear(screen);

	logos_datafile = load_datafile(getResource("logos.dat"));

	//alex = (BITMAP *)logos_datafile[ALLEGRO_PCX].dat;
	nooskewl = (BITMAP *)logos_datafile[NOOSKEWL_PCX].dat;

	#define center_blit(src, dst) \
		blit(src, dst, 0, 0, (dst->w-src->w)/2, (dst->h-src->h)/2, \
			src->w, src->h);

	/* Fade Allegro logo in */
	bb = scr->getBackBuffer();
	clear(bb);
	//center_blit(alex, bb);
	center_blit(nooskewl, bb);
	scr->fadeIn();

#if 0
	rest(1000);


	fx2 = scr->getFXBuffer2();
	center_blit(nooskewl, fx2);

	/* Cross fade-in Nooskewl logo */
	start_time = currentTimeMillis();

	while (currentTimeMillis() < start_time+xfade_time) {
		int elapsed = currentTimeMillis() - start_time;
		int alpha = (int)((float)elapsed / xfade_time * 255);
		center_blit(alex, bb);
		set_trans_blender(0, 0, 0, alpha);
		draw_trans_sprite(bb, fx2, 0, 0);
		scr->draw();
	}

	clear(screen);
	center_blit(nooskewl, bb);
	scr->draw();
#endif

	playOgg("nooskewl.ogg");
	rest(2000);
	
	scr->fadeOut();
	clear(bb);

	#undef center_blit

	unload_datafile(logos_datafile);

	rest(1000);

end_intro:
	startMusic("title.ogg");

	bool firstRun = true;
	DATAFILE* textDat = load_datafile_object(getResource("monster.dat"), "MONSTER_PCX");
	BITMAP* text = (BITMAP*)textDat->dat;

	for (;;) {
		TGUIWidget* pressed = 0;

		MWgtFrame* frame = new MWgtFrame(320/2-128/2, 122, 128, 80, makecol(0, 0, 0),
				255);
		MWgtTextButton* cont = new MWgtTextButton(320/2-mWgtTextWidth("Continue")/2, 130, "Continue");
		MWgtTextButton* erase = new MWgtTextButton(320/2-mWgtTextWidth("Erase")/2, 145, "Erase");
		MWgtTextButton* settings = new MWgtTextButton(320/2-mWgtTextWidth("Settings")/2, 160, "Settings");
		MWgtTextButton* quit = new MWgtTextButton(320/2-mWgtTextWidth("Quit")/2, 175, "Quit");

		tguiSetParent(0);
		tguiAddWidget(frame);
		tguiSetParent(frame);
		tguiAddWidget(cont);
		tguiAddWidget(erase);
		tguiAddWidget(settings);
		tguiAddWidget(quit);

		tguiSetFocus(cont);

		BITMAP* bb = scr->getBackBuffer();
		BITMAP* fx = scr->getFXBuffer();

		if (firstRun) {
			firstRun = false;
		}
		else {
			blit(bb, fx, 0, 0, 0, 0, bb->w, bb->h);
			scr->fadeOut();
			clear(bb);
			tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT);
			tguiClearDirtyRectangle();
			blit(bb, fx, 0, 0, 0, 0, bb->w, bb->h);
			scr->fadeIn();
		}

		long start = tguiCurrentTimeMillis();
		int flameUpdateCount = 0;
		unsigned int frameSkip = 0;

		for (;;) {
			updateMusic();
			if (frameSkip++ >= config.getFrameSkip()) {
				frameSkip = 0;
				tguiUpdateDirtyRectangle(0, 0, bb->w, bb->h);
				BITMAP* flames = getFlames();
				blit(flames, bb, 0, 0, bb->w/2-flames->w/2, 0, flames->w, flames->h);
				masked_blit(text, bb, 0, 0, bb->w/2-text->w/2, 0, text->w, text->h);
				tguiClearDirtyRectangle();
				scr->draw();
			}
			if (keypressed()) {
				pressed = tguiUpdate();
				if (pressed) {
					for (int i = 0; i < 250; i++) {
						updateMusic();
						rest(1);
					}
					clear_keybuf();
					break;
				}
			}

			flameUpdateCount += tguiCurrentTimeMillis() - start;

			if (flameUpdateCount > FLAME_DELAY) {
				drawFlames(flameUpdateCount/FLAME_DELAY);
				flameUpdateCount %= FLAME_DELAY;
			}

			long now = tguiCurrentTimeMillis();
			long duration = now - start;
			start = now;
			gamepad2Keypresses(duration);

			rest(1);

			if (escapePressed()) {
				break;
			}
		}

		blit(bb, fx, 0, 0, 0, 0, bb->w, bb->h);
		scr->fadeOut();
		clear(bb);

		tguiDeleteWidget(frame);

		TGUIWidget* contWidget = cont;
		TGUIWidget* eraseWidget = erase;
		TGUIWidget* settingsWidget = settings;
		TGUIWidget* quitWidget = quit;

		delete frame;
		delete cont;
		delete erase;
		delete settings;
		delete quit;

		if (pressed == contWidget) {
			int ss = selectSaveState("Load Game", 0, 0, 0, true);
			if (ss >= 0) {
				stopMusic();
				blit(bb, fx, 0, 0, 0, 0, bb->w, bb->h);
				scr->fadeOut();
				loadGame(getUserResource("save/%d.save", ss+1));
				currArea->draw(scr->getBackBuffer());
				scr->fadeIn();
				if (argc > 1 && strncmp(argv[1], "-", 1)) {
					startArea(argv[1]);
					int x = atoi(argv[2]);
					int y = atoi(argv[3]);
					Object* pl = currArea->getObject(0);
					pl->setPosition(x, y);
					currArea->draw(scr->getBackBuffer());
					scr->draw();
				}
				run();
				stopMusic();
				startMusic("title.ogg");
			}
		}
		else if (pressed == eraseWidget) {
		    int n = selectSaveState("Erase Game", "*** Are you sure you want to erase ***", "*** this save state? ***", "*** No undo! ***", false);
		    if (n >= 0) {
		    	delete_file(getUserResource("save/%d.save", n+1));
		    }
		}
		else if (pressed == settingsWidget) {
			if (configurate()) {
				break;
			}
		}
		else {
			break;
		}
	}

	unload_datafile_object(textDat);

	stop();

	return 0;
}
END_OF_MAIN()
