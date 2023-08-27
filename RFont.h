/*
* Copyright (c) 2021-23 ColleagueRiley ColleagueRiley@gmail.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following r estrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/

/*
preprocessor args

make sure 

** #define RFONT_IMPLEMENTATION ** - include function defines

is in at least one of your files or arguments

#define RFONT_NO_OPENGL - do not define graphics functions (that use opengl)
#define RFONT_NO_STDIO - do not include stdio.h
#define RFONT_NO_GRAPHICS - do not include any graphics functions at all
#define RFONT_RENDER_RLGL - use rlgl functions for rendering
#define RFONT_RENDER_LEGACY - use opengl legacy functions for rendering (if rlgl is not chosen)
-- NOTE: By default, opengl 3.3 vbos are used for rendering --
*/


/*
TODO :
- make sure text is being loaded into the atlas correctly
- make sure texture coords are correct

- set up simple shader 

- support non rlgl rendering

- get text width
*/

#ifndef RFONT_NO_STDIO
#include <stdio.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <math.h>

#if !defined(u8)
    #include <stdint.h>

    typedef uint8_t     u8;
	typedef int8_t      i8;
	typedef uint16_t   u16;
	typedef int16_t    i16;
	typedef uint32_t   u32;
	typedef int32_t    i32;
	typedef uint64_t   u64;
	typedef int64_t    i64;
#endif

#ifndef RFONT_MAX_GLYPHS
#define RFONT_MAX_GLYPHS 256
#endif

#ifndef RFONT_ATLAS_WIDTH
#define RFONT_ATLAS_WIDTH 15000
#endif

#ifndef RFONT_ATLAS_HEIGHT
#define RFONT_ATLAS_HEIGHT 400
#endif

#ifndef RFONT_INIT_TEXT_SIZE
#define RFONT_INIT_TEXT_SIZE 500
#endif

#ifndef RFONT
#define RFONT

typedef struct RFont_font RFont_font;

typedef struct {
    char ch; /* the character (for checking) */
    size_t size;
    u32 x, x2, h; /* coords of the character on the texture */

    /* source glyph data */
    int src, x0, y0, x1, y1;
} RFont_glyph;

/* sets the framebuffer size AND runs the graphics init function */
inline void RFont_init(size_t width, size_t height);
/* just updates the framebuffer size */
inline void RFont_update_framebuffer(size_t width, size_t height);

#ifndef RFONT_NO_STDIO
inline RFont_font* RFont_font_init(char* font_name);
#endif

inline RFont_font* RFont_font_init_data(u8* font_data);

inline void RFont_font_free(RFont_font* font);

inline RFont_glyph RFont_font_add_char(RFont_font* font, char ch, size_t size);

inline void RFont_draw_text(RFont_font* font, const char* text, i32 x, i32 y, u32 size);
inline void RFont_draw_text_len(RFont_font* font, const char* text, size_t len, i32 x, i32 y, u32 size);

#ifndef RFONT_NO_GRAPHICS
/* 
    if you do not want to use opengl (or want to create your own implemntation of these functions), 
    you'll have to define these yourself 
    and add `#define RFONT_NO_OPENGL`
*/
inline void RFont_render_init(void);
inline u32 RFont_create_atlas(u32 atlasWidth, u32 atlasHeight);
inline void RFont_bitmap_to_atlas(u32 atlnitas, u8* bitmap, i32 x, i32 y, i32 w, i32 h);
inline void RFont_render_text(u32 atlas, float* verts, float* tcoords, size_t nverts);
inline void RFont_render_free(void);
#endif

#endif /* RFONT */

#ifdef RFONT_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define RFONT_GET_TEXPOSX(x) (float)((float)(x) / (float)(RFONT_ATLAS_WIDTH))
#define RFONT_GET_TEXPOSY(y) (float)((float)(y) / (float)(RFONT_ATLAS_HEIGHT))

