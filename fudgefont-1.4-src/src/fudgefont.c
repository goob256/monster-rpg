#include <allegro.h>
#include <allegro/internal/aintern.h>

#ifdef ALLEGROGL
#include <alleggl.h>
#ifdef FUDGEFONT_USE_KERNING
#include <allegro/internal/aintern.h>
extern FONT_VTABLE *_fudgefont_color_kerning_vtable;
extern FONT_VTABLE *_fudgefont_agl_kerning_vtable;
void _fudgefont_agl_kerning(FONT *, FONT *);
#endif
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fudgefont.h"

typedef struct GlyphHelper GlyphHelper;
struct GlyphHelper
{
    BITMAP *bmp;
    int left, top, right;
    int advance, offset;
    int ft_index;
    FT_ULong unicode;
};

static FT_Library ft;
static int force_mono;
static int use_kerning;

void _fudgefont_make_font_use_kerning(FONT *f, FT_Face face,
    GlyphHelper **glyphs, int rn, int n);

/* Get a GlyphHelper for a single unicode character. */
static GlyphHelper *get_ft_glyph(FT_Face face, int unicode)
{
    GlyphHelper *glyph;
    int w, h, ew;
    BITMAP *bmp;
    int x, y;
    unsigned char *line;

    int ft_index = FT_Get_Char_Index(face, unicode);
    FT_Load_Glyph(face, ft_index, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT);

    glyph = calloc(1, sizeof *glyph);
    glyph->ft_index = ft_index;

    w = face->glyph->bitmap.width;
    h = face->glyph->bitmap.rows;
    ew = 0;

    if (!w)
        ew = 1;

    if (!h)
        h = 1;

    bmp = create_bitmap_ex(8, w + ew, h);
    clear_to_color(bmp, bitmap_mask_color(bmp));

    line = face->glyph->bitmap.buffer;
    for (y = 0; y < face->glyph->bitmap.rows; y++)
    {
        unsigned char *buffer = line;
        for (x = 0; x < face->glyph->bitmap.width; x++)
        {
            putpixel(bmp, x, y, *buffer++);
        }
        line += face->glyph->bitmap.pitch;
    }
    glyph->bmp = bmp;
    glyph->left = face->glyph->bitmap_left;
    glyph->offset = 0;
    if (glyph->left < 0)
    {
        glyph->offset = glyph->left;
        glyph->left = 0;
    }
    glyph->top = face->glyph->bitmap_top;
    glyph->right = face->glyph->advance.x >> 6;
    if (glyph->right < bmp->w)
        glyph->right = bmp->w;
    glyph->unicode = unicode;
    glyph->advance = face->glyph->advance.x >> 6;

    return glyph;
}

/* Construct an Allegro FONT out of a FreeType font face. */
static FONT *get_ft_font(FT_Face face)
{
    int maxy, miny, maxw;
    int h, r, first;
    int rn = 0;
    int *ranges = NULL;
    GlyphHelper **glyphs = NULL;

    FONT *font = _AL_MALLOC(sizeof *font);
    memset(font, 0, sizeof *font);

    /* Scan the font for glyphs and ranges. */
    int is_mono = 1;
    int advance = 0;
    int n, i = 0;
    FT_UInt g;
    FT_ULong unicode = FT_Get_First_Char(face, &g);
    while (g)
    {
        glyphs = realloc(glyphs, sizeof *glyphs * (i + 1));
        glyphs[i] = get_ft_glyph(face, unicode);
        if (advance && glyphs[i]->advance != advance)
            is_mono = 0;
        advance = glyphs[i]->advance;
        if (ranges == NULL || unicode != glyphs[ranges[rn - 1]]->unicode + 1)
        {
            rn++;
            ranges = realloc(ranges, sizeof *ranges * rn);
        }
        ranges[rn - 1] = i;
        unicode = FT_Get_Next_Char(face, unicode, &g);
        i++;
    }
    n = i;

    /* Determine height. */
    maxy = 0;
    miny = 0;
    maxw = 0;
    for (i = 0; i < n; i++)
    {
        if (glyphs[i]->top > maxy)
        {
            maxy = glyphs[i]->top;
        }
        if (glyphs[i]->top - glyphs[i]->bmp->h < miny)
        {
            miny = glyphs[i]->top - glyphs[i]->bmp->h;
        }
        if (glyphs[i]->left + glyphs[i]->right > maxw)
            maxw = glyphs[i]->left + glyphs[i]->right;
    }

    h = maxy - miny;

    /* Create FONT_COLOR_DATA glyph ranges. */
    first = 0;
    FONT_COLOR_DATA *prev = NULL;
    for (r = 0; r < rn; r++)
    {
        int last = ranges[r];

        FONT_COLOR_DATA *fcd = _AL_MALLOC(sizeof *fcd);
        memset(fcd, 0, sizeof *fcd);
        if (!font->data)
            font->data = fcd;
        fcd->begin = glyphs[first]->unicode;
        fcd->end = glyphs[last]->unicode + 1;

        fcd->bitmaps = _AL_MALLOC(sizeof *fcd->bitmaps * (fcd->end - fcd->begin));
        fcd->next = NULL;
        if (prev)
            prev->next = fcd;
        prev = fcd;

        for (i = first; i <= last; i++)
        {
            GlyphHelper *g = glyphs[i];
            int w;
            if (force_mono || is_mono)
                w = maxw;
            else
                w = g->left + g->right;
            BITMAP *bmp = create_bitmap_ex(8, w, h);
            clear_to_color(bmp, bitmap_mask_color(bmp));
            blit(g->bmp, bmp, 0, 0, g->left, h + miny - g->top, g->bmp->w, g->bmp->h);
            fcd->bitmaps[i - first] = bmp;
        }

        first = last + 1;
    }

    font->vtable = font_vtable_color;
    font->height = h;

    #ifdef FUDGEFONT_USE_KERNING
    /* Kerning. */
    if (use_kerning)
    {        
        _fudgefont_make_font_use_kerning(font, face, glyphs, rn, n);
    }
    #endif

    /* Cleanup. */
    for (i = 0; i < n; i++)
    {
        destroy_bitmap(glyphs[i]->bmp);
        free(glyphs[i]);
    }
    free(glyphs);
    free(ranges);

    return font;
}

