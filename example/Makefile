CC = gcc

LIBS := -lshell32 -lXInput -lwinmm -lgdi32 -lm -lopengl32 -ggdb -lShcore
EXT = .exe


WARNINGS = -Werror -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wold-style-definition -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn

FLAGS = $(WARNINGS) -std=c89

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
	LIBS := -lXInput -ggdb -lwinmm -lshell32 -lgdi32 -lShcore -lm -ldwmapi -lopengl32 --static
endif
ifeq ($(detected_OS),Darwin)        # Mac OS X
	LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo -framework IOKit -w
	EXT =
endif
ifeq ($(detected_OS),Linux)
    LIBS := -I./include -lXrandr -lX11 -lm -lGL
	EXT =
endif

all: gl gl3

RGFW.o: ./ext/RGFW.h ./ext/RGFW.c
	$(CC) -c ext/RGFW.c

gl: ../RFont.h RFont_gl1.h main.c RGFW.o
	$(CC) -g3 $(FLAGS) main.c RGFW.o $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall  -o gl$(EXT)
gl3: ../RFont.h  RFont_gl.h main.c RGFW.o
	$(CC) -g3 $(FLAGS) main.c RGFW.o $(LIBS) -I./ext -I../ -Wall -o gl3$(EXT)

clean:
	rm gl3$(EXT) gl$(EXT) gl3.exe.so gl.exe.so *.o -f

debug: gl gl3
	./gl$(EXT) gl
	./gl3$(EXT) gl3
