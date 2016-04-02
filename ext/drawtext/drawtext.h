/*
libdrawtext - a simple library for fast text rendering in OpenGL
Copyright (C) 2011-2014  John Tsiombikas <nuclear@member.fsf.org>

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
#ifndef LIBDRAWTEXT_H_
#define LIBDRAWTEXT_H_

#include <stdio.h>
#include <stdlib.h>

struct dtx_font;
struct dtx_glyphmap;

/* draw buffering modes */
enum {
	DTX_NBF,	/* unbuffered */
	DTX_LBF,	/* line buffered */
	DTX_FBF		/* fully buffered */
};

struct dtx_box {
	float x, y;
	float width, height;
};

#ifdef __cplusplus
extern "C" {
#endif

/* Open a truetype/opentype/whatever font.
 *
 * If sz is non-zero, the default ASCII glyphmap at the requested point size is
 * automatically created as well, and ready to use.
 *
 * To use other unicode ranges and different font sizes you must first call
 * dtx_prepare or dtx_prepare_range before issuing any drawing calls, otherwise
 * nothing will be rendered.
 */
struct dtx_font *dtx_open_font(const char *fname, int sz);
/* open a font by loading a precompiled glyphmap (see: dtx_save_glyphmap)
 * this works even when compiled without freetype support
 */
struct dtx_font *dtx_open_font_glyphmap(const char *fname);
/* close a font opened by either of the above */
void dtx_close_font(struct dtx_font *fnt);

/* prepare an ASCII glyphmap for the specified font size */
void dtx_prepare(struct dtx_font *fnt, int sz);
/* prepare an arbitrary unicode range glyphmap for the specified font size */
void dtx_prepare_range(struct dtx_font *fnt, int sz, int cstart, int cend);

/* Finds the glyphmap that contains the specified character code and matches the requested size
 * Returns null if it hasn't been created (you should call dtx_prepare/dtx_prepare_range).
 */
struct dtx_glyphmap *dtx_get_font_glyphmap(struct dtx_font *fnt, int sz, int code);

/* Finds the glyphmap that contains the specified unicode range and matches the requested font size
 * Will automatically generate one if it can't find it.
 */
struct dtx_glyphmap *dtx_get_font_glyphmap_range(struct dtx_font *fnt, int sz, int cstart, int cend);

/* Creates and returns a glyphmap for a particular unicode range and font size.
 * The generated glyphmap is added to the font's list of glyphmaps.
 */
struct dtx_glyphmap *dtx_create_glyphmap_range(struct dtx_font *fnt, int sz, int cstart, int cend);
/* free a glyphmap */
void dtx_free_glyphmap(struct dtx_glyphmap *gmap);

/* returns a pointer to the raster image of a glyphmap (1 byte per pixel grayscale) */
unsigned char *dtx_get_glyphmap_image(struct dtx_glyphmap *gmap);
/* returns the width of the glyphmap image in pixels */
int dtx_get_glyphmap_width(struct dtx_glyphmap *gmap);
/* returns the height of the glyphmap image in pixels */
int dtx_get_glyphmap_height(struct dtx_glyphmap *gmap);

/* The following functions can be used even when the library is compiled without
 * freetype support.
 */
struct dtx_glyphmap *dtx_load_glyphmap(const char *fname);
struct dtx_glyphmap *dtx_load_glyphmap_stream(FILE *fp);
int dtx_save_glyphmap(const char *fname, const struct dtx_glyphmap *gmap);
int dtx_save_glyphmap_stream(FILE *fp, const struct dtx_glyphmap *gmap);

/* adds a glyphmap to a font */
void dtx_add_glyphmap(struct dtx_font *fnt, struct dtx_glyphmap *gmap);

/* ---- rendering ---- */

/* before drawing anything this function must set the font to use */
void dtx_use_font(struct dtx_font *fnt, int sz);

/* sets the buffering mode
 * - DTX_NBUF: every call to dtx_string gets rendered immediately.
 * - DTX_LBUF: renders when the buffer is full or the string contains a newline.
 * - DTX_FBUF: renders only when the buffer is full (you must call dtx_flush explicitly).
 */
void dtx_draw_buffering(int mode);

/* Sets the vertex attribute indices to use for passing vertex and texture coordinate
 * data. By default both are -1 which means you don't have to use a shader, and if you
 * do they are accessible through gl_Vertex and gl_MultiTexCoord0, as usual.
 *
 * NOTE: If you are using OpenGL ES 2.x or OpenGL >= 3.1 pure (non-compatibility) context
 * you must specify valid attribute indices.
 */
void dtx_vertex_attribs(int vert_attr, int tex_attr);

/* draws a single glyph at the origin */
void dtx_glyph(int code);
/* draws a utf-8 string starting at the origin. \n \r and \t are handled appropriately. */
void dtx_string(const char *strz, struct dtx_box *box);

void dtx_printf(const char *fmt, ...);

/* render any pending glyphs (see dtx_draw_buffering) */
void dtx_flush(void);


/* ---- encodings ---- */

/* returns a pointer to the next character in a utf-8 stream */
char *dtx_utf8_next_char(char *str);

/* returns the unicode character codepoint of the utf-8 character starting at str */
int dtx_utf8_char_code(const char *str);

/* returns the number of bytes of the utf-8 character starting at str */
int dtx_utf8_nbytes(const char *str);

/* returns the number of utf-8 character in a zero-terminated utf-8 string */
int dtx_utf8_char_count(const char *str);

/* Converts a unicode code-point to a utf-8 character by filling in the buffer
 * passed at the second argument, and returns the number of bytes taken by that
 * utf-8 character.
 * It's valid to pass a null buffer pointer, in which case only the byte count is
 * returned (useful to figure out how much memory to allocate for a buffer).
 */
size_t dtx_utf8_from_char_code(int code, char *buf);

/* Converts a unicode utf-16 wchar_t string to utf-8, filling in the buffer passed
 * at the second argument. Returns the size of the resulting string in bytes.
 *
 * It's valid to pass a null buffer pointer, in which case only the size gets
 * calculated and returned, which is useful for figuring out how much memory to
 * allocate for the utf-8 buffer.
 */
size_t dtx_utf8_from_string(const wchar_t *str, char *buf);


/* ---- metrics ---- */
float dtx_line_height(void);

/* rendered dimensions of a single glyph */
void dtx_glyph_box(int code, struct dtx_box *box);
float dtx_glyph_width(int code);
float dtx_glyph_height(int code);

/* rendered dimensions of a string */
void dtx_string_box(const char *str, struct dtx_box *box);
float dtx_string_width(const char *str);
float dtx_string_height(const char *str);

/* returns the horizontal position of the n-th character of the rendered string
 * (useful for placing cursors)
 */
float dtx_char_pos(const char *str, int n);

int dtx_char_at_pt(const char *str, float pt);

#ifdef __cplusplus
}
#endif

#endif	/* LIBDRAWTEXT_H_ */
