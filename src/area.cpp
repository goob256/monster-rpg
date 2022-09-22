#include <allegro.h>

#include "monster.h"

#ifdef EDITOR
#include "editor.h"
#endif

static Animation** tileAnims = 0;

int numTiles;
Area* currArea = 0;
Area* oldArea = 0;
TransitionType transitionType = TRANSITION_FADE;
char* prevAreaName = 0;

/*
 * Blit a frame of a tile in a tilemap to a smaller tile-sized bitmap.
 */
static void blitTile(int tile, int frame, BITMAP* tilemap, BITMAP* dest)
{
	int width = tilemap->w / TILE_SIZE;
	int x = (tile % width) * TILE_SIZE;
	int y = (tile / width) * (TILE_SIZE * TILE_FRAMES);
	y += frame * TILE_SIZE;

	blit(tilemap, dest, x, y, 0, 0, TILE_SIZE, TILE_SIZE);
}

/*
 * Load a tilemap into an array of Animations
 */
void loadTileAnimations() throw (std::bad_alloc, ReadError)
{
	DATAFILE* dat = load_datafile_object(getResource("tilemap.dat"),
			"TILEMAP_PCX");
	if (!dat) {
		debug_message("Error loading tilemap.\n");
		throw new ReadError();
	}

	BITMAP* tilemap = (BITMAP*)dat->dat;

	int width = tilemap->w / TILE_SIZE;
	int height = tilemap->h / TILE_SIZE * TILE_FRAMES;
	numTiles = width * height;

	try {
		tileAnims = new Animation*[numTiles];
	}
	catch (std::bad_alloc e) {
		unload_datafile_object(dat);
		throw new std::bad_alloc();
	}

	/*
	 * Initialize tileAnims to 0's so if an error occurs,
	 * we can delete the ones that have been allocated.
	 */
	for (int i = 0; i < numTiles; i++)
		tileAnims[i] = 0;

	BITMAP** bmps;

	bmps = new BITMAP*[TILE_FRAMES];
	if (!bmps) {
		unload_datafile_object(dat);
		delete[] tileAnims;
		throw new std::bad_alloc();
	}

	for (int i = 0; i < TILE_FRAMES; i++) {
		bmps[i] = create_bitmap(TILE_SIZE, TILE_SIZE);
		if (!bmps[i]) {
			unload_datafile_object(dat);
			delete[] tileAnims;
			while (--i >= 0)
				destroy_bitmap(bmps[i]);
			delete[] bmps;
			throw new std::bad_alloc();
		}
	}

	BITMAP** frames;
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int index = y * width + x;
			frames = 0;
			int* delays = 0;
			int numFrames = 0;
			try {
				for (int i = 0; i < TILE_FRAMES; i++) {
					blitTile(index, i, tilemap, bmps[i]);
					/*
					 * The first frame with a first pixel that is
					 * bright yellow (RGB 255,255,0) marks the
					 * end of the frames to use in the tile
					 * animation.
					 */
					if (getpixel(bmps[i], 0, 0) == makecol(255, 255, 0)) {
						if (numFrames == 0)
							numFrames = 1;
						break;
					}
					numFrames++;
				}
				frames = new BITMAP*[numFrames];
				delays = new int[numFrames];
				for (int i = 0; i < numFrames; i++) {
					frames[i] = 0;
					delays[i] = 0;
				}
				for (int i = 0; i < numFrames; i++) {
					frames[i] = create_bitmap(TILE_SIZE, TILE_SIZE);
					if (!frames[i]) {
						throw new std::bad_alloc();
					}
					blit(bmps[i], frames[i], 0, 0, 0, 0,
							TILE_SIZE,
							TILE_SIZE);
					delays[i] = TILE_DELAY;
				}
				char name[100];
				sprintf(name, "tile%d-%d", x, y);
				tileAnims[index] = new Animation(name,
						numFrames, frames, delays);
			}
			catch (std::bad_alloc e) {
				unload_datafile_object(dat);
				for (int i = 0; i < numTiles; i++) {
					if (tileAnims[i])
						delete tileAnims[i];
					else
						break;
				}
				delete[] tileAnims;
				if (bmps) {
					for (int i = 0; i < TILE_FRAMES; i++) {
						if (bmps[i])
							destroy_bitmap(bmps[i]);
						else
							break;
					}
					delete[] bmps;
				}
				delete[] delays;
				if (frames) {
					for (int i = 0; i < TILE_FRAMES; i++) {
						if (frames[i])
							destroy_bitmap(frames[i]);
					}
					delete[] frames;
				}
				throw new std::bad_alloc();
			}
		}
	}

	for (int i = 0; i < TILE_FRAMES; i++)
		destroy_bitmap(bmps[i]);
	delete[] bmps;

	unload_datafile_object(dat);
}

