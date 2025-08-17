#ifndef RFONT_H
#include "RFont.h"
#endif

#ifndef RFONT_GL1_HEADER
#define RFONT_GL1_HEADER

RFONT_API RFont_renderer RFont_gl1_renderer(void);

#endif

#ifdef RFONT_IMPLEMENTATION

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

typedef struct RFont_GL1_info{
	u32 width, height;
	float color[4];
} RFont_GL1_info;


#define RFONT_MULTILINE_STR(s) #s

void RFont_gl1_renderer_set_framebuffer(void* ctx, u32 w, u32 h) {
	((RFont_GL1_info*)ctx)->width = w;
	((RFont_GL1_info*)ctx)->height = h;
}

void RFont_gl1_renderer_set_color(void* ctx, float r, float g, float b, float a) {
	((RFont_GL1_info*)ctx)->color[0] = r;
	((RFont_GL1_info*)ctx)->color[1] = g;
	((RFont_GL1_info*)ctx)->color[2] = b;
	((RFont_GL1_info*)ctx)->color[3] = a;
}

RFont_texture RFont_gl1_create_atlas(void* ctx, u32 atlasWidth, u32 atlasHeight) {
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

b8 RFont_gl1_resize_atlas(void* ctx, RFont_texture* atlas, u32 newWidth, u32 newHeight) {
	static GLint swizzleRgbaParams[4] = {GL_ONE, GL_ONE, GL_ONE, GL_RED};
	GLuint newAtlas;
	RFONT_UNUSED(ctx);

	glGenTextures(1, &newAtlas);
	glBindTexture(GL_TEXTURE_2D, newAtlas);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (i32)newWidth, (i32)newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glBindTexture(GL_TEXTURE_2D, *atlas);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, (i32)newWidth - RFONT_ATLAS_RESIZE_LEN, (i32)newHeight);

	glDeleteTextures(1, (u32*)atlas);

	glBindTexture(GL_TEXTURE_2D, newAtlas);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* swizzle new atlas */
	glBindTexture(GL_TEXTURE_2D, newAtlas);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleRgbaParams);

	glBindTexture(GL_TEXTURE_2D, 0);

	*atlas = newAtlas;
	return 1;
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

void RFont_gl1_bitmap_to_atlas(void* ctx, RFont_texture atlas, u8* bitmap, float x, float y, float w, float h) {
	GLint alignment, rowLength, skipPixels, skipRows;
	RFONT_UNUSED(ctx);

	glEnable(GL_TEXTURE_2D);

	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);

	glBindTexture(GL_TEXTURE_2D, atlas);

	RFont_gl1_push_pixel_values(1, (i32)w, 0, 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, (i32)x, (i32)y, (i32)w, (i32)h, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	RFont_gl1_push_pixel_values(alignment, rowLength, skipPixels, skipRows);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void RFont_gl1_renderer_text(void* ctx, RFont_texture atlas, float* verts, float* tcoords, size_t nverts) {
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

	glBindTexture(GL_TEXTURE_2D, atlas);

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, ((RFont_GL1_info*)ctx)->width, ((RFont_GL1_info*)ctx)->height, 0, -1.0, 1.0);

	glBegin(GL_TRIANGLES);
	glColor4f(((RFont_GL1_info*)ctx)->color[0],
		   ((RFont_GL1_info*)ctx)->color[1],
		   ((RFont_GL1_info*)ctx)->color[2],
		   ((RFont_GL1_info*)ctx)->color[3]
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

void RFont_gl1_free_atlas(void* ctx, RFont_texture atlas) { glDeleteTextures(1, &atlas); RFONT_UNUSED(ctx); }
void RFont_gl1_renderer_init(void** ctx) {
	*ctx = (void*)RFONT_MALLOC(sizeof(RFont_GL1_info));
	RFont_gl1_renderer_set_color(*ctx, 0, 0, 0, 1);
}

void RFont_gl1_renderer_free(void** ctx) {
	RFONT_FREE(*ctx);
}

RFont_renderer RFont_gl1_renderer(void) {
	RFont_renderer renderer;

	renderer.init = RFont_gl1_renderer_init;
	renderer.create_atlas = RFont_gl1_create_atlas;
	renderer.free_atlas = RFont_gl1_free_atlas;
	renderer.bitmap_to_atlas = RFont_gl1_bitmap_to_atlas;
	renderer.render = RFont_gl1_renderer_text;
	renderer.set_color = RFont_gl1_renderer_set_color;
	renderer.set_framebuffer = RFont_gl1_renderer_set_framebuffer;
	renderer.free = RFont_gl1_renderer_free;

	return renderer;
}
#endif /*  RFONT_IMPLEMENTATION  */
