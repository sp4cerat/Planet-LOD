obj = unicode.o
bin = unicode

CC = gcc
CFLAGS = -pedantic -Wall -g -I../../src -I/usr/local/include
LDFLAGS = -L. -L/usr/local/lib -ldrawtext $(libgl)

fonts = serif.ttf cjk.ttf klingon.ttf

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -framework GLUT
	lib_so = libdrawtext.dylib
else
	libgl = -lGL -lGLU -lglut
	lib_so = libdrawtext.so.0.2
endif

$(bin): $(obj) $(lib_so) $(fonts)
	$(CC) -o $@ $(obj) $(LDFLAGS)

$(lib_so): ../../$(lib_so)
	rm -f $@
	ln -s $< $@

serif.ttf: ../fonts/linux-libertine.ttf
	rm -f $@
	ln -s $< $@

cjk.ttf: ../fonts/sazanami-mincho.ttf
	rm -f $@
	ln -s $< $@

klingon.ttf: ../fonts/klingon.ttf
	rm -f $@
	ln -s $< $@

.PHONY: clean
clean:
	rm -f $(obj) $(bin) $(lib_so)
