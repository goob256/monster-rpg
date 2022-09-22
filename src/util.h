#ifndef UTIL_H
#define UTIL_H

struct Point {
	int x;
	int y;
};

extern volatile bool close_button_pressed;

extern char* getUserResource(char* fmt, ...);
extern char* getResource(char* fmt, ...);
extern char* getScriptExtension();
extern bool initGamepad(void);
extern void start(bool read_cfg);
extern void stop();
extern void clearStats(struct PlayerStats* stats);
extern char* itoa(int i);

extern bool redrawEverything;

#endif