void destroyTileAnimations()
{
	for (int i = 0; i < numTiles; i++) {
		delete tileAnims[i];
	}
	delete[] tileAnims;
}

Animation* getTile(int n)
{
	return tileAnims[n];
}

void startArea(const char* name)
{
	oldArea = currArea;	

	char resName[1000];
	/* -1 because MSVC lib is dumb */
	snprintf(resName, (sizeof(resName)/sizeof(*resName))-1, "areas/%s.area", name);

	if (prevAreaName)
		free(prevAreaName);

	if (oldArea)
		prevAreaName = strdup(oldArea->getName());
	else
		prevAreaName = 0;

	currArea = new Area(getResource(resName));

	currArea->addObject(player);

	currArea->initLua();
}

void Area::draw(BITMAP* dest)
{
#ifdef EDITOR
	clear(dest);
#endif

	// Draw bottom layer

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			Tile* tile = tiles[y*WIDTH+x];

			Animation* a;

			if (tile->anims[0] >= 0) {
				a = tileAnims[tile->anims[0]];
				a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
			}
		}
	}

	// Draw middle layer and regular objects

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {

			Tile* tile = tiles[y*WIDTH+x];

			Animation* a;

			if (tile->anims[1] >= 0) {
				a = tileAnims[tile->anims[1]];
				a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
			}
		}
	}

	// Sort and draw the lower objects
	sortObjects(0, objects.size());

	for (unsigned int i = 0; i < sObjects.size(); i++) {
		if (!sObjects[i]->isHigh() && !sObjects[i]->isHidden()) {
			sObjects[i]->draw(dest);
		}
	}

	// Draw top layer

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			Tile* tile = tiles[y*WIDTH+x];

			Animation* a;

			if (tile->anims[2] >= 0) {
				a = tileAnims[tile->anims[2]];
				a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
			}
			
#ifdef EDITOR
			int tx = tilePos[x][y].x;
			int ty = tilePos[x][y].y;

			if (editor_x == x && editor_y == y) {
				rect(dest, tx, ty, tx + TILE_SIZE - 1,
						ty + TILE_SIZE - 1, 0);
				rect(dest, tx+1, ty+1, tx + TILE_SIZE - 2,
						ty + TILE_SIZE - 2,
						makecol(0, 255, 0));
				rect(dest, tx+2, ty+2, tx + TILE_SIZE - 3,
						ty + TILE_SIZE - 3, 0);
			}
#endif
		}
	}

	// Draw high and flying objects

	for (unsigned int i = 0; i < sObjects.size(); i++) {
		if (sObjects[i]->isHigh() && !sObjects[i]->isHidden())
			sObjects[i]->draw(dest);
	}
}

/*
 * Draw the tiles surrounding a tile.
 * Draw nxn tiles
 */
void Area::drawSurrounding(BITMAP* dest, int layer, int tx, int ty, int n)
{
	Animation* a;

	int stx = MAX(0, tx - (n / 2));
	int etx = MIN(tx + (n / 2) + 1, WIDTH-1);
	int sty = MAX(0, ty - (n / 2));
	int ety = MIN(ty + (n / 2) + 1, HEIGHT-1);

	for (int y = sty; y <= ety; y++) {
		for (int x = stx; x <= etx; x++) {
			if (y < 0 || y >= HEIGHT || x < 0 || x >= WIDTH)
				continue;
			Tile* tile = tiles[y*WIDTH+x];
			if (tile->anims[layer] >= 0) {
				a = tileAnims[tile->anims[layer]];
				a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
			}
		}
	}
}

