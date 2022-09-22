#include <fstream>
#include <iostream>
#include <sys/stat.h>

#include "monster.h"

#include <allegro.h>
#ifdef ALLEGRO_WINDOWS
#define mkdir(a, b) mkdir(a)
#endif

Configuration config;

struct Resolution {
	int width;
	int height;
	char* name;
};

const int NUM_RESOLUTIONS = 4;

Resolution resolutions[NUM_RESOLUTIONS] = {
	{ 320, 240, "320x240" },
	{ 640, 480, "640x480" },
	{ 800, 600, "800x600" },
	{ 1024, 768, "1024x768" },
};

const int NUM_DEPTHS = 4;

int depths[NUM_DEPTHS] = {
	15,
	16,
	24,
	32
};

const int MAX_FRAMESKIP = 32;

void getCurrentVideoValues(int* res, int* depth, bool* fullscreen)
{
	ScreenDescriptor* sd = config.getWantedGraphicsMode();

	*res = -1;
	for (int i = 0; i < NUM_RESOLUTIONS; i++) {
		if (sd->width == resolutions[i].width && sd->height == resolutions[i].height) {
			*res = i;
			break;
		}
	}

	*depth = -1;
	for (int i = 0; i < NUM_DEPTHS; i++) {
		if (sd->depth == depths[i]) {
			*depth = i;
			break;
		}
	}

	*fullscreen = sd->fullscreen;
}

static void fillGamepadButtonNumbers(MWgtText **names)
{
	static char numbers[3][10];
	sprintf(numbers[0], "%d", config.getJoyButton1());
	sprintf(numbers[1], "%d", config.getJoyButton2());
	sprintf(numbers[2], "%d", config.getJoyButton3());
	names[0]->setText(numbers[0]);
	names[1]->setText(numbers[1]);
	names[2]->setText(numbers[2]);
}

static int getGamepadButton(void)
{
	drawFrame(scr->getBackBuffer(), 60, 90, 200, 75,
		makecol(255, 0, 0), 200);
	mPrintf(scr->getBackBuffer(), Screen::BUFFER_WIDTH/2,
		Screen::BUFFER_HEIGHT/2-15,
		makecol(255, 255, 255), makecol(255, 0, 0),
		true, true, "Press a gamepad button");
	mPrintf(scr->getBackBuffer(), Screen::BUFFER_WIDTH/2,
		Screen::BUFFER_HEIGHT/2+15,
		makecol(255, 255, 255), makecol(255, 0, 0),
		true, true, "or escape to cancel...");
	scr->draw();

	int ret;

	rest(300);

	for (;;) {
		updateMusic();
		if (keyboard_needs_poll())
			poll_keyboard();
		poll_joystick();
		if (key[KEY_ESC]) {
			ret = -1;
			break;
		}
		for (int i = 0; i < joy[0].num_buttons; i++) {
			if (joy[0].button[i].b) {
				ret = i;
				goto done;
			}
		}
		rest(1);
	}

done:
	playOgg("button.ogg");
	rest(300);
	clear_keybuf();
	return ret;
}

static void setGamepadButton(int number, int value)
{
	int current[3];
	current[0] = config.getJoyButton1();
	current[1] = config.getJoyButton2();
	current[2] = config.getJoyButton3();

	for (int i = 0; i < 3; i++) {
		if (i == number)
			continue;
		if (current[i] == value) {
			current[i] = current[number];
			break;
		}
	}

	current[number] = value;

	config.setJoyButton1(current[0]);
	config.setJoyButton2(current[1]);
	config.setJoyButton3(current[2]);
}