#define RFONT_GET_WORLD_X(x, w) (float)((x) / (((w) / 2.0f)) - 1.0f)
#define RFONT_GET_WORLD_Y(y, h) (float)(1.0f - ((y) / ((h) / 2.0f)))

struct RFont_font {
    stbtt_fontinfo info; /* source stb font */
    int fheight; /* font height from stb */

    RFont_glyph glyphs[RFONT_MAX_GLYPHS]; /* glyphs */

    u32 atlas; /* atlas texture */
    i32 atlasX; /* the current x position inside the atlas */
};

size_t RFont_width = 0, RFont_height = 0;

void RFont_update_framebuffer(size_t width, size_t height) {
    /* set size of the framebuffer (for rendering later on) */
    RFont_width = width;
    RFont_height = height;
}

void RFont_init(size_t width, size_t height) {
    RFont_update_framebuffer(width, height);

    #ifndef RFONT_NO_GRAPHICS
    /* init any rendering stuff that needs to be initalized (eg. vbo objects) */
    RFont_render_init();
    #endif
}

#ifndef RFONT_NO_STDIO
char RFont_ttf_buffer[1 << 25];

RFont_font* RFont_font_init(char* font_name) {
    FILE* ttf_file = fopen(font_name, "rb");
    fread(RFont_ttf_buffer, 1, 1<<25, ttf_file);

    return RFont_font_init_data((u8*)RFont_ttf_buffer);
}
#endif

RFont_font* RFont_font_init_data(u8* font_data) {
    RFont_font* font = malloc(sizeof(RFont_font));
    
    stbtt_InitFont(&font->info, font_data, 0);

    font->fheight = ttSHORT(font->info.data + font->info.hhea + 4) - ttSHORT(font->info.data + font->info.hhea + 6);

    #ifndef RFONT_NO_GRAPHICS
    font->atlas = RFont_create_atlas(RFONT_ATLAS_WIDTH, RFONT_ATLAS_HEIGHT);
    #endif
    font->atlasX = 0;
    
    return font;
}

void RFont_font_free(RFont_font* font) {
    #ifndef RFONT_NO_GRAPHICS
    glDeleteTextures(1, &font->atlas);
    RFont_render_free();
    #endif

    free(font);
}

RFont_glyph RFont_font_add_char(RFont_font* font, char ch, size_t size) {
    i32 w, h;
    
    const i32 i = ch - ' ';

    if (font->glyphs[i].ch == ch && font->glyphs[i].size == size)
        return font->glyphs[i];

    float height = stbtt_ScaleForPixelHeight(&font->info, size);
    u8* bitmap = stbtt_GetCodepointBitmap(&font->info, 0, height, ch, &w, &h, 0,0);

    font->glyphs[i].ch = ch;
    font->glyphs[i].x = font->atlasX;
    font->glyphs[i].x2 = font->atlasX + w;
    font->glyphs[i].h = h;
    font->glyphs->src = stbtt_FindGlyphIndex(&font->info, ch);
    font->glyphs[i].size = size;

    stbtt_GetGlyphBox(&font->info, font->glyphs->src, &font->glyphs[i].x0, &font->glyphs[i].y0, &font->glyphs[i].x1, &font->glyphs[i].y1);


    #ifndef RFONT_NO_GRAPHICS
    RFont_bitmap_to_atlas(font->atlas, bitmap, font->atlasX, 0, w, h);
    #endif

    font->atlasX += w;

    free(bitmap);

    return font->glyphs[i];
}

void RFont_draw_text(RFont_font* font, const char* text, i32 x, i32 y, u32 size) {
    RFont_draw_text_len(font, text, 0, x, y, size);
}

