#include <cstring>
#include <stdint.h>
#include <cmath>

#include <allegro.h>

#include "monster.h"

Screen* scr = 0;
bool redrawAnimatedTiles = true;

static void my_acquire_screen(void)
{
#if !defined __linux__ && !defined ALLEGRO_MACOSX
	acquire_screen();
#endif
}

static void my_release_screen(void)
{
#if !defined __linux__ && !defined ALLEGRO_MACOSX
	release_screen();
#endif
}

/*
 * Try the mode passed to the Screen constructor then
 * the remainder of these modes in order until one is
 * found.
 */
static ScreenDescriptor modesToTry[] = {
	//	w	h	bpp	fullscreen
	{	640,	480,	32,	true	},
	{	640,	480,	16,	true	},
	{	640,	480,	15,	true	},
	{	640,	480,	24,	true	},
	{	1024,	768,	32,	true	},
	{	1024,	768,	16,	true	},
	{	1024,	768,	15,	true	},
	{	1024,	768,	24,	true	},
	{	320,	240,	32,	true	},
	{	320,	240,	16,	true	},
	{	320,	240,	15,	true	},
	{	320,	240,	24,	true	},
	{	640,	480,	32,	false	},
	{	640,	480,	16,	false	},
	{	640,	480,	15,	false	},
	{	640,	480,	24,	false	},
	{	1024,	768,	32,	false	},
	{	1024,	768,	16,	false	},
	{	1024,	768,	15,	false	},
	{	1024,	768,	24,	false	},
	{	320,	240,	32,	false	},
	{	320,	240,	16,	false	},
	{	320,	240,	15,	false	},
	{	320,	240,	24,	false	},
	// This marks the last mode
	{	0,				}
};

static inline void memory_putpixel15_or_16(BITMAP* bmp, int x, int y, int color)
{
	((short *)bmp->line[y])[x] = color;
}

static inline void memory_putpixel24(BITMAP* bmp, int x, int y, int color)
{
	putpixel(bmp, x, y, color);
}

static inline void memory_putpixel32(BITMAP* bmp, int x, int y, int color)
{
	((int32_t *)bmp->line[y])[x] = (int32_t)color;
}

static inline int memory_getpixel15_or_16(BITMAP *bmp, int x, int y)
{
	return ((short *)bmp->line[y])[x];
}

static inline int memory_getpixel24(BITMAP* bmp, int x, int y)
{
	return getpixel(bmp, x, y);
}

static inline int memory_getpixel32(BITMAP* bmp, int x, int y)
{
	return ((int *)bmp->line[y])[x];
}

static inline void video_putpixel15_or_16(uintptr_t addr, int x, int color)
{
	bmp_write16(addr+x*2, color);
}

static inline void video_putpixel24(uintptr_t addr, int x, int color)
{
	bmp_write24(addr+x*3, color);
}

static inline void video_putpixel32(uintptr_t addr, int x, int color)
{
	bmp_write32(addr+x*4, color);
}

static inline int video_getpixel15_or_16(uintptr_t addr, int x)
{
	return bmp_read16(addr+x*2);
}

static inline int video_getpixel24(uintptr_t addr, int x)
{
	return bmp_read24(addr+x*3);
}

static inline int video_getpixel32(uintptr_t addr, int x)
{
	return bmp_read32(addr+x*4);
}

static struct {
		int xcstart; /* x counter start */
		int sxinc; /* amount to increment src x every time */
		int xcdec; /* amount to deccrement counter by, increase sptr when this reaches 0 */
		int xcinc; /* amount to increment counter by when it reaches 0 */
		int linesize; /* size of a whole row of pixels */
} _al_stretch;

#define DECLARE_STRETCHER(type, size, put, get) \
		int xc = _al_stretch.xcstart; \
		uintptr_t dend = dptr + _al_stretch.linesize; \
		ASSERT(dptr); \
		ASSERT(sptr); \
		for (; dptr < dend; dptr += size, sptr += _al_stretch.sxinc) { \
			put(dptr, get((type*)sptr)); \
			if (xc <= 0) { \
      			sptr += size; \
				xc += _al_stretch.xcinc; \
			} \
			else \
				xc -= _al_stretch.xcdec; \
		}

#define DECLARE_MASKED_STRETCHER(type, size, put, get, mask) \
		int xc = _al_stretch.xcstart; \
		uintptr_t dend = dptr + _al_stretch.linesize; \
		ASSERT(dptr); \
		ASSERT(sptr); \
		for (; dptr < dend; dptr += size, sptr += _al_stretch.sxinc) { \
			int color = get((type*)sptr); \
			if (color != mask) \
				put(dptr, get((type*)sptr)); \
			if (xc <= 0) { \
      			sptr += size; \
				xc += _al_stretch.xcinc; \
			} \
			else \
				xc -= _al_stretch.xcdec; \
		}

#ifdef GFX_HAS_VGA
/*
 * Mode-X line stretcher.
 */
static void stretch_linex(uintptr_t dptr, unsigned char *sptr)
{
   int plane;
   int first_xc = _al_stretch.xcstart;
   int dw = _al_stretch.linesize;

   ASSERT(dptr);
   ASSERT(sptr);

   for (plane = 0; plane < 4; plane++) {
      int xc = first_xc;
      unsigned char *s = sptr;
      uintptr_t d = dptr / 4;
      uintptr_t dend = (dptr + dw) / 4;

      outportw(0x3C4, (0x100 << (dptr & 3)) | 2);
      for (; d < dend; d++, s += 4 * _al_stretch.sxinc) {
	 bmp_write8(d, *s);
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
      }

      /* Move to the beginning of next plane.  */
	  if (first_xc <= 0) {
         sptr++;
		 first_xc += _al_stretch.xcinc;
	  }
	  else
         first_xc -= _al_stretch.xcdec;
      dptr++;
      sptr += _al_stretch.sxinc;
      dw--;
   }
}

/*
 * Mode-X masked line stretcher.
 */
static void stretch_masked_linex(uintptr_t dptr, unsigned char *sptr)
{
   int plane;
   int dw = _al_stretch.linesize;
   int first_xc = _al_stretch.xcstart;

   ASSERT(dptr);
   ASSERT(sptr);

   for (plane = 0; plane < 4; plane++) {
      int xc = first_xc;
      unsigned char *s = sptr;
      uintptr_t d = dptr / 4;
      uintptr_t dend = (dptr + dw) / 4;

      outportw(0x3C4, (0x100 << (dptr & 3)) | 2);
      for (; d < dend; d++, s += 4 * _al_stretch.sxinc) {
	 unsigned long color = *s;
	 if (color != 0)
	    bmp_write8(d, color);
      }
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;
	 if (xc <= 0) s++, xc += _al_stretch.xcinc;
     else xc -= _al_stretch.xcdec;

      /* Move to the beginning of next plane.  */
	  if (first_xc <= 0) {
         sptr++;
		 first_xc += _al_stretch.xcinc;
	  }
	  else
         first_xc -= _al_stretch.xcdec;
      dptr++;
      sptr += _al_stretch.sxinc;
      dw--;
   }
}
#endif

#ifdef ALLEGRO_COLOR8
static void stretch_line8(uintptr_t dptr, unsigned char *sptr)
{
   DECLARE_STRETCHER(unsigned char, 1, bmp_write8, *);
}

static void stretch_masked_line8(uintptr_t dptr, unsigned char *sptr)
{
   DECLARE_MASKED_STRETCHER(unsigned char, 1, bmp_write8, *, 0);
}
#endif

#ifdef ALLEGRO_COLOR16
static void stretch_line15(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_STRETCHER(unsigned short, 2, bmp_write16, *);
}

static void stretch_line16(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_STRETCHER(unsigned short, 2, bmp_write16, *);
}

static void stretch_masked_line15(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_MASKED_STRETCHER(unsigned short, 2, bmp_write16, *, MASK_COLOR_15);
}