static void configureGamepadButtons()
{
	MWgtFrame* frame = new MWgtFrame(10, 55, 300, 120, makecol(0, 0, 0),
			255);
	
	MWgtText* title = new MWgtText(0, 0, "Gamepad:", makecol(255, 255, 255));
	MWgtButton** buttons = new MWgtButton*[3];
	buttons[0] = new MWgtButton(0, 0, 80, "Action");
	buttons[1] = new MWgtButton(0, 0, 80, "Cancel");
	buttons[2] = new MWgtButton(0, 0, 80, "Run");
	MWgtText** names = new MWgtText*[3];
	for (int i = 0; i < 3; i++) {
		names[i] = new MWgtText(0, 0, "", makecol(255, 255, 255));
	}
	fillGamepadButtonNumbers(names);
	MWgtButton* ok = new MWgtButton(0, 0, 40, "OK");
	MWgtButton* reset = new MWgtButton(0, 0, 60, "Reset");

	tguiCenterWidget(title, Screen::BUFFER_WIDTH/2, 70);
	for (int i = 0; i < 3; i++) {
		tguiCenterWidget(buttons[i], Screen::BUFFER_WIDTH/4, 90+i*20);
		tguiCenterWidget(names[i], Screen::BUFFER_WIDTH*3/4, 90+i*20);
	}
	tguiCenterWidget(ok, Screen::BUFFER_WIDTH/2-50, 150);
	tguiCenterWidget(reset, Screen::BUFFER_WIDTH/2+50, 150);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	for (int i = 0; i < 3; i++)
		tguiAddWidget(buttons[i]);
	tguiAddWidget(ok);
	tguiAddWidget(reset);
	tguiAddWidget(title);
	for (int i = 0; i < 3; i++)
		tguiAddWidget(names[i]);

	tguiSetFocus(buttons[0]);

	long start = tguiCurrentTimeMillis();
	bool update = true;
	
	for (;;) {
		updateMusic();

		if (redrawEverything) {
		    	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			redrawEverything = false;
		}

		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			for (int i = 0; i < 3; i++) {
				if (widget == buttons[i]) {
					int b = getGamepadButton();
					if (b >= 0) {
						setGamepadButton(i, b);
						fillGamepadButtonNumbers(names);
					}
					for (int i = 0; i < 3; i++) {
						tguiMakeDirty(names[i]);
					}
					break;
				}
			}
			if (widget == ok) {
				break;
			}
			else if (widget == reset) {
				config.setJoyButton1(0);
				config.setJoyButton2(1);
				config.setJoyButton3(2);
				fillGamepadButtonNumbers(names);
				for (int i = 0; i < 3; i++) {
					tguiMakeDirty(names[i]);
				}
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			break;
		}
	}

	tguiDeleteWidget(frame);

	delete frame;
	for (int i = 0; i < 3; i++) {
		delete buttons[i];
		delete names[i];
	}
	delete[] buttons;
	delete[] names;
	delete title;
	delete ok;
	delete reset;

	waitForRelease();
}

static void fillKeyboardButtonNames(MWgtText **names)
{
	static char n[6][100];

	strcpy(n[0], scancode_to_name(config.getKey1()));
	strcpy(n[1], scancode_to_name(config.getKey2()));
	strcpy(n[2], scancode_to_name(config.getKeyLeft()));
	strcpy(n[3], scancode_to_name(config.getKeyRight()));
	strcpy(n[4], scancode_to_name(config.getKeyUp()));
	strcpy(n[5], scancode_to_name(config.getKeyDown()));

	names[0]->setText(n[0]);
	names[1]->setText(n[1]);
	names[2]->setText(n[2]);
	names[3]->setText(n[3]);
	names[4]->setText(n[4]);
	names[5]->setText(n[5]);
}

static int getKeyboardButton(void)
{
	drawFrame(scr->getBackBuffer(), 60, 90, 200, 75,
		makecol(255, 0, 0), 200);
	mPrintf(scr->getBackBuffer(), Screen::BUFFER_WIDTH/2,
		Screen::BUFFER_HEIGHT/2,
		makecol(255, 255, 255), makecol(255, 0, 0),
		true, true, "Press a key...");
	scr->draw();

	int ret;

	rest(300);

	for (;;) {
		updateMusic();
		if (keyboard_needs_poll())
			poll_keyboard();
		if (keypressed()) {
			ret = readkey() >> 8;
			break;
		}
		rest(1);
	}

done:
	playOgg("button.ogg");
	rest(300);
	clear_keybuf();
	return ret;
}

static void setKeyboardButton(int number, int value)
{
	int current[6];
	current[0] = config.getKey1();
	current[1] = config.getKey2();
	current[2] = config.getKeyLeft();
	current[3] = config.getKeyRight();
	current[4] = config.getKeyUp();
	current[5] = config.getKeyDown();

	for (int i = 0; i < 6; i++) {
		if (i == number)
			continue;
		if (current[i] == value) {
			current[i] = current[number];
			break;
		}
	}

	current[number] = value;

	config.setKey1(current[0]);
	config.setKey2(current[1]);
	config.setKeyLeft(current[2]);
	config.setKeyRight(current[3]);
	config.setKeyUp(current[4]);
	config.setKeyDown(current[5]);
}

