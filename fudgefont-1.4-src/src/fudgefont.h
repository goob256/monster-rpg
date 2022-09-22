#ifndef _FUDGEFONT_H_
#define _FUDGEFONT_H_

#include <allegro.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define FUDGEFONT_VERSION 1.4

typedef struct FUDGEFONT_INFO
{
    float version;
    bool allegrogl;
    bool kerning;
} FUDGEFONT_INFO;

void install_fudgefont(void);
void fudgefont_info(FUDGEFONT_INFO *info);
void fudgefont_color_range(int r1, int g1, int b1, int r2, int g2, int b2);
void fudgefont_force_mono(int onoff);
void fudgefont_set_kerning(int onoff);
FONT *fudgefont_make_agl_font(FONT *);
int allegro_gl_printf_ex_kerning(AL_CONST FONT *f, float x, float y, float z,
    AL_CONST char *format, ...);
bool fudgefont_font_has_kerning(FONT *f);

#ifdef __cplusplus
}
#endif

#endif
