#include <allegro.h>
#include <allegro/internal/aintern.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "fudgefont.h"

typedef struct FudgefontKerningInfo FudgefontKerningInfo;
typedef struct RangeKerning RangeKerning;
typedef struct GlyphKerning GlyphKerning;
typedef struct GlyphHelper GlyphHelper;
typedef struct FONT_COLOR_DATA_KERNING FONT_COLOR_DATA_KERNING;

struct GlyphHelper
{
    BITMAP *bmp;
    int left, top, right;
    int advance, offset;
    int ft_index;
    FT_ULong unicode;
};

struct FudgefontKerningInfo
{
    FONT *font;
    int rn;
    RangeKerning **ranges;
};

struct RangeKerning
{
    int gn;
    GlyphKerning **glyphs;
};

struct GlyphKerning
{
    int offset; /* Always draw the glyph at the given offset. */
    int advance; /* Advance by that many pixels. */
    /* If a glyph in the list below is left of us, add the given extra
     * kerning offset.
     */
    char **ranges;
};

struct FONT_COLOR_DATA_KERNING
{
    FONT_COLOR_DATA fcd;
    int opengl;
    FudgefontKerningInfo *kerning;
    FONT_VTABLE *old;
};

#ifdef ALLEGROGL
#include <alleggl.h>
/* Copy this over from AllegroGL if it's not found. */
#include <allglint.h>
typedef struct FONT_COLOR_DATA_KERNING_GL FONT_COLOR_DATA_KERNING_GL;
struct FONT_COLOR_DATA_KERNING_GL
{
    FONT_AGL_DATA fcd;
    int opengl;
    FudgefontKerningInfo *kerning;
    FONT_VTABLE *old;
};
#endif

FONT_VTABLE *_fudgefont_color_kerning_vtable;
#ifdef ALLEGROGL
FONT_VTABLE *_fudgefont_agl_kerning_vtable;
#endif

static int get_range(FONT *f, int unicode, int *range, int *index, int *n)
{
    FONT_COLOR_DATA *fcd;
    int r = 0;
    fcd = f->data;
    while (fcd)
    {
        if (fcd->begin <= unicode && fcd->end > unicode)
        {
            *range = r;
            *index = unicode - fcd->begin;
            *n = fcd->end - fcd->begin;
            return 1;
        }
        r++;
        fcd = fcd->next;
    }
    return 0;
}

static int add_kerning_pair(FudgefontKerningInfo *ki, FT_Face face,
    GlyphHelper **glyphs, int gi1, int gi2)
{
    FT_Vector v;
    int kx;
    int r1, i1, n1;
    int r2, i2, n2;
    GlyphHelper *g1 = glyphs[gi1];
    GlyphHelper *g2 = glyphs[gi2];
    int unicode1 = g1->unicode;
    int unicode2 = g2->unicode;

    if (!get_range(ki->font, unicode2, &r2, &i2, &n2)) return 0;

    RangeKerning *rk = ki->ranges[r2];
    if (!rk)
    {
        rk = calloc(1, sizeof *rk);
        rk->gn = n2;
        rk->glyphs = calloc(n2, sizeof *rk->glyphs);
        ki->ranges[r2] = rk;
    }

    GlyphKerning *gk = rk->glyphs[i2];
    if (!gk)
    {
        gk = calloc(1, sizeof *gk);
        gk->ranges = calloc(ki->rn, sizeof *gk->ranges);
        rk->glyphs[i2] = gk;
    }
    gk->offset = g2->offset;
    gk->advance = g2->advance;

    FT_Get_Kerning(face, g1->ft_index, g2->ft_index, 0, &v);
    kx = v.x >> 6;
    if (kx == 0) return 0;

    if (!get_range(ki->font, unicode1, &r1, &i1, &n1)) return 0;

    char *offsets = gk->ranges[r1];
    if (!offsets)
    {
        offsets = calloc(n1, sizeof *offsets);
        gk->ranges[r1] = offsets;
    }
    
    offsets[i1] = kx;
    return 1;
}

static int get_kerning(FudgefontKerningInfo *ki,
    int unicode1, int unicode2, int *advance, int *offset)
{
    char *offsets;
    int r1, i1, n1;
    int r2, i2, n2;
    if (!get_range(ki->font, unicode2, &r2, &i2, &n2)) return 0;
    RangeKerning *rk = ki->ranges[r2];
    if (!rk) return 0;
    GlyphKerning *gk = rk->glyphs[i2];
    if (!gk) return 0;

    *advance = gk->advance;
    *offset = gk->offset;

    if (!get_range(ki->font, unicode1, &r1, &i1, &n1)) return 0;
    offsets = gk->ranges[r1];
    if (!offsets) return 0;
    return offsets[i1];
}