/*
 * Draw only the dirty parts of the area (parts that have changed).
 * This is much faster than Area::draw.
 */
void Area::drawDirty(BITMAP* dest)
{
#ifdef EDITOR
	clear(dest);
#endif
	// Draw animated tiles (first two layers)
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			Tile* tile = tiles[y*WIDTH+x];
			Animation* a;
			if (tileIsAnimated(x, y)) {
				if (tile->anims[0] >= 0) {
					a = tileAnims[tile->anims[0]];
					a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
				}
				if (tile->anims[1] >= 0) {
					a = tileAnims[tile->anims[1]];
					a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
				}
			}
		}
	}

	// Draw bottom two layer

	for (unsigned int i = 0; i < objects.size(); i++) {
		Object* o = objects[i];
		int tx = o->getX() / TILE_SIZE;
		int ty = o->getY() / TILE_SIZE;
		int n = MAX(MAX(1, o->getWidth()/TILE_SIZE) + 2, MAX(1, o->getHeight()/TILE_SIZE) + TILE_SIZE/2);
		drawSurrounding(dest, 0, tx, ty, n);
		drawSurrounding(dest, 1, tx, ty, n);
	}

	// Sort and draw the lower objects
	sortObjects(0, objects.size());

	for (unsigned int i = 0; i < sObjects.size(); i++) {
		if (!sObjects[i]->isHigh() && !sObjects[i]->isHidden()) {
			sObjects[i]->draw(dest);
		}
	}

	// Draw animated tiles (top layer)
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			Tile* tile = tiles[y*WIDTH+x];
			if (tileIsAnimated(x, y)) {
				if (tile->anims[2] >= 0) {
					Animation* a = tileAnims[tile->anims[2]];
					a->draw(dest, tilePos[x][y].x, tilePos[x][y].y);
				}
			}
		}
	}

	// Draw top layer

	for (unsigned int i = 0; i < objects.size(); i++) {
		Object* o = objects[i];
		int tx = o->getX() / TILE_SIZE;
		int ty = o->getY() / TILE_SIZE;
		int n = MAX(MAX(1, o->getWidth()/TILE_SIZE) + 2, MAX(1, o->getHeight()/TILE_SIZE) + TILE_SIZE/2);
		drawSurrounding(dest, 2, tx, ty, n);
	}

	// Draw high and flying objects

	for (unsigned int i = 0; i < sObjects.size(); i++) {
		if (sObjects[i]->isHigh() && !sObjects[i]->isHidden())
			sObjects[i]->draw(dest);
	}
}

Tile* Area::getTile(int x, int y)
{
	return tiles[y*WIDTH+x];
}

void Area::activate(unsigned int objNum, Direction direction)
{
	Object* o = objects[objNum];

	int x1=0;
	int y1=0;
	int x2=0;
	int y2=0;

	switch (direction) {
		case DIRECTION_NORTH:
			x1 = o->getX();
			y1 = o->getY()+o->getHeight()-(TILE_SIZE*2);
			y2 = o->getY()+o->getHeight();
			break;
		case DIRECTION_EAST:
			x1 = o->getX()+o->getWidth();
			y1 = o->getY()+o->getHeight()-TILE_SIZE;
			y2 = y1+TILE_SIZE;
			break;
		case DIRECTION_SOUTH:
			x1 = o->getX();
			y1 = o->getY()+o->getHeight();
			y2 = y1+TILE_SIZE;
			break;
		case DIRECTION_WEST:
			x1 = o->getX() - TILE_SIZE;
			y1 = o->getY()+o->getHeight()-TILE_SIZE;
			y2 = y1+TILE_SIZE;
			break;
	}

	x2 = x1+TILE_SIZE;

	for (unsigned int i = 0; i < objects.size(); i++) {
		if (i == objNum)
			continue;
		Object* obj = objects[i];
		int ox1 = obj->getX();
		int oy1 = obj->getY();
		int ox2 = ox1+obj->getWidth();
		int oy2 = oy1+obj->getHeight();
		if (checkBoxCollision(x1, y1, x2, y2, ox1, oy1, ox2, oy2)) {
			callLua(luaState, "activate", "ii>", objNum+1, i+1);
			clear_keybuf();
		}
	}
}

