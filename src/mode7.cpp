#include "monster.h"

Mode7Params mode7Params = {
	itofix(10),
	1,
	itofix(512), itofix(512)
};

void mode_7(BITMAP *bmp, BITMAP *tile, fixed angle,
	fixed cx, fixed cy, Mode7Params& params)
{
    // current screen position
    int screen_x, screen_y;

    // the distance and horizontal scale of the line we are drawing
    fixed distance, horizontal_scale;

    // masks to make sure we don't read pixels outside the tile
    int mask_x = (tile->w - 1);
    int mask_y = (tile->h - 1);

    // step for points in space between two pixels on a horizontal line
    fixed line_dx, line_dy;

    // current space position
    fixed space_x, space_y;

    for (screen_y = 0; screen_y < bmp->h; screen_y++)
    {
        // first calculate the distance of the line we are drawing
        distance = fixmul (params.space_z, params.scale_y) /
            (screen_y + params.horizon);
        // then calculate the horizontal scale, or the distance between
        // space points on this horizontal line
        horizontal_scale = fixdiv (distance, params.scale_x);

        // calculate the dx and dy of points in space when we step
        // through all points on this line
        line_dx = fixmul (-fixsin(angle), horizontal_scale);
        line_dy = fixmul (fixcos(angle), horizontal_scale);

        // calculate the starting position
        space_x = cx + fixmul (distance, fixcos(angle)) - bmp->w/2 * line_dx;
        space_y = cy + fixmul (distance, fixsin(angle)) - bmp->w/2 * line_dy;

        // go through all points in this screen line
        for (screen_x = 0; screen_x < bmp->w; screen_x++)
        {
            // get a pixel from the tile and put it on the screen
            putpixel (bmp, screen_x, screen_y,
                getpixel (tile,
                    fixtoi (space_x) & mask_x,
                    fixtoi (space_y) & mask_y));
            // advance to the next position in space
            space_x += line_dx;
            space_y += line_dy;
        }
    }
}
