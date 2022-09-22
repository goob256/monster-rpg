#include <allegro.h>

#include "monster.h"

static void handleMusicVolumeDown()
{
	int volume = config.getMusicVolume();
	volume = MAX(0, volume-25);
	config.setMusicVolume(volume);
	scr->showVolume(volume);
}

static void handleMusicVolumeUp()
{
	int volume = config.getMusicVolume();
	volume = MIN(255, volume+25);
	config.setMusicVolume(volume);
	scr->showVolume(volume);
}

static void handleSFXVolumeDown()
{
	int volume = config.getSFXVolume();
	volume = MAX(0, volume-25);
	config.setSFXVolume(volume);
	scr->showVolume(volume);
}

static void handleSFXVolumeUp()
{
	int volume = config.getSFXVolume();
	volume = MIN(255, volume+25);
	config.setSFXVolume(volume);
	scr->showVolume(volume);
}

void handleSpecialKeys()
{
	bool restart = false;

	if (key[KEY_COMMA] && key_shifts & KB_SHIFT_FLAG) {
		handleMusicVolumeDown();
		restart = true;
	}
	else if (key[KEY_STOP] && key_shifts & KB_SHIFT_FLAG) {
		handleMusicVolumeUp();
		restart = true;
	}
	else if (key[KEY_OPENBRACE]) {
		handleSFXVolumeDown();
	}
	else if (key[KEY_CLOSEBRACE]) {
		handleSFXVolumeUp();
	}
	else {
		return;
	}

	if (restart) {
		restartMusic();
	}

	scr->draw();
	rest(200);
}

bool escapePressed()
{
	if (key[config.getKey2()]) {
		return true;
	}

	if (!config.getGamepadAvailable())
		return false;

	poll_joystick();

	if (joy[0].button[config.getJoyButton2()].b) {
		return true;
	}

	return false;
}

void Input::set(bool l, bool r, bool u, bool d)
{
	if (l) {
		if ((!descriptor.up && !descriptor.down) || !descriptor.right)
			descriptor.direction = DIRECTION_WEST;
		descriptor.left = true;
	}
	else
		descriptor.left = false;

	if (r) {
		if ((!descriptor.up && !descriptor.down) || !descriptor.left)
			descriptor.direction = DIRECTION_EAST;
		descriptor.right = true;
	}
	else
		descriptor.right = false;

	if (u) {
		if ((!descriptor.left && !descriptor.right) || !descriptor.down)
			descriptor.direction = DIRECTION_NORTH;
		descriptor.up = true;
	}
	else
		descriptor.up = false;

	if (d) {
		if ((!descriptor.left && !descriptor.right) || !descriptor.up)
			descriptor.direction = DIRECTION_SOUTH;
		descriptor.down = true;
	}
	else
		descriptor.down = false;
}

void Input::set(bool l, bool r, bool u, bool d, bool b1, bool b2)
{
	if (l) {
		if ((!descriptor.up && !descriptor.down) || !descriptor.right)
			descriptor.direction = DIRECTION_WEST;
		descriptor.left = true;
	}
	else
		descriptor.left = false;

	if (r) {
		if ((!descriptor.up && !descriptor.down) || !descriptor.left)
			descriptor.direction = DIRECTION_EAST;
		descriptor.right = true;
	}
	else
		descriptor.right = false;

	if (u) {
		if ((!descriptor.left && !descriptor.right) || !descriptor.down)
			descriptor.direction = DIRECTION_NORTH;
		descriptor.up = true;
	}
	else
		descriptor.up = false;

	if (d) {
		if ((!descriptor.left && !descriptor.right) || !descriptor.up)
			descriptor.direction = DIRECTION_SOUTH;
		descriptor.down = true;
	}
	else
		descriptor.down = false;

	descriptor.button1 = b1;
	descriptor.button2 = b2;
}

void Input::set(bool b1, bool b2)
{
	descriptor.button1 = b1;
	descriptor.button2 = b2;
}

InputDescriptor* Input::getDescriptor()
{
	return &descriptor;
}

void Input::setDirection(Direction direction)
{
	descriptor.direction = direction;
}

Input::Input()
{
	descriptor.left = false;
	descriptor.right = false;
	descriptor.up = false;
	descriptor.down = false;
	descriptor.button1 = false;
	descriptor.button2 = false;
	descriptor.direction = DIRECTION_SOUTH;
}

void KeyboardInput::update()
{
	if (keyboard_needs_poll())
		poll_keyboard();
	
	bool l, r, u, d, b1, b2;

	l = key[config.getKeyLeft()];
	r = key[config.getKeyRight()];
	u = key[config.getKeyUp()];
	d = key[config.getKeyDown()];
	b1 = key[KEY_ENTER] || key[config.getKey1()];
	b2 = key[config.getKey2()];
	
	set(l, r, u, d, b1, b2);
}

void GamepadInput::update()
{
	poll_joystick();

	bool l, r, u, d, b1, b2;

	l = joy[0].stick[0].axis[0].d1;
	r = joy[0].stick[0].axis[0].d2;
	u = joy[0].stick[0].axis[1].d1;
	d = joy[0].stick[0].axis[1].d2;
	b1 = joy[0].button[config.getJoyButton1()].b;
	b2 = joy[0].button[config.getJoyButton2()].b;

	set(l, r, u, d, b1, b2);
}