void RFont_draw_text_len(RFont_font* font, const char* text, size_t len, i32 x, i32 y, u32 size) {
    static float verts[1024 * 2];
    static float tcoords[1024 * 2];

    i32 startX = x,
        i = 0;
    
    char* str;

    for (str = (char*)text; (len == 0 || (str - text) < len) && *str; str++) {        
        if (*str == '\n') { 
            x = startX;
            y += size;
            continue;
        }

        float scale = (float)size / font->fheight;

        /* 
        I would like to use this method, 
        it makes it so I only have to load each glyph once 
        instead of once per size
        however, it has some issues so I've decided not to use it for now

        RFont_glyph glyph = RFont_font_add_char(font, *str, RFONT_ATLAS_HEIGHT);
        */

        RFont_glyph glyph = RFont_font_add_char(font, *str, size);

		int ix0 = STBTT_ifloor(glyph.x0 * scale);
		int ix1 = STBTT_iceil(glyph.x1 * scale);

        int iy0 = STBTT_ifloor(-glyph.y1 * scale + 0);
        int iy1 = STBTT_iceil(-glyph.y0 * scale + 0);

		int w = (ix1 - ix0);
        int h = (iy1 - iy0);

	    int realY = y + iy0;

        switch (*str) {
            case '_':
                x += (w / 5);
                realY -= (size / 6);
                break;
            case '(':
            case ')':
            case ';':
                realY -= (size / 16);
                break;
            default: break;
        }

        x += ix0;

        verts[i] = RFONT_GET_WORLD_X(x, RFont_width); 
        verts[i + 1] = RFONT_GET_WORLD_Y(realY, RFont_height);
        /*  */
        verts[i + 2] = RFONT_GET_WORLD_X(x, RFont_width);
        verts[i + 3] = RFONT_GET_WORLD_Y(realY + h, RFont_height);
        /*  */
        verts[i + 4] = RFONT_GET_WORLD_X(x + w, RFont_width);
        verts[i + 5] = RFONT_GET_WORLD_Y(realY + h, RFont_height);
        /*  */
        /*  */
        verts[i + 6] = RFONT_GET_WORLD_X(x + w, RFont_width);
        verts[i + 7] = RFONT_GET_WORLD_Y(realY, RFont_height);
        /*  */
        verts[i + 8] = RFONT_GET_WORLD_X(x, RFont_width); 
        verts[i + 9] = RFONT_GET_WORLD_Y(realY, RFont_height);
        /*  */
        verts[i + 10] = RFONT_GET_WORLD_X(x + w, RFont_width);
        verts[i + 11] = RFONT_GET_WORLD_Y(realY + h, RFont_height);

        /* texture coords */

        tcoords[i] = RFONT_GET_TEXPOSX(glyph.x);
        tcoords[i + 1] = 0;
        /*  */
        tcoords[i + 2] = RFONT_GET_TEXPOSX(glyph.x); 
        tcoords[i + 3] = RFONT_GET_TEXPOSY(glyph.h);
        /*  */
        tcoords[i + 4] = RFONT_GET_TEXPOSX(glyph.x2);
        tcoords[i + 5] = RFONT_GET_TEXPOSY(glyph.h);
        /*  */
        /*  */
        tcoords[i + 6] = RFONT_GET_TEXPOSX(glyph.x2);
        tcoords[i + 7] = 0;
        /*  */
        tcoords[i + 8] = RFONT_GET_TEXPOSX(glyph.x);
        tcoords[i + 9] = 0;
        /*  */
        tcoords[i + 10] = RFONT_GET_TEXPOSX(glyph.x2);
        tcoords[i + 11] = RFONT_GET_TEXPOSY(glyph.h);

        i += 12;
        x += w + 1;
    }
    
    #ifndef RFONT_NO_GRAPHICS
    RFont_render_text(font->atlas, verts, tcoords, i / 2);
    #endif
}

#if !defined(RFONT_NO_OPENGL) && !defined(RFONT_NO_GRAPHICS)
#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#ifndef GL_TEXTURE_SWIZZLE_RGBA
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46
#endif

#ifdef RFONT_DEBUG
#ifdef RFONT_RENDER_LEGACY
GLAPI void APIENTRY glDebugMessageCallback (GLDEBUGPROC callback, const void *userParam);
#endif

