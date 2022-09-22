#ifndef ANIM_H
#define ANIM_H

#include <new>

#include <allegro.h>

#include "error.h"

class Animation {
public:
	char* getName();
	int getFrame();
	void setFrame(int frame);
	int getNumFrames();
	int getDelay(int frame);
	void draw(BITMAP* bmp, int x, int y);
	void drawTrans(BITMAP* bmp, int x, int y);
	void saveFrame(BITMAP* bmp, int delay, PACKFILE* f) throw (WriteError);
	void save(PACKFILE* f) throw (WriteError);
	void loadFrame(int w, int h, BITMAP* dest, int* delay, PACKFILE* f)
		throw (ReadError);
	void load(PACKFILE* f) throw (std::bad_alloc, ReadError);
	void load(char  *filename) throw (std::bad_alloc, ReadError);
	void advance();
	bool update(int step);
	void reset();
	BITMAP* getBitmap(int frame);
	BITMAP* getCurrentBitmap();
	Animation(char* name, int numFrames, BITMAP** frames, int* delays)
		throw (std::bad_alloc);
	Animation();	// does no initialization (for loading)
	~Animation();
private:
	char* name;
	BITMAP** frames;
	/*
	 * Delay between each frame of the animation
	 */
	int* delays;
	int count;	// how long before changing frames?
	int numFrames;
	int currFrame;
};

class AnimationSet {
public:
	void setSubAnimation(int anim);
	bool setSubAnimation(const char* subName);
	void setFrame(int frame);
	int getFrame();
	void draw(BITMAP* dest, int x, int y);
	bool update(int step);
	Animation* getCurrentAnimation();
	AnimationSet(const char* filename) throw (std::bad_alloc, ReadError);
	~AnimationSet();
private:
	char* name;
	Animation** anims;
	int currAnim;
	int numAnims;
};

#endif
