CC = gcc

LIBS := -lshell32 -lXInput -lwinmm -lgdi32 -lm -lopengl32 -ggdb -lShcore
EXT = .exe

FLAGS = -Wall -Werror -Wstrict-prototypes -Wextra

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
	LIBS := -lm -framework Foundation -framework AppKit -framework OpenGL -framework CoreVideo -w
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

all:
	make gl
	make gl3
	make rgl
	make es3

gl:
	$(CC) $(FLAGS) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall -O3 -o gl$(EXT)
rgl:
	$(CC) $(FLAGS) main.c $(LIBS) -I./ext -I../ -D RFONT_RENDER_RGL -Wall -O3 -o rgl$(EXT)
gl3:
	$(CC) $(FLAGS) main.c $(LIBS) -O3 -I./ext -I../ -Wall -o gl3$(EXT)
es3:
ifeq ($(GLES3),1)
	$(CC) $(FLAGS) main.c $(LIBS) -D RFONT_RENDER_ES3 -lEGL -O3 -I./ext -I../ -Wall -o es3$(EXT)
endif
clean:
	rm gl3$(EXT) es3$(EXT) gl$(EXT) rgl$(EXT) gl3.exe.so es3.exe.so gl.exe.so rgl.exe.so -f

debug:
	make clean

	$(CC) main.c $(FLAGS) $(LIBS) -I./ext -I../ -D RFONT_RENDER_LEGACY -Wall -o gl -D RFONT_DEBUG
	$(CC) main.c $(FLAGS) $(LIBS) -I./ext -I../ -D RFONT_RENDER_RGL -Wall -o rgl -D RFONT_DEBUG
	$(CC) main.c $(FLAGS) $(LIBS) -I./ext -I../ -Wall -o gl3 -D RFONT_DEBUG

ifeq ($(GLES3),1)
	$(CC) main.c $(FLAGS) $(LIBS) -lEGL -I./ext -I../ -Wall -D RFONT_RENDER_ES3 -o es3 -D RFONT_DEBUG
endif

	./gl$(EXT) gl
	./gl3$(EXT) gl3

ifeq ($(GLES3),1)
	./es3$(EXT) es3
endif
	./rgl$(EXT) rgl
