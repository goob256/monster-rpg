#include <allegro.h>

#include "monster.h"

const int SELECTOR_W = (800 - 640) / TILE_SIZE;
const int SELECTOR_H = 480 / TILE_SIZE - 1;

int editor_x = 0;
int editor_y = 0;
int editor_tile = 0;
bool editor_show_solids = true;
int paint_layer = -1;
int tile_page = 0;
bool redraw = true;

ScreenDescriptor gfx_mode;
Area* area;

int timeStep = 60;

BITMAP* buffer;

bool handleKeys()
{
	int k;

	if (keypressed()) {
	    k = readkey() >> 8;
	    redraw = true;
	}
	else {
	    rest(10);
	    return true;
	}

	switch (k) {
		case KEY_LEFT:
			if (editor_x)
				editor_x--;
			switch (paint_layer) {
				case 0:
					clear_keybuf();
					simulate_keypress(KEY_1 << 8);
					break;
				case 1:
					clear_keybuf();
					simulate_keypress(KEY_2 << 8);
					break;
				case 2:
					clear_keybuf();
					simulate_keypress(KEY_3 << 8);
					break;
			}
			break;
		case KEY_RIGHT:
			if ((editor_x+1) < area->WIDTH)
				editor_x++;
			switch (paint_layer) {
				case 0:
					clear_keybuf();
					simulate_keypress(KEY_1 << 8);
					break;
				case 1:
					clear_keybuf();
					simulate_keypress(KEY_2 << 8);
					break;
				case 2:
					clear_keybuf();
					simulate_keypress(KEY_3 << 8);
					break;
			}
			break;
		case KEY_UP:
			if (editor_y)
				editor_y--;
			switch (paint_layer) {
				case 0:
					clear_keybuf();
					simulate_keypress(KEY_1 << 8);
					break;
				case 1:
					clear_keybuf();
					simulate_keypress(KEY_2 << 8);
					break;
				case 2:
					clear_keybuf();
					simulate_keypress(KEY_3 << 8);
					break;
			}
			break;
		case KEY_DOWN:
			if ((editor_y+1) < area->HEIGHT)
				editor_y++;
			switch (paint_layer) {
				case 0:
					clear_keybuf();
					simulate_keypress(KEY_1 << 8);
					break;
				case 1:
					clear_keybuf();
					simulate_keypress(KEY_2 << 8);
					break;
				case 2:
					clear_keybuf();
					simulate_keypress(KEY_3 << 8);
					break;
			}
			break;
		case KEY_Q:
			if (editor_tile < (numTiles-1))
				editor_tile++;
			break;
		case KEY_A:
			if (editor_tile)
				editor_tile--;
			break;
		case KEY_1:
			if (key_shifts & KB_SHIFT_FLAG) {
				for (int y = 0; y < area->HEIGHT; y++) {
					for (int x = 0; x < area->WIDTH; x++) {
						Tile* tile = area->getTile(x, y);
						tile->anims[0] = editor_tile;
					}
				}
			}
			else if (key_shifts & KB_CTRL_FLAG) {
				paint_layer = 0;
			}
			else {
				Tile* tile = area->getTile(editor_x, editor_y);
				tile->anims[0] = editor_tile;
			}
			break;
		case KEY_2:
			if (key_shifts & KB_SHIFT_FLAG) {
				for (int y = 0; y < area->HEIGHT; y++) {
					for (int x = 0; x < area->WIDTH; x++) {
						Tile* tile = area->getTile(x, y);
						tile->anims[1] = editor_tile;
					}
				}
			}
			else if (key_shifts & KB_CTRL_FLAG) {
				paint_layer = 1;
			}
			else {
				Tile* tile = area->getTile(editor_x, editor_y);
				tile->anims[1] = editor_tile;
			}
			break;
		case KEY_3:
			if (key_shifts & KB_SHIFT_FLAG) {
				for (int y = 0; y < area->HEIGHT; y++) {
					for (int x = 0; x < area->WIDTH; x++) {
						Tile* tile = area->getTile(x, y);
						tile->anims[2] = editor_tile;
					}
				}
			}
			else if (key_shifts & KB_CTRL_FLAG) {
				paint_layer = 2;
			}
			else {
				Tile* tile = area->getTile(editor_x, editor_y);
				tile->anims[2] = editor_tile;
			}
			break;
		case KEY_DEL:
			{
				Tile* tile = area->getTile(editor_x, editor_y);
				tile->anims[0] = -1;
				tile->anims[1] = -1;
				tile->anims[2] = -1;
			}
			break;
		case KEY_ENTER:
			{
				char filename[1000];
				strcpy(filename, "");
				if (file_select_ex("Save As:", filename, NULL,
						1000, 600, 400) != 0) {
					try {
						if (file_exists(filename, FA_ALL, 0)) {
							clear(screen);
							textprintf_ex(screen, font, 350, 300, makecol(255, 0, 0), -1, "Overwrite? (y/n)");
							rest(1000);
							clear_keybuf();
							int k = readkey() >> 8;
							if (k == KEY_Y)
								area->save(filename);
						}
						else
							area->save(filename);
					}
					catch (WriteError e) {
						allegro_message("Not saved.");
					}
				}
			}
			break;
		case KEY_S:
			{
				Tile* tile = area->getTile(editor_x, editor_y);
				tile->solid = !tile->solid;
			}
			break;
		case KEY_V:
			editor_show_solids = !editor_show_solids;
			break;
		case KEY_ESC:
			if (prompt(false, "Really quit?", "", ""))
				return false;
		case KEY_SPACE:
			paint_layer = -1;
			break;
		case KEY_PGUP:
			if (tile_page)
			    tile_page--;
			break;
		case KEY_PGDN:
			{
			int top = (tile_page+1)*(SELECTOR_W*SELECTOR_H);
			if (top < numTiles) {
			    tile_page++;
			}
			}
			break;
		case KEY_L:
			{
			char filename[1000];
			strcpy(filename, "");
			if (file_select_ex("Load:", filename, NULL,
					1000, 600, 400) != 0) {
				Area *new_area = new Area(filename);
				if (area) {
					delete area;
					area = new_area;
				}
				else {
					allegro_message("Couldn't load area.");
				}
			}
			}
			break;
	}

	return true;
}