bool Area::alreadyCollided(int o1, int o2)
{
	for (unsigned int i = 0; i < collisions.size(); i++) {
		Point* p = collisions[i];
		if ((p->x == o1 && p->y == o2) ||
			(p->x == o2 && p->y == o1))
				return true;
	}
	return false;
}

void Area::update(int step)
{
	if (oldArea) {
		delete oldArea;
		oldArea = 0;
	}

	Area* oldArea = currArea;

	for (unsigned int i = 0; i < collisions.size(); i++) {
		callLua(luaState, "collide", "ii>", collisions[i]->x+1,
				collisions[i]->y+1);
		delete collisions[i];
	}
	collisions.clear();

	if (currArea != oldArea)
		return;

	callLua(luaState, "update", "i>", step);

	for (unsigned int i = 0; i < objects.size(); i++) {
		objects[i]->update(this, step);
		Input* input = objects[i]->getInput();
		if (input) {
			InputDescriptor* id = input->getDescriptor();
			if (id->button1) {
				activate(i, id->direction);
			}
		}
	}

	for (int i = 0; i < numTiles; i++) {
		int startFrame = tileAnims[i]->getFrame();
		tileAnims[i]->update(step);
		if (tileAnims[i]->getFrame() != startFrame) {
			redrawAnimatedTiles = true;
		}
	}
}

void Area::writeTile(int tile, PACKFILE* f) throw (WriteError)
{
	iputl(tiles[tile]->anims[0], f);
	iputl(tiles[tile]->anims[1], f);
	iputl(tiles[tile]->anims[2], f);
	my_pack_putc((char)tiles[tile]->solid, f);
}

Tile* Area::loadTile(PACKFILE* f)
{
	Tile* tile = 0;

	try {
		tile = new Tile;
		tile->anims[0] = igetl(f);
		tile->anims[1] = igetl(f);
		tile->anims[2] = igetl(f);
		tile->solid = my_pack_getc(f);
	}
	catch (...) {
		if (tile)
			delete tile;
		throw new ReadError();
	}

	return tile;
}

void Area::save(char *filename) throw (WriteError)
{
	PACKFILE* f = pack_fopen(filename, "wp");
	if (!f)
		throw new WriteError();

	try {
		iputl(WIDTH, f);
		iputl(HEIGHT, f);
		for (int i = 0; i < WIDTH*HEIGHT; i++)
			writeTile(i, f);
	}
	catch (WriteError) {
		pack_fclose(f);
		throw new WriteError();
	}

	pack_fclose(f);
}

void Area::calcTilePositions()
{
	for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++) {
			tilePos[x][y].x = x * TILE_SIZE;
			tilePos[x][y].y = y * TILE_SIZE;
		}
}

/*
 * Sort objects by the Y positions,
 * lowest (top of the screen) first.
 */
void Area::sortObjects(int beg, int end)
{
	Object* tmp;
	
	if (end > beg + 1) {
		int piv = sObjects[beg]->getY(), l = beg + 1, r = end;
		while (l < r) {
			if (sObjects[l]->getY() <= piv) {
				l++;
			}
			else {
				--r;
				tmp = sObjects[l];
				sObjects[l] = sObjects[r];
				sObjects[r] = tmp;
			}
		}
		--l;
		tmp = sObjects[l];
		sObjects[l] = sObjects[beg];
		sObjects[beg] = tmp;
		sortObjects(beg, l);
		sortObjects(r, end);
	}
}

