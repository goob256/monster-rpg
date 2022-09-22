#include <cstdarg>
#include <cstring>
#include <cstdio>

#include "monster.h"

#if defined __linux__ || defined ALLEGRO_MACOSX
#define MAX_PATH 5000
#else
#include <winalleg.h>
#define _WIN32_IE 0x400
#include <shlobj.h>
#endif

/*
 * Used to store the sound volume when switching away from the program.
 * The sound is muted until the program is switched back in.
 */
static int orig_volume;

bool redrawEverything = false;

/*
 * Return the path to user resources (save states, configuration)
 */
#if defined __linux__ || defined ALLEGRO_MACOSX
static char* userResourcePath()
{
	static char path[MAX_PATH];

	char* env = getenv("HOME");

	if (env) {
		strcpy(path, env);
		if (path[strlen(path)-1] != '/') {
			strncat(path, "/.monster/", (sizeof(path)/sizeof(*path))-1);
		}
		else {
			strncat(path, ".monster/", (sizeof(path)/sizeof(*path))-1);
		}
	}
	else {
		strcpy(path, "save/");
	}

	return path;
}
#else
static char* userResourcePath()
{
	static char path[MAX_PATH];

	bool success = SHGetSpecialFolderPath(0, path, CSIDL_APPDATA, false);

	if (success) {
		if (path[strlen(path)-1] != '/') {
			strncat(path, "/Monster/", (sizeof(path)/sizeof(*path))-1);	
		}
		else {
			strncat(path, "Monster/", (sizeof(path)/sizeof(*path))-1);
		}
	}
	else {
		strcpy(path, "save/");
	}

	return path;
}
#endif

char* getUserResource(char* fmt, ...)
{
	va_list ap;
	static char name[MAX_PATH];

	strcpy(name, userResourcePath());

	va_start(ap, fmt);
	vsnprintf(name+strlen(name), (sizeof(name)/sizeof(*name))-1, fmt, ap);
	return name;
}

/*
 * Get the path to the game resources. First checks for a
 * MONSTER_DATA environment variable that points to the resources,
 * then a system-wide resource directory then the directory
 * "data" from the current directory.
 */
#if defined __linux__ || defined ALLEGRO_MACOSX
static char* resourcePath()
{
	static char path[MAX_PATH];

	char* env = getenv("MONSTER_DATA");

	if (env) {
		strcpy(path, env);
		if (path[strlen(path)-1] != '/') {
			strncat(path, "/", (sizeof(path)/sizeof(*path))-1);
		}
	}
	else {
		strcpy(path, "data/");
	}

	return path;
}
#else
static char* resourcePath()
{
	static char path[MAX_PATH];

	char* env = getenv("MONSTER_DATA");

	if (env) {
		strcpy(path, env);
		if (path[strlen(path)-1] != '\\' && path[strlen(path)-1] != '/') {
			strncat(path, "/", (sizeof(path)/sizeof(*path))-1);
		}
	}
	else {
		strcpy(path, "data/");
	}

	return path;
}
#endif

char* getResource(char* fmt, ...)
{
	va_list ap;
	static char name[MAX_PATH];

	strcpy(name, resourcePath());

	va_start(ap, fmt);
	vsnprintf(name+strlen(name), (sizeof(name)/sizeof(*name))-1, fmt, ap);
	return name;
}

char* getScriptExtension()
{
	if (config.useCompiledScripts())
		return "scr";
	else
		return "lua";
}

void clearStats(PlayerStats* stats)
{
	clearMilestones(stats->milestones);
	for (int i = 0; i < MAX_INVENTORY; i++) {
		stats->inventory[i].id = -1;
		stats->inventory[i].quantity = 0;
	}
	stats->experience = 0;
	stats->weapon = -1;
	stats->armor = -1;
	stats->mana = 0;
	stats->maxMana = getMaxMana(getLevel(stats->experience));
	stats->hp = getMaxHP(getLevel(stats->experience));
	stats->maxHP = getMaxHP(getLevel(stats->experience));
	stats->gold = 0;
	stats->mana = 0;

	for (int i = 0; i < MAX_SPELLS; i++) {
		stats->spells[i] = 0;
	}
}

volatile bool close_button_pressed = false;

/*
 * Called when window close button is pressed.
 */
void close_button_callback()
{
	close_button_pressed = true;
}

/*
 * Parts of the game that don't redraw continuosly (e.g. when using
 * dirty rectangles) must be redrawn when switching back in.
 */
static void switch_in_callback()
{
	redrawEverything = true;
}

bool initGamepad(void)
{
	if (install_joystick(JOY_TYPE_AUTODETECT) != 0) {
		//config.setNoGamepadAvailable();
		return false;
	}
	else {
		//config.setGamepadAvailable();
		return true;
	}
}

/*
 * Call at start of program
 */
void start(bool read_cfg)
{
	srand(time(NULL));
	
	allegro_init();

	if (read_cfg) {
		config.read();
	}

	install_timer();

#ifdef __linux__
	if (install_sound(DIGI_ALSA, MIDI_NONE, 0) == -1) {
#else
	if (install_sound(DIGI_AUTODETECT, MIDI_NONE, 0) == -1) {
#endif
		printf("%s\n", allegro_error);
	}

	rest(1000); // double click fix for Gnome

	install_keyboard();

	initGamepad();

	try {
		scr = new Screen(config.getWantedGraphicsMode());
	}
	catch (...) {
		allegro_message("Error setting graphics mode.");
		throw new InitError();
	}

	debug_message("Using %dx%d@%dbpp\n", SCREEN_W, SCREEN_H, get_color_depth());

	loadTileAnimations();

	player = new Object(0, 0, TILE_SIZE, FEET_H, 16);

	Input* playerInput;
	AnimationSet* animSet;

	playerInput = createInput(config.getInputType());
	player->setInput(playerInput);

	animSet = new AnimationSet(getResource("objects/Coro.anims"));
	player->setAnimationSet(animSet);

	player->setType(OBJECT_CHARACTER);

	clearStats(&stats);
	clearStats(&partnerStats);
	partner = -1;

	set_close_button_callback(close_button_callback);

	tguiInit(scr->getBackBuffer());
	mWgtInit();

	loadMenuData();

	initBattleSystem();
	initSpells();

	logg_set_buffer_size(1024*64);

	initFlames();

	set_display_switch_mode(SWITCH_BACKGROUND);
	set_display_switch_callback(SWITCH_IN, switch_in_callback);
}

/*
 * Call before exiting program
 */
void stop()
{
	delete currArea;

	tguiShutdown();
	mWgtShutdown();

	delete player;

	destroyTileAnimations();

	if (prevAreaName)
		free(prevAreaName);

	clearSampleCache();

	destroyMenuData();
	
	destroyBattleSystem();

	destroyMusic();

	destroyFlames();

	for (int i = 0; i < MAX_SPELLS; i++) {
		if (stats.spells[i] != 0) {
			free(stats.spells[i]);
		}
	}

	destroySpells();

	config.write();

	delete scr;

	set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
	allegro_exit();
}

#ifndef itoa
char* itoa(int i)
{
	static char s[MAX_PATH];
	snprintf(s, (sizeof(s)/sizeof(*s))-1, "%d", i);
	return s;
}
#endif
