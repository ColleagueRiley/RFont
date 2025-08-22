#ifndef RFONT_H
#include "RFont.h"
#endif

#ifndef RFONT_GL1_HEADER
#define RFONT_GL1_HEADER

typedef struct RFont_GL1_info{
	u32 width, height;
	float color[4];
} RFont_GL1_info;

RFONT_API RFont_renderer_proc RFont_gl1_renderer_proc(void);

RFONT_API RFont_renderer* RFont_gl1_renderer_init(void);
RFONT_API void RFont_gl1_renderer_initPtr(void* ptr, RFont_renderer* renderer);

#endif

#ifdef RFONT_IMPLEMENTATION

RFont_renderer* RFont_gl1_renderer_init(void) { return RFont_renderer_init(RFont_gl1_renderer_proc()); }
void RFont_gl1_renderer_initPtr(void* ptr, RFont_renderer* renderer) { RFont_renderer_initPtr(RFont_gl1_renderer_proc(), ptr, renderer); }

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#ifndef GL_PERSPECTIVE_CORRECTION_HINT
#define GL_PERSPECTIVE_CORRECTION_HINT		0x0C50
#endif

#ifndef GL_TEXTURE_SWIZZLE_RGBA
#define GL_TEXTURE_SWIZZLE_RGBA           0x8E46
#endif

#ifndef GL_TEXTURE0
#define GL_TEXTURE0				0x84C0
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE			0x812F
#endif

#define RFONT_MULTILINE_STR(s) #s

void RFont_gl1_renderer_set_framebuffer(RFont_GL1_info* ctx, u32 w, u32 h) {
	ctx->width = w;
	ctx->height = h;
}

void RFont_gl1_renderer_set_color(RFont_GL1_info* ctx, float r, float g, float b, float a) {
	ctx->color[0] = r;
	ctx->color[1] = g;
	ctx->color[2] = b;
	ctx->color[3] = a;
}

RFont_texture RFont_gl1_create_atlas(RFont_GL1_info* ctx, u32 atlasWidth, u32 atlasHeight) {
	static GLint swizzleRgbaParams[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
	u32 id = 0;

	u8* data = (u8*)RFONT_MALLOC(atlasWidth * atlasHeight * 4);
	memset(data, 0, atlasWidth * atlasHeight * 4);

	RFONT_UNUSED(ctx);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &id);

	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (i32)atlasWidth, (i32)atlasHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	RFONT_FREE(data);

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleRgbaParams);

	glBindTexture(GL_TEXTURE_2D, 0);
	return id;
}

#ifndef GL_UNPACK_ROW_LENGTH
#define GL_UNPACK_ROW_LENGTH 0x0CF2
#define GL_UNPACK_SKIP_PIXELS 0x0CF4
#define GL_UNPACK_SKIP_ROWS 0x0CF3
#endif


void RFont_gl1_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows);
void RFont_gl1_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
}

void RFont_gl1_bitmap_to_atlas(RFont_GL1_info* ctx, RFont_texture atlas, u32 atlasWidth, u32 atlasHeight, u32 maxHeight, u8* bitmap, float w, float h, float* x, float* y) {
	GLint alignment, rowLength, skipPixels, skipRows;
	RFONT_UNUSED(ctx); RFONT_UNUSED(atlasHeight);
	if ((*x + w) >= atlasWidth) {
		*x = 0;
		*y += (float)maxHeight;
	}

	glEnable(GL_TEXTURE_2D);

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);

	glBindTexture(GL_TEXTURE_2D, (u32)atlas);

	RFont_gl1_push_pixel_values(1, (i32)w, 0, 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, (i32)(*x), (i32)*y, (i32)w, (i32)h, GL_RGBA, GL_UNSIGNED_BYTE, bitmap);

	RFont_gl1_push_pixel_values(alignment, rowLength, skipPixels, skipRows);

	glBindTexture(GL_TEXTURE_2D, 0);
	*x += w;
}

void RFont_gl1_renderer_text(RFont_GL1_info* ctx, RFont_texture atlas, float* verts, float* tcoords, size_t nverts) {
	size_t i;
	size_t tIndex = 0;

	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, (u32)atlas);

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, ctx->width, ctx->height, 0, -1.0, 1.0);

	glBegin(GL_TRIANGLES);
	glColor4f(ctx->color[0],
		   ctx->color[1],
		   ctx->color[2],
		   ctx->color[3]
		   );

	for (i = 0; i < (nverts * 3); i += 3) {
		glTexCoord2f(tcoords[tIndex], tcoords[tIndex + 1]);
		tIndex += 2;

		glVertex2f(verts[i], verts[i + 1]);
	}
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnable(GL_DEPTH_TEST);
}

void RFont_gl1_free_atlas(RFont_GL1_info* ctx, RFont_texture atlas) { glDeleteTextures(1, (u32*)&atlas); RFONT_UNUSED(ctx); }
void RFont_gl1_renderer_internal_initPtr(RFont_GL1_info* ctx) { RFont_gl1_renderer_set_color(ctx, 0, 0, 0, 1); }
void RFont_gl1_renderer_freePtr(RFont_GL1_info* ctx) { RFONT_UNUSED(ctx);}
size_t RFont_gl1_renderer_size(void) { return sizeof(RFont_GL1_info); }

RFont_renderer_proc RFont_gl1_renderer_proc(void) {
	RFont_renderer_proc proc;
	proc.initPtr = (void(*)(void*))RFont_gl1_renderer_internal_initPtr;
	proc.create_atlas = (RFont_texture (*)(void*, u32, u32))RFont_gl1_create_atlas;
	proc.free_atlas = (void (*)(void*, RFont_texture))RFont_gl1_free_atlas;
	proc.bitmap_to_atlas = (void(*)(void*, RFont_texture, u32, u32, u32, u8*, float, float, float*, float*))RFont_gl1_bitmap_to_atlas;
	proc.render = (void (*)(void*, RFont_texture, float*, float*, size_t))RFont_gl1_renderer_text;
	proc.set_color = (void (*)(void*, float, float, float, float))RFont_gl1_renderer_set_color;
	proc.set_framebuffer = (void (*)(void*, u32, u32))RFont_gl1_renderer_set_framebuffer;
	proc.freePtr = (void (*)(void*))RFont_gl1_renderer_freePtr;
	proc.size = (size_t (*)(void))RFont_gl1_renderer_size;

	return proc;
}
#endif /*  RFONT_IMPLEMENTATION  */
