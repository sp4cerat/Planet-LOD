/*
libdrawtext - a simple library for fast text rendering in OpenGL
Copyright (C) 2011-2012  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef TEXT_IMPL_H_
#define TEXT_IMPL_H_

struct glyph {
	int code;
	float x, y, width, height;
	/* normalized coords [0, 1] */
	float nx, ny, nwidth, nheight;
	float orig_x, orig_y;
	float advance;
	struct glyph *next;
};

struct dtx_glyphmap {
	int ptsize;

	int xsz, ysz;
	unsigned char *pixels;
	unsigned int tex;

	int cstart, cend;	/* character range */
	int crange;

	float line_advance;

	struct glyph *glyphs;
	struct dtx_glyphmap *next;
};

struct dtx_font {
	/* freetype FT_Face */
	void *face;

	/* list of glyphmaps */
	struct dtx_glyphmap *gmaps;

	/* last returned glyphmap (cache) */
	struct dtx_glyphmap *last_gmap;
};


struct dtx_font *dtx_font;
int dtx_font_sz;


#define fperror(str) \
	fprintf(stderr, "%s: %s: %s\n", __FUNCTION__, (str), strerror(errno))

int dtx_gl_init(void);

/* returns zero if it should NOT be printed and modifies xpos/ypos */
/* implemented in font.c */
struct dtx_glyphmap *dtx_proc_char(int code, float *xpos, float *ypos);

#endif	/* TEXT_IMPL_H_ */