static void free_kerning_info(FudgefontKerningInfo *ki)
{
    int r, g, i;
    for (r = 0; r < ki->rn; r++)
    {
        RangeKerning *rk = ki->ranges[r];
        if (!rk) continue;
        for (g = 0; g < rk->gn; g++)
        {
            GlyphKerning *gk = rk->glyphs[g];
            if (!gk) continue;   
            for (i = 0; i < ki->rn; i++)
            {     
                char *offsets = gk->ranges[i];
                if (!offsets) continue;
                free(offsets);
            }
            free(gk->ranges);
            free(gk);
        }
        free(rk->glyphs);
        free(rk);
    }
    free(ki->ranges);
    free(ki);   
}

static int do_length(char const *text, FudgefontKerningInfo *ki)
{
    int c, w = 0, prev = 0;
    char const *p = text;

    while((c = ugetxc(&p)))
    {
        int a = 0, o = 0;
        w += get_kerning(ki, prev, c, &a, &o);
        w += a;
        prev = c;
    }
    return w;
}

static void do_render(FONT const *f, char const *text, int fg, int bg,
    BITMAP *bmp, int x, int y, FudgefontKerningInfo *ki)
{
    char const *p = text;
    int c, prev = 0;

    acquire_bitmap(bmp);

    if(fg < 0 && bg >= 0)
    {
        rectfill(bmp, x, y, x + text_length(f, text) - 1,
            y + text_height(f) - 1, bg);
        bg = -1;
    }

    while((c = ugetxc(&p)))
    {
        int a = 0, o = 0;
        int k = get_kerning(ki, prev, c, &a, &o);
        x += k;
        f->vtable->render_char(f, c, fg, bg, bmp, x + o, y);
        x += a;
        prev = c;
    }
    
    release_bitmap(bmp);
}

static int length(FONT const *f, char const *text)
{
    FONT_COLOR_DATA_KERNING *data = f->data;
    return do_length(text, data->kerning);
}

static void render(FONT const *f, char const *text, int fg, int bg,
    BITMAP *bmp, int x, int y)
{
    FONT_COLOR_DATA_KERNING *data = f->data;
    do_render(f, text, fg, bg, bmp, x, y, data->kerning);
}

static void destroy(FONT* f)
{
    FONT_COLOR_DATA_KERNING *data = f->data;
    void (*real_destroy)(FONT *f) = data->old->destroy;
    free_kerning_info(data->kerning);
    real_destroy(f);
}

#ifdef ALLEGROGL
static int length_gl(FONT const *f, char const *text)
{
    FONT_COLOR_DATA_KERNING_GL *data = f->data;
    return do_length(text, data->kerning);
}

static void render_gl(FONT const *f, char const *text, int fg, int bg,
    BITMAP *bmp, int x, int y)
{
    printf("%s\n", text);
    FONT_COLOR_DATA_KERNING_GL *data = f->data;
    do_render(f, text, fg, bg, bmp, x, y, data->kerning);
}

static void destroy_gl(FONT* f)
{
    FONT_COLOR_DATA_KERNING_GL *data = f->data;
    void (*real_destroy)(FONT *f) = data->old->destroy;
    free_kerning_info(data->kerning);
    real_destroy(f);
}

static AL_CONST FONT_AGL_DATA *find_range(AL_CONST FONT_AGL_DATA *f, int c) {

	while (f) {
		if ((c >= f->start) && (c < f->end))
			return f;

		f = f->next;
	}

	return NULL;
}

