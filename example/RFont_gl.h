#ifndef RFONT_H
#include "RFont.h"
#endif

#ifndef RFONT_GL_HEADER
#define RFONT_GL_HEADER

typedef struct RFont_mat4 {
    float m[16];
} RFont_mat4;

typedef struct RFont_GL_info{
	u32 vao, vbo, tbo, cbo, ebo, program, vShader, fShader;
	i32 matrixLoc;

	u32 width, height;
	float color[4];
	RFont_mat4 matrix;
	float colors[RFONT_INIT_VERTS * 2];
} RFont_GL_info;

RFONT_API RFont_renderer_proc RFont_gl_renderer_proc(void);

RFONT_API RFont_renderer* RFont_gl_renderer_init(void);
RFONT_API void RFont_gl_renderer_initPtr(void* ptr, RFont_renderer* renderer);

#endif

#ifdef RFONT_IMPLEMENTATION

RFont_renderer* RFont_gl_renderer_init(void) { return RFont_renderer_init(RFont_gl_renderer_proc()); }
void RFont_gl_renderer_initPtr(void* ptr, RFont_renderer* renderer) { RFont_renderer_initPtr(RFont_gl_renderer_proc(), ptr, renderer); }

#ifndef __APPLE__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif

#define GL_GLEXT_PROTOTYPES

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

RFont_mat4 RFont_ortho(float left, float right, float bottom, float top, float znear, float zfar);

RFont_texture RFont_gl_create_atlas(void* ctx, u32 atlasWidth, u32 atlasHeight) {
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


b8 RFont_gl_resize_atlas(void* ctx, RFont_texture* atlas, u32 newWidth, u32 newHeight) {
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

void RFont_gl_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows);
void RFont_gl_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
}


void RFont_gl_bitmap_to_atlas(void* ctx, RFont_texture atlas, u8* bitmap, float x, float y, float w, float h) {
	GLint alignment, rowLength, skipPixels, skipRows;
	RFONT_UNUSED(ctx);

	glEnable(GL_TEXTURE_2D);
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowLength);
	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skipPixels);
	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skipRows);

	glActiveTexture(GL_TEXTURE0 + atlas - 1);

	glBindTexture(GL_TEXTURE_2D, atlas);

	RFont_gl_push_pixel_values(1, (i32)w, 0, 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, (i32)x, (i32)y, (i32)w, (i32)h, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	RFont_gl_push_pixel_values(alignment, rowLength, skipPixels, skipRows);

	glBindTexture(GL_TEXTURE_2D, 0);
}

#define RFONT_MULTILINE_STR(s) #s

void RFont_gl_renderer_set_framebuffer(void* ctx, u32 w, u32 h) {
	((RFont_GL_info*)ctx)->width = w;
	((RFont_GL_info*)ctx)->height = h;
}

void RFont_gl_renderer_set_color(void* ctx, float r, float g, float b, float a) {
   ((RFont_GL_info*)ctx)->color[0] = r;
   ((RFont_GL_info*)ctx)->color[1] = g;
   ((RFont_GL_info*)ctx)->color[2] = b;
   ((RFont_GL_info*)ctx)->color[3] = a;
}

void RFont_gl_renderer_internal_initPtr(void* ctx) {
	static const char* defaultVShaderCode = RFONT_MULTILINE_STR(
		\x23version 330 core       \n
		layout (location = 0) in vec3 vertexPosition;
		layout (location = 1) in vec2 vertexTexCoord;
		layout (location = 2) in vec4 inColor;
		out vec2 fragTexCoord;
		out vec4 fragColor;

		uniform mat4 matrix;

		void main() {
			fragColor = inColor;
			gl_Position = matrix * vec4(vertexPosition, 1.0);
			fragTexCoord = vertexTexCoord;
		}
	);

	static const char* defaultFShaderCode = RFONT_MULTILINE_STR(
		\x23version 330 core                \n
		out vec4 FragColor;

		in vec4 fragColor;
		in vec2 fragTexCoord;

		uniform sampler2D texture0;

		void main() {
			FragColor = texture(texture0, fragTexCoord) * fragColor;
		}
	);

	RFont_gl_renderer_set_color(ctx, 0, 0, 0, 1);
	glGenVertexArrays(1, &((RFont_GL_info*)ctx)->vao);
	glBindVertexArray(((RFont_GL_info*)ctx)->vao);

	glGenBuffers(1, &((RFont_GL_info*)ctx)->vbo);
	glGenBuffers(1, &((RFont_GL_info*)ctx)->tbo);
	glGenBuffers(1, &((RFont_GL_info*)ctx)->cbo);
	glGenBuffers(1, &((RFont_GL_info*)ctx)->ebo);
	/* compile vertex shader */
	((RFont_GL_info*)ctx)->vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(((RFont_GL_info*)ctx)->vShader, 1, &defaultVShaderCode, NULL);
	glCompileShader(((RFont_GL_info*)ctx)->vShader);

	/* compile fragment shader */
	((RFont_GL_info*)ctx)->fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(((RFont_GL_info*)ctx)->fShader, 1, &defaultFShaderCode, NULL);
	glCompileShader(((RFont_GL_info*)ctx)->fShader);

	/* create program and link vertex and fragment shaders */
	((RFont_GL_info*)ctx)->program = glCreateProgram();

	glAttachShader(((RFont_GL_info*)ctx)->program, ((RFont_GL_info*)ctx)->vShader);
	glAttachShader(((RFont_GL_info*)ctx)->program, ((RFont_GL_info*)ctx)->fShader);

	glBindAttribLocation(((RFont_GL_info*)ctx)->program, 0, "vertexPosition");
	glBindAttribLocation(((RFont_GL_info*)ctx)->program, 1, "vertexTexCoord");
	glBindAttribLocation(((RFont_GL_info*)ctx)->program, 2, "inColor");

	glLinkProgram(((RFont_GL_info*)ctx)->program);

	glUseProgram(((RFont_GL_info*)ctx)->program);
	((RFont_GL_info*)ctx)->matrixLoc = glGetUniformLocation(((RFont_GL_info*)ctx)->program, "matrix");
	glUseProgram(0);
}

