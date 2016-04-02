/* Simple libdrawtext example.
 *
 * Important parts are marked with XXX comments.
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

/* XXX fonts are represented by the opaque struct dtx_font type, so you
 * need to create at least one with dtx_open_font (see main).
 */
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

	/* XXX dtx_open_font opens a font file and returns a pointer to dtx_font */
	if(!(font = dtx_open_font("serif.ttf", 24))) {
		fprintf(stderr, "failed to open font\n");
		return 1;
	}
	/* XXX select the font and size to render with by calling dtx_use_font
	 * if you want to use a different font size, you must first call:
	 * dtx_prepare(font, size) once.
	 */
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
	/* XXX call dtx_string to draw utf-8 text.
	 * any transformations and the current color apply
	 */
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
