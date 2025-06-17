CC = gcc

LIBS := -lshell32 -lXInput -lwinmm -lgdi32 -lm -lopengl32 -ggdb -lShcore
EXT = .exe


WARNINGS = -Werror -Wall -Wextra -Wpedantic -Wstrict-prototypes -Wold-style-definition -Wconversion -Wsign-conversion -Wshadow -Wpointer-arith -Wvla -Wcast-align -Wstrict-overflow -Wnested-externs -Wstrict-aliasing -Wredundant-decls -Winit-self -Wmissing-noreturn 

FLAGS = $(WARNINGS) -std=c89 
GLES3=0

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
	GLES3 = 1
endif
ifeq ($(CC),emcc)
	EXPORTED_JS = -s EXPORTED_RUNTIME_METHODS="['stringToNewUTF8']"
	GET_FILES =  --preload-file DejaVuSans.ttf@DejaVuSans.ttf \
				--preload-file DroidSansJapanese.ttf@DroidSansJapanese.ttf
	LIBS = $(GET_FILES) -s WASM=1 -s ASYNCIFY -s GL_SUPPORT_EXPLICIT_SWAP_CONTROL=1 -s FULL_ES3 -s USE_WEBGL2 $(EXPORTED_JS)
	EXT = .js
	GLES3=1
endif

all: gl gl3 es3

RGFW.o: ./ext/RGFW.h ./ext/RGFW.c
	$(CC) -c ext/RGFW.c 

RGFW_egl.o: ./ext/RGFW.h ./ext/RGFW.c
ifeq ($(GLES3),1)
	$(CC) -c ext/RGFW.c -D RFONT_RENDER_ES3 -o RGFW_egl.o
else
	echo gles is disabled 
endif

gl: ../RFont.h RGFW.o
	$(CC) $(FLAGS) main.c RGFW.o $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall -O3 -o gl$(EXT)
gl3: ../RFont.h RGFW.o  
	$(CC) $(FLAGS) main.c RGFW.o $(LIBS) -O3 -I./ext -I../ -Wall -o gl3$(EXT)
es3: ../RFont.h RGFW_egl.o  
ifeq ($(GLES3),1)
	$(CC) $(FLAGS) main.c RGFW_egl.o $(LIBS) -D RFONT_RENDER_ES3 -lEGL -O3 -I./ext -I../ -Wall -o es3$(EXT)
else
	echo gles is disabled 
endif

clean: 
	rm gl3$(EXT) es3$(EXT) gl$(EXT) gl3.exe.so es3.exe.so gl.exe.so *.o -f

debug: gl gl3 es3 
	./gl$(EXT) gl
	./gl3$(EXT) gl3

ifeq ($(GLES3),1)
	./es3$(EXT) es3
endif
