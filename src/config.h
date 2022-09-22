#ifndef CONFIG_H
#define CONFIG_H

#include "graphics.h"
#include "input.h"

const int NUM_BATTLE_SPEEDS = 2;

class Configuration {
public:
	ScreenDescriptor* getWantedGraphicsMode();
	void setWantedGraphicsMode(ScreenDescriptor& sd);
	InputType getInputType();
	void setInputType(InputType it);
	bool showDebugMessages();
	bool useCompiledScripts();
	unsigned int getFrameSkip();
	void setFrameSkip(unsigned int frameSkip);
	bool getWaitForVsync();
	void setWaitForVsync(bool vs);
	//void setNoGamepadAvailable();
	//void setGamepadAvailable();
	bool getGamepadAvailable();
	bool useSmoothScaling();
	void setUseSmoothScaling(bool smooth);
	bool useDirtyRectangles();
	void setUseDirtyRectangles(bool dirty);
	int getBattleSpeed();
	void setBattleSpeed(int speed);
	int getSFXVolume();
	void setSFXVolume(int volume);
	int getMusicVolume();
	void setMusicVolume(int volume);
	int getJoyButton1();
	void setJoyButton1(int b1);
	int getJoyButton2();
	void setJoyButton2(int b2);
	int getJoyButton3();
	void setJoyButton3(int b3);
	int getKey1(void);
	void setKey1(int k1);
	int getKey2(void);
	void setKey2(int k2);
	int getKeyLeft(void);
	void setKeyLeft(int kl);
	int getKeyRight(void);
	void setKeyRight(int kr);
	int getKeyUp(void);
	void setKeyUp(int ku);
	int getKeyDown(void);
	void setKeyDown(int kd);
	void read() throw (ReadError);
	void write() throw (WriteError);
	Configuration();
private:
	InputType inputType;		// keyboard or gamepad
	ScreenDescriptor wantedMode;	// wanted screen resolution & depth
	bool debugMessages;		// show debugging messages?
	bool compiledScripts;		// use compiled scripts?
	unsigned int frameSkip;
	bool waitForVsync;
	bool loaded;	// loaded from file or default?
	bool gamepadAvailable;
	bool smoothScaling;
	bool dirtyRectangles;
	int battleSpeed;
	int sfxVolume;
	int musicVolume;
	int joyB1;
	int joyB2;
	int joyB3;
	int key1;
	int key2;
	int keyLeft;
	int keyRight;
	int keyUp;
	int keyDown;
};

extern Configuration config;

extern bool configurate();
extern char* getConfigFilename();

#endif
