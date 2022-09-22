#ifndef IO_H
#define IO_H

#include <allegro.h>

#include "monster.h"

extern void iputl(long l, PACKFILE *f) throw (WriteError);
extern long igetl(PACKFILE *f) throw (ReadError);
extern void my_pack_putc(int c, PACKFILE* f) throw (WriteError);
extern int my_pack_getc(PACKFILE* f) throw (ReadError);
extern bool readMilestones(bool* ms, int num, PACKFILE* f);
extern bool WriteMilestones(bool* ms, int num, PACKFILE* f);
extern char* readString(PACKFILE* f) throw (ReadError);
extern void writeString(char* s, PACKFILE* f) throw (WriteError);
extern void readStats(struct PlayerStats* stats, PACKFILE* f) throw (ReadError);
extern void writeStats(struct PlayerStats* stats, PACKFILE* f) throw (WriteError);
extern void readItem(struct Item* item, PACKFILE* f) throw (ReadError);
extern void writeItem(struct Item* item, PACKFILE* f) throw (WriteError);
extern void saveGame(char* filename) throw (WriteError);
extern void loadGame(char* filename) throw (ReadError);

#endif