void gamepad2Keypresses(int step)
{
	const int INITIAL_REPEAT_DELAY = 400;
	const int REPEAT_DELAY = 25;
	const int CLEAR_DELAY = 400;

	static int l_count = INITIAL_REPEAT_DELAY;
	static int r_count = INITIAL_REPEAT_DELAY;
	static int u_count = INITIAL_REPEAT_DELAY;
	static int d_count = INITIAL_REPEAT_DELAY;
	static int b1_count = INITIAL_REPEAT_DELAY;
	static int b2_count = INITIAL_REPEAT_DELAY;
	static int clear_count = CLEAR_DELAY;
	static int l_delay = INITIAL_REPEAT_DELAY;
	static int r_delay = INITIAL_REPEAT_DELAY;
	static int u_delay = INITIAL_REPEAT_DELAY;
	static int d_delay = INITIAL_REPEAT_DELAY;
	static int b1_delay = INITIAL_REPEAT_DELAY;
	static int b2_delay = INITIAL_REPEAT_DELAY;

	if (joy[0].stick[0].axis[0].d1) {
		l_count -= step;
	}
	if (joy[0].stick[0].axis[0].d2) {
		r_count -= step;
	}
	if (joy[0].stick[0].axis[1].d1) {
		u_count -= step;
	}
	if (joy[0].stick[0].axis[1].d2) {
		d_count -= step;
	}
	if (joy[0].button[config.getJoyButton1()].b) {
		b1_count -= step;
	}
	if (joy[0].button[config.getJoyButton2()].b) {
		b2_count -= step;
	}
	clear_count -= step;

	poll_joystick();

	if (clear_count <= 0) {
		clear_count = CLEAR_DELAY;
		clear_keybuf();
	}

	if (l_count <= 0 && joy[0].stick[0].axis[0].d1) {
		if (l_delay == INITIAL_REPEAT_DELAY) {
			l_count = INITIAL_REPEAT_DELAY;
			l_delay = REPEAT_DELAY;
		}
		else {
			l_count = REPEAT_DELAY;
		}
		simulate_keypress(config.getKeyLeft() << 8);
	}
	else if (!joy[0].stick[0].axis[0].d1) {
		l_count = 0;
		l_delay = INITIAL_REPEAT_DELAY;
	}

	if (r_count <= 0 && joy[0].stick[0].axis[0].d2) {
		if (r_delay == INITIAL_REPEAT_DELAY) {
			r_count = INITIAL_REPEAT_DELAY;
			r_delay = REPEAT_DELAY;
		}
		else {
			r_count = REPEAT_DELAY;
		}
		simulate_keypress(config.getKeyRight() << 8);
	}
	else if (!joy[0].stick[0].axis[0].d2) {
		r_count = 0;
		r_delay = INITIAL_REPEAT_DELAY;
	}

	if (u_count <= 0 && joy[0].stick[0].axis[1].d1) {
		if (u_delay == INITIAL_REPEAT_DELAY) {
			u_count = INITIAL_REPEAT_DELAY;
			u_delay = REPEAT_DELAY;
		}
		else {
			u_count = REPEAT_DELAY;
		}
		simulate_keypress(config.getKeyUp() << 8);
	}
	else if (!joy[0].stick[0].axis[1].d1) {
		u_count = 0;
		u_delay = INITIAL_REPEAT_DELAY;
	}

	if (d_count <= 0 && joy[0].stick[0].axis[1].d2) {
		if (d_delay == INITIAL_REPEAT_DELAY) {
			d_count = INITIAL_REPEAT_DELAY;
			d_delay = REPEAT_DELAY;
		}
		else {
			d_count = REPEAT_DELAY;
		}
		simulate_keypress(config.getKeyDown() << 8);
	}
	else if (!joy[0].stick[0].axis[1].d2) {
		d_count = 0;
		d_delay = INITIAL_REPEAT_DELAY;
	}
	
	if (b1_count <= 0 && joy[0].button[config.getJoyButton1()].b) {
		if (b1_delay == INITIAL_REPEAT_DELAY) {
			b1_count = INITIAL_REPEAT_DELAY;
			b1_delay = REPEAT_DELAY;
		}
		else {
			b1_count = REPEAT_DELAY;
		}
		simulate_keypress(KEY_ENTER << 8);
	}
	else if (!joy[0].button[config.getJoyButton1()].b) {
		b1_count = 0;
		b1_delay = INITIAL_REPEAT_DELAY;
	}
	
	if (b2_count <= 0 && joy[0].button[config.getJoyButton2()].b) {
		if (b2_delay == INITIAL_REPEAT_DELAY) {
			b2_count = INITIAL_REPEAT_DELAY;
			b2_delay = REPEAT_DELAY;
		}
		else {
			b2_count = REPEAT_DELAY;
		}
		simulate_keypress(config.getKey2() << 8);
	}
	else if (!joy[0].button[config.getJoyButton2()].b) {
		b2_count = 0;
		b2_delay = INITIAL_REPEAT_DELAY;
	}
}

Input* createInput(InputType type)
{
	switch (type) {
		case INPUT_GAMEPAD:
			return new GamepadInput();
		default:
			return new KeyboardInput();
	}
}

static void doWait()
{
	long start = currentTimeMillis();
	
	rest(1);
	if (keyboard_needs_poll())
		poll_keyboard();
	if (config.getGamepadAvailable())
		poll_joystick();
	long now = currentTimeMillis();
	int step = now - start;
	start = now;
	gamepad2Keypresses(step);
	updateMusic();
}

void waitForRelease()
{
	if (config.getGamepadAvailable()) {
		do {
			doWait();
		} while (key[config.getKey2()] || key[KEY_ENTER] || key[config.getKey1()]
			|| joy[0].button[config.getJoyButton1()].b
			|| joy[0].button[config.getJoyButton2()].b
			|| joy[0].button[config.getJoyButton3()].b);
	}
	else {
		do {
			doWait();
		} while (key[config.getKey2()] || key[KEY_ENTER] || key[config.getKey1()]);
	}
}
