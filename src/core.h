#pragma once
/*------------------------------------------------------*/
#define _USE_MATH_DEFINES
//#define WGL_EXT_swap_control
/*------------------------------------------------------*/
#define WIN32_DEFAULT_LIBS
#define _USE_MATH_DEFINES
#include <windows.h>
#include <mmsystem.h>
#include <malloc.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <map>
#include <algorithm>    // std::sort
#include <string>
#include "libs.h" // linker libs

#include "glew.h"
#include "wglew.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"
/*
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_audio.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_mixer.h"
#include "SDL/SDL_syswm.h"
*/
#include "error.h"
/*------------------------------------------------------*/
/*#define f32 float
#define f64 double
#define PP printf
#define fPP fprintf
#define sPP sprintf*/
#define ushort unsigned short
#define uint unsigned int
#define uchar unsigned char
/*------------------------------------------------------*/
#include "mathlib/vector.h"
#include "mathlib/matrix.h"
/*------------------------------------------------------*/
#define loop0i(end_l) for ( int i=0;i<end_l;++i )
#define loop0j(end_l) for ( int j=0;j<end_l;++j )
#define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
#define loopj(start_l,end_l) for ( int j=start_l;j<end_l;++j )
#define loopk(start_l,end_l) for ( int k=start_l;k<end_l;++k )
#define loopl(start_l,end_l) for ( int l=start_l;l<end_l;++l )
#define loopm(start_l,end_l) for ( int m=start_l;m<end_l;++m )
#define loopn(start_l,end_l) for ( int n=start_l;n<end_l;++n )
#define loop(a_l,start_l,end_l) for ( int a_l = start_l;a_l<end_l;++a_l )
/*------------------------------------------------------*/
#define loopij(_sti,_stj,_eni,_enj) loopi(_sti,_eni)loopj (_stj,_enj)
#define loopijk(_sti,_stj,_stk,_eni,_enj,_enk) loopi(_sti,_eni) loopj (_stj,_enj) loopk (_stk,_enk)
#define looplmn(_stl,_stm,_stn,_enl,_enm,_enn) loopl(_stl,_enl) loopm (_stm,_enm) loopn (_stn,_enn)
/*------------------------------------------------------*/
#define foreach(var, container) for( auto var = (container).begin(); var != (container).end(); ++var)
/*------------------------------------------------------*/
#define f_min min
#define f_max max
#define vswap(a,b) { auto c=a;a=b;b=c; }
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
#define clamp(a_,b_,c_) (min(max((a_),(b_)),(c_)))
#define frac(a) (a-floor(a))
#define dot3(a,b) (a.x*b.x+a.y*b.y+a.z*b.z)
#define cross3(a,b) vec3f( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z , a.x * b.y - a.y * b.x )
#define min3f(a,b) vec3f(min(a.x,b.x),min(a.y,b.y),min(a.z,b.z))
#define max3f(a,b) vec3f(max(a.x,b.x),max(a.y,b.y),max(a.z,b.z))
#define lerp(a_,b_,t_) ( a_*(1-t_) + b_*t_ )
#define normalize3(a) (a*(1.0/sqrt(a.x*a.x+a.y*a.y+a.z*a.z)))
#define normalize4(a) (a*(1.0/sqrt(a.x*a.x+a.y*a.y+a.z*a.z+a.w*a.w)))
/*------------------------------------------------------*/
#define ifeq(sa,sb) if( 0==strncmp(sa,sb,min(strnlen(sa,1000),strnlen(sb,1000)) )) 
/*------------------------------------------------------*/
#define loopall(var, container) for( auto var = (container).begin(); var != (container).end(); ++var)
#define ifeq_lower(a,b) if( strnlen(a,1000)>0 && strnlen(b,1000)>0 && strcmp_lower(a,b)) 
#define ifexist( fname ) if( fexist(fname) )
#define ifnotexist( fname ) if( !fexist(fname) )
/*------------------------------------------------------*/

/*------------------------------------------------------*/
struct intfloat
{
	union {int i;float f;uint ui;};
};
/*------------------------------------------------------*/
class uchar4 
{ 
	public:

	uchar x,y,z,w;

	uchar4(){};
	uchar4(uint x,uint y,uint z,uint w)
	{
		this->x=x;
		this->y=y;
		this->z=z;
		this->w=w;
	}
	
	uint to_uint()
	{
		return x+(y<<8)+(z<<24);//*((uint*)this);
	};
	void from_uint(uint a)
	{
		x=a;y=a>>8;z=a>>24;//*((uint*)this)=a;
	};
};
/*------------------------------------------------------*/
float cubicInterpolate (float p[4], float x) ;
float bicubicInterpolate (float p[4][4], float x, float y);
std::string get_pure_filename ( std::string filename );
std::string get_path ( std::string filename );
std::string int_to_str(const int x);
char* str( const char* format, ... );
bool file_exists(char* f);
std::string get_current_dir();
void file_get_dir(const char* dirname,
			 std::vector<std::string>	&listdirs,
			 std::vector<std::string>	&listfiles,
			 std::vector<long>			&listfilesize,
			 char* extension);
FILE* fopenx( const char *name,const char *mode );
int splitstr(char* s,char** sp1,char** sp2,char** sp3);
/*------------------------------------------------------*/
		