int allegro_gl_printf_ex_kerning(AL_CONST FONT *f, float x, float y, float z,
    AL_CONST char *format, ...)
{
	#define BUF_SIZE 1024
	char buf[BUF_SIZE];
	va_list ap;

    FONT_COLOR_DATA_KERNING_GL *data = f->data;
    FudgefontKerningInfo *ki = data->kerning;
	AL_CONST FONT_AGL_DATA *range = NULL;
	int c, pos = 0;
	int count = 0;
	AL_CONST FONT_AGL_DATA *d;
	GLint vert_order, cull_mode;
	GLint matrix_mode;
	int prev = 0;

	int restore_rasterpos = 0;
	GLuint old_texture_bind = 0;
	GLfloat old_raster_pos[4];

	if (!__allegro_gl_valid_context)
		return 0;

	if (f->vtable != _fudgefont_agl_kerning_vtable) {
		return 0;
	}

	d = (AL_CONST FONT_AGL_DATA*)f->data;

	/* Get the string */
	va_start(ap, format);
		uvszprintf(buf, BUF_SIZE, format, ap);
	va_end(ap);

#undef BUF_SIZE

	glGetIntegerv(GL_MATRIX_MODE, &matrix_mode);
	glGetIntegerv(GL_FRONT_FACE, &vert_order);
	glGetIntegerv(GL_CULL_FACE_MODE, &cull_mode);	
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	{	GLint temp;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &temp);
		old_texture_bind = (GLuint)temp;
	}

	if (d->type == AGL_FONT_TYPE_BITMAP) {
		glTranslatef(0, 0, -1);
		glBindTexture(GL_TEXTURE_2D, 0);
		
		glGetFloatv(GL_CURRENT_RASTER_POSITION, old_raster_pos);
		glRasterPos2f(x, y);
		restore_rasterpos = 1;
	}
	else if (d->type == AGL_FONT_TYPE_OUTLINE) {
		glTranslatef(x, y, z);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (d->type == AGL_FONT_TYPE_TEXTURED) {
		glTranslatef(x, y, z);
	}


	while ((c = ugetc(buf + pos)) != 0) {

		pos += ucwidth(c);

		if ((!range) || (c < range->start) || (c >= range->end)) {
			/* search for a suitable character range */
			range = find_range(d, c);

			if (!range) {
				range = find_range(d, (c = '^'));

				if (!range)
					continue;
			}
		}
		
		/* Set up texture */
		if (d->type == AGL_FONT_TYPE_TEXTURED) {
			glBindTexture(GL_TEXTURE_2D, range->texture);
		}
		
		int rc = c;
		/* draw the character */
		c -= range->start;
		c += range->list_base;
		
		int a = 0, o = 0;
        int k = get_kerning(ki, prev, rc, &a, &o);
		
		glTranslatef(k, 0, 0);
		glPushMatrix();
		glTranslatef(o, 0, 0);
		glCallList(c);
		glPopMatrix();
		glTranslatef(a, 0, 0);
		
		prev = rc;

		count++;
	}
	
	glPopMatrix();

	glMatrixMode(matrix_mode);
	glFrontFace(vert_order);
	glCullFace(cull_mode);	

	glBindTexture(GL_TEXTURE_2D, old_texture_bind);

	if (restore_rasterpos) {
		glRasterPos4fv(old_raster_pos);
	}

	return count;
}

#endif

void _fudgefont_make_color_font_use_kerning(FONT *f, FT_Face face,
    GlyphHelper **glyphs, int rn, int n)
{
    FONT_COLOR_DATA_KERNING *data;
    int i;
    
    if (!_fudgefont_color_kerning_vtable)
    {
        _fudgefont_color_kerning_vtable = malloc(sizeof *_fudgefont_color_kerning_vtable);
        *_fudgefont_color_kerning_vtable = *f->vtable;
        _fudgefont_color_kerning_vtable->text_length = length;
        _fudgefont_color_kerning_vtable->render = render;
        _fudgefont_color_kerning_vtable->destroy = destroy;
    }
#ifdef ALLEGROGL
    if (!_fudgefont_agl_kerning_vtable)
    {
        _fudgefont_agl_kerning_vtable = malloc(sizeof *_fudgefont_agl_kerning_vtable );
        *_fudgefont_agl_kerning_vtable = *f->vtable;
        _fudgefont_agl_kerning_vtable->text_length = length_gl;
        _fudgefont_agl_kerning_vtable->render = render_gl;
        _fudgefont_agl_kerning_vtable->destroy = destroy_gl;
    }
#endif
    
    printf("kerning: %d x %d glyphs.\n", n, n);
    
    FudgefontKerningInfo *kerning = calloc(1, sizeof *kerning);
    kerning->ranges = calloc(rn, sizeof *kerning->ranges);
    kerning->font = font;
    kerning->rn = rn;
    int kps = 0;
    for (i = 0; i < n; i++)
    {
        int j;
        for (j = 0; j < n; j++)
        {
            kps += add_kerning_pair(kerning, face, glyphs, i, j);
        }
    }
    
    printf(" %d pairs created.\n", kps);

    data = _AL_REALLOC(f->data, sizeof *data);
    f->data = data;
    data->kerning = kerning;
    data->old = f->vtable;
    f->vtable = _fudgefont_color_kerning_vtable;
}

void _fudgefont_make_font_use_kerning(FONT *f, FT_Face face,
    GlyphHelper **glyphs, int rn, int n)
{
    if (f->vtable == font_vtable_color)
        _fudgefont_make_color_font_use_kerning(f, face, glyphs, rn, n);
}

#ifdef ALLEGROGL
void _fudgefont_agl_kerning(FONT *f, FONT *allegro_font)
{
    FONT_COLOR_DATA_KERNING_GL *data;
    FONT_COLOR_DATA_KERNING *allegro_data = allegro_font->data;
    data = _AL_REALLOC(f->data, sizeof *data);
    f->data = data;
    data->kerning = allegro_data->kerning;
    data->old = f->vtable;
    f->vtable = _fudgefont_agl_kerning_vtable;
}
#endif