static void configureKeyboardButtons()
{
	MWgtFrame* frame = new MWgtFrame(10, 25, 300, 180, makecol(0, 0, 0),
			255);
	
	MWgtText* title = new MWgtText(0, 0, "Keyboard:", makecol(255, 255, 255));
	MWgtButton** buttons = new MWgtButton*[6];
	buttons[0] = new MWgtButton(0, 0, 80, "Action");
	buttons[1] = new MWgtButton(0, 0, 80, "Cancel");
	buttons[2] = new MWgtButton(0, 0, 80, "Left");
	buttons[3] = new MWgtButton(0, 0, 80, "Right");
	buttons[4] = new MWgtButton(0, 0, 80, "Up");
	buttons[5] = new MWgtButton(0, 0, 80, "Down");
	MWgtText** names = new MWgtText*[6];
	for (int i = 0; i < 6; i++) {
		names[i] = new MWgtText(0, 0, "", makecol(255, 255, 255));
	}
	fillKeyboardButtonNames(names);
	MWgtButton* ok = new MWgtButton(0, 0, 40, "OK");
	MWgtButton* reset = new MWgtButton(0, 0, 60, "Reset");

	tguiCenterWidget(title, Screen::BUFFER_WIDTH/2, 37);
	for (int i = 0; i < 6; i++) {
		tguiCenterWidget(buttons[i], Screen::BUFFER_WIDTH/4, 60+i*20);
		tguiCenterWidget(names[i], Screen::BUFFER_WIDTH*3/4, 60+i*20);
	}
	tguiCenterWidget(ok, Screen::BUFFER_WIDTH/2-50, 180);
	tguiCenterWidget(reset, Screen::BUFFER_WIDTH/2+50, 180);

	tguiSetParent(0);
	tguiAddWidget(frame);
	tguiSetParent(frame);
	for (int i = 0; i < 6; i++)
		tguiAddWidget(buttons[i]);
	tguiAddWidget(ok);
	tguiAddWidget(reset);
	tguiAddWidget(title);
	for (int i = 0; i < 6; i++)
		tguiAddWidget(names[i]);

	tguiSetFocus(buttons[0]);

	long start = tguiCurrentTimeMillis();
	bool update = true;
	
	for (;;) {
		updateMusic();

		if (redrawEverything) {
		    	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			redrawEverything = false;
		}

		if (tguiScreenIsDirty()) {
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			for (int i = 0; i < 6; i++) {
				if (widget == buttons[i]) {
					int b = getKeyboardButton();
					setKeyboardButton(i, b);
					fillKeyboardButtonNames(names);
					for (int i = 0; i < 6; i++) {
						tguiMakeDirty(names[i]);
					}
					break;
				}
			}
			if (widget == ok) {
				break;
			}
			else if (widget == reset) {
				config.setKey1(KEY_SPACE);
				config.setKey2(KEY_ESC);
				config.setKeyLeft(KEY_LEFT);
				config.setKeyRight(KEY_RIGHT);
				config.setKeyUp(KEY_UP);
				config.setKeyDown(KEY_DOWN);
				fillKeyboardButtonNames(names);
				for (int i = 0; i < 6; i++) {
					tguiMakeDirty(names[i]);
				}
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			break;
		}
	}

	tguiDeleteWidget(frame);

	delete frame;
	for (int i = 0; i < 6; i++) {
		delete buttons[i];
		delete names[i];
	}
	delete[] buttons;
	delete[] names;
	delete title;
	delete ok;
	delete reset;

	waitForRelease();
}

bool configurate()
{
	bool ret = false;

	MWgtFrame* frame1 = new MWgtFrame(10, 27, 300, 205, makecol(0, 0, 0),
			255);
	MWgtFrame* frame2 = new MWgtFrame(10, 27, 300, 205, makecol(0, 0, 0),
			255);
	MWgtFrame* frame3 = new MWgtFrame(10, 27, 300, 205, makecol(0, 0, 0),
			255);
	std::vector<std::string> tabNames;
	tabNames.push_back("Game");
	tabNames.push_back("GFX");
	tabNames.push_back("Sound");
	std::vector<MWgtFrame*> frames;
	frames.push_back(frame1);
	frames.push_back(frame2);
	frames.push_back(frame3);
	MWgtTabs* tabs = new MWgtTabs(30, 12, makecol(100, 100, 100), 0, tabNames, frames);

	MWgtText* inputText = new MWgtText(0, 0, "Input Device:", makecol(255, 255, 255));
	std::vector<char*> inputOptions;
	inputOptions.push_back(strdup("Keyboard"));
	inputOptions.push_back(strdup("Gamepad"));
	MWgtOption* inputDevice = new MWgtOption(0, 0, 120, &inputOptions);
	InputType startInput = config.getInputType();
	if (config.getInputType() == INPUT_KEYBOARD) {
		inputDevice->setSelected(0);
	}
	else {
		inputDevice->setSelected(1);
	}
	MWgtButton *cfgButtons = new MWgtButton(0, 0, 170, "Configure Buttons");
	MWgtButton *calibrate = new MWgtButton(0, 0, 160, "Calibrate Gamepad");
	MWgtCheckbox* battleSpeed = new MWgtCheckbox(0, 0, "Fast Battles");
	if (config.getBattleSpeed() == 0) {
		battleSpeed->setChecked(false);
	}
	else {
		battleSpeed->setChecked(true);
	}
	MWgtButton* save = new MWgtButton(0, 0, 150, "Save Configuration");
	MWgtButton* cancel = new MWgtButton(0, 0, 50, "Cancel");
	tguiCenterWidget(inputText, Screen::BUFFER_WIDTH/4, 50);
	tguiCenterWidget(inputDevice, Screen::BUFFER_WIDTH*3/4, 50);
	tguiCenterWidget(cfgButtons, Screen::BUFFER_WIDTH/2, 72);
	tguiCenterWidget(calibrate, Screen::BUFFER_WIDTH/2, 94);
	tguiCenterWidget(battleSpeed, Screen::BUFFER_WIDTH/2, 116);
	tguiCenterWidget(save, Screen::BUFFER_WIDTH/2, 165);
	tguiCenterWidget(cancel, Screen::BUFFER_WIDTH/2, 190);

	ScreenDescriptor startSD;
	ScreenDescriptor* sd = config.getWantedGraphicsMode();
	startSD.width = sd->width;
	startSD.height = sd->height;
	startSD.depth = sd->depth;
	startSD.fullscreen = sd->fullscreen;

	int res;
	int dep;
	bool full;
	getCurrentVideoValues(&res, &dep, &full);
	std::vector<char*> resNames;
	for (int i = 0; i < NUM_RESOLUTIONS; i++) {
		resNames.push_back(resolutions[i].name);
	}
	MWgtText* resText = new MWgtText(0, 0, "Resolution:", makecol(255, 255, 255));
	MWgtOption* resolution = new MWgtOption(0, 0, 100, &resNames);
	resolution->setSelected(res);
	std::vector<char*> depthNames;
	depthNames.push_back(strdup("15"));
	depthNames.push_back(strdup("16"));
	depthNames.push_back(strdup("24"));
	depthNames.push_back(strdup("32"));
	MWgtText* depthText = new MWgtText(0, 0, "Color Depth:", makecol(255, 255, 255));
	MWgtOption* depth = new MWgtOption(0, 0, 75, &depthNames);
	depth->setSelected(dep);
	MWgtCheckbox* fullscreen = new MWgtCheckbox(0, 0, "Fullscreen");
	fullscreen->setChecked(full);
	char fs[10];
	std::vector<char*> frameskipNames;
	for (int i = 0; i <= MAX_FRAMESKIP; i++) {
		sprintf(fs, "%d", i);
		frameskipNames.push_back(strdup(fs));
	}
	MWgtText* frameskipText = new MWgtText(0, 0, "Frameskip:", makecol(255, 255, 255));
	MWgtOption* frameskip = new MWgtOption(0, 0, 50, &frameskipNames);
	frameskip->setSelected(config.getFrameSkip());
	MWgtCheckbox* vSync = new MWgtCheckbox(0, 0, "Vsync");
	vSync->setChecked(config.getWaitForVsync());
	MWgtCheckbox* smoothScaling = new MWgtCheckbox(0, 0, "Smooth Scaling");
	smoothScaling->setChecked(config.useSmoothScaling());
	MWgtCheckbox* dirtyRectangles = new MWgtCheckbox(0, 0, "Dirty Rectangles");
	dirtyRectangles->setChecked(config.useDirtyRectangles());
	tguiCenterWidget(resText, Screen::BUFFER_WIDTH/4, 50);
	tguiCenterWidget(resolution, Screen::BUFFER_WIDTH*3/4, 50);
	tguiCenterWidget(depthText, Screen::BUFFER_WIDTH/4, 70);
	tguiCenterWidget(depth, Screen::BUFFER_WIDTH*3/4, 70);
	tguiCenterWidget(fullscreen, Screen::BUFFER_WIDTH/2, 90);
	tguiCenterWidget(frameskipText, Screen::BUFFER_WIDTH/4, 110);
	tguiCenterWidget(frameskip, Screen::BUFFER_WIDTH*3/4, 110);
	tguiCenterWidget(vSync, Screen::BUFFER_WIDTH/2, 130);
	tguiCenterWidget(smoothScaling, Screen::BUFFER_WIDTH/2, 150);
	tguiCenterWidget(dirtyRectangles, Screen::BUFFER_WIDTH/2, 170);

	MWgtText* sfxVolumeText = new MWgtText(0, 0, "SFX Volume:", makecol(255, 255, 255));
	MWgtSlider* sfxVolume = new MWgtSlider(0, 0, 100, 100);
	int stop = MIN(99, config.getSFXVolume() * 100 / 255);
	sfxVolume->setStop(stop);
	MWgtText* musicVolumeText = new MWgtText(0, 0, "Music Volume:", makecol(255, 255, 255));
	MWgtSlider* musicVolume = new MWgtSlider(0, 0, 100, 100);
	stop = MIN(99, config.getMusicVolume() * 100 / 255);
	musicVolume->setStop(stop);
	tguiCenterWidget(sfxVolumeText, Screen::BUFFER_WIDTH/4, 50);
	tguiCenterWidget(sfxVolume, Screen::BUFFER_WIDTH*3/4, 50);
	tguiCenterWidget(musicVolumeText, Screen::BUFFER_WIDTH/4, 70);
	tguiCenterWidget(musicVolume, Screen::BUFFER_WIDTH*3/4, 70);

	tguiSetParent(0);
	tguiAddWidget(frame3);
	tguiSetParent(frame3);
	tguiAddWidget(sfxVolumeText);
	tguiAddWidget(sfxVolume);
	tguiAddWidget(musicVolumeText);
	tguiAddWidget(musicVolume);
	tguiSetParent(0);
	tguiAddWidget(frame2);
	tguiSetParent(frame2);
	tguiAddWidget(resText);
	tguiAddWidget(resolution);
	tguiAddWidget(depthText);
	tguiAddWidget(depth);
#ifndef __linux__ // see comment about fullscreen on linux below
	tguiAddWidget(fullscreen);
#endif
	tguiAddWidget(frameskipText);
	tguiAddWidget(frameskip);
	tguiAddWidget(vSync);
	tguiAddWidget(smoothScaling);
	tguiAddWidget(dirtyRectangles);
	tguiSetParent(0);
	tguiAddWidget(frame1);
	tguiSetParent(frame1);
	tguiAddWidget(inputText);
	tguiAddWidget(inputDevice);
	tguiAddWidget(cfgButtons);
	tguiAddWidget(calibrate);
	tguiAddWidget(battleSpeed);
	tguiAddWidget(save);
	tguiAddWidget(cancel);
	tguiAddWidget(tabs);
	tguiSetParent(0);

	tabs->focusCurrentTab();
	tguiSetFocus(tabs);

	BITMAP* bb = scr->getBackBuffer();
	clear(bb);
	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT);
	tguiClearDirtyRectangle();
	blit(bb, scr->getFXBuffer(), 0, 0, 0, 0, bb->w, bb->h);
	scr->fadeIn();
	
	long start = tguiCurrentTimeMillis();
	int lastVolume = musicVolume->getStop();
	long lastVolumeUpdate = currentTimeMillis();
	
	for (;;) {
		updateMusic();

		if (redrawEverything) {
		    	tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH,
				    Screen::BUFFER_HEIGHT);
			redrawEverything = false;
		}

		if  ((lastVolume != musicVolume->getStop()) && (currentTimeMillis() > lastVolumeUpdate+1000)) {
			lastVolume = musicVolume->getStop();
			lastVolumeUpdate = currentTimeMillis();
			config.setMusicVolume(255*lastVolume/100);
			restartMusic();
		}
		bool update;
		if (tguiScreenIsDirty()) {
			clear(bb);
			tguiClearDirtyRectangle();
			scr->draw();
			update = true;
		}
		else
			update = false;
		if (keypressed() || update) {
			TGUIWidget* widget = tguiUpdate();
			if (widget == save) {
				if (inputDevice->getSelected() == 0) {
					config.setInputType(INPUT_KEYBOARD);
				}
				else {
					config.setInputType(INPUT_GAMEPAD);
				}
				if (battleSpeed->isChecked()) {
					config.setBattleSpeed(1);
				}
				else {
					config.setBattleSpeed(0);
				}
				ScreenDescriptor sd;
				int res = resolution->getSelected();
				sd.width = resolutions[res].width;
				sd.height = resolutions[res].height;
				sd.depth = depths[depth->getSelected()];
				sd.fullscreen = fullscreen->isChecked();
				config.setWantedGraphicsMode(sd);
				config.setFrameSkip(frameskip->getSelected());
				config.setWaitForVsync(vSync->isChecked());
				config.setUseSmoothScaling(smoothScaling->isChecked());
				config.setUseDirtyRectangles(dirtyRectangles->isChecked());
				config.setSFXVolume(sfxVolume->getStop()*255/100);
				config.setMusicVolume(musicVolume->getStop()*255/100);
				if (startInput != config.getInputType() ||
						startSD.width != sd.width ||
						startSD.height != sd.height ||
						startSD.depth != sd.depth ||
						startSD.fullscreen != sd.fullscreen) {
					notify("You must restart the program", "for the changes to take affect...", "");
					ret = true;
				}
				break;
			}
			else if (widget == cancel) {
				break;
			}
			else if (widget == cfgButtons) {
				if (inputDevice->getSelected() == 0) {
					configureKeyboardButtons();
				}
				else  {
					bool available = true;
					if (!config.getGamepadAvailable()) {
						available = false;
						/*
						if (!initGamepad())
							available = false;
						*/
					}
					if (available) {
						configureGamepadButtons();
						int b1 = config.getJoyButton1();
						int b2 = config.getJoyButton2();
						int b3 = config.getJoyButton3();
						if (b1 == b2 || b2 == b3 || b1 == b3) {
							notify("*** Warning: Two or more buttons ***", "*** Have the same value! ***", "");
						}
					}
					else {
						notify("No gamepad available...", "You may need to restart", "the program...");
					}
				}
				tguiSetFocus(cfgButtons);
				tguiUpdateDirtyRectangle(0, 0, Screen::BUFFER_WIDTH, Screen::BUFFER_HEIGHT);
			}
			else if (widget == calibrate) {
				if (joy[0].flags & JOYFLAG_CALIBRATE) {
					const char *msg = calibrate_joystick_name(0);

					drawFrame(scr->getBackBuffer(), 50, 50, Screen::BUFFER_WIDTH-100, Screen::BUFFER_HEIGHT-100,
						makecol(255, 50, 50), 200);
					mPrintf(scr->getBackBuffer(), Screen::BUFFER_WIDTH/2, Screen::BUFFER_HEIGHT/2-20, makecol(255, 255, 255), makecol(255, 50, 50), true,
						true, msg);
					mPrintf(scr->getBackBuffer(), Screen::BUFFER_WIDTH/2, Screen::BUFFER_HEIGHT/2+20, makecol(255, 255, 255), makecol(255, 50, 50), true,
						true, "and press a key");
					scr->draw();

					pushMusic();
					readkey();
					/*
					 * I know it makes no sense to call
					 * "stopMusic" to restart the music,
					 * the functions are named poorly or
					 * should be redesigned.
					 */
					stopMusic();

					if (calibrate_joystick(0) != 0) {
						notify("", "Error calibrating gamepad!", "");
					}
				}
				else {
					notify("The gamepad does not", "need calibration", "");
				}
			}
		}
		long end = tguiCurrentTimeMillis();
		long duration = end - start;
		start = end;
		int timeStep = 80;
		gamepad2Keypresses(duration);
		if (duration < timeStep)
			rest(timeStep-duration);
		if (escapePressed()) {
			break;
		}
	}

	tguiDeleteWidget(frame1);
	tguiDeleteWidget(frame2);
	tguiDeleteWidget(frame3);

	delete frame1;
	delete frame2;
	delete frame3;

	for (unsigned int i = 0; i < inputOptions.size(); i++) {
		free(inputOptions[i]);
	}
	inputOptions.clear();
	delete inputText;
	delete inputDevice;
	delete battleSpeed;
	delete save;
	delete cancel;

	resNames.clear();
	for (unsigned int i = 0; i < depthNames.size(); i++) {
		free(depthNames[i]);
	}
	depthNames.clear();
	for (unsigned int i = 0; i < frameskipNames.size(); i++) {
		free(frameskipNames[i]);
	}
	frameskipNames.clear();
	delete resText;
	delete resolution;
	delete depthText;
	delete depth;
	delete fullscreen;
	delete frameskipText;
	delete frameskip;
	delete vSync;
	delete dirtyRectangles;
	delete smoothScaling;

	delete sfxVolumeText;
	delete sfxVolume;
	delete musicVolumeText;
	delete musicVolume;

	delete tabs;


	for (int i = 0; i < 250; i++) {
		updateMusic();
		rest(1);
	}

	clear_keybuf();

	return ret;
}

