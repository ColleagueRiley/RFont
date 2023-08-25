ifeq '$(findstring ;,$(PATH))' ';'
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(detected_OS),Windows)
	LIBS := -lopengl32 -lshell32 -lgdi32 -lm
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := ./ext/Silicon/*.o -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lX11 -lGLX -lm -lGL
endif

all:
	@if [ $(shell uname) = Darwin ]; then\
		make deps/Silicon/source/mac.o;\
	fi

	make gl
	make gl3
	make rlgl

deps/Silicon/mac.o:
	cd ext/Silicon/ && gcc -c source/*.m

gl:
	gcc main.c $(LIBS) -I./ext -I../ $(LIBS) -D RFONT_RENDER_LEGACY -o gl
rlgl:
	gcc main.c $(LIBS) -I./ext -I../ $(LIBS) -D RFONT_RENDER_RLGL -o rlgl
gl3:
	gcc main.c $(LIBS) -I./ext -I../ $(LIBS) -o gl3

clean:
	rm gl3 gl rlgl -f

debug:
	make clean
	make
	./gl gl &
	./gl3 gl3 &
	./rlgl rlgl