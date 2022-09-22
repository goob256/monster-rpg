#ifndef AREA_H
#define AREA_H

#include <new>
#include <vector>

extern "C" {
#include <lua.h>
}

#include "object.h"
#include "util.h"

const int TILE_SIZE = 16;
const int TILE_FRAMES = 2;
/*
 * Delay between frames for animated tiles.
 */
const int TILE_DELAY = 333;
const int TILE_LAYERS = 3;

struct Tile {
	int anims[TILE_LAYERS];		// -1 for none
	bool solid;
};

enum TransitionType {
	TRANSITION_FADE = 1,
	TRANSITION_SCROLL_N = 2,
	TRANSITION_SCROLL_E = 3,
	TRANSITION_SCROLL_S = 4,
	TRANSITION_SCROLL_W = 5
};

extern TransitionType transitionType;

class Area {
public:
	static const int MAX_AREA_NAME = 256;
	static const int WIDTH = 20;
	static const int HEIGHT = 15;
	static const int MAX_ANIMATED_TILES_FOR_DIRTY_RECTANGLES = (WIDTH * HEIGHT) / 5;
	void updateDirty(unsigned int objNum);
	void update(int step);
	void draw(BITMAP* bmp);
	void drawSurrounding(BITMAP* dest, int layer, int tx, int ty, int n);
	void drawDirty(BITMAP* bmp);
	Tile* getTile(int x, int y);
	void writeTile(int tile, PACKFILE* f) throw (WriteError);
	Tile* loadTile(PACKFILE* f);
	void save(char* filename) throw (WriteError);
	void calcTilePositions();
	void sortObjects(int beg, int end);
	void addObject(Object* obj) throw (std::bad_alloc);
	void removeObject(int n);
	Object* getObject(int i);
	bool checkCollision(Object* o, int x, int y, BoundingBox* box);
	void initLua() throw (std::bad_alloc, ReadError, ScriptError);
	char* getName();
	std::vector<Object*>* getObjects();
	bool tileIsAnimated(int x, int y);
	void activate(unsigned int objNum, Direction direction);
	bool alreadyCollided(int o1, int o2);
	lua_State* getLuaState();
	Area(const char* filename) throw (std::bad_alloc, ReadError);
#ifdef EDITOR
	Area() throw (std::bad_alloc);	// create empty area for editor
#endif
	~Area();
private:
	void load() throw (std::bad_alloc, ReadError);
	char name[MAX_AREA_NAME];
	Tile** tiles;
	Point tilePos[WIDTH][HEIGHT];
	/*
	 * Object 0 is always the player.
	 */
	std::vector<Object*> objects;
	std::vector<Object*> sObjects;	// sorted objects
	lua_State* luaState;
	std::vector<Point*> collisions;
	bool oldDirtyRectanglesState;
};

extern Area* currArea;
extern Area* oldArea;
extern int numTiles;
extern char* prevAreaName;

extern void loadTileAnimations() throw (std::bad_alloc, ReadError);
extern void destroyTileAnimations();
extern Animation* getTile(int n);
extern void startArea(const char* name);

#endif
