/* The "simple" example, modified to use a pre-built glyphmap instead of using freetype.
 * This can be used when using libdrawtext-noft (built without freetype support/dependency).
 *
 * There is only one difference between this, and the regular usage demonstrated in
 * examples/simple, and it's marked with an XXX comment in the code. For the rest of
 * the details of libdrawtext usage in this code, refer to examples/simple/simple.c.
 */
#include <stdio.h>
#include <stdlib.h>

#ifndef __APPLE__
#include <GL/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include "drawtext.h"

void disp(void);
void reshape(int x, int y);
void keyb(unsigned char key, int x, int y);

struct dtx_font *font;

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitWindowSize(512, 384);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("libdrawtext example: simple");

	glutDisplayFunc(disp);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyb);

	/* XXX: only difference is that we have to call dtx_open_font_glyphmap,
	 * instead of dtx_open_font, passing it the filename of a pre-built
	 * glyphmap (see tools/font2glyphmap).
	 */
	if(!(font = dtx_open_font_glyphmap("serif_s24.glyphmap"))) {
		fprintf(stderr, "failed to open font\n");
		return 1;
	}
	dtx_use_font(font, 24);

	glutMainLoop();
	return 0;
}

const char *text = "Some sample text goes here.\n"
	"Yada yada yada, more text...\n"
	"foobar xyzzy\n";

void disp(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	glTranslatef(-200, 150, 0);
	glColor3f(1, 1, 1);
	dtx_string(text);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-200, 50, 0);
	glScalef(2, 0.7, 1);
	glColor3f(0.6, 0.7, 1.0);
	dtx_string(text);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-80, -90, 0);
	glRotatef(20, 0, 0, 1);
	glColor3f(1.0, 0.7, 0.6);
	dtx_string(text);
	glPopMatrix();

	glutSwapBuffers();
}

void reshape(int x, int y)
{
	glViewport(0, 0, x, y);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-x/2, x/2, -y/2, y/2, -1, 1);
}

void keyb(unsigned char key, int x, int y)
{
	if(key == 27) {
		exit(0);
	}
}
