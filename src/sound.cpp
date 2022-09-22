#include <list>

#include <allegro.h>

extern "C" {
#include <logg.h>
#include <loggint.h>
}

#include "monster.h"

static std::vector<LOGG_Stream*> music;

const int MAX_CACHE_SIZE = 32;

static std::list<Sound*> cache;
static int cacheSize = 0;

static Sound* findInCache(char* name)
{
	std::list<Sound*>::iterator it = cache.begin();

	while (it != cache.end()) {
		Sound* s = *it;
		if (!strcmp(s->name, name))
			return s;
		it++;
	}
	return 0;
}

static void deleteSound(Sound* s)
{
	destroy_sample(s->sample);
	delete s;
}

static void deleteOldest()
{
	if (cacheSize <= 0)
		return;

	long oldestTime = currentTimeMillis();
	std::list<Sound*>::iterator it = cache.begin();
	std::list<Sound*>::iterator oldest;

	while (it != cache.end()) {
		Sound* s = *it;
		if (s->lastPlayedTime < oldestTime) {
			oldestTime = s->lastPlayedTime;
			oldest = it;
		}
		it++;
	}

	deleteSound(*oldest);
	cache.erase(oldest);
}

static void addToCache(Sound* sound)
{
	cache.push_back(sound);
}

static int playSound(Sound* s)
{
	return play_sample(s->sample, config.getSFXVolume(), 128, 1000, 0);
}

static Sound* loadSound(char* name)
{
	Sound* s = new Sound;
	strncpy(s->name, name, 255);
	s->sample = logg_load(getResource("sounds/%s", name));
	if (s->sample)
		return s;
	else {
		delete s;
		return 0;
	}
}

int playOgg(char* name)
{
	int ret;

	Sound* s = findInCache(name);
	if (!s) {
		s = loadSound(name);
		if (s) {
			addToCache(s);
			cacheSize++;
			if (cacheSize > MAX_CACHE_SIZE) {
				deleteOldest();
				cacheSize--;
			}
		}
		else
			return -1;
	}
	s->lastPlayedTime = currentTimeMillis();
	return playSound(s);
}

void clearSampleCache()
{
	while (cacheSize) {
		deleteOldest();
		cacheSize--;
	}
}

void startMusic(const char* name)
{
	char resName[1000];
	strcpy(resName, getResource("music/%s", name));

	int music_size = music.size();
	int music_end = music_size-1;

	if (music_size != 0 && music[music_end] && !strcmp(music[music_end]->filename, resName)) {
		return;
	}

	LOGG_Stream* s = logg_get_stream(resName, config.getMusicVolume(), 128, 1);

	if (music_size == 0) {
		music.push_back(s);
	}
	else {
		if (music[music_end]) {
			logg_destroy_stream(music[music_end]);
		}
		music[music_end] = s;
	}
}

bool stopMusic()
{
	int music_size = music.size();
	int music_end = music_size-1;

	if (music_size > 0) {
		if (music[music_end])
			logg_destroy_stream(music[music_end]);
		music.pop_back();
		music_size = music.size();
		music_end = music_size-1;
		if (music_size > 0) {
			logg_restart_stream(music[music_end]);
		}
		return true;
	}
	return false;
}

void pushMusic()
{
	int music_size = music.size();
	if (music_size > 0) {
		logg_stop_stream(music[music_size-1]);
	}
	music.push_back(0);
}

void updateMusic()
{
	int music_size = music.size();
	if (music_size > 0) {
		logg_update_stream(music[music_size-1]);
	}
}

void restartMusic()
{
	int music_end = music.size()-1;
	music[music_end]->volume = config.getMusicVolume();
	logg_stop_stream(music[music_end]);
	logg_restart_stream(music[music_end]);
}

void destroyMusic()
{
	while (stopMusic())
		;
}