/* Load a TTF font using FreeType, and convert to an Allegro font. */
static FONT *load_ft_font(char const *filename, RGB *pal, void *data)
{
    FT_Face face;
    FT_New_Face(ft, filename, 0, &face);

    int *size = data;
    FT_Set_Pixel_Sizes(face, 0, *size);

    if (pal)
    {
        int i;
        for (i = 0; i < 256; i++)
        {
            int c = i * 63 / 255;
            RGB rgb = {c, c, c, 0};
            pal[i] = rgb;
        }
    }

    return get_ft_font(face);
}

/* Create a palette ranging from r1/g1/b1 to r2/g2/b2, and select it. */
void fudgefont_color_range(int r1, int g1, int b1, int r2, int g2, int b2)
{
    PALETTE pal;
    int i;
    for (i = 0; i < 256; i++)
    {
        int red = r1 + (r2 - r1) * i / 255;
        int green = g1 + (g2 - g1) * i / 255;
        int blue = b1 + (b2 - b1) * i / 255;
        RGB rgb = {red * 63 / 255, green * 63 / 255, blue * 63 / 255, 0};
        pal[i] = rgb;
    }
    select_palette(pal);
}

/* Force a non-monospace font to be a monospace font. This simply adds space to
 * the right of all glyphs, so they are as wide as the widest one. It will
 * affect all subsequently loaded fonts.
 */
void fudgefont_force_mono(int onoff)
{
    force_mono = onoff;
}

/* Toggles loading of kerning info from subsequently loaded fonts. As Allegro's
 * fonts normally do not support kerning, and Fudgefont is about making TTF
 * fonts available as Allegro fonts instead of rendering directly, support for
 * this is somewhat fudgy and it is off by default.
 * You also need to enable kerning support by defining FUDGEFONT_USE_KERNING
 * during compile time.
 * 
 */
void fudgefont_set_kerning(int onoff)
{
#ifdef FUDGEFONT_USE_KERNING
    use_kerning = onoff;
#else
    (void)onoff;
#endif
}

/* Register with Allegro. */
void install_fudgefont(void)
{
    force_mono = 0;
    use_kerning = 0;
    FT_Init_FreeType(&ft);
    register_font_file_type("ttf", load_ft_font);
}

#ifdef ALLEGROGL
FONT *fudgefont_make_agl_font(FONT *f)
{
#ifdef FUDGEFONT_USE_KERNING
    int kerning = 0;
    if (f->vtable == _fudgefont_color_kerning_vtable)
    {
        kerning = 1;
        f->vtable = font_vtable_color;
    }
#endif
    FONT *af = allegro_gl_convert_allegro_font_ex(f, AGL_FONT_TYPE_TEXTURED,
        -1, GL_ALPHA8);

#ifdef FUDGEFONT_USE_KERNING
    if (kerning)
    {
        _fudgefont_agl_kerning(af, f);
    }
#endif
    return af;
}
#else
FONT *fudgefont_make_agl_font(FONT *f)
{
    (void)f;
    return NULL;
}
#endif

void fudgefont_info(FUDGEFONT_INFO *info)
{
    info->version = FUDGEFONT_VERSION;
#ifdef FUDGEFONT_USE_KERNING
    info->kerning = true;
#else
    info->kerning = false;
#endif
#ifdef ALLEGROGL
    info->allegrogl = true;
#else
    info->allegrogl = false;
#endif
}

bool fudgefont_font_has_kerning(FONT *f)
{
#ifdef FUDGEFONT_USE_KERNING
    return f->vtable == _fudgefont_color_kerning_vtable ||
        f->vtable == _fudgefont_agl_kerning_vtable;
#endif
    return false;
}
