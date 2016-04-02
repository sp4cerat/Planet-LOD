#define ogl_check_error() { \
	GLenum error = glGetError();\
    if( error != GL_NO_ERROR )\
	{	error_stop( "OpenGL Error: %s\n", gluErrorString( error ) ); } }

#include "glsl.h"
#include "fbo.h"

void ogl_init()
{
	
    //Initialize Projection Matrix
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    //Initialize Modelview Matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
	
	glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 1);

	if( glewInit() != GLEW_OK)error_stop("GLEW initialization failed\n");

	ogl_check_error();
}

int ogl_tex_new(unsigned int size_x, unsigned int size_y, int filter=GL_LINEAR,int repeat=GL_CLAMP_TO_EDGE,int type1=GL_RGBA,int type2=GL_RGBA,uchar* data_in=0,int type3=GL_UNSIGNED_BYTE)
{
	uchar *data=data_in;	
	//if(!data) data=(uchar*)malloc(size_x*size_y*);//error_stop("ogl_tex_new data==0");

	int id=0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // create a texture
    glGenTextures(1, (GLuint *)&id);ogl_check_error();
    glBindTexture(GL_TEXTURE_2D, id);ogl_check_error();

    // set basic parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);ogl_check_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);ogl_check_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);ogl_check_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
		(filter==GL_LINEAR_MIPMAP_LINEAR ||
	   filter==GL_NEAREST_MIPMAP_LINEAR ) ? GL_LINEAR : GL_NEAREST );ogl_check_error();
	
    // buffer data
	if(filter==GL_LINEAR_MIPMAP_LINEAR ||
	   filter==GL_NEAREST_MIPMAP_LINEAR || 
	   filter==GL_LINEAR_MIPMAP_NEAREST || 
	   filter==GL_NEAREST_MIPMAP_NEAREST)

		
		gluBuild2DMipmaps( GL_TEXTURE_2D, type1, size_x,   size_y, type2, type3, data );
	else
		glTexImage2D(GL_TEXTURE_2D, 0, type1, size_x, size_y, 0, type2, type3, data);
		

	ogl_check_error();
	glBindTexture(GL_TEXTURE_2D, 0);

	return id;
}

int ogl_tex_bmp(const Bmp &bmp, int filter=GL_LINEAR,int repeat=GL_REPEAT)//CLAMP_TO_EDGE)
{
	if(bmp.bpp==32)
	return ogl_tex_new(bmp.width, bmp.height,filter,repeat, GL_RGBA, GL_RGBA,(uchar*)&bmp.data[0]);

	return ogl_tex_new(bmp.width, bmp.height,filter,repeat, GL_RGB, GL_RGB,(uchar*)&bmp.data[0]);
}

int ogl_tex_float_bmp(const Bmp &bmp, int filter=GL_LINEAR,int repeat=GL_CLAMP_TO_EDGE)
{
	if(bmp.bpp!=32)return -1;
	return ogl_tex_new(bmp.width, bmp.height,filter,repeat, GL_LUMINANCE16F_ARB, GL_LUMINANCE,(uchar*)&bmp.data[0], GL_FLOAT);
}

GLuint ogl_pbo_new(int size)
{
	GLuint pbo;
    glGenBuffers(1, &pbo);
    glBindBuffer(GL_ARRAY_BUFFER, pbo);
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
	return pbo;
}

void ogl_pbo_del(const GLuint pbo)
{
    glBindBuffer(GL_ARRAY_BUFFER,pbo);
    glDeleteBuffers(1, &pbo);
}

float ogl_read_z(int x, int y)
{
    GLint viewport[4];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;

	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

	return winZ;
}
vec3f ogl_unproject(int x, int y, float winZ=-10)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;

	if(winZ<-5) winZ=ogl_read_z(x,y);
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
 
    return vec3f(posX, posY, posZ);
}

void ogl_drawline(	float x0,float y0,float z0,float x1,float y1,float z1)
{
	glBegin(GL_LINES);
	glVertex3f(x0,y0,z0);
	glVertex3f(x1,y1,z1);
	glEnd();
}
void ogl_drawline( vec3f p1, vec3f p2 )
{
	ogl_drawline(p1.x,p1.y,p1.z,p2.x,p2.y,p2.z);
}