void RFont_debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    printf("OpenGL Debug Message: %s\n", message);
}
#endif

u32 RFont_create_atlas(u32 atlasWidth, u32 atlasHeight) {
    #ifdef RFONT_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // For synchronous output
    glDebugMessageCallback(RFont_debugCallback, 0); // Set the callback function
    #endif

    u32 id = 0;
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH, atlasWidth);
    
    u8* data = calloc(sizeof(u8), atlasWidth * atlasHeight * 4);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlasWidth, atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    free(data);
	
    glBindTexture(GL_TEXTURE_2D, id);
	static GLint swizzleRgbaParams[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleRgbaParams);

    glBindTexture(GL_TEXTURE_2D, 0);
    return id;
}


void RFont_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows);
void RFont_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
}

void RFont_bitmap_to_atlas(u32 atlas, u8* bitmap, i32 x, i32 y, i32 w, i32 h) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

	GLint alignment, rowLength, skipPixels, skipRows;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);
    
	glBindTexture(GL_TEXTURE_2D, atlas);

	RFont_push_pixel_values(1, w, 0, 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	RFont_push_pixel_values(alignment, rowLength, skipPixels, skipRows);

    glBindTexture(GL_TEXTURE_2D, 0);
}

#ifdef RFONT_RENDER_RLGL
void RFont_render_text(u32 atlas, float* verts, float* tcoords, size_t nverts) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
	rlSetTexture(atlas);

	glEnable(GL_BLEND);

	rlPushMatrix();

	rlBegin(GL_QUADS);

	i32 i, j = 0;
	for (i = 0; i < (nverts * 6); i += 2) {
		rlTexCoord2f(tcoords[i], tcoords[i + 1]);

		if (j++ && j == 2 && (j -= 3))
			rlVertex2f(verts[i], verts[i + 1]);

		rlVertex2f(verts[i], verts[i + 1]);
	}
	rlEnd();
	rlPopMatrix();

	rlSetTexture(0);
}

void RFont_render_free(void) {}
void RFont_render_init() {}
#endif /* RFONT_RENDER_RLGL */

#if defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
void RFont_render_text(u32 atlas, float* verts, float* tcoords, size_t nverts) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, atlas);

	glPushMatrix();

	glBegin(GL_TRIANGLES);
    
	i32 i;
	for (i = 0; i < (nverts * 2); i += 2) {
		glTexCoord2f(tcoords[i], tcoords[i + 1]);
		
        glVertex2f(verts[i], verts[i + 1]);
	}
	glEnd();
	glPopMatrix();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void RFont_render_free() {}
void RFont_render_init() {}
#endif /* defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)  */

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
typedef struct {
    u32 vao, 
        verties, 
        tcoords, 
        program, 
        vShader, 
        fShader;
} RFont_gl_info;

RFont_gl_info RFont_gl;

#ifdef RFONT_DEBUG
inline void RFont_debug_linkage(u32 src, const char* shader, const char* action);
void RFont_debug_linkage(u32 src, const char* shader, const char* action) {
    GLint status;
    if (action[0] == 'l')
        glGetProgramiv(src, GL_LINK_STATUS, &status);
    else
        glGetShaderiv(src, GL_COMPILE_STATUS, &status);

    if (status == GL_TRUE) 
        printf("%s Shader %s successfully.\n", shader, action);
    else {
        printf("%s Shader failed to %s.\n", shader, action);

        GLint infoLogLength;
        glGetShaderiv(src, GL_INFO_LOG_LENGTH, &infoLogLength);

        if (infoLogLength > 0) {
            GLchar* infoLog = (GLchar*)malloc(infoLogLength);
            glGetShaderInfoLog(src, infoLogLength, NULL, infoLog);
            printf("%s Shader info log:\n%s\n", shader, infoLog);
            free(infoLog);
        }
        
        exit(1);
    }
}
#endif

