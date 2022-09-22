#ifndef GFX_H
#define GFX_H

#include <new>
#include <vector>

#include <allegro.h>

#include "error.h"
#include "input.h"
#include "object.h"

enum ScreenMode { FULLSCREEN = 0, WINDOWED = 1 };

const int DEFAULT_SCREEN_WIDTH = 640;
const int DEFAULT_SCREEN_HEIGHT = 480;
const int DEFAULT_COLOR_DEPTH = 16;
const int DEFAULT_SCREEN_MODE = FULLSCREEN;

const int FIRE_W = 200;
const int FIRE_H = 100;
const int FLAME_DELAY = 10;

const int SHOW_VOLUME_TIME = 2000;

struct ScreenDescriptor {
	int width;	// screen width
	int height;	// screen height
	int depth;	// color depth (bits per pixel)
	bool fullscreen;
};

/*
 * A simple double buffered screen object.
 */
class Screen {
public:
	static const int BUFFER_WIDTH = 320;
	static const int BUFFER_HEIGHT = 240;
	static const int FADE_TIME = 600;
	static const int SCROLL_TIME_X = 600;
	static const int SCROLL_TIME_Y = SCROLL_TIME_X * BUFFER_HEIGHT / BUFFER_WIDTH;
	static const int VOLUME_W = 64;
	static const int VOLUME_H = 8;
	static const int MAX_SCALE = 3;
	ScreenDescriptor* getDescriptor();
	void drawRect(int x, int y, int w, int h);
	void drawDirty(std::vector<Object*>* objects);
	void draw();
	/*
	 * Set the pixel where the areas top left corner will be drawn.
	 * Used when the buffer is smaller than the screen.
	 * By default the buffer is centered on the screen.
	 * The area editor uses this to shift the displayed area
	 * to the left so a panel can be drawn to the right.
	 */
	void setOffset(int x, int y);
	BITMAP* getBackBuffer();
	BITMAP* getFXBuffer();
	BITMAP* getFXBuffer2();
	void fadeOut();
	void fadeIn();
	void scroll(Direction direction);
	int getFPS(long elapsedTime);
	void resetFrameCounter();
	int getXOffset();
	int getYOffset();
	int getScale();
	void showVolume(int volume);
	void drawVolume();
	/*
	 * This constructor throws a std::bad_alloc exception if the
	 * back buffer could not be allocated, or a NoMode exception
	 * if no graphics mode could be set.
	 */
	Screen(ScreenDescriptor* wantedMode) throw (std::bad_alloc, NoMode);
	~Screen();
private:
	void scrollNorth();
	void scrollEast();
	void scrollSouth();
	void scrollWest();
	bool setMode(ScreenDescriptor* mode);
	bool createBuffer();
	ScreenDescriptor mode;
	BITMAP* backBuffer;
	BITMAP* fxBuffer;
	BITMAP* fxBuffer2;
	BITMAP* scaleBuffer;
	BITMAP* volumeBuffer;
	int scale;
	int x_offset;
	int y_offset;
	int framesDrawn;
	bool _showVolume;
	int showVolumeCount;
	long lastVolumeUpdate;
	int volume;
};

struct ParticleUnique
{
	float x;
	float y;
	float alpha;
};

struct Particle
{
	float start_x;
	float start_y;
	float start_alpha;
	float dx;
	float dy;
	float r;
	float g;
	float b;
	ParticleUnique trail[32];
	int trail_size;
	int trail_used;
	int lifetime;
};

extern Screen* scr;
extern bool redrawAnimatedTiles;

extern int getGradientColor(float ratio, int top_color, int bottom_color);
extern void drawMeter(BITMAP* bmp, int x, int y, int w, int h, float percent);
extern void makeBlackAndWhite(BITMAP* bmp);
extern void brighten(BITMAP* bmp, int amount);
extern int updateParticles(Particle* p, int num, bool gravity,
		bool resetWhenInvisible,
		float dr, float dg, float db,
		float da, int step, int totalLifetime);
extern void drawParticles(BITMAP* buffer, Particle* p, int num);
extern void makeRed(BITMAP* bmp);
extern void drawFocusEffect(bool reverse, int maxSize);

extern void initFlames();
extern void destroyFlames();
extern void drawFlames(int step);
extern BITMAP* getFlames();

extern void initDustEffect(int x, int y, int color);
extern bool updateDustEffect(int step);
extern void drawDustEffect(BITMAP* buffer);
extern void initFireworkEffect(int x, int y, int color);
extern bool updateFireworkEffect(int step);
extern void drawFireworkEffect(BITMAP* buffer);

#endif
