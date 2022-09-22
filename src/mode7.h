/* Mode7Params is a struct containing all the different parameters
that are relevant for Mode 7, so you can pass them to the functions
as a single unit */
typedef struct Mode7Params
{
    fixed space_z; // this is the height of the camera above the plane
    int horizon; // this is the number of pixels line 0 is below the horizon
    fixed scale_x, scale_y; // this determines the scale of space coordinates
    // to screen coordinates
} Mode7Params;

extern Mode7Params mode7Params;

void mode_7(BITMAP *bmp, BITMAP *tile, fixed angle,
	fixed cx, fixed cy, Mode7Params& params);