void RFont_render_init() {
    static const char* defaultVShaderCode =
        "#version 330                       \n"
        "layout(location = 0) in vec2 vertexPosition;            \n"
        "layout(location = 1) in vec2 vertexTexCoord;            \n"
        "out vec2 fragTexCoord;             \n"
        "out vec4 fragColor;                \n"

        "uniform mat4 mvp;                  \n"
        "void main() {                      \n"
        "    fragTexCoord = vertexTexCoord; \n"
        "    fragColor = vec4(1.0, 1.0, 1.0, 1.0);      \n"
        "    gl_Position = vec4(vertexPosition, 0.0, 1.0);              \n"
        "}                                              \n";

    static const char* defaultFShaderCode =
        "#version 330                       \n"
        "in vec2 fragTexCoord;              \n"
        "in vec4 fragColor;                 \n"
        "out vec4 finalColor;               \n"
        "uniform sampler2D texture0;        \n"
        "void main() {                      \n"
        "    vec4 texelColor = texture(texture0, fragTexCoord);     \n"
        "    finalColor = texelColor * fragColor;                   \n"
        "}                                                          \n";

	glGenVertexArrays(1, &RFont_gl.vao);

	glBindVertexArray(RFont_gl.vao);

    glGenBuffers(1, &RFont_gl.verties);
	glGenBuffers(1, &RFont_gl.tcoords);
    
    /* compile vertex shader */
    RFont_gl.vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(RFont_gl.vShader, 1, &defaultVShaderCode, NULL);
    glCompileShader(RFont_gl.vShader);
    
    #ifdef RFONT_DEBUG
    RFont_debug_linkage(RFont_gl.vShader, "Vertex", "compile");
    #endif

    /* compile fragment shader */
    RFont_gl.fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(RFont_gl.fShader, 1, &defaultFShaderCode, NULL);
    glCompileShader(RFont_gl.fShader);

    
    #ifdef RFONT_DEBUG
    RFont_debug_linkage(RFont_gl.fShader, "Fragment", "compile");
    #endif
    
    /* create program and link vertex and fragment shaders */
    RFont_gl.program = glCreateProgram();
    glAttachShader(RFont_gl.program, RFont_gl.vShader);
    
    #ifdef RFONT_DEBUG
    RFont_debug_linkage(RFont_gl.program, "Vertex", "link to the program");
    #endif

    glAttachShader(RFont_gl.program, RFont_gl.fShader);
    
    #ifdef RFONT_DEBUG
    RFont_debug_linkage(RFont_gl.program, "Fragment", "link to the program");
    #endif

    glLinkProgram(RFont_gl.program);
}

void RFont_render_text(u32 atlas, float* verts, float* tcoords, size_t nverts) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glEnable(GL_TEXTURE_2D);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glEnable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas);

	glBindVertexArray(RFont_gl.vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, RFont_gl.verties);
	glBufferData(GL_ARRAY_BUFFER, nverts * 2 * sizeof(float), verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, RFont_gl.tcoords);
	glBufferData(GL_ARRAY_BUFFER, nverts * 2 * sizeof(float), tcoords, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glUseProgram(RFont_gl.program);
	glDrawArrays(GL_TRIANGLES, 0, nverts);
    glUseProgram(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glBindVertexArray(0);
}

void RFont_render_free() {
    if (RFont_gl.vao == 0)
        return;
    
    /* free vertex array */
    glDeleteVertexArrays(1, &RFont_gl.vao);

    /* free buffers */
    glDeleteBuffers(1, &RFont_gl.tcoords);
    glDeleteBuffers(1, &RFont_gl.verties);

    /* free program data */
    glDeleteShader(RFont_gl.vShader);
    glDeleteShader(RFont_gl.fShader);
    glDeleteProgram(RFont_gl.program);
}

#endif /* !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL) */
#endif /*  !defined(RFONT_NO_OPENGL) && !defined(RFONT_NO_GRAPHICS) */

#endif /* RFONT_IMPLEMENTATION */