void RFont_gl_renderer_text(void* ctx, RFont_texture atlas, float* verts, float* tcoords, size_t nverts) {
	u32 i = 0;
	assert(ctx && nverts);

	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	glBindVertexArray(((RFont_GL_info*)ctx)->vao);

	glUseProgram(((RFont_GL_info*)ctx)->program);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, ((RFont_GL_info*)ctx)->vbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 3 * sizeof(float)), verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, ((RFont_GL_info*)ctx)->tbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 2 * sizeof(float)), tcoords, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	for (i = 0; i < (nverts * 4); i += 4)
		memcpy(&((RFont_GL_info*)ctx)->colors[i], ((RFont_GL_info*)ctx)->color, sizeof(float) * 4);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, ((RFont_GL_info*)ctx)->cbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 4 * sizeof(float)), ((RFont_GL_info*)ctx)->colors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, atlas);
	((RFont_GL_info*)ctx)->matrix = RFont_ortho(0, (float)((RFont_GL_info*)ctx)->width, (float)((RFont_GL_info*)ctx)->height, 0, -1.0, 1.0);

	glUniformMatrix4fv(((RFont_GL_info*)ctx)->matrixLoc, 1, GL_FALSE, ((RFont_GL_info*)ctx)->matrix.m);

	glDrawArrays(GL_TRIANGLES, 0, (i32)nverts);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void RFont_gl_free_atlas(void* ctx, RFont_texture atlas) { glDeleteTextures(1, &atlas); RFONT_UNUSED(ctx); }
void RFont_gl_renderer_freePtr(void* ctx) {
	if (((RFont_GL_info*)ctx)->vao != 0) {
		/* free vertex array */
		glDeleteVertexArrays(1, &((RFont_GL_info*)ctx)->vao);
		((RFont_GL_info*)ctx)->vao = 0;

		/* free buffers */
		glDeleteBuffers(1, &((RFont_GL_info*)ctx)->tbo);
		glDeleteBuffers(1, &((RFont_GL_info*)ctx)->vbo);

		/* free program data */
		glDeleteShader(((RFont_GL_info*)ctx)->vShader);
		glDeleteShader(((RFont_GL_info*)ctx)->fShader);
		glDeleteProgram(((RFont_GL_info*)ctx)->program);
	}
}

size_t RFont_gl_renderer_size(void) { return sizeof(RFont_GL_info); }

RFont_renderer_proc RFont_gl_renderer_proc(void) {
	RFont_renderer_proc proc;
	proc.initPtr = RFont_gl_renderer_internal_initPtr;
	proc.create_atlas = RFont_gl_create_atlas;
	proc.free_atlas = RFont_gl_free_atlas;
	proc.bitmap_to_atlas = RFont_gl_bitmap_to_atlas;
	proc.render = RFont_gl_renderer_text;
	proc.set_color = RFont_gl_renderer_set_color;
	proc.set_framebuffer = RFont_gl_renderer_set_framebuffer;
	proc.freePtr = RFont_gl_renderer_freePtr;
	proc.size = RFont_gl_renderer_size;

	return proc;
}

RFont_mat4 RFont_ortho(float left, float right, float bottom, float top, float znear, float zfar) {
	float matrix[16];
	float rl = right - left;
    float tb = top - bottom;
    float fn = zfar - znear;

    float matOrtho[16];
    RFont_mat4 result;
    int row, col, i;

	/* load identity */
	for (i = 0; i < 16; i++) {
        matrix[i] = 0.0f;
    }

    matrix[0] = 1.0f;
    matrix[5] = 1.0f;
    matrix[10] = 1.0f;
    matrix[15] = 1.0f;

	/* ortho */
    matOrtho[0]  = 2.0f / rl;
    matOrtho[1]  = 0.0f;
    matOrtho[2]  = 0.0f;
    matOrtho[3]  = 0.0f;

    matOrtho[4]  = 0.0f;
    matOrtho[5]  = 2.0f / tb;
    matOrtho[6]  = 0.0f;
    matOrtho[7]  = 0.0f;

    matOrtho[8]  = 0.0f;
    matOrtho[9]  = 0.0f;
    matOrtho[10] = -2.0f / fn;
    matOrtho[11] = 0.0f;

    matOrtho[12] = -(left + right) / rl;
    matOrtho[13] = -(top + bottom) / tb;
    matOrtho[14] = -(zfar + znear) / fn;
    matOrtho[15] = 1.0f;

	/* multiply */
    for (row = 0; row < 4; ++row) {
        for (col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (i = 0; i < 4; ++i) {
                sum += matrix[row * 4 + i] * matOrtho[i * 4 + col];
            }
            result.m[row * 4 + col] = sum;
        }
    }

    return result;
}

#endif /* RFONT_IMPLEMENTATION */