void run()
{
	BITMAP* backBuffer = scr->getBackBuffer();

	for (;;) {
		area->draw(backBuffer);
		if (editor_show_solids) {
			for (int y = 0; y < area->HEIGHT; y++) {
				for (int x = 0; x < area->WIDTH; x++) {
					Tile* t = area->getTile(x, y);
					if (t->solid) {
						int lx = x * TILE_SIZE;
						int ly = y * TILE_SIZE;
						line(backBuffer, lx, ly, lx+TILE_SIZE-1,
							ly+TILE_SIZE-1, makecol(255, 255, 0));
						line(backBuffer, lx, ly+TILE_SIZE-1,
							lx+TILE_SIZE-1, ly,
							makecol(255, 255, 0));
					}
				}
			}
		}
		clear_to_color(buffer, makecol(255, 0, 255));
		Animation* tile;
		if (redraw) {
		    for (int y = 0; y < SELECTOR_H; y++) {
			for (int x = 0; x < SELECTOR_W; x++) {
			    int t = (tile_page*(SELECTOR_W*SELECTOR_H))+(y*SELECTOR_W)+x;
			    if (t >= numTiles)
				continue;
			    tile = getTile(t);
			    tile->draw(buffer, x*TILE_SIZE, y*TILE_SIZE);
			}
		    }
		    tile = getTile(editor_tile);
		    rectfill(screen, 640, 0, 640+TILE_SIZE, 0+TILE_SIZE, 0);
		    tile->draw(screen, 640, 0);
		    rectfill(screen, 0, 490, 800, 520, 0);
		    textprintf_ex(screen, font, 0, 490, makecol(255, 255, 255), -1,
				    "%d,%d (%d,%d)", editor_x, editor_y,
				    editor_x*TILE_SIZE,
				    editor_y*TILE_SIZE);
		    blit(buffer, screen, 0, 0, 640, 16, buffer->w, buffer->h);
		    scr->draw();
		}
		redraw = false;
		if (!handleKeys())
			break;
		if (mouse_needs_poll())
		    poll_mouse();
		if (mouse_b & 1) {
		    redraw = true;
		    if (mouse_x > 640 && mouse_y < 480 && mouse_y > 16) {
			int x = (mouse_x-640) / 16;
			int y = (mouse_y/16)-1;
			editor_tile = (tile_page*(SELECTOR_W*SELECTOR_H))+y*SELECTOR_W+x;
		    }
		}
	}
}

int main(int argc, char** argv)
{
	start(false);
	/*
	allegro_init();
	install_keyboard();
	install_timer();
	*/

	gfx_mode.width = 800;
	gfx_mode.height = 600;
	gfx_mode.depth = desktop_color_depth();
	gfx_mode.fullscreen = false;

	/*
	try {
		scr = new Screen(&gfx_mode);
	}
	catch (...) {
		allegro_message("Error setting graphics mode.");
		return 1;
	}
	*/

	scr->setOffset(0, 0);

	if (argc < 2) {
		area = new Area();
	}
	else {
		area = new Area(argv[1]);
	}

	loadTileAnimations();

	install_mouse();
	show_mouse(screen);

	buffer = create_bitmap(TILE_SIZE*SELECTOR_W, TILE_SIZE*SELECTOR_H);

	run();

	return 0;
}
END_OF_MAIN()
