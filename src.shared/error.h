#pragma once
#include "stackwalker/StackWalker.h"
//extern HWND WINDOW_HWND; 
#define error_stop(fmt, ...) \
{\
	StackWalker sw; \
	char* callstack=sw.ShowCallstack(); \
	char text[10000];\
	sprintf(text,("" fmt "\n\nCallstack:\n%s [%s:%d]\n%s"),\
	##__VA_ARGS__,__FUNCTION__,__FILE__,  __LINE__, callstack);\
	printf(text);\
	MessageBoxA(0,text,"Error",0); ::exit(0);\
	while(1);;\
};
/*
void error_check_mem(void* p,int size)
{
	bool pass=false;
	try
	{
		memcmp(p, p, size);
		pass=true;
	}
	catch(int e) 
	{
		if(!pass) error_stop("pointer check failed (error %d)",e);
	};
};
*/
/*
#define error_check_mem(_p,_size)\
{\
	bool _pass=false;\
	try{\
		memcmp(_p, _p, _size);\
		_pass=true;\
	}catch(int e)\
	{\
		if(!_pass) error_stop("pointer check failed (error %d)",e);\
	}\
}
*/
