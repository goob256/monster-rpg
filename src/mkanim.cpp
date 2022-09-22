#include <cstdio>

#include <allegro.h>

#include "monster.h"

int main(int argc, char** argv)
{
	if (argc < 6) {
		printf("Usage: mkanim <out.anim> <name> <numframes> <bmpfile> <delay ...>\n");
		return 1;
	}

	int numFrames = atoi(argv[3]);

	if (numFrames < 1) {
		printf("Error: numframes < 1.\n");
		return 1;
	}
	if (argc < (5 + numFrames)) {
		printf("Error: Too few arguments.\n");
		return 1;
	}

	allegro_init();
	set_color_depth(32);

	BITMAP* bmp = load_bitmap(argv[4], 0);
	int width = bmp->w / numFrames;

	BITMAP** bmps = new BITMAP*[numFrames];
	int* delays = new int[numFrames];

	for (int i = 0; i < numFrames; i++) {
		bmps[i] = create_bitmap(width, bmp->h);
		blit(bmp, bmps[i], i*width, 0, 0, 0, width, bmp->h);
		delays[i] = atoi(argv[5+i]);
	}

	Animation* a = new Animation(argv[2], numFrames, bmps, delays);
	PACKFILE* f = pack_fopen(argv[1], "wp");
	a->save(f);
	pack_fclose(f);
	delete a;

	return 0;
}
END_OF_MAIN()