/*
 * Add an object to an area. Player should always be object 1.
 */
void Area::addObject(Object* obj) throw (std::bad_alloc)
{
	objects.push_back(obj);
	sObjects.push_back(obj);
}

/*
 * Remove an object from the object vectors and delete it.
 */
void Area::removeObject(int n)
{
	Object* o = objects[n];

	for (unsigned int i = 0; i < sObjects.size(); i++) {
		if (sObjects[i] == o) {
			std::vector<Object*>::iterator it = sObjects.begin() + i;
			sObjects.erase(it);
			break;
		}
	}

	delete objects[n];

	std::vector<Object*>::iterator it = objects.begin() + n;
	objects.erase(it);

	draw(scr->getBackBuffer());
	scr->draw();
}

/*
 * Get the ith object in the area.
 */
Object* Area::getObject(int i)
{
	return objects[i];
}

bool Area::tileIsAnimated(int x, int y)
{
	Tile* tile = tiles[y*WIDTH+x];
	Animation* a;
	if (tile->anims[0] >= 0) {
	       	a = tileAnims[tile->anims[0]];
		if (a->getNumFrames() > 1)
			return true;
	}
	if (tile->anims[1] >= 0) {
		a = tileAnims[tile->anims[1]];
		if (a->getNumFrames() > 1)
			return true;
	}
	if (tile->anims[2] >= 0) {
		a = tileAnims[tile->anims[2]];
		if (a->getNumFrames() > 1)
			return true;
	}
	return false;
}

bool Area::checkCollision(Object* obj, int x, int y, BoundingBox* box)
{
	int x1 = x / TILE_SIZE;
	int y1 = y / TILE_SIZE;
	int x2 = (x + box->width - 1) / TILE_SIZE;
	int y2 = (y + box->height - 1) / TILE_SIZE;

	x1 = MAX(0, x1);
	y1 = MAX(0, y1);
	x2 = MIN(WIDTH-1, x2);
	y2 = MIN(HEIGHT-1, y2);

	Tile* tile = tiles[y1*WIDTH+x1];
	if (tile->solid) return true;
	
	tile = tiles[y1*WIDTH+x2];
	if (tile->solid) return true;
	
	tile = tiles[y2*WIDTH+x2];
	if (tile->solid) return true;
	
	tile = tiles[y2*WIDTH+x1];
	if (tile->solid) return true;

	bool collided = false;

	for (unsigned int i = 0; i < objects.size(); i++) {
		Object* o = objects[i];
		if (obj == o || !o->isSolid())
			continue;
		unsigned int objNum;
		for (objNum = 0; objNum < objects.size(); objNum++)
			if (objects[objNum] == obj)
				break;
		int ox1 = o->getX();
		int oy1 = o->getY();
		int ox2 = ox1 + o->getWidth() - 1;
		int oy2 = oy1 + o->getHeight() - 1;
		int x2 = x + box->width - 1;
		int y2 = y + box->height - 1;
//		if (objNum == 0 || objNum == 2)
//		rect(screen, x*2, y*2, x2*2, y2*2, makecol(255, 255, 255));
//		rect(screen, ox1*2, oy1*2, ox2*2, oy2*2, makecol(255, 255, 255));
		if (obj->isSolid() && checkBoxCollision(ox1, oy1, ox2, oy2, x, y, x2, y2)) {
			collided = true;
			if (!alreadyCollided(objNum, i)) {
				Point* p = new Point;
				p->x = objNum;
				p->y = i;
				collisions.push_back(p);
			}
		}
	}

	return collided;
}