ScreenDescriptor* Configuration::getWantedGraphicsMode()
{
	if (!loaded)
		wantedMode.depth = desktop_color_depth();
	return &wantedMode;
}

void Configuration::setWantedGraphicsMode(ScreenDescriptor& sd)
{
	wantedMode.width = sd.width;
	wantedMode.height = sd.height;
	wantedMode.depth = sd.depth;
	wantedMode.fullscreen = sd.fullscreen;
}

InputType Configuration::getInputType()
{
	return inputType;
}

void Configuration::setInputType(InputType it)
{
	inputType = it;
}

bool Configuration::showDebugMessages()
{
	return debugMessages;
}

bool Configuration::useCompiledScripts()
{
	return compiledScripts;
}

unsigned int Configuration::getFrameSkip()
{
	return frameSkip;
}

void Configuration::setFrameSkip(unsigned int frameSkip)
{
	this->frameSkip = frameSkip;
}

bool Configuration::getWaitForVsync()
{
	return waitForVsync;
}

void Configuration::setWaitForVsync(bool vs)
{
	waitForVsync = vs;
}

/*
void Configuration::setGamepadAvailable()
{
	gamepadAvailable = true;
}

void Configuration::setNoGamepadAvailable()
{
	gamepadAvailable = false;
}
*/