static void stretch_masked_line16(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_MASKED_STRETCHER(unsigned short, 2, bmp_write16, *, MASK_COLOR_16);
}
#endif

#ifdef ALLEGRO_COLOR24
static void stretch_line24(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_STRETCHER(unsigned char, 3, bmp_write24, READ3BYTES);
}

static void stretch_masked_line24(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_MASKED_STRETCHER(unsigned char, 3, bmp_write24, READ3BYTES, MASK_COLOR_24);
}
#endif

#ifdef ALLEGRO_COLOR32
static void stretch_line32(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_STRETCHER(uint32_t, 4, bmp_write32, *);
}

static void stretch_masked_line32(uintptr_t dptr, unsigned char* sptr)
{
	DECLARE_MASKED_STRETCHER(uint32_t, 4, bmp_write32, *, MASK_COLOR_32);
}
#endif

void _al_stretch_blit(BITMAP *src, BITMAP *dst,
    int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh,
	int masked)
{
	int y; /* current dst y */
	int yc; /* y counter */
	int sxofs, dxofs; /* start offsets */
	int syinc; /* amount to increment src y each time */
	int ycdec; /* amount to deccrement counter by, increase sy when this reaches 0 */
	int ycinc; /* amount to increment counter by when it reaches 0 */
	int size; /* pixel size */
	int dxbeg, dxend; /* clipping information */
	int dybeg, dyend;
	int i;

	void (*stretch_line)(uintptr_t, unsigned char*);

	ASSERT(src);
	ASSERT(dst);
	ASSERT(bitmap_color_depth(src) == bitmap_color_depth(dst));
	ASSERT(src != dst);

	if (sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0) return;

	if (masked) {
		switch (bitmap_color_depth(src)) {
			case 8:
				if (is_linear_bitmap(dst))
					stretch_line = stretch_masked_line8;
#ifdef GFX_HAS_VGA
				else
					stretch_line = stretch_masked_linex;
#endif
				size = 1;
				break;
			case 15:
				stretch_line = stretch_masked_line15;
				size = 2;
				break;
			case 16:
				stretch_line = stretch_masked_line16;
				size = 2;
				break;
			case 24:
				stretch_line = stretch_masked_line24;
				size = 3;
				break;
			default:
				stretch_line = stretch_masked_line32;
				size = 4;
				break;
		}
	}
	else {
		switch (bitmap_color_depth(src)) {
			case 8:
				if (is_linear_bitmap(dst))
					stretch_line = stretch_line8;
#ifdef GFX_HAS_VGA
				else
					stretch_line = stretch_linex;
#endif
				size = 1;
				break;
			case 15:
				stretch_line = stretch_line15;
				size = 2;
				break;
			case 16:
				stretch_line = stretch_line16;
				size = 2;
				break;
			case 24:
				stretch_line = stretch_line24;
				size = 3;
				break;
			default:
				stretch_line = stretch_line32;
				size = 4;
				break;
		}
	}

	if (dst->clip) {
		dybeg = ((dy > dst->ct) ? dy : dst->ct);
		dyend = (((dy + dh) < dst->cb) ? (dy + dh) : dst->cb);
		if (dybeg >= dyend)
			return;

		dxbeg = ((dx > dst->cl) ? dx : dst->cl);
		dxend = (((dx + dw) < dst->cr) ? (dx + dw) : dst->cr);
		if (dxbeg >= dxend)
			return;
	}
	else {
		dxbeg = dx;
		dxend = dx + dw;
		dybeg = dy;
		dyend = dy + dh;
	}

	syinc = sh / dh;
	ycdec = sh - (syinc*dh);
	ycinc = dh - ycdec;
	yc = ycinc;
	sxofs = sx * size;
	dxofs = dx * size;

	_al_stretch.sxinc = sw / dw * size;
	_al_stretch.xcdec = sw - ((sw/dw)*dw);
	_al_stretch.xcinc = dw - _al_stretch.xcdec;
	_al_stretch.linesize = (dxend-dxbeg)*size;
	
	/* get start state (clip) */
	_al_stretch.xcstart = _al_stretch.xcinc;
	for (i = 0; i < dxbeg-dx; i++) {
		if (_al_stretch.xcstart <= 0) {
			_al_stretch.xcstart += _al_stretch.xcinc;
			sxofs+=size;
		}
		else
			_al_stretch.xcstart -= _al_stretch.xcdec;
	}

	dxofs += i * size;

	bmp_select(dst);

	/* skip clipped lines */
	for (y = dy; y < dybeg; y++, sy += syinc) {
		if (yc <= 0) {
			sy++;
			yc += ycinc;
		}
		else
				yc -= ycdec;
	}

	for (; y < dyend; y++, sy += syinc) {
		(*stretch_line)(bmp_write_line(dst, y) + dxofs, src->line[sy] + sxofs);
		if (yc <= 0) {
			sy++;
			yc += ycinc;
		}
		else
				yc -= ycdec;
	}
	
	bmp_unwrite_line(dst);
}

void my_stretch(BITMAP *src, BITMAP *dst,
    int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
	_al_stretch_blit(src, dst, sx, sy, sw, sh, dx, dy, dw, dh, 0);
}

void my_stretch_masked(BITMAP *src, BITMAP *dst,
    int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
		_al_stretch_blit(src, dst, sx, sy, sw, sh, dx, dy, dw, dh, 1);
}

static inline void get9_15_or_16(BITMAP* bmp, int x, int y,
	int* a, int* b, int* c, int* d, int * e, int* f, int* g,
	int* h, int* i)
{
	*a = memory_getpixel15_or_16(bmp, x-1, y-1);
	*b = memory_getpixel15_or_16(bmp, x, y-1);
	*c = memory_getpixel15_or_16(bmp, x+1, y-1);
	*d = memory_getpixel15_or_16(bmp, x-1, y);
	*e = memory_getpixel15_or_16(bmp, x, y);
	*f = memory_getpixel15_or_16(bmp, x+1, y);
	*g = memory_getpixel15_or_16(bmp, x-1, y+1);
	*h = memory_getpixel15_or_16(bmp, x, y+1);
	*i = memory_getpixel15_or_16(bmp, x+1, y+1);
}

static inline void get9_24(BITMAP* bmp, int x, int y,
	int* a, int* b, int* c, int* d, int * e, int* f, int* g,
	int* h, int* i)
{
	*a = memory_getpixel24(bmp, x-1, y-1);
	*b = memory_getpixel24(bmp, x, y-1);
	*c = memory_getpixel24(bmp, x+1, y-1);
	*d = memory_getpixel24(bmp, x-1, y);
	*e = memory_getpixel24(bmp, x, y);
	*f = memory_getpixel24(bmp, x+1, y);
	*g = memory_getpixel24(bmp, x-1, y+1);
	*h = memory_getpixel24(bmp, x, y+1);
	*i = memory_getpixel24(bmp, x+1, y+1);
}

static inline void get9_32(BITMAP* bmp, int x, int y,
	int* a, int* b, int* c, int* d, int * e, int* f, int* g,
	int* h, int* i)
{
	*a = memory_getpixel32(bmp, x-1, y-1);
	*b = memory_getpixel32(bmp, x, y-1);
	*c = memory_getpixel32(bmp, x+1, y-1);
	*d = memory_getpixel32(bmp, x-1, y);
	*e = memory_getpixel32(bmp, x, y);
	*f = memory_getpixel32(bmp, x+1, y);
	*g = memory_getpixel32(bmp, x-1, y+1);
	*h = memory_getpixel32(bmp, x, y+1);
	*i = memory_getpixel32(bmp, x+1, y+1);
}