inline void ogl_drawquad(	float x0,float y0,float x1,float y1,
							float tx0=0,float ty0=0,float tx1=0,float ty1=0)
{
	float vertices[]={ x0,y0,0,    x0,y1,0,    x1,y0,0,    x1,y1,0  };
	float uvs[]     ={ tx0,ty0,0,  tx0,ty1,0,  tx1,ty0,0,  tx1,ty1,0  };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer( 3,GL_FLOAT,3*4,vertices);
	glTexCoordPointer( 3,GL_FLOAT,3*4,uvs );

	glDrawArrays(GL_TRIANGLE_STRIP,0,4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	/*
	glBegin(GL_QUADS);
	glTexCoord2f(tx0,ty0);glVertex2f(x0,y0);
	glTexCoord2f(tx1,ty0);glVertex2f(x1,y0);
	glTexCoord2f(tx1,ty1);glVertex2f(x1,y1);
	glTexCoord2f(tx0,ty1);glVertex2f(x0,y1);
	glEnd();
	*/
	ogl_check_error();
}

void ogl_draw_colorquad(	
	float x0,float y0,float x1,float y1,
	vec3f c0,vec3f c1,vec3f c2,vec3f c3)
{
	float rx=4,ry=4;
	loopijk(0,0,0,int(rx),int(ry),4)
	{
		if (k==0)glBegin(GL_QUADS);
		float a=float(i+1*(k==1||k==2))/rx;
		float b=float(j+1*(k>=2))/ry;
		vec3f c01=c0+(c1-c0)*a;
		vec3f c32=c3+(c2-c3)*a;
		vec3f c=c01+(c32-c01)*b;
		glColor3f(c.x,c.y,c.z);
		float x=a*(x1-x0)+x0;
		float y=b*(y1-y0)+y0;
		glVertex2f(x,y);		
		if (k==3)glEnd();
	}
}

void ogl_drawlinequad(float x0,float y0,float x1,float y1)
{
	glBegin(GL_LINE_LOOP);
	glVertex2f(x0,y0);
	glVertex2f(x1,y0);
	glVertex2f(x1,y1);
	glVertex2f(x0,y1);
	glEnd();
}

void ogl_subdiv_tri(vec3f p0,vec3f p1,vec3f p2)
{
	static int iteration=0;

	if(iteration==4)
	{
		vec3f mid=(p0+p1+p2)*(1.0/3.0);
		mid.norm();
		float size=0.01;
		ogl_drawlinequad(
			mid.x-size,
			mid.y-size,
			mid.x+size,
			mid.y+size);
		//p0.norm();
		//p1.norm();
		//p2.norm();

		ogl_drawline(p0,p1);
		ogl_drawline(p1,p2);
		ogl_drawline(p2,p0);
		return;
	}

	iteration++;

	vec3f p01=(p0+p1)*0.5;p01.norm();
	vec3f p12=(p1+p2)*0.5;p12.norm();
	vec3f p20=(p2+p0)*0.5;p20.norm();

	ogl_subdiv_tri(p0,p20,p01);
	ogl_subdiv_tri(p1,p01,p12);
	ogl_subdiv_tri(p2,p12,p20);
	ogl_subdiv_tri(p01,p20,p12);

	iteration--;
}

void ogl_bind(int tmu,int tex)
{
	glActiveTextureARB(GL_TEXTURE0+tmu );
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	ogl_check_error();
}

void ogl_screenshot()
{
	Bmp bmp;
	int tmp[4],width,height;
	glGetIntegerv(GL_VIEWPORT, tmp);
	width=tmp[2];height=tmp[3];
	bmp.set(width,height,24,0);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,&bmp.data[0]);
	glFlush();			
	CreateDirectory("../screenshots",0);
	int count=0;
	while(file_exists(str("../screenshots/%d.png",count)))count++;
	bmp.save(str("../screenshots/%d.png",count));
}