bool Configuration::getGamepadAvailable()
{
	return num_joysticks > 0;
	//return gamepadAvailable;
}

bool Configuration::useSmoothScaling()
{
	return smoothScaling;
}

void Configuration::setUseSmoothScaling(bool smooth)
{
	smoothScaling = smooth;
}

bool Configuration::useDirtyRectangles()
{
	return dirtyRectangles;
}

void Configuration::setUseDirtyRectangles(bool dirty)
{
	dirtyRectangles = dirty;
}

int Configuration::getBattleSpeed()
{
	return battleSpeed;
}

void Configuration::setBattleSpeed(int speed)
{
	battleSpeed = speed;
}

int Configuration::getSFXVolume()
{
	return sfxVolume;
}

void Configuration::setSFXVolume(int volume)
{
	sfxVolume = volume;
}

int Configuration::getMusicVolume()
{
	return musicVolume;
}

void Configuration::setMusicVolume(int volume)
{
	musicVolume = volume;
}

int Configuration::getJoyButton1()
{
	return joyB1;
}

void Configuration::setJoyButton1(int b1)
{
	joyB1 = b1;
}

int Configuration::getJoyButton2()
{
	return joyB2;
}

void Configuration::setJoyButton2(int b2)
{
	joyB2 = b2;
}

