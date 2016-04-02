//#################################################################//
#include "Bmp.h"
//#include "error.h"
#include <IL/devil_cpp_wrapper.hpp>
//#################################################################//
void init_ilu_lib()
{
	static bool ini=false;

	if(!ini)
	{
		ilInit();
		iluInit();
		ini=true;
	}
}
//#################################################################//
Bmp::Bmp()
{
	init_ilu_lib();
	width=height=0;
	data.clear();
}
//#################################################################//
Bmp::Bmp(const char*filename, bool convert32)
{
	init_ilu_lib();
	width=height=0;
	load(filename,convert32);
}
//#################################################################//
Bmp::Bmp(int x,int y,int b,unsigned char*buffer)
{
	width=height=0;
	set(x,y,b,buffer);
}
//#################################################################//
Bmp::~Bmp()
{
}
//#################################################################//
void Bmp::save(const char*filename)
{
	flip();
	printf("saving bmp %dx%dx%d\n",width, height, bpp);
	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);
	if(bpp==24) 	ilTexImage(width, height, 1, bpp/8, IL_RGB, IL_UNSIGNED_BYTE, &data[0]); //the third agrument is usually always 1 (its depth. Its never 0 and I can't think of a time when its 2)
	if(bpp==32) 	ilTexImage(width, height, 1, bpp/8, IL_RGBA, IL_UNSIGNED_BYTE, &data[0]); //the third agrument is usually always 1 (its depth. Its never 0 and I can't think of a time when its 2)
	ilEnable(IL_FILE_OVERWRITE);
	ilSaveImage(filename);	
	ilDeleteImages(1, &imageID); // Delete the image name. 
	flip();
}
//#################################################################//
void  Bmp::blur(int count)
{
	int x,y,b,c;
	int bytes=bpp/8;
	for(c=0;c<count;c++)
		for(x=0;x<width-1;x++)
			for(y=0;y<height-1;y++)
				for(b=0;b<bytes;b++)
					data[(y*width+x)*bytes+b]=
					    (	(int)data[((y+0)*width+x+0)*bytes+b]+
					      (int)data[((y+0)*width+x+1)*bytes+b]+
					      (int)data[((y+1)*width+x+0)*bytes+b]+
					      (int)data[((y+1)*width+x+1)*bytes+b] ) /4;

}
//#################################################################//
void Bmp::crop(int x,int y,int x0,int y0)
{
	if(data.size()==0)return;
	
	int i,j;
	int bytes=bpp/8;
	unsigned char* newdata=(unsigned char*)malloc(x*y*bytes);

	if(!newdata) error_stop("Bmp::crop : out of memory");

	memset(newdata,0,x*y*bytes);

	int start=bytes*(x0+y0*width);

	for(i=0;i<y;i++)
		if(i<height)
			for(j=0;j<x*bytes;j++)
				if(j<width*bytes)
					newdata[i*x*bytes+j]=data[i*width*bytes+j+start];
	
	set(x,y,bpp,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::convert_24_32()
{
	if(data.size()==0)return ;

	int x=width, y=height;
	if(x==0)return ;
	if(y==0)return ;

	unsigned char* newdata=(unsigned char*)malloc(x*y*4);
	int bytes=bpp/8;

	loopijk(0,0,0, y,x,bytes)
		newdata[i*x*4+j*4+k]=
			data[i*x*bytes+j*bytes+k];

	loopi(0,x*y) newdata[i*4+3]=0;
	set(x,y,32,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::scale(int x,int y)
{
	if(data.size()==0)return ;
	if(x==0)return ;
	if(y==0)return ;

	int bytes=bpp/8;
	unsigned char* newdata=(unsigned char*)malloc(x*y*bytes);;

	if(!newdata) error_stop("Bmp::scale : out of memory");

	int ofs=0;
	loopijk(0,0,0,y,x,bytes)
	{
		newdata[ofs]=
			data[(	(i*height/y)*width+ j*width/x ) *  bytes + k ];ofs++;
	}
	width=x;
	height=y;
	set(x,y,bpp,newdata);
	free(newdata);
}
//#################################################################//
void Bmp::set(int x,int y,int b,unsigned char*buffer)
{
	width=x;height=y;bpp=b;

	data.resize(width*height*(bpp/8));
	if(buffer)memmove(&data[0],buffer,width*height*(bpp/8));
}
//#################################################################//
void Bmp::load(const char *filename, bool convert32)
{
	ilImage i;

	if(!i.Load(filename))
	{
		error_stop("Bmp::load file %s not found\n",filename);
	}
	
	if(i.GetData()==0)
	{
		error_stop("Bmp::load 0 pointer\n");
	}
	
	if(i.Format()==IL_RGBA || convert32) i.Convert(IL_RGBA); else i.Convert(IL_RGB);

	printf("Bmp::loaded %s : %dx%dx%d \n",filename,i.Width(),i.Height(),i.Bpp());

	set(i.Width(),i.Height(),i.Bpp()*8,i.GetData());
}

//#################################################################//
void Bmp::save_float(const char*filename, float* fdata)
{
	if(fdata==0)fdata=(float*)&this->data[0];
	FILE* fn;
	if ((fn = fopen (filename,"wb")) == NULL)  error_stop("Bmp::save_float");
	fwrite(fdata,1,4*width*height,fn);
	fclose(fn);
}
//#################################################################//
bool Bmp::load_float(const char*filename, float* fdata)
{
	if (!fdata)fdata=(float*)&data[0];

	FILE* fn;
	if ((fn = fopen (filename,"rb")) == NULL) return false;// error_stop("Bmp::load_float");
	fread(fdata,1,4*width*height,fn);
	fclose(fn);
	return true;
}
//#################################################################//