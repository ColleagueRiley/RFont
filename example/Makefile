CC = gcc

LIBS := -lshell32 -lgdi32 -lm -lopengl32 --static
EXT = .exe

ifneq (,$(filter $(CC),winegcc x86_64-w64-mingw32-gcc))
    detected_OS := Windows
else
	ifeq '$(findstring ;,$(PATH))' ';'
		detected_OS := Windows
	else
		detected_OS := $(shell uname 2>/dev/null || echo Unknown)
		detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
		detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
		detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
	endif
endif

ifeq ($(detected_OS),Windows)
	LIBS := -lshell32 -lgdi32 -lm -lopengl32 --static
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -I./ext/Silicon/ -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo -w
	EXT = 
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lX11 -lGLX -lm -lGL
	EXT =
endif

all:
	make gl
	make gl3
	make rgl

gl:
	$(CC) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall -O3 -o gl
rgl:
	$(CC) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_RGL -Wall -O3 -o rgl
gl3:
	$(CC) main.c $(LIBS) -O3 -I./ext -I../ -Wall -o gl3

clean:
	rm gl3$(EXT) gl$(EXT) rgl$(EXT) gl3.exe.so gl.exe.so rgl.exe.so -f

debug:
	make clean

	$(CC) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall -o gl -D RFONT_DEBUG
	$(CC) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_RGL -Wall -o rgl -D RFONT_DEBUG
	$(CC) main.c $(LIBS) -I./ext -I../ -Wall -o gl3 -D RFONT_DEBUG

	./gl$(EXT) gl
	./gl3$(EXT) gl3
	./rgl$(EXT) rgl