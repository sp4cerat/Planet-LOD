///////////////////////////////////////////
#include "core.h"
///////////////////////////////////////////
float cubicInterpolate (float p[4], float x) 
{
	return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}
///////////////////////////////////////////
float bicubicInterpolate (float p[4][4], float x, float y)
{
	float arr[4];
	arr[0] = cubicInterpolate(p[0], y);
	arr[1] = cubicInterpolate(p[1], y);
	arr[2] = cubicInterpolate(p[2], y);
	arr[3] = cubicInterpolate(p[3], y);
	return cubicInterpolate(arr, x);
}
///////////////////////////////////////////
std::string int_to_str(const int x)
{
	static char int_to_str_out[100];
	sprintf(int_to_str_out,"%d",x);
	return std::string(int_to_str_out);
};
///////////////////////////////////////////
std::string get_pure_filename ( std::string filename )
{
	uint pos1 = filename.find_last_of( "/" );
	uint pos2 = filename.find_last_of( "\\" );
	uint pos3 = filename.find_last_of( "." );

	if ( pos1 ==  std::string::npos ) pos1 = pos2;
	if ( pos1 ==  std::string::npos ) pos1 = 0;
	if ( pos1 < filename.size())
	if ( pos1 != 0 )pos1++;

	if ( pos3 == std::string::npos ) 
	{
		pos3 = filename.size();
	}		
	//printf( "input %s substr = %s\n" ,filename.c_str(), filename.substr(pos1,pos3-pos1).c_str());
	return (filename.substr(pos1,pos3-pos1));
}
///////////////////////////////////////////
std::string get_path ( std::string filename )
{
	uint pos1 = filename.find_last_of( "/" );
	uint pos2 = filename.find_last_of( "\\" );

	if ( pos1 ==  std::string::npos ) pos1 = pos2;
	if ( pos1 ==  std::string::npos ) return "./";

	if (pos1 < filename.size())
	if (pos1 != 0)
		pos1++;
	//printf( "substr = %s\n" , filename.substr(0,pos1).c_str());
	return (filename.substr(0,pos1));
}
///////////////////////////////////////////
char* str( const char* format, ... ) 
{
	static char txt[50000];
    va_list args;
    va_start( args, format );
    vsprintf( txt, format, args );
    va_end( args );
	return &txt[0];
}
///////////////////////////////////////////
bool file_exists(char* fn)
{
	FILE* f=fopen(fn,"rb");
	if(f) fclose(f);
	return f ? 1 : 0;
}
///////////////////////////////////////////
#ifdef WIN32
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
 #endif

std::string get_current_dir()
{
	char cCurrentPath[10000]={0};
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) return ".";
	//char* s=cCurrentPath;
	//while(s[0]!=0){if((*s)=='\\')*s='/';s++;}
	return cCurrentPath;
}
///////////////////////////////////////////


#ifdef WIN32
#include "dirent_win.h"
#else
#include "dirent.h"
#endif

bool strcmp_lower(char* a,char* b)
{
	char *a0=a;
	while( (*a)!=0 && (*b)!=0 && tolower(*a) == tolower(*b) ) {a++;b++;}
	return ( (a!=a0) && ((*a)==0 || (*b)==0) && tolower(*(a-1)) == tolower(*(b-1))) ? true : false;
}
bool isfileextension(char* fname,char* extension)
{
	if(!fname) return 0;
	if(!extension || extension[0]==0) return 1;

	int len1=strlen(fname);
	int len2=strlen(extension);

	if(len1<len2) return 0;

	ifeq_lower((&fname[len1-len2]),extension) return 1; else return 0;
};
long get_file_size(std::string filename) // path to file
{
    FILE *p_file = fopen(filename.c_str(),"rb");
	if(!p_file)return 0;
    fseek(p_file,0,SEEK_END);
    long size = ftell(p_file);
    fclose(p_file);
    return size;
}
void file_get_dir(const char* dirname,
			 std::vector<std::string>	&listdirs,
			 std::vector<std::string>	&listfiles,
			 std::vector<long>			&listfilesize,
			 char* extension)
{
	DIR *dir = opendir (dirname);//".\\"
	struct dirent *ent;

	if (dir != NULL) 
	{
	  while ((ent = readdir (dir)) != NULL) 
	  {
		  //printf ("%d type %s\n", ent->d_type>>15, ent->d_name );
		  if(ent->d_type>>15)
		  {
			  if( isfileextension(ent->d_name,extension))
			  {
				listfiles.push_back(ent->d_name);
				listfilesize.push_back(get_file_size(str("%s/%s",dirname,ent->d_name)));
			  }
		  }	
		  else if(strcmp(ent->d_name,".")!=0) listdirs.push_back(ent->d_name);

	  }
	  closedir (dir);
	} 
}
////////////////////////
FILE* fopenx( const char *name,const char *mode )
{
	FILE* f=fopen(name,mode);
	if(f==0){error_stop("cannot open %s [%s]\n",name,mode);}
	return f;
}
///////////////////////////
int splitstr(char* s,char** sp1,char** sp2,char** sp3)
{
	char *s1,*s2,*s3;
	if(s==0) return 0;
	if(s[strlen(s)-1]==10)s[strlen(s)-1]=0;
	if(s[strlen(s)-1]==13)s[strlen(s)-1]=0;
	s1 = s;
	*sp1=s1;

	s2 = strchr(s1,' ');	
	*sp2=s2;
	if(s2==0) return 1;	
	s2++;
	s3 = strchr(s2,' ');	
	*sp2=s2;
	if(s3==0) return 2;
	s3++;*(s2-1)=*(s3-1)=0;

	*sp1=s1;
	*sp2=s2;
	*sp3=s3;
	return 3;
};
////////////////////////////