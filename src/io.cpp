#include <cstdio>
#include <cstring>

#include "monster.h"

/*
 * Write 32 bits, little endian.
 */
void iputl(long l, PACKFILE *f) throw (WriteError)
{
	if (pack_putc(l & 0xFF, f) == EOF) {
		throw new WriteError();
	}
	if (pack_putc((l >> 8) & 0xFF, f) == EOF) {
		throw new WriteError();
	}
	if (pack_putc((l >> 16) & 0xFF, f) == EOF) {
		throw new WriteError();
	}
	if (pack_putc((l >> 24) & 0xFF, f) == EOF) {
		throw new WriteError();
	}
}

/*
 * Read 32 bits, little endian
 */
long igetl(PACKFILE *f) throw (ReadError)
{
	int c1 = pack_getc(f);
	if (c1 == EOF) throw new ReadError();
	int c2 = pack_getc(f);
	if (c2 == EOF) throw new ReadError();
	int c3 = pack_getc(f);
	if (c3 == EOF) throw new ReadError();
	int c4 = pack_getc(f);
	if (c4 == EOF) throw new ReadError();
	return (long)c1 | ((long)c2 << 8) | ((long)c3 << 16) | ((long)c4 << 24);
}

void my_pack_putc(int c, PACKFILE* f) throw (WriteError)
{
	if (pack_putc(c, f) == EOF)
		throw new WriteError();
}

int my_pack_getc(PACKFILE* f) throw (ReadError)
{
	int c = pack_getc(f);
	if (c == EOF)
		throw new ReadError();
	return c;
}

char* readString(PACKFILE* f) throw (ReadError)
{
	static char buf[1024];
	int length = igetl(f);
	int i;

	debug_message("readString: length=%d\n", length);

	for (i = 0; i < length; i++)
		buf[i] = my_pack_getc(f);
	buf[i] = 0;

	return buf;
}

void writeString(char* s, PACKFILE* f) throw (WriteError)
{
	int length = strlen(s);

	iputl(length, f);

	for (int i = 0; i < length; i++)
		my_pack_putc(s[i], f);
}

/*
 * Read milestones from a file.
 * Milestones are stored as one big bit field, the
 * first bit in the file being the first milestone.
 */
bool readMilestones(bool* ms, int num, PACKFILE* f)
{
	for (int i = 0; i < num/8; i++) {
		int c = pack_getc(f);
		if (c == EOF) {
			return false;
		}
		for (int j = 0; j < 8; j++) {
			if (i*8+j >= num)
				break;
			ms[i*8+j] = c & 0x80;
			c <<= 1;
		}
	}
	return true;
}

bool writeMilestones(bool* ms, int num, PACKFILE* f)
{
	int i;
	int count = 0;
	for (i = 0; i < num/8; i++) {
		int c = 0;
		for (int j = 0; j+i*8 < num && j < 8; j++) {
			c |= (ms[j+i*8] << (7-j));
		}
		if (pack_putc(c, f) == EOF)
			return false;
		else
			count++;
	}
	debug_message("writeMilestones: wrote %d bytes\n", count);
	return true;
}

void readItem(Item* item, PACKFILE* f) throw (ReadError)
{
	if (item) {
		item->id = igetl(f);
		item->quantity = igetl(f);
		// FIXME:
		/*
		if (item->id < 0) {
				item->id = 1;
				item->quantity = 1;
		}
		else {
				item->quantity++;
		}
		*/
	}
	else {
		igetl(f);
		igetl(f);
	}
}

void writeItem(Item* item, PACKFILE* f) throw (WriteError)
{
	iputl(item->id, f);
	iputl(item->quantity, f);
}

void writeStats(PlayerStats* stats, PACKFILE* f) throw (WriteError)
{
	iputl(stats->experience, f);
	iputl(stats->weapon, f);
	iputl(stats->armor, f);
	iputl(stats->mana, f);
	iputl(stats->maxMana, f);
	iputl(stats->hp, f);
	iputl(stats->maxHP, f);
	iputl(stats->gold, f);
}

void readStats(PlayerStats* stats, PACKFILE* f) throw (ReadError)
{
	stats->experience = igetl(f);
	stats->weapon = igetl(f);
	stats->armor = igetl(f);
	stats->mana = igetl(f);
	stats->maxMana = igetl(f);
	stats->hp = igetl(f);
	stats->maxHP = igetl(f);
	stats->gold = igetl(f);
}

void saveGame(char* filename) throw (WriteError)
{
	PACKFILE* f = pack_fopen(filename, "wp");
	if (!f)
		throw WriteError();

	char* areaName = currArea->getName();
	debug_message("Saving in area %s\n", areaName);
	int x = player->getX();
	int y = player->getY();
	int direction = player->getDirection();

	if (!writeMilestones(stats.milestones, MAX_MILESTONES, f)) {
		pack_fclose(f);
		throw WriteError();
	}

	try {
		writeString(areaName, f);
		iputl(x, f);
		iputl(y, f);
		iputl((int)direction, f);
		writeStats(&stats, f);
		for (int i = 0; i < MAX_INVENTORY; i++) {
			writeItem(&(stats.inventory[i]), f);
		}
		/*
		 * Workaround because the initial alpha was released without
		 * spells being saved, this allows the old save games to still work
		 */
		iputl(9999, f);
		for (int i = 0; i < MAX_SPELLS; i++) {
				if (stats.spells[i] != 0) {
					debug_message("writing spell %s\n", stats.spells[i]);
					writeString(stats.spells[i], f);
				}
				else {
						writeString("", f);
				}
		}
		iputl(partner, f);
		writeStats(&partnerStats, f);
		iputl(accumulated_distance, f);
	}
	catch (...) {
		pack_fclose(f);
		throw WriteError();
	}

	pack_fclose(f);
}

void loadGame(char* filename) throw (ReadError)
{
	PACKFILE* f = pack_fopen(filename, "rp");
	if (!f)
		throw ReadError();

	char* areaName;

	readMilestones(stats.milestones, MAX_MILESTONES, f);

	try {
		areaName = strdup(readString(f));
		int x = igetl(f);
		int y = igetl(f);
		Direction direction = (Direction)igetl(f);

		player->setPosition(x, y);
		player->setDirection(direction);

		readStats(&stats, f);
		for (int i = 0; i < MAX_INVENTORY; i++) {
			readItem(&stats.inventory[i], f);
		}
		/*
		 * Hack check that can be removed at some point.
		 * I just want my old save states to work :)
		 */
		partner = igetl(f);
		if (partner == 9999) {
			for (int i = 0; i < MAX_SPELLS; i++) {
					char* spell = readString(f);
					if (strlen(spell) > 0) {
							debug_message("found spell %s\n", spell);
							stats.spells[i] = strdup(spell);
					}
					else {
							stats.spells[i] = 0;
					}
			}
			partner = igetl(f);
		}

		readStats(&partnerStats, f);
		accumulated_distance = igetl(f);
	}
	catch (...) {
		pack_fclose(f);
		throw ReadError();
	}

	pack_fclose(f);

	startArea(areaName);
	free(areaName);
}
