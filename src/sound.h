#ifndef SOUND_H
#define SOUND_H

struct Sound {
	char name[256];
	long lastPlayedTime;
	SAMPLE* sample;
};

extern int playOgg(char* name);
extern void clearSampleCache();
extern void startMusic(const char* name);
extern bool stopMusic();
extern void pushMusic();
extern void updateMusic();
extern void restartMusic();
extern void destroyMusic();

#endif