// FIXME: check for errors
void Area::initLua() throw (std::bad_alloc, ReadError, ScriptError)
{
	luaState = lua_open();

	openLuaLibs(luaState);

	registerCFunctions(luaState);

	debug_message("Loading global script...\n");
	if (luaL_loadfile(luaState, getResource("scripts/global.%s", getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running global script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Loading global area script...\n");
	if (luaL_loadfile(luaState, getResource("area_scripts/global.%s", getScriptExtension()))) {
		dumpLuaStack(luaState);
		throw new ReadError();
	}

	debug_message("Running global area script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Loading area script (%s)...\n", name);
	if (luaL_loadfile(luaState, getResource("area_scripts/%s.%s", name, getScriptExtension()))) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ReadError();
	}

	debug_message("Running area script...\n");
	if (lua_pcall(luaState, 0, 0, 0)) {
		dumpLuaStack(luaState);
		lua_close(luaState);
		throw new ScriptError;
	}

	debug_message("Calling 'start'...\n");

	callLua(luaState, "start", ">");
}

char* Area::getName()
{
	return name;
}

std::vector<Object*>* Area::getObjects()
{
	return &objects;
}

lua_State* Area::getLuaState()
{
	return luaState;
}

Area::Area(const char* filename) throw (std::bad_alloc, ReadError)
{
	PACKFILE* f = pack_fopen(filename, "rp");
	if (!f)
		throw new ReadError();

	replace_extension(name, get_filename(filename), "", 256);
	name[strlen(name)-1] = 0;

	try {
		tiles = new Tile*[WIDTH*HEIGHT];
	}
	catch (std::bad_alloc e) {
		pack_fclose(f);
	}

	for (int i = 0; i < WIDTH*HEIGHT; i++)
		tiles[i] = 0;

	luaState = 0;

	try {
		igetl(f);	// width
		igetl(f);	// height
		for (int i = 0; i < WIDTH*HEIGHT; i++) {
			tiles[i] = loadTile(f);
		}
	}
	catch (...) {
		for (int i = 0; i < WIDTH*HEIGHT; i++) {
			if (tiles[i])
				delete tiles[i];
			else
				break;
		}
		delete[] tiles;
		pack_fclose(f);
	}

	pack_fclose(f);

	calcTilePositions();

#ifndef EDITOR	
	int animatedTiles = 0;
	oldDirtyRectanglesState = config.useDirtyRectangles();

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (tileIsAnimated(x, y)) {
				animatedTiles++;
				if (animatedTiles > MAX_ANIMATED_TILES_FOR_DIRTY_RECTANGLES) {
					debug_message("Turning off dirty rectangles for this area...\n");
					config.setUseDirtyRectangles(false);
					goto here;
				}
			}
		}
	}

here:
	return;
#endif
}

#ifdef EDITOR
Area::Area() throw (std::bad_alloc)
{
	strcpy(name, "");

	tiles = new Tile*[WIDTH*HEIGHT];
	for (int i = 0; i < WIDTH*HEIGHT; i++)
		tiles[i] = 0;

	luaState = 0;

	try {
		for (int i = 0; i < WIDTH*HEIGHT; i++) {
			tiles[i] = new Tile;
			tiles[i]->anims[0] = -1;
			tiles[i]->anims[1] = -1;
			tiles[i]->anims[2] = -1;
			tiles[i]->solid = false;
		}
	}
	catch (std::bad_alloc e) {
		for (int i = 0; i < WIDTH*HEIGHT; i++) {
			if (tiles[i])
				delete tiles[i];
			else
				break;
		}
		delete[] tiles;
	}

	calcTilePositions();

	oldDirtyRectanglesState = true;
}
#endif

void Area::load() throw (std::bad_alloc, ReadError)
{
}

Area::~Area()
{
	if (luaState)
		callLua(luaState, "stop", ">");

	for (int i = 0; i < WIDTH*HEIGHT; i++) {
		delete tiles[i];
	}
	delete[] tiles;

	for (unsigned int i = 1; i < objects.size(); i++) {
		delete objects[i];
	}
	objects.clear();

	for (unsigned int i = 0; i < collisions.size(); i++) {
		delete collisions[i];
	}
	collisions.clear();

	if (luaState)
		lua_close(luaState);

	debug_message("Setting dirty rectangles to %d\n", oldDirtyRectanglesState);
	config.setUseDirtyRectangles(oldDirtyRectanglesState);
}