static void scale2x_15_or_16(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int* buf1=0;
	int* buf2=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int[w*2];
		buf2 = new int[w*2];
	}
	
	for (int y = sy; y <= ey; y++, yy += 2) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 2) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int p = memory_getpixel15_or_16(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf2[i] = p;
					buf2[i+1] = p;
				}
				else {
					memory_putpixel15_or_16(dst, xx, yy, p);
					memory_putpixel15_or_16(dst, xx+1, yy, p);
					memory_putpixel15_or_16(dst, xx, yy+1, p);
					memory_putpixel15_or_16(dst, xx+1, yy+1, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				get9_15_or_16(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					if (G == E) {
						E0 = E;
					}
					else {
						E0 = D == B ? D : E;
					}
					E1 = B == F ? F : E;
					E2 = D == H ? D : E;
					if (A == E)
						E3 = E;
					else
						E3 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf2[i] = E2;
					buf2[i+1] = E3;
				}
				else {
					memory_putpixel15_or_16(dst, xx, yy, E0);
					memory_putpixel15_or_16(dst, xx+1, yy, E1);
					memory_putpixel15_or_16(dst, xx, yy+1, E2);
					memory_putpixel15_or_16(dst, xx+1, yy+1, E3);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*2; i++)
				bmp_write16(addr+((dx+xo+i)<<1), buf1[i]);
			bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+1+yo);
			for (i = 0; i < w*2; i++)
				bmp_write16(addr+((dx+xo+i)<<1), buf2[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
	}
}

static void scale2x_24(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int* buf1=0;
	int* buf2=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int[w*2];
		buf2 = new int[w*2];
	}
	
	for (int y = sy; y <= ey; y++, yy += 2) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 2) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int p = memory_getpixel24(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf2[i] = p;
					buf2[i+1] = p;
				}
				else {
					memory_putpixel24(dst, xx, yy, p);
					memory_putpixel24(dst, xx+1, yy, p);
					memory_putpixel24(dst, xx, yy+1, p);
					memory_putpixel24(dst, xx+1, yy+1, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				get9_24(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					if (G == E) {
						E0 = E;
					}
					else {
						E0 = D == B ? D : E;
					}
					E1 = B == F ? F : E;
					E2 = D == H ? D : E;
					if (A == E)
						E3 = E;
					else
						E3 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf2[i] = E2;
					buf2[i+1] = E3;
				}
				else {
					memory_putpixel24(dst, xx, yy, E0);
					memory_putpixel24(dst, xx+1, yy, E1);
					memory_putpixel24(dst, xx, yy+1, E2);
					memory_putpixel24(dst, xx+1, yy+1, E3);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*2; i++)
				bmp_write24(addr+((dx+i+xo)*3), buf1[i]);
			bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+1+yo);
			for (i = 0; i < w*2; i++)
				bmp_write24(addr+((dx+i+xo)*3), buf2[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
	}
}

static void scale2x_32(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int32_t* buf1=0;
	int32_t* buf2=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int32_t[w*2];
		buf2 = new int32_t[w*2];
	}
	
	for (int y = sy; y <= ey; y++, yy += 2) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 2) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int32_t p = memory_getpixel32(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf2[i] = p;
					buf2[i+1] = p;
				}
				else {
					memory_putpixel32(dst, xx, yy, p);
					memory_putpixel32(dst, xx+1, yy, p);
					memory_putpixel32(dst, xx, yy+1, p);
					memory_putpixel32(dst, xx+1, yy+1, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				get9_32(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					if (G == E) {
						E0 = E;
					}
					else {
						E0 = D == B ? D : E;
					}
					E1 = B == F ? F : E;
					E2 = D == H ? D : E;
					if (A == E)
						E3 = E;
					else
						E3 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf2[i] = E2;
					buf2[i+1] = E3;
				}
				else {
					memory_putpixel32(dst, xx, yy, E0);
					memory_putpixel32(dst, xx+1, yy, E1);
					memory_putpixel32(dst, xx, yy+1, E2);
					memory_putpixel32(dst, xx+1, yy+1, E3);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*2; i++)
				bmp_write32(addr+((dx+i+xo)<<2), buf1[i]);
			//bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+1+yo);
			for (i = 0; i < w*2; i++)
				bmp_write32(addr+((dx+i+xo)<<2), buf2[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
	}
}

static void scale2x(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int depth = get_color_depth();

	switch (depth) {
		case 15:
		case 16:
			scale2x_15_or_16(src, dst, sx, sy, dx, dy, w, h);
			break;
		case 24:
			scale2x_24(src, dst, sx, sy, dx, dy, w, h);
			break;
		case 32:
			scale2x_32(src, dst, sx, sy, dx, dy, w, h);
			break;
	}
}

static void scale3x_15_or_16(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int* buf1=0;
	int* buf2=0;
	int* buf3=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int[w*3];
		buf2 = new int[w*3];
		buf3 = new int[w*3];
	}
	
	for (int y = sy; y <= ey; y++, yy += 3) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 3) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int p = memory_getpixel15_or_16(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf1[i+2] = p;
					buf2[i] = p;
					buf2[i+1] = p;
					buf2[i+2] = p;
					buf3[i] = p;
					buf3[i+1] = p;
					buf3[i+2] = p;
				}
				else {
					memory_putpixel15_or_16(dst, xx, yy, p);
					memory_putpixel15_or_16(dst, xx+1, yy, p);
					memory_putpixel15_or_16(dst, xx+2, yy, p);
					memory_putpixel15_or_16(dst, xx, yy+1, p);
					memory_putpixel15_or_16(dst, xx+1, yy+1, p);
					memory_putpixel15_or_16(dst, xx+2, yy+1, p);
					memory_putpixel15_or_16(dst, xx, yy+2, p);
					memory_putpixel15_or_16(dst, xx+1, yy+2, p);
					memory_putpixel15_or_16(dst, xx+2, yy+2, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				int E4;
				int E5;
				int E6;
				int E7;
				int E8;
				int E9;
				get9_15_or_16(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					E0 = D == B ? D : E;
					E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
					E2 = B == F ? F : E;
					E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
					E4 = E;
					E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
					E6 = D == H ? D : E;
					E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
					E8 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
					E4 = E;
					E5 = E;
					E6 = E;
					E7 = E;
					E8 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf1[i+2] = E2;
					buf2[i] = E3;
					buf2[i+1] = E4;
					buf2[i+2] = E4;
					buf3[i] = E6;
					buf3[i+1] = E7;
					buf3[i+2] = E8;
				}
				else {
					memory_putpixel15_or_16(dst, xx, yy, E0);
					memory_putpixel15_or_16(dst, xx+1, yy, E1);
					memory_putpixel15_or_16(dst, xx+2, yy, E2);
					memory_putpixel15_or_16(dst, xx, yy+1, E3);
					memory_putpixel15_or_16(dst, xx+1, yy+1, E4);
					memory_putpixel15_or_16(dst, xx+2, yy+1, E5);
					memory_putpixel15_or_16(dst, xx, yy+2, E6);
					memory_putpixel15_or_16(dst, xx+1, yy+2, E7);
					memory_putpixel15_or_16(dst, xx+2, yy+2, E8);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*3; i++)
				bmp_write16(addr+((dx+xo+i)<<1), buf1[i]);
			bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo+1);
			for (i = 0; i < w*3; i++)
				bmp_write16(addr+((dx+xo+i)<<1), buf2[i]);
			addr = bmp_write_line(dst, yy+yo+2);
			for (i = 0; i < w*3; i++)
				bmp_write16(addr+((dx+xo+i)<<1), buf3[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
		delete[] buf3;
	}
}

static void scale3x_24(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int* buf1=0;
	int* buf2=0;
	int* buf3=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int[w*3];
		buf2 = new int[w*3];
		buf3 = new int[w*3];
	}
	
	for (int y = sy; y <= ey; y++, yy += 3) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 3) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int p = memory_getpixel24(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf1[i+2] = p;
					buf2[i] = p;
					buf2[i+1] = p;
					buf2[i+2] = p;
					buf3[i] = p;
					buf3[i+1] = p;
					buf3[i+2] = p;
				}
				else {
					memory_putpixel24(dst, xx, yy, p);
					memory_putpixel24(dst, xx+1, yy, p);
					memory_putpixel24(dst, xx+2, yy, p);
					memory_putpixel24(dst, xx, yy+1, p);
					memory_putpixel24(dst, xx+1, yy+1, p);
					memory_putpixel24(dst, xx+2, yy+1, p);
					memory_putpixel24(dst, xx, yy+2, p);
					memory_putpixel24(dst, xx+1, yy+2, p);
					memory_putpixel24(dst, xx+2, yy+2, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				int E4;
				int E5;
				int E6;
				int E7;
				int E8;
				int E9;
				get9_24(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					E0 = D == B ? D : E;
					E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
					E2 = B == F ? F : E;
					E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
					E4 = E;
					E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
					E6 = D == H ? D : E;
					E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
					E8 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
					E4 = E;
					E5 = E;
					E6 = E;
					E7 = E;
					E8 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf1[i+2] = E2;
					buf2[i] = E3;
					buf2[i+1] = E4;
					buf2[i+2] = E4;
					buf3[i] = E6;
					buf3[i+1] = E7;
					buf3[i+2] = E8;
				}
				else {
					memory_putpixel24(dst, xx, yy, E0);
					memory_putpixel24(dst, xx+1, yy, E1);
					memory_putpixel24(dst, xx+2, yy, E2);
					memory_putpixel24(dst, xx, yy+1, E3);
					memory_putpixel24(dst, xx+1, yy+1, E4);
					memory_putpixel24(dst, xx+2, yy+1, E5);
					memory_putpixel24(dst, xx, yy+2, E6);
					memory_putpixel24(dst, xx+1, yy+2, E7);
					memory_putpixel24(dst, xx+2, yy+2, E8);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*3; i++)
				bmp_write24(addr+((dx+xo+i)*3), buf1[i]);
			bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo+1);
			for (i = 0; i < w*3; i++)
				bmp_write24(addr+((dx+xo+i)*3), buf2[i]);
			addr = bmp_write_line(dst, yy+yo+2);
			for (i = 0; i < w*3; i++)
				bmp_write24(addr+((dx+xo+i)*3), buf3[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
		delete[] buf3;
	}
}

static void scale3x_32(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int ex = MIN(src->w-1, sx + w - 1);
	int ey = MIN(src->h-1, sy + h - 1);

	sx = MAX(0, sx);
	sy = MAX(0, sy);
	dx = MAX(0, dx);
	dy = MAX(0, dy);

	int yy = dy;

	int32_t* buf1=0;
	int32_t* buf2=0;
	int32_t* buf3=0;

	if (is_video_bitmap(dst)) {
		buf1 = new int32_t[w*3];
		buf2 = new int32_t[w*3];
		buf3 = new int32_t[w*3];
	}
	
	for (int y = sy; y <= ey; y++, yy += 3) {
		int xx = dx;
		for (int x = sx; x <= ex; x++, xx += 3) {
			if (x == 0 || y == 0 || x == (src->w-1) ||
					y == (src->h-1)) {
				int32_t p = memory_getpixel32(src, x, y);
				if (is_video_bitmap(dst)) {
					int i = xx - dx;
					buf1[i] = p;
					buf1[i+1] = p;
					buf1[i+2] = p;
					buf2[i] = p;
					buf2[i+1] = p;
					buf2[i+2] = p;
					buf3[i] = p;
					buf3[i+1] = p;
					buf3[i+2] = p;
				}
				else {
					memory_putpixel32(dst, xx, yy, p);
					memory_putpixel32(dst, xx+1, yy, p);
					memory_putpixel32(dst, xx+2, yy, p);
					memory_putpixel32(dst, xx, yy+1, p);
					memory_putpixel32(dst, xx+1, yy+1, p);
					memory_putpixel32(dst, xx+2, yy+1, p);
					memory_putpixel32(dst, xx, yy+2, p);
					memory_putpixel32(dst, xx+1, yy+2, p);
					memory_putpixel32(dst, xx+2, yy+2, p);
				}
			}
			else {
				int A;
				int B;
				int C;
				int D;
				int E;
				int F;
				int G;
				int H;
				int I;
				int E0;
				int E1;
				int E2;
				int E3;
				int E4;
				int E5;
				int E6;
				int E7;
				int E8;
				int E9;
				get9_32(src, x, y, &A, &B, &C, &D, &E,
						&F, &G, &H, &I);
				if (B != H && D != F) {
					E0 = D == B ? D : E;
					E1 = (D == B && E != C) || (B == F && E != A) ? B : E;
					E2 = B == F ? F : E;
					E3 = (D == B && E != G) || (D == H && E != A) ? D : E;
					E4 = E;
					E5 = (B == F && E != I) || (H == F && E != C) ? F : E;
					E6 = D == H ? D : E;
					E7 = (D == H && E != I) || (H == F && E != G) ? H : E;
					E8 = H == F ? F : E;
				} else {
					E0 = E;
					E1 = E;
					E2 = E;
					E3 = E;
					E4 = E;
					E5 = E;
					E6 = E;
					E7 = E;
					E8 = E;
				}
				if (is_video_bitmap(dst)) {
					int i = xx-dx;
					buf1[i] = E0;
					buf1[i+1] = E1;
					buf1[i+2] = E2;
					buf2[i] = E3;
					buf2[i+1] = E4;
					buf2[i+2] = E4;
					buf3[i] = E6;
					buf3[i+1] = E7;
					buf3[i+2] = E8;
				}
				else {
					memory_putpixel32(dst, xx, yy, E0);
					memory_putpixel32(dst, xx+1, yy, E1);
					memory_putpixel32(dst, xx+2, yy, E2);
					memory_putpixel32(dst, xx, yy+1, E3);
					memory_putpixel32(dst, xx+1, yy+1, E4);
					memory_putpixel32(dst, xx+2, yy+1, E5);
					memory_putpixel32(dst, xx, yy+2, E6);
					memory_putpixel32(dst, xx+1, yy+2, E7);
					memory_putpixel32(dst, xx+2, yy+2, E8);
				}
			}
		}
		if (is_video_bitmap(dst)) {
			int i;
			uintptr_t addr;
			int xo = scr->getXOffset();
			int yo = scr->getYOffset();
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo);
			for (i = 0; i < w*3; i++)
				bmp_write32(addr+((dx+xo+i)<<2), buf1[i]);
			bmp_unwrite_line(dst);
			bmp_select(dst);
			addr = bmp_write_line(dst, yy+yo+1);
			for (i = 0; i < w*3; i++)
				bmp_write32(addr+((dx+xo+i)<<2), buf2[i]);
			addr = bmp_write_line(dst, yy+yo+2);
			for (i = 0; i < w*3; i++)
				bmp_write32(addr+((dx+xo+i)<<2), buf3[i]);
			bmp_unwrite_line(dst);
		}
	}

	if (is_video_bitmap(dst)) {
		delete[] buf1;
		delete[] buf2;
		delete[] buf3;
	}
}

static void scale3x(BITMAP* src, BITMAP* dst, int sx, int sy, int dx, int dy,
		int w, int h)
{
	int depth = get_color_depth();

	switch (depth) {
		case 15:
		case 16:
			scale3x_15_or_16(src, dst, sx, sy, dx, dy, w, h);
			break;
		case 24:
			scale3x_24(src, dst, sx, sy, dx, dy, w, h);
			break;
		case 32:
			scale3x_32(src, dst, sx, sy, dx, dy, w, h);
			break;
	}
}

/*
 * Returns true if the modes match, false otherwise.
 * Used to check if the current mode has been checked already.
 */
static bool screenDescriptorsMatch(ScreenDescriptor* sd1, ScreenDescriptor* sd2)
{
    // FIXME:
    return true;
}

/*
 * Copy one screen descriptors information to another
 */
static void copyScreenDescriptor(ScreenDescriptor* dest, ScreenDescriptor* src)
{
	memcpy(dest, src, sizeof(ScreenDescriptor));
}

void Screen::showVolume(int volume)
{
	_showVolume = true;
	showVolumeCount = SHOW_VOLUME_TIME;
	lastVolumeUpdate = tguiCurrentTimeMillis();
	this->volume = volume;
}

void Screen::drawVolume()
{
	long now = tguiCurrentTimeMillis();
	showVolumeCount -= (now - lastVolumeUpdate);
	lastVolumeUpdate = now;
	if (showVolumeCount <= 0) {
		showVolumeCount = 0;
		_showVolume = false;
		draw();
		return;
	}

	float percent = (float)volume / (float)255;

	drawMeter(volumeBuffer, 0, 0, VOLUME_W*scale, VOLUME_H*scale, percent);
	blit(volumeBuffer, screen, 0, 0, 10+x_offset, 10+y_offset,
		VOLUME_W*scale, VOLUME_H*scale);
}

/*
 * Returns true if the mode was set, false otherwise.
 */
bool Screen::setMode(ScreenDescriptor* mode)
{
	set_color_depth(mode->depth);

	int driver = mode->fullscreen ? GFX_AUTODETECT_FULLSCREEN :
		GFX_AUTODETECT_WINDOWED;

	if (set_gfx_mode(driver, mode->width, mode->height, 0, 0) != 0) {
		debug_message("Error setting graphics mode: %s\n", allegro_error);
		return false;
	}

	copyScreenDescriptor(&(this->mode), mode);

	return true;
}

bool Screen::createBuffer()
{
	backBuffer = create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
	if (backBuffer == 0)
		return false;
	fxBuffer = create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
	if (fxBuffer == 0) {
		destroy_bitmap(backBuffer);
		return false;
	}
	fxBuffer2 = create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
	if (fxBuffer2 == 0) {
		destroy_bitmap(backBuffer);
		destroy_bitmap(fxBuffer);
		return false;
	}

	clear(backBuffer);
	clear(fxBuffer);
	clear(fxBuffer2);

	/*
	 * Set up the scaling factor. Since the area is drawn to a
	 * small buffer, we need to scale it to fit into a higher resolution
	 * graphics mode. The aspect ratio is maintained by offseting
	 * the screen if it doesn't fit exactly into the mode.
	 */
	scale = MIN(mode.width / BUFFER_WIDTH, mode.height / BUFFER_HEIGHT);
	x_offset = (mode.width - (BUFFER_WIDTH * scale)) / 2;
	y_offset = (mode.height - (BUFFER_HEIGHT * scale)) / 2;

	scaleBuffer = create_bitmap(SCREEN_W, SCREEN_H);
	if (!scaleBuffer) {
		destroy_bitmap(backBuffer);
		destroy_bitmap(fxBuffer);
		destroy_bitmap(fxBuffer2);
		return false;
	}
	clear(scaleBuffer);

	volumeBuffer = create_bitmap(VOLUME_W*MAX_SCALE, VOLUME_H*MAX_SCALE);
	if (!volumeBuffer) {
		destroy_bitmap(backBuffer);
		destroy_bitmap(fxBuffer);
		destroy_bitmap(fxBuffer2);
		destroy_bitmap(scaleBuffer);
		return false;
	}

	return true;
}

ScreenDescriptor* Screen::getDescriptor()
{
	return &mode;
}

/*
 * Draw a rectangle from the back buffer to the screen.
 */
void Screen::drawRect(int x, int y, int w, int h)
{
	if (config.getWaitForVsync())
		vsync();

	//stretch_blit(backBuffer, screen, x, y, w, h,
	my_acquire_screen();
	my_stretch(backBuffer, screen, x, y, w, h,
		x*scale+x_offset, y*scale+y_offset,
		w*scale, h*scale);
	my_release_screen();
}

/*
 * Redraw only the areas of the screen that have changed.
 */
void Screen::drawDirty(std::vector<Object*>* objects)
{
	if (config.getWaitForVsync())
		vsync();

	my_acquire_screen();
	
	if (currArea && redrawAnimatedTiles) {
		redrawAnimatedTiles = false;
		for (int y = 0; y < Area::HEIGHT; y++) {
			for (int x = 0; x < Area::WIDTH; x++) {
				if (currArea->tileIsAnimated(x, y)) {
					int rx = x * TILE_SIZE;
					int ry = y * TILE_SIZE;
					if (config.useSmoothScaling() && scale == 2) {
						scale2x(backBuffer, screen,
							rx, ry, (rx<<1), (ry<<1),
							TILE_SIZE, TILE_SIZE);
					}
					else if (config.useSmoothScaling() && scale == 3) {
						scale3x(backBuffer, screen,
							rx, ry, rx*3, ry*3,
							TILE_SIZE, TILE_SIZE);
					}
					else {
						//stretch_blit(backBuffer, screen, rx, ry,
						my_stretch(backBuffer, screen, rx, ry,
							TILE_SIZE, TILE_SIZE,
							rx*scale+x_offset, ry*scale+y_offset,
							TILE_SIZE*scale,
							TILE_SIZE*scale);
					}
				}
			}
		}
	}

	/*
	 * Draw objects and one tile width surrounding them.
	 */
	for (unsigned int i = 0; i < objects->size(); i++) {
		Object* o = (*objects)[i];
		int x = o->getX();
		int y = o->getY();
		int w = o->getWidth();
		int h = o->getHeight();
		int sx = MAX(0, x - TILE_SIZE);
		int ex = MIN(x + w + TILE_SIZE, BUFFER_WIDTH-1);
		int dw = ex - sx;
		int sy = MAX(0, y - (TILE_SIZE - h) - TILE_SIZE);
		int dh = (y + h + TILE_SIZE) - sy;
		dh = MIN(dh, (BUFFER_HEIGHT-1) - sy);
		if (config.useSmoothScaling() && scale == 2) {
			scale2x(backBuffer, screen, sx, sy, (sx<<1), (sy<<1),
					dw, dh);
		}
		else if (config.useSmoothScaling() && scale == 3) {
			scale3x(backBuffer, screen, sx, sy, sx*3, sy*3,
					dw, dh);
		}
		else {
			//stretch_blit(backBuffer, screen, sx, sy, dw, dh,
			my_stretch(backBuffer, screen, sx, sy, dw, dh,
				sx*scale+x_offset, sy*scale+y_offset, dw*scale, dh*scale);
		}
	}

	if (_showVolume) {
		drawVolume();
	}

	my_release_screen();

	framesDrawn++;
}

/*
 * Draw the back buffer to the screen, stretching it if necessary.
 */
void Screen::draw()
{
	if (config.getWaitForVsync())
		vsync();
	
	my_acquire_screen();

	if (scale == 2 && config.useSmoothScaling()) {
		scale2x(backBuffer, scaleBuffer, 0, 0, 0, 0, backBuffer->w,
				backBuffer->h);
		blit(scaleBuffer, screen, 0, 0, x_offset, y_offset,
				BUFFER_WIDTH*scale, BUFFER_HEIGHT*scale);
	}
	else if (scale == 3 && config.useSmoothScaling()) {
		scale3x(backBuffer, scaleBuffer, 0, 0, 0, 0, backBuffer->w,
				backBuffer->h);
		blit(scaleBuffer, screen, 0, 0, x_offset, y_offset,
				BUFFER_WIDTH*scale, BUFFER_HEIGHT*scale);
	}
	else if (scale == 1) {
		blit(backBuffer, screen, 0, 0, x_offset, y_offset,
				backBuffer->w, backBuffer->h);
	}
	else {
	//	stretch_blit(backBuffer, screen, 0, 0, backBuffer->w, backBuffer->h,
		my_stretch(backBuffer, screen, 0, 0, backBuffer->w, backBuffer->h,
			x_offset, y_offset, backBuffer->w*scale,
			backBuffer->h*scale);
	}
	
	if (_showVolume) {
		drawVolume();
	}

	my_release_screen();

	framesDrawn++;
}

void Screen::setOffset(int x, int y)
{
	x_offset = x;
	y_offset = y;
}

BITMAP* Screen::getBackBuffer()
{
	return backBuffer;
}

BITMAP* Screen::getFXBuffer()
{
	return fxBuffer;
}

BITMAP* Screen::getFXBuffer2()
{
	return fxBuffer2;
}

void Screen::fadeOut(void)
{
	int count = 0;
	int level = 0;

	blit(backBuffer, fxBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);

	long start = currentTimeMillis();

	while (count < FADE_TIME) {
		updateMusic();
		level = (int)(((float)count / (float)FADE_TIME) * 255.0);
		blit(fxBuffer, backBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		set_trans_blender(0, 0, 0, level);
		rectfill(backBuffer, 0, 0, BUFFER_WIDTH-1, BUFFER_HEIGHT-1, 0);
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		draw();
		long now = currentTimeMillis();
		long duration = now - start;
		start = now;
		count += duration;
	}
	
	blit(fxBuffer, backBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
}

/*
 * Original image should be in fxBuffer before calling this
 */
void Screen::fadeIn(void)
{
	int count = 0;
	int level = 0;

	blit(backBuffer, fxBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);

	long start = currentTimeMillis();

	while (count < FADE_TIME) {
		updateMusic();
		level = 255 - (int)(((float)count / (float)FADE_TIME) * 255.0);
		blit(fxBuffer, backBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
		drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
		set_trans_blender(0, 0, 0, level);
		rectfill(backBuffer, 0, 0, BUFFER_WIDTH-1, BUFFER_HEIGHT-1, 0);
		drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
		draw();
		long now = currentTimeMillis();
		long duration = now - start;
		start = now;
		count += duration;
	}

	blit(fxBuffer, backBuffer, 0, 0, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT);
	draw();
}

void Screen::scroll(Direction direction)
{
	player->setHidden(true);

	switch (direction) {
		case DIRECTION_NORTH:
			scrollNorth();
			break;
		case DIRECTION_EAST:
			scrollEast();
			break;
		case DIRECTION_SOUTH:
			scrollSouth();
			break;
		case DIRECTION_WEST:
			scrollWest();
			break;
	}

	currArea->draw(backBuffer);
	draw();

	player->setHidden(false);
}

/*
 * Get the frames per second base on the elapsed time (in milliseconds)
 */
int Screen::getFPS(long elapsedTime)
{
	if (elapsedTime >= 1000)
		return framesDrawn / (elapsedTime / 1000);
	else
		return 0;
}

void Screen::resetFrameCounter()
{
	framesDrawn = 0;
}

int Screen::getXOffset()
{
	return x_offset;
}

int Screen::getYOffset()
{
	return y_offset;
}

int Screen::getScale()
{
	return scale;
}

Screen::Screen(ScreenDescriptor* wantedMode) throw (std::bad_alloc, NoMode) :
	scaleBuffer(0),
	framesDrawn(0),
	_showVolume(false),
	showVolumeCount(0)
{
	/*
	 * Try the requested mode first.
	 */
	if (setMode(wantedMode)) {
		if (createBuffer())
			return;
		throw new std::bad_alloc();
	}

	/*
	 * Couldn't set the requested mode, try others.
	 */
	for (int i = 0; modesToTry[i].width > 0; i++) {
		if (screenDescriptorsMatch(&modesToTry[i], wantedMode))
			continue;
		if (setMode(&modesToTry[i])) {
			if (createBuffer())
				return;
			break;
		}
	}

	throw new std::bad_alloc();
}

Screen::~Screen()
{
	destroy_bitmap(backBuffer);
	destroy_bitmap(fxBuffer);
	destroy_bitmap(fxBuffer2);
	destroy_bitmap(scaleBuffer);
	destroy_bitmap(volumeBuffer);
}

void Screen::scrollNorth()
{
	long count = 0;
	int py = 0;
	int px = player->getX();
	int pdy = BUFFER_HEIGHT - player->getHeight() - 1;

	while (count < SCROLL_TIME_Y) {
		updateMusic();
		long start = currentTimeMillis();
		float ratio = (float)count / float(SCROLL_TIME_Y);
		int offset = (int)((float)BUFFER_HEIGHT * ratio);
		int fx_y = BUFFER_HEIGHT - offset;
		int fx_h = BUFFER_HEIGHT - fx_y;
		int fx2_h = BUFFER_HEIGHT - fx_h;
		py = (int)((float)pdy * ratio);
		player->setPosition(px, py);
		blit(fxBuffer, backBuffer, 0, fx_y, 0, 0, BUFFER_WIDTH, fx_h);
		blit(fxBuffer2, backBuffer, 0, 0, 0, offset, BUFFER_WIDTH, fx2_h);
		player->draw(backBuffer);
		draw();
		long duration = currentTimeMillis() - start;
		player->updateAnimation(duration);
		count += duration;
	}
	player->setPosition(player->getX(), BUFFER_HEIGHT-player->getHeight()-1);
}

void Screen::scrollEast()
{
	long count = 0;
	int px = BUFFER_WIDTH - player->getWidth();
	int psx = px;
	int py = player->getY();

	while (count < SCROLL_TIME_X) {
		updateMusic();
		long start = currentTimeMillis();
		float ratio = (float)count / float(SCROLL_TIME_X);
		int offset = (int)((float)BUFFER_WIDTH * ratio);
		int fx_x = BUFFER_WIDTH - offset;
		int fx_w = BUFFER_WIDTH - fx_x;
		int fx2_x = offset;
		int fx2_w = BUFFER_WIDTH - fx_w;
		px = psx - (int)((float)psx * ratio);
		player->setPosition(px, py);
		blit(fxBuffer, backBuffer, 0, 0, fx_x, 0, fx_w, BUFFER_HEIGHT);
		blit(fxBuffer2, backBuffer, fx2_x, 0, 0, 0, fx2_w, BUFFER_HEIGHT);
		player->draw(backBuffer);
		draw();
		long duration = currentTimeMillis() - start;
		player->updateAnimation(duration);
		count += duration;
	}
	player->setPosition(0, player->getY());
}

void Screen::scrollSouth()
{
	long count = 0;
	int py = BUFFER_HEIGHT - player->getHeight();
	int psy = py;
	int px = player->getX();

	while (count < SCROLL_TIME_Y) {
		updateMusic();
		long start = currentTimeMillis();
		float ratio = (float)count / float(SCROLL_TIME_Y);
		int offset = (int)((float)BUFFER_HEIGHT * ratio);
		int fx_y = BUFFER_HEIGHT - offset;
		int fx_h = BUFFER_HEIGHT - fx_y;
		int fx2_y = offset;
		int fx2_h = BUFFER_HEIGHT - fx_h;
		py = psy - (int)((float)psy * ratio);
		player->setPosition(px, py);
		blit(fxBuffer, backBuffer, 0, 0, 0, fx_y, BUFFER_WIDTH, fx_h);
		blit(fxBuffer2, backBuffer, 0, fx2_y, 0, 0, BUFFER_WIDTH, fx2_h);
		player->draw(backBuffer);
		draw();
		long duration = currentTimeMillis() - start;
		player->updateAnimation(duration);
		count += duration;
	}
	player->setPosition(player->getX(), TILE_SIZE-player->getHeight());
}

void Screen::scrollWest()
{
	long count = 0;
	int px = 0;
	int py = player->getY();
	int pdx = BUFFER_WIDTH - player->getWidth() - 1;

	while (count < SCROLL_TIME_X) {
		updateMusic();
		long start = currentTimeMillis();
		float ratio = (float)count / float(SCROLL_TIME_X);
		int offset = (int)((float)BUFFER_WIDTH * ratio);
		int fx_x = BUFFER_WIDTH - offset;
		int fx_w = BUFFER_WIDTH - fx_x;
		int fx2_w = BUFFER_WIDTH - fx_w;
		px = (int)((float)pdx * ratio);
		player->setPosition(px, py);
		blit(fxBuffer, backBuffer, fx_x, 0, 0, 0, fx_w, BUFFER_HEIGHT);
		blit(fxBuffer2, backBuffer, 0, 0, offset, 0, fx2_w, BUFFER_HEIGHT);
		player->draw(backBuffer);
		draw();
		long duration = currentTimeMillis() - start;
		player->updateAnimation(duration);
		count += duration;
	}
	player->setPosition(BUFFER_WIDTH-player->getWidth()-1, player->getY());
}

/*
 * Call with ratio = 0.0 - 1.0 
 */ 
int getGradientColor(float ratio, int start_color, int end_color)
{
	int r = +(int)(getr(start_color) + ((getr(end_color) - getr(start_color)) * ratio));
	int g = +(int)(getg(start_color) + ((getg(end_color) - getg(start_color)) * ratio));
	int b = +(int)(getb(start_color) + ((getb(end_color) - getb(start_color)) * ratio));

	return makecol(r, g, b);
}

/*
 * Call with percent = 0.0 - 1.0
 */
void drawMeter(BITMAP* bmp, int x, int y, int w, int h, float percent)
{
	int borderColor = makecol(200, 200, 200);
	int firstGradStart = makecol(255, 0, 0);
	int firstGradEnd = makecol(255, 255, 0);
	int secondGradStart = makecol(255, 255, 0);
	int secondGradEnd = makecol(0, 255, 0);

	rectfill(bmp, x, y, x+w-1, y+h-1, 0);

	line(bmp, x, y, x+w-1, y, borderColor);
	line(bmp, x, y+h-1, x+w-1, y+h-1, borderColor);
	line(bmp, x, y, x, y+h-1, borderColor);
	line(bmp, x+w-1, y, x+w-1, y+h-1, borderColor);

	int i;

	for (i = 0; i < w - 2; i++) {
		float currPercent = (float)i / (w-2);
		int color;
		if (currPercent >= percent) {
			break;
		}
		if (currPercent < 0.5) {
			currPercent = (float)i / ((w-2)/2);
			color = getGradientColor(currPercent, firstGradStart, firstGradEnd);
		}
		else {
			currPercent = (float)(i-((w-2)/2)) / ((w-2)/2);
			color = getGradientColor(currPercent, secondGradStart, secondGradEnd);
		}
		line(bmp, x+i+1, y+1, x+i+1, y+h-2, color);
	}
}

void makeBlackAndWhite(BITMAP* bmp)
{
	for (int y = 0; y < bmp->h; y++) {
		for (int x = 0; x < bmp->w; x++) {
			int pixel = getpixel(bmp, x, y);
			if (pixel == makecol(255, 0, 255))
				continue;
			int r = getr(pixel);
			int g = getg(pixel);
			int b = getb(pixel);
			int avg = (r + g + b) / 3;
			putpixel(bmp, x, y, makecol(avg, avg, avg));
		}
	}
}

void brighten(BITMAP* bmp, int amount)
{
	for (int y = 0; y < bmp->h; y++) {
		for (int x = 0; x < bmp->w; x++) {
			int pixel = getpixel(bmp, x, y);
			if (pixel == makecol(255, 0, 255))
				continue;
			int r = MIN(255, getr(pixel)+amount);
			int g = MIN(255, getg(pixel)+amount);
			int b = MIN(255, getb(pixel)+amount);
			putpixel(bmp, x, y, makecol(r, g, b));
		}
	}
}

void applyGravity(float* dx, float* dy, int step)
{
	const float GRAVITY = 0.00005f;
	const float FRICTION = 0.00001f;

	if (*dx < 0) {
		*dx = MIN(0, *dx + FRICTION * step);
	}
	else {
		*dx = MAX(0, *dx - FRICTION * step);
	}

	*dy += GRAVITY * step;
}

int updateParticles(Particle* p, int num, bool gravity,
		bool resetWhenInvisible,
		float dr, float dg, float db,
		float da, int step, int totalLifetime)
{
	int alive = 0;

	for (int i = 0; i < num; i++) {
		if ((p[i].trail_used == 0) || (((int)p[i].trail[0].x != (int)p[i].trail[1].x ||
							   (int)p[i].trail[0].y != (int)p[i].trail[1].y))) {
			for (int j = p[i].trail_size-1; j > 0; j--) {
				memcpy(&p[i].trail[j], &p[i].trail[j-1], sizeof(ParticleUnique));
			}
			p[i].trail_used = MIN(p[i].trail_size, p[i].trail_used+1);
		}
		p[i].trail[0].x += p[i].dx * step;
		p[i].trail[0].y += p[i].dy * step;
		p[i].trail[0].alpha = MIN(255, MAX(0, p[i].trail[0].alpha + da * step));
		if (gravity) {
			applyGravity(&p[i].dx, &p[i].dy, step);
		}
		p[i].r = MIN(255, MAX(0, p[i].r + dr * step));
		p[i].g = MIN(255, MAX(0, p[i].g + dg * step));
		p[i].b = MIN(255, MAX(0, p[i].b + db * step));
		if (resetWhenInvisible && p[i].trail[0].alpha <= 0.0f) {
			p[i].trail[0].x = p[i].start_x;
			p[i].trail[0].y = p[i].start_y;
			p[i].trail[0].alpha = p[i].start_alpha;
		}
		p[i].lifetime += step;
		if (p[i].lifetime < totalLifetime) {
			alive++;
		}
	}

	return alive;
}

void drawParticles(BITMAP* buffer, Particle* p, int num)
{
	drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

	for (int i = 0; i < num; i++) {
		int color = makecol((int)p[i].r, (int)p[i].g, (int)p[i].b);
		for (int j = 0; j < p[j].trail_used; j++) {
			set_trans_blender(0, 0, 0, (int)p[i].trail[j].alpha);
			putpixel(buffer, (int)p[i].trail[j].x, (int)p[i].trail[j].y, color);
		}
	}

	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
}

void makeRed(BITMAP* bmp)
{
	for (int y = 0; y < bmp->h; y++) {
		for (int x = 0; x < bmp->w; x++) {
			int pixel = getpixel(bmp, x, y);
			if (pixel == makecol(255, 0, 255))
				continue;
			int r = getr(pixel);
			int g = getg(pixel);
			int b = getb(pixel);
			int avg = (r + g + b) / 3;
			putpixel(bmp, x, y, makecol(avg, 0, 0));
		}
	}
}

static void realDrawFocusEffect(BITMAP* readBuf, BITMAP* writeBuf, int size)
{
	if (get_color_depth() == 32) {
		for (int y = size/2; y < readBuf->h; y += size) {
			for (int x = size/2; x < writeBuf->w; x += size) {
				int color = memory_getpixel32(readBuf, x, y);
				int rx = x-size/2;
				int ry = y-size/2;
				rectfill(writeBuf, rx, ry, rx+size-1, ry+size-1, color);
			}
		}
	}
	else if (get_color_depth() == 24) {
		for (int y = size/2; y < readBuf->h; y += size) {
			for (int x = size/2; x < writeBuf->w; x += size) {
				int color = memory_getpixel24(readBuf, x, y);
				int rx = x-size/2;
				int ry = y-size/2;
				rectfill(writeBuf, rx, ry, rx+size-1, ry+size-1, color);
			}
		}
	}
	else {
		for (int y = size/2; y < readBuf->h; y += size) {
			for (int x = size/2; x < writeBuf->w; x += size) {
				int color = memory_getpixel15_or_16(readBuf, x, y);
				int rx = x-size/2;
				int ry = y-size/2;
				rectfill(writeBuf, rx, ry, rx+size-1, ry+size-1, color);
			}
		}
	}
}

void drawFocusEffect(bool reverse, int maxSize)
{
	const int DELAY = 100;

	BITMAP* readBuf = scr->getFXBuffer();
	BITMAP* writeBuf = scr->getBackBuffer();

	int currSize = reverse ? maxSize : 2;

	long start = tguiCurrentTimeMillis();

	for (int i = 0; i < maxSize; i++) {
		realDrawFocusEffect(readBuf, writeBuf, currSize);
		scr->draw();
		long now = tguiCurrentTimeMillis();
		long duration = now - start;
		if (duration < DELAY) {
			int r = DELAY-duration;
			for (int i = 0; i < r; i++) {
				updateMusic();
				rest(1);
			}
		}
		if (reverse) {
			currSize--;
		}
		else {
			currSize++;
		}
	}
}

static const int NUM_HOT_SPOTS = 30;
static const int NUM_FIRE_COLORS = 100;

static int hot_spots[NUM_HOT_SPOTS];
static int fire_colors[NUM_FIRE_COLORS];

static int fire_hot;
static int fire_mid;
static int fire_cold;
static int fire_count = 0;

static BITMAP* fire_bmp;
static int** fire_data;

static void moveFlames(void)
{
	// move flames
	for (int i = 0; i < NUM_HOT_SPOTS; i++) {
		hot_spots[i] += (AL_RAND() & 6) - 3;
		if (hot_spots[i] < 0)
			hot_spots[i] += FIRE_W;
		else if (hot_spots[i] >= FIRE_W)
			hot_spots[i] -= FIRE_W;
	}
}

void drawFlames(int step)
{
	for (int count = 0; count < step; count++) {
		int y = FIRE_H-1;
		for (int i = 0; i < FIRE_W; i++)
			fire_data[y][i] = 0;

		// draw bottom row
		for (int i = 0; i < NUM_HOT_SPOTS; i++) {
			for (int j = hot_spots[i]-20; j < hot_spots[i]+20; j++) {
				if (j >= 0 && j < FIRE_W) {
					fire_data[y][j] = MIN(fire_data[y][j] +
						20 - ABS(hot_spots[i]-j), NUM_FIRE_COLORS-1);
				}
			}
		}

		// move everything up
		for (int i = 0; i < FIRE_H-1; i++) {
			memcpy(fire_data[i], fire_data[i+1], FIRE_W*sizeof(int));
			for (int j = 0; j < FIRE_W; j++) {
				fire_data[i][j] = MIN(fire_data[i][j]+1, NUM_FIRE_COLORS-1);
			}
		}

		// draw to bmp
		if (get_color_depth() == 32) {	
			for (y = 0; y < FIRE_H; y++)
				for (int x = 0; x < FIRE_W; x++)
					memory_putpixel32(fire_bmp, x, y, fire_colors[fire_data[y][x]]);
		}
		else if (get_color_depth() == 24) {	
			for (y = 0; y < FIRE_H; y++)
				for (int x = 0; x < FIRE_W; x++)
					memory_putpixel24(fire_bmp, x, y, fire_colors[fire_data[y][x]]);
		}
		else {	
			for (y = 0; y < FIRE_H; y++)
				for (int x = 0; x < FIRE_W; x++)
					memory_putpixel15_or_16(fire_bmp, x, y, fire_colors[fire_data[y][x]]);
		}

		moveFlames();
	}
}

void initFlames()
{
	for (int i = 0; i < NUM_HOT_SPOTS; i++) {
		//hot_spots[i] = rand() % (FIRE_W-100) + 50;
		hot_spots[i] = rand() % FIRE_W;
	}
	
	fire_hot = makecol(255, 255, 200);
	fire_mid = makecol(255, 100, 0);
	fire_cold = makecol(10, 5, 0);
	
	for (int i = 0; i < NUM_FIRE_COLORS/2; i++) {
		fire_colors[i] = 
			getGradientColor((float)i/((float)NUM_FIRE_COLORS/2.0),
			fire_hot, fire_mid);
	}
	
	for (int i = 0; i < NUM_FIRE_COLORS/2; i++) {
		fire_colors[i+NUM_FIRE_COLORS/2] = 
			getGradientColor((float)i/((float)NUM_FIRE_COLORS/2.0),
			fire_mid, fire_cold);
	}
	
	fire_bmp = create_bitmap(FIRE_W, FIRE_H);
	clear_to_color(fire_bmp, makecol(0, 0, 0));

	fire_data = new int*[FIRE_H];
	for (int i = 0; i < FIRE_H; i++) {
		fire_data[i] = new int[FIRE_W];
		for (int j = 0; j < FIRE_W; j++)
			fire_data[i][j] = NUM_FIRE_COLORS-1;
	}

	for (int i = 0; i < FIRE_H; i++) {
		drawFlames(1);
		//moveFlames();
	}
}

void destroyFlames()
{
	destroy_bitmap(fire_bmp);
	for (int i = 0; i < FIRE_H; i++)
		delete[] fire_data[i];
	delete[] fire_data;
}

BITMAP* getFlames()
{
	return fire_bmp;
}

const int NUM_DUST_PARTICLES = 100;
static Particle dustParticles[NUM_DUST_PARTICLES];

void initDustEffect(int x, int y, int dustColor)
{
	for (int i = 0; i < NUM_DUST_PARTICLES; i++) {
		dustParticles[i].trail_size = 1;
		dustParticles[i].trail_used = 0;
		dustParticles[i].trail[0].x = x;
		dustParticles[i].trail[0].y = y;
		dustParticles[i].trail[0].alpha = 255.0f;
		dustParticles[i].dx = (float)((rand() % 100) - 50) / 2000.0f;
		dustParticles[i].dy = -((float)((rand() % 100)) / 2000.0f) - 0.02;
		dustParticles[i].r = getr(dustColor);
		dustParticles[i].g = getg(dustColor);
		dustParticles[i].b = getb(dustColor);
		dustParticles[i].lifetime = 0;
	}
}

bool updateDustEffect(int step)
{
	if (updateParticles(dustParticles, NUM_DUST_PARTICLES, true, false,
			0.0f, 0.0f, 0.0f, -0.2f, step, 1500) <= 0) {
		return true;
	}

	return false;
}

void drawDustEffect(BITMAP* buffer)
{
	drawParticles(buffer, dustParticles, NUM_DUST_PARTICLES);			
}

const int NUM_FIREWORK_PARTICLES = 150;
std::list<Particle*> fireworkParticles;

void initFireworkEffect(int x, int y, int dustColor)
{
	Particle* p = new Particle[NUM_FIREWORK_PARTICLES];

	for (int i = 0; i < NUM_FIREWORK_PARTICLES; i++) {
		p[i].trail_size = 3;
		p[i].trail_used = 0;
		p[i].trail[0].x = x;
		p[i].trail[0].y = y;
		p[i].trail[0].alpha = 255.0f;
		float angle = ((float)rand() / (float)RAND_MAX) * M_PI*2;
		float speed = ((float)rand() / (float)RAND_MAX) / 30.0f;
		p[i].dx = speed * cos(angle);
		p[i].dy = speed * sin(angle);
		p[i].r = getr(dustColor);
		p[i].g = getg(dustColor);
		p[i].b = getb(dustColor);
		p[i].lifetime = 0;
	}

	fireworkParticles.push_back(p);
}

bool updateFireworkEffect(int step)
{
	std::list<Particle*>::iterator it;
	for (it = fireworkParticles.begin(); it != fireworkParticles.end(); it++) {
		Particle* p = *it;
		if (updateParticles(p, NUM_FIREWORK_PARTICLES, false, false,
				0.0f, 0.0f, 0.0f, -0.2f, step, 1500) <= 0) {
			for (it = fireworkParticles.begin(); it != fireworkParticles.end(); it++) {
				delete[] *it;
			}
			fireworkParticles.clear();
			return true;
		}
	}

	return false;
}

void drawFireworkEffect(BITMAP* buffer)
{
	std::list<Particle*>::iterator it;
	for (it = fireworkParticles.begin(); it != fireworkParticles.end(); it++) {
		drawParticles(buffer, *it, NUM_FIREWORK_PARTICLES);			
	}
}


