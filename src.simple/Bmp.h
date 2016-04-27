///////////////////////////////////////////
#pragma once
///////////////////////////////////////////
#include "core.h"
#include "mathlib/Vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "error.h"
///////////////////////////////////////////
#pragma comment(lib,"DevIL.lib")
#pragma comment(lib,"ILU.lib")
#pragma comment(lib,"ILUT.lib")
///////////////////////////////////////////
class Bmp
{
public:

	Bmp();
	Bmp(int x,int y,int bpp,unsigned char*data=0);
	Bmp(const char*filename, bool convert32=0);
	~Bmp();

	void load(const char*filename, bool convert32=0);
	void save(const char*filename);
	void set(int x,int y,int bpp,unsigned char*data);
	void crop(int x,int y,int x0=0,int y0=0);
	void scale(int x,int y);
	void blur(int count);
	void hblur(int count);
	void vblur(int count);
	void convert_24_32();
	void save_float(const char*filename, float* fdata=0);
	bool load_float(const char*filename, float* fdata=0);
	void MakeBump()
	{
		int stride=bpp/8;
		std::vector<vec3f> normals;normals.resize(width*height);
		loopi(0,width)loopj(0,height)
		{
			float h =data[( i+j*width)*stride+0];
			float hx=data[((i+2)%width+j*width)*stride+0];
			float hy=data[( i+((j+2)%height)*width)*stride+0];
			vec3f d1(64,0,hx-h);
			vec3f d2(0,64,hy-h);
			vec3f n;n.cross(d1,d2);n.norm();
			normals[i+j*width]=n;
		}
		loopi(0,width)loopj(0,height)
		{
			vec3f n=normals[i+j*width];
			data[(i+j*width)*stride+0] = n.x*127.5+127.5;
			data[(i+j*width)*stride+1] = n.y*127.5+127.5;
			data[(i+j*width)*stride+2] = n.z*127.5+127.5;
		}

	}
	void set_pixel(int x,int y,int r,int g,int b)
	{
		data[(x+y*width)*(bpp/8)+2]=r;
		data[(x+y*width)*(bpp/8)+1]=g;
		data[(x+y*width)*(bpp/8)+0]=b;
	}	
	int get_pixel(int x,int y)
	{
		if(data.size()==0) error_stop("get_pixel data=0");
		if(x>=width)return 0;
		if(y>=height)return 0;
		return
			data[(x+y*width)*(bpp/8)+0]+
			data[(x+y*width)*(bpp/8)+1]*256+
			data[(x+y*width)*(bpp/8)+2]*256*256;
	}
	vec3f get_pixel3f(int x,int y)
	{
		int color=get_pixel(x,y);
		float r=float(color&255)/255.0f;
		float g=float((color>>8)&255)/255.0f;
		float b=float((color>>16)&255)/255.0f;
		return vec3f(r,g,b);
	}
	void flip()
	{
		loopijk(0,0,0,width,height/2,bpp/8)
		{
			vswap(	data[(i+j*width)*(bpp/8)+k],
					data[(i+(height-1-j)*width)*(bpp/8)+k] );
		}
	}

public:
	std::vector<uchar> data;
	int width;
	int height;
	int bpp;
	int tex_handle;
};

