#ifndef INPUT_H
#define INPUT_H

enum InputType {
	INPUT_KEYBOARD = 0,
	INPUT_GAMEPAD
};

enum Direction {
	DIRECTION_NORTH = 0,
	DIRECTION_EAST,
	DIRECTION_SOUTH,
	DIRECTION_WEST
};

struct InputDescriptor {
	bool left;
	bool right;
	bool up;
	bool down;
	bool button1;
	bool button2;
	Direction direction;
};

class Input {
public:
	void set(bool l, bool r, bool u, bool d);
	void set(bool b1, bool b2);
	void set(bool l, bool r, bool u, bool d, bool b1, bool b2);
	virtual void update() = 0; 
	InputDescriptor* getDescriptor();
	void setDirection(Direction direction);
	bool isPlayerControlled() { return playerControlled; }
	Input();
	virtual ~Input() {}
protected:
	InputDescriptor descriptor;
	bool playerControlled;
};

class KeyboardInput : public Input {
public:
	void update();
	KeyboardInput() { playerControlled = true; }
	~KeyboardInput() {}
};

class GamepadInput : public Input {
public:
	void update();
	GamepadInput() { playerControlled = true; }
	~GamepadInput() {}
};

class ScriptInput : public Input {
public:
    ScriptInput() { playerControlled = false; }
    void update() {}
};

extern void gamepad2Keypresses(int step);
extern Input* createInput(InputType type);
extern bool escapePressed();
extern void handleSpecialKeys();
extern void waitForRelease();

#endif
