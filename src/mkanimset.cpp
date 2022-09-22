#include <cstdio>
#include <cstring>
#include <allegro.h>

#include "io.h"

int main(int argc, char** argv)
{
	if (argc < 5) {
		printf("Usage: mkanimset <out.anims> <name> <numanims> <file.anim ...>\n");
		return 1;
	}

	int numAnims = atoi(argv[3]);

	if (numAnims < 1) {
		printf("Error: numframes < 1.\n");
		return 1;
	}
	if (argc < (4 + numAnims)) {
		printf("Error: too few arguments.\n");
		return 1;
	}

	allegro_init();

	PACKFILE* f = pack_fopen(argv[1], "wp");
	if (!f) {
		printf("Error: couldn't open %s for writing.\n", argv[1]);
		return 1;
	}

	iputl(strlen(argv[2]), f);
	for (int i = 0; i < strlen(argv[2]); i++)
		pack_putc(argv[2][i], f);

	iputl(numAnims, f);

	for (int i = 0; i < numAnims; i++) {
		PACKFILE* in = pack_fopen(argv[4+i], "rp");
		int c;
		while ((c = pack_getc(in)) != EOF)
			pack_putc(c, f);
		pack_fclose(in);
	}

	pack_fclose(f);

	return 0;
}
END_OF_MAIN()
