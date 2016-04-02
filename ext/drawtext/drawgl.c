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
#ifndef NO_OPENGL
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include <stdlib.h>

#ifndef _MSC_VER
#include <alloca.h>
#else
#include <malloc.h>
#endif

#ifdef TARGET_IPHONE
#include <OpenGLES/ES2/gl.h>
#else
#include <glew.h>
#endif

#include "drawtext.h"
#include "drawtext_impl.h"

struct vertex {
	float x, y;
	float s, t;
};

struct quad {
	struct vertex v[6];
};

static void cleanup(void);
static void add_glyph(struct glyph *g, float x, float y);

#define QBUF_SZ		512
static struct quad *qbuf;
static int num_quads;
static int vattr = -1;
static int tattr = -1;
static unsigned int font_tex;
static int buf_mode = DTX_NBF;
struct dtx_box *bbox=0;

int dtx_gl_init(void)
{
	if(qbuf) {
		return 0;	/* already initialized */
	}

	glewInit();

	if(!(qbuf = (struct quad*)malloc(QBUF_SZ * sizeof *qbuf))) {
		return -1;
	}
	num_quads = 0;

	atexit(cleanup);
	return 0;
}

static void cleanup(void)
{
	free(qbuf);
}


void dtx_vertex_attribs(int vert_attr, int tex_attr)
{
	vattr = vert_attr;
	tattr = tex_attr;
}

static void set_glyphmap_texture(struct dtx_glyphmap *gmap)
{
	if(!gmap->tex) {
		glGenTextures(1, &gmap->tex);
		glBindTexture(GL_TEXTURE_2D, gmap->tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

#ifdef GL_ES
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, gmap->xsz, gmap->ysz, 0, GL_ALPHA, GL_UNSIGNED_BYTE, gmap->pixels);
		glGenerateMipmap(GL_TEXTURE_2D);
#else
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, gmap->xsz, gmap->ysz, GL_ALPHA, GL_UNSIGNED_BYTE, gmap->pixels);
#endif
	}

	if(font_tex != gmap->tex) {
		dtx_flush();
	}
	font_tex = gmap->tex;
}

void dtx_glyph(int code)
{
	struct dtx_glyphmap *gmap;

	if(!dtx_font || !(gmap = dtx_get_font_glyphmap(dtx_font, dtx_font_sz, code))) {
		return;
	}
	set_glyphmap_texture(gmap);

	add_glyph(gmap->glyphs + code - gmap->cstart, 0, 0);
	dtx_flush();
}

static const char *put_char(const char *str, float *pos_x, float *pos_y, int *should_flush )
{
	struct dtx_glyphmap *gmap;
	float px, py;
	int code = dtx_utf8_char_code(str);
	str = dtx_utf8_next_char((char*)str);

	if(buf_mode == DTX_LBF && code == '\n') {
		*should_flush = 1;
	}

	px = *pos_x;
	py = *pos_y;

	if((gmap = dtx_proc_char(code, pos_x, pos_y))) {
		int idx = code - gmap->cstart;

		set_glyphmap_texture(gmap);
		add_glyph(gmap->glyphs + idx, px, py);
	}
	return str;
}

void dtx_string(const char *str, struct dtx_box *box)
{
	int should_flush = buf_mode == DTX_NBF;
	float pos_x = 0.0f;
	float pos_y = 0.0f;

	if(!dtx_font) {
		return;
	}

	pos_y=dtx_line_height();

	while(*str) {
		
		if(box)
		{
			if(*str>=32)if(pos_x+dtx_glyph_width(*str)>box->width)
			{
				pos_y+=dtx_line_height();
				pos_x=0;

				
			}
			if(pos_y>box->y+box->height) break;
		}
		/**/
		str = put_char(str, &pos_x, &pos_y, &should_flush);
		
	}

	if(should_flush) {
		dtx_flush();
	}
}

void dtx_printf(const char *fmt, ...)
{
	va_list ap;
	int buf_size;
	char *buf, tmp;

	if(!dtx_font) {
		return;
	}

	va_start(ap, fmt);
	buf_size = vsnprintf(&tmp, 0, fmt, ap);
	va_end(ap);

	if(buf_size == -1) {
		buf_size = 512;
	}

	buf = (char*) alloca(buf_size + 1);
	va_start(ap, fmt);
	vsnprintf(buf, buf_size + 1, fmt, ap);
	va_end(ap);

	dtx_string(buf,0);
}

static void qvertex(struct vertex *v, float x, float y, float s, float t)
{
	v->x = x;
	v->y = y;
	v->s = s;
	v->t = t;
}

static void add_glyph(struct glyph *g, float x, float y)
{
	struct quad *qptr = qbuf + num_quads;

	x -= g->orig_x;
	y += g->orig_y;

	qvertex(qptr->v + 0, x, y, g->nx, g->ny + g->nheight);
	qvertex(qptr->v + 1, x + g->width, y, g->nx + g->nwidth, g->ny + g->nheight);
	qvertex(qptr->v + 2, x + g->width, y - g->height, g->nx + g->nwidth, g->ny);

	qvertex(qptr->v + 3, x, y, g->nx, g->ny + g->nheight);
	qvertex(qptr->v + 4, x + g->width, y - g->height, g->nx + g->nwidth, g->ny);
	qvertex(qptr->v + 5, x, y - g->height, g->nx, g->ny);

	if(++num_quads >= QBUF_SZ) {
		dtx_flush();
	}
}

void dtx_flush(void)
{
	int vbo;

	if(!num_quads) {
		return;
	}

	if(glBindBuffer) {
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

#ifndef GL_ES
	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
#endif
	glBindTexture(GL_TEXTURE_2D, font_tex);

	if(vattr != -1 && glEnableVertexAttribArray) {
		glEnableVertexAttribArray(vattr);
		glVertexAttribPointer(vattr, 2, GL_FLOAT, 0, sizeof(struct vertex), qbuf);
#ifndef GL_ES
	} else {
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, sizeof(struct vertex), qbuf);
#endif
	}
	if(tattr != -1 && glEnableVertexAttribArray) {
		glEnableVertexAttribArray(tattr);
		glVertexAttribPointer(tattr, 2, GL_FLOAT, 0, sizeof(struct vertex), &qbuf->v[0].s);
#ifndef GL_ES
	} else {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(struct vertex), &qbuf->v[0].s);
#endif
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthMask(0);

	glDrawArrays(GL_TRIANGLES, 0, num_quads * 6);

	glDepthMask(1);

	if(vattr != -1 && glDisableVertexAttribArray) {
		glDisableVertexAttribArray(vattr);
#ifndef GL_ES
	} else {
		glDisableClientState(GL_VERTEX_ARRAY);
#endif
	}
	if(tattr != -1 && glDisableVertexAttribArray) {
		glDisableVertexAttribArray(tattr);
#ifndef GL_ES
	} else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	}

#ifndef GL_ES
	glPopAttrib();
#else
	glDisable(GL_BLEND);
#endif

	if(glBindBuffer && vbo) {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
	}

	num_quads = 0;
}

#endif	/* !def NO_OPENGL */
