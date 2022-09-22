#include <stdio.h>
#include <allegro.h>

int main(int argc, char** argv)
{
	if (argc != 6) {
		printf("Usage: mktilemap [w] [h] [num_frames] [tile_size] [outfile]\n");
		return 1;
	}

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	int num_frames = atoi(argv[3]);
	int tile_size = atoi(argv[4]);
	char* outfile = argv[5];

	allegro_init();
	set_color_depth(32);

	BITMAP* bmp = create_bitmap(w * tile_size, h * tile_size * num_frames);
	clear_to_color(bmp, makecol(255, 255, 0));

	/*
	 * Mark the bottom right corners of each frame so they're
	 * easy to spot when editing the tilemap.
	 */
	int x;
	int y;
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			int px = x * tile_size + tile_size - 1;
			int i;
			for (i = 0; i < num_frames; i++) {
				int py = y * num_frames * tile_size + (i * tile_size) + tile_size - 1;
				putpixel(bmp, px, py, 0);
			}
		}
	}

	save_bitmap(outfile, bmp, 0);

	return 0;
}
END_OF_MAIN()
