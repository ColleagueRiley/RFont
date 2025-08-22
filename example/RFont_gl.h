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

RFont_texture RFont_gl_create_atlas(RFont_GL_info* ctx, u32 atlasWidth, u32 atlasHeight) {
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

void RFont_gl_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows);
void RFont_gl_push_pixel_values(GLint alignment, GLint rowLength, GLint skipPixels, GLint skipRows) {
	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skipPixels);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, skipRows);
}

void RFont_gl_bitmap_to_atlas(RFont_GL_info* ctx, RFont_texture atlas, u32 atlasWidth, u32 atlasHeight, u32 maxHeight, u8* bitmap, float w, float h, float* x, float* y) {
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

	glActiveTexture(GL_TEXTURE0 + (u32)atlas - 1);
	glBindTexture(GL_TEXTURE_2D, (u32)atlas);

	RFont_gl_push_pixel_values(1, (i32)w, 0, 0);

	glTexSubImage2D(GL_TEXTURE_2D, 0, (i32)(*x), (i32)*y, (i32)w, (i32)h, GL_RED, GL_UNSIGNED_BYTE, bitmap);

	RFont_gl_push_pixel_values(alignment, rowLength, skipPixels, skipRows);

	glBindTexture(GL_TEXTURE_2D, 0);
	*x += w;
}

#define RFONT_MULTILINE_STR(s) #s

void RFont_gl_renderer_set_framebuffer(RFont_GL_info* ctx, u32 w, u32 h) {
	ctx->width = w;
	ctx->height = h;
}

void RFont_gl_renderer_set_color(RFont_GL_info* ctx, float r, float g, float b, float a) {
   ctx->color[0] = r;
   ctx->color[1] = g;
   ctx->color[2] = b;
   ctx->color[3] = a;
}

void RFont_gl_renderer_internal_initPtr(RFont_GL_info* ctx) {
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
	glGenVertexArrays(1, &ctx->vao);
	glBindVertexArray(ctx->vao);

	glGenBuffers(1, &ctx->vbo);
	glGenBuffers(1, &ctx->tbo);
	glGenBuffers(1, &ctx->cbo);
	glGenBuffers(1, &ctx->ebo);
	/* compile vertex shader */
	ctx->vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(ctx->vShader, 1, &defaultVShaderCode, NULL);
	glCompileShader(ctx->vShader);

	/* compile fragment shader */
	ctx->fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(ctx->fShader, 1, &defaultFShaderCode, NULL);
	glCompileShader(ctx->fShader);

	/* create program and link vertex and fragment shaders */
	ctx->program = glCreateProgram();

	glAttachShader(ctx->program, ctx->vShader);
	glAttachShader(ctx->program, ctx->fShader);

	glBindAttribLocation(ctx->program, 0, "vertexPosition");
	glBindAttribLocation(ctx->program, 1, "vertexTexCoord");
	glBindAttribLocation(ctx->program, 2, "inColor");

	glLinkProgram(ctx->program);

	glUseProgram(ctx->program);
	ctx->matrixLoc = glGetUniformLocation(ctx->program, "matrix");
	glUseProgram(0);
}

void RFont_gl_renderer_text(RFont_GL_info* ctx, RFont_texture atlas, float* verts, float* tcoords, size_t nverts) {
	u32 i = 0;
	assert(ctx && nverts);

	glEnable(GL_TEXTURE_2D);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);

	glBindVertexArray(ctx->vao);

	glUseProgram(ctx->program);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 3 * sizeof(float)), verts, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->tbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 2 * sizeof(float)), tcoords, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	for (i = 0; i < (nverts * 4); i += 4)
		memcpy(&ctx->colors[i], ctx->color, sizeof(float) * 4);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, ctx->cbo);
	glBufferData(GL_ARRAY_BUFFER, (i32)(nverts * 4 * sizeof(float)), ctx->colors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (u32)atlas);
	ctx->matrix = RFont_ortho(0, (float)ctx->width, (float)ctx->height, 0, -1.0, 1.0);

	glUniformMatrix4fv(ctx->matrixLoc, 1, GL_FALSE, ctx->matrix.m);

	glDrawArrays(GL_TRIANGLES, 0, (i32)nverts);
	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

void RFont_gl_free_atlas(RFont_GL_info* ctx, RFont_texture atlas) { glDeleteTextures(1, (u32*)&atlas); RFONT_UNUSED(ctx); }
void RFont_gl_renderer_freePtr(RFont_GL_info* ctx) {
	if (ctx->vao != 0) {
		/* free vertex array */
		glDeleteVertexArrays(1, &ctx->vao);
		ctx->vao = 0;

		/* free buffers */
		glDeleteBuffers(1, &ctx->tbo);
		glDeleteBuffers(1, &ctx->vbo);

		/* free program data */
		glDeleteShader(ctx->vShader);
		glDeleteShader(ctx->fShader);
		glDeleteProgram(ctx->program);
	}
}

size_t RFont_gl_renderer_size(void) { return sizeof(RFont_GL_info); }

RFont_renderer_proc RFont_gl_renderer_proc(void) {
	RFont_renderer_proc proc;
	proc.initPtr = (void(*)(void*))RFont_gl_renderer_internal_initPtr;
	proc.create_atlas = (RFont_texture (*)(void*, u32, u32))RFont_gl_create_atlas;
	proc.free_atlas = (void (*)(void*, RFont_texture))RFont_gl_free_atlas;
	proc.bitmap_to_atlas = (void(*)(void*, RFont_texture, u32, u32, u32, u8*, float, float, float*, float*))RFont_gl_bitmap_to_atlas;
	proc.render = (void (*)(void*, RFont_texture, float*, float*, size_t))RFont_gl_renderer_text;
	proc.set_color = (void (*)(void*, float, float, float, float))RFont_gl_renderer_set_color;
	proc.set_framebuffer = (void (*)(void*, u32, u32))RFont_gl_renderer_set_framebuffer;
	proc.freePtr = (void (*)(void*))RFont_gl_renderer_freePtr;
	proc.size = (size_t (*)(void))RFont_gl_renderer_size;

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