int Configuration::getJoyButton3()
{
	return joyB3;
}

void Configuration::setJoyButton3(int b3)
{
	joyB3 = b3;
}

int Configuration::getKey1(void)
{
	return key1;
}

void Configuration::setKey1(int k)
{
	key1 = k;
}

int Configuration::getKey2(void)
{
	return key2;
}

void Configuration::setKey2(int k)
{
	key2 = k;
}

int Configuration::getKeyLeft(void)
{
	return keyLeft;
}

void Configuration::setKeyLeft(int k)
{
	keyLeft = k;
}

int Configuration::getKeyRight(void)
{
	return keyRight;
}

void Configuration::setKeyRight(int k)
{
	keyRight = k;
}

int Configuration::getKeyUp(void)
{
	return keyUp;
}

void Configuration::setKeyUp(int k)
{
	keyUp = k;
}

int Configuration::getKeyDown(void)
{
	return keyDown;
}

void Configuration::setKeyDown(int k)
{
	keyDown = k;
}

void Configuration::read() throw (ReadError)
{
	char userDir[1000];
	sprintf(userDir, "%s", getUserResource(""));

#ifndef __linux__
	userDir[strlen(userDir)-1] = 0;
#endif

	mkdir(userDir, 0770);

	if (!exists(getUserResource("config"))) {
		loaded = false;
		return;
	}

	// load
	
	XMLData* xml = new XMLData(getUserResource("config"));

	XMLData* monster = xml->find("monster");

	XMLData* game = monster->find("game");
	XMLData* input = game->find("input");
	XMLData* battle_speed = game->find("battle_speed");
	XMLData* joyb1 = game->find("joyb1");
	XMLData* joyb2 = game->find("joyb2");
	XMLData* joyb3 = game->find("joyb3");
	XMLData* k1 = game->find("keyb1");
	XMLData* k2 = game->find("keyb2");
	XMLData* kl = game->find("keyLeft");
	XMLData* kr = game->find("keyRight");
	XMLData* ku = game->find("keyUp");
	XMLData* kd = game->find("keyDown");
	setInputType((InputType)atoi(input->getValue().c_str()));
	setBattleSpeed(atoi(battle_speed->getValue().c_str()));
	setJoyButton1(atoi(joyb1->getValue().c_str()));
	setJoyButton2(atoi(joyb2->getValue().c_str()));
	if (joyb3) // This wasn't always here
		setJoyButton3(atoi(joyb3->getValue().c_str()));
	if (k1) {
		setKey1(atoi(k1->getValue().c_str()));
		setKey2(atoi(k2->getValue().c_str()));
		setKeyLeft(atoi(kl->getValue().c_str()));
		setKeyRight(atoi(kr->getValue().c_str()));
		setKeyUp(atoi(ku->getValue().c_str()));
		setKeyDown(atoi(kd->getValue().c_str()));
	}

	XMLData* gfx = monster->find("gfx");
	XMLData* width = gfx->find("width");
	XMLData* height = gfx->find("height");
	XMLData* depth = gfx->find("depth");
	XMLData* fullscreen = gfx->find("fullscreen");
	XMLData* frameskip = gfx->find("frameskip");
	XMLData* vsync = gfx->find("vsync");
	XMLData* smooth_scaling = gfx->find("smooth_scaling");
	XMLData* dirty_rectangles = gfx->find("dirty_rectangles");
	ScreenDescriptor sd;
	sd.width = atoi(width->getValue().c_str());
	sd.height = atoi(height->getValue().c_str());
	sd.depth = atoi(depth->getValue().c_str());
// NOTE: Linux fullscreen support causes a crash on exit that doesn't reset the mode, so it's disabled now
#ifdef __linux__
	sd.fullscreen = false;
#else
	sd.fullscreen = atoi(fullscreen->getValue().c_str());
#endif
	setWantedGraphicsMode(sd);
	setFrameSkip(atoi(frameskip->getValue().c_str()));
	setWaitForVsync(atoi(vsync->getValue().c_str()));
	setUseSmoothScaling(atoi(smooth_scaling->getValue().c_str()));
	setUseDirtyRectangles(atoi(dirty_rectangles->getValue().c_str()));

	XMLData* sound = monster->find("sound");
	XMLData* music_volume = sound->find("music_volume");
	XMLData* sfx_volume = sound->find("sfx_volume");
	if (sfx_volume) { // Compatability with alphas
		setSFXVolume(atoi(sfx_volume->getValue().c_str()));
	}
	setMusicVolume(atoi(music_volume->getValue().c_str()));

	delete xml;

	loaded = true;
}

