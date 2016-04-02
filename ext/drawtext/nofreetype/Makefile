obj = simple-noft.o
bin = simple-noft

CC = gcc
CFLAGS = -pedantic -Wall -g -I../../src -I/usr/local/include
LDFLAGS = -Wl,-rpath=. $(lib_so) $(libgl)

font = serif.ttf

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT
	lib_so = libdrawtext-noft.dylib
else
	libgl = -lGL -lGLU -lglut
	lib_so = libdrawtext-noft.so.0.2
	lib_soname = libdrawtext-noft.so.0
endif

$(bin): $(obj) $(lib_so) $(lib_soname)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(lib_so): ../../$(lib_so)
	rm -f $@
	ln -s $< $@

$(lib_soname): ../../$(lib_so)
	rm -f $@
	ln -s $< $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin) $(lib_so)