void Configuration::write() throw (WriteError)
{
	XMLData* monster = new XMLData("monster", "");

	XMLData* game = new XMLData("game", "");
	XMLData* input = new XMLData("input", itoa(getInputType()));
	XMLData* battle_speed = new XMLData("battle_speed", itoa(getBattleSpeed()));
	XMLData* joyb1 = new XMLData("joyb1", itoa(getJoyButton1()));
	XMLData* joyb2 = new XMLData("joyb2", itoa(getJoyButton2()));
	XMLData* joyb3 = new XMLData("joyb3", itoa(getJoyButton3()));
	XMLData* k1 = new XMLData("keyb1", itoa(getKey1()));
	XMLData* k2 = new XMLData("keyb2", itoa(getKey2()));
	XMLData* kl = new XMLData("keyLeft", itoa(getKeyLeft()));
	XMLData* kr = new XMLData("keyRight", itoa(getKeyRight()));
	XMLData* ku = new XMLData("keyUp", itoa(getKeyUp()));
	XMLData* kd = new XMLData("keyDown", itoa(getKeyDown()));
	game->add(input);
	game->add(battle_speed);
	game->add(joyb1);
	game->add(joyb2);
	game->add(joyb3);
	game->add(k1);
	game->add(k2);
	game->add(kl);
	game->add(kr);
	game->add(ku);
	game->add(kd);

	XMLData* gfx = new XMLData("gfx", "");
	ScreenDescriptor* sd = getWantedGraphicsMode();
	XMLData* width = new XMLData("width", itoa(sd->width));
	XMLData* height = new XMLData("height", itoa(sd->height));
	XMLData* depth = new XMLData("depth", itoa(sd->depth));
	XMLData* fullscreen = new XMLData("fullscreen", itoa(sd->fullscreen));
	XMLData* frameskip = new XMLData("frameskip", itoa(getFrameSkip()));
	XMLData* vsync = new XMLData("vsync", itoa(getWaitForVsync()));
	XMLData* smooth_scaling = new XMLData("smooth_scaling", itoa(useSmoothScaling()));
	XMLData* dirty_rectangles = new XMLData("dirty_rectangles", itoa(useDirtyRectangles()));
	gfx->add(width);
	gfx->add(height);
	gfx->add(depth);
	gfx->add(fullscreen);
	gfx->add(frameskip);
	gfx->add(vsync);
	gfx->add(smooth_scaling);
	gfx->add(dirty_rectangles);

	XMLData* sound = new XMLData("sound", "");
	XMLData* music_volume = new XMLData("music_volume", itoa(getMusicVolume()));
	XMLData* sfx_volume = new XMLData("sfx_volume", itoa(getSFXVolume()));
	sound->add(sfx_volume);
	sound->add(music_volume);

	monster->add(game);
	monster->add(gfx);
	monster->add(sound);

	std::ofstream fout(getUserResource("config"), std::ios::out);
	monster->write(fout, 0);
	fout.close();

	delete monster;
}

Configuration::Configuration() :
	inputType(INPUT_KEYBOARD),
	debugMessages(false), 
	compiledScripts(false),
	frameSkip(0),
	waitForVsync(false),
	smoothScaling(false), 
	dirtyRectangles(true),
	battleSpeed(0),
	sfxVolume(255),
	musicVolume(128),
	joyB1(0),
	joyB2(1),
	joyB3(2),
	key1(KEY_SPACE),
	key2(KEY_ESC),
	keyLeft(KEY_LEFT),
	keyRight(KEY_RIGHT),
	keyUp(KEY_UP),
	keyDown(KEY_DOWN),
	gamepadAvailable(true)
{
#ifdef EDITOR
	wantedMode.width = 800;
	wantedMode.height = 600;
#else
	wantedMode.width = 640;
	wantedMode.height = 480;
#endif
	wantedMode.fullscreen = false;
}
