#define RFONT_IMPLEMENTATION

#define RFONT_C89
#ifndef RFONT_RENDER_LEGACY
#define RGL_LOAD_IMPLEMENTATION
#include "ext/rglLoad.h"
#endif

#define RGFWDEF
#define RGFW_C89
#define RGFW_OPENGL
#include "RGFW.h"
#define RFONT_INT_DEFINED
#include "RFont.h"

#ifndef RFONT_RENDER_LEGACY
#include "RFont_gl.h"
#endif

#include "RFont_gl1.h"

RFont_font* english;
RFont_font* japanese;

RFont_glyph glyphFallback(RFont_renderer* renderer, RFont_font* font, u32 codepoint, size_t size) {
    RFont_glyph g;
    RFONT_UNUSED(font); RFONT_UNUSED(size);

   if (font == japanese)
      g = RFont_font_add_codepoint_ex(renderer, english, codepoint, size, 0);
   else
      g = RFont_font_add_codepoint_ex(renderer, japanese, codepoint, size, 0);

   if (g.codepoint != 0 && g.size != 0) {
      printf("Codepoint found in fallback font\n");
      return g;
   } else {
      printf("Codepoint %s not found in fallback font either\n", RFont_codepoint_to_utf8(codepoint));
   }

   memset(&g, 0, sizeof(g));
   return g;
}

int main(int argc, char **argv) {
    RGFW_window* win;
	i32 w, h;

    #if !defined(RFONT_RENDER_LEGACY)
		RFont_renderer renderer = RFont_gl_renderer();

		RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
		hints->profile = RGFW_glCore;
		hints->major = 3;
		hints->minor = 3;
		RGFW_setGlobalHints_OpenGL(hints);
	#else
		RFont_renderer renderer = RFont_gl1_renderer();
	#endif

    win = RGFW_createWindow((argc > 1) ? argv[1] : "window", 200, 200, 1000, 500, RGFW_windowCenter | RGFW_windowOpenGL);
	RGFW_window_setExitKey(win, RGFW_escape);

    #if !defined(RFONT_RENDER_LEGACY)
    if (RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress_OpenGL)) {
		renderer = RFont_gl1_renderer();
        printf("Failed to load OpenGL, defaulting to OpenGL 2\n");
    }
    #endif

	RFont_renderer_init(&renderer);

    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

    english = RFont_font_init(&renderer, "DejaVuSans.ttf");
    japanese = RFont_font_init(&renderer, "DroidSansJapanese.ttf");

    /*RFont_set_glyph_fallback_callback(glyphFallback); */

    while (RGFW_window_shouldClose(win) == 0) {
		RGFW_pollEvents();
		RGFW_window_getSize(win, &w, &h);

        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		glViewport(0, 0, (size_t)w, (size_t)h);
		RFont_renderer_set_framebuffer(&renderer, (u32)w, (u32)h);

		RFont_renderer_set_color(&renderer, 0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text(&renderer, english, "abcdefghijklmnopqrstuvwxyz\n1234567890@.<>,/?\\|[{]}", 0, 0, 60);
        RFont_draw_text_spacing(&renderer, english, "`~!#$%^&*()_-=+", 0, 120, 60, 1.0f);
        RFont_renderer_set_color(&renderer, 1.0f, 0.0f, 0, 1.0f);
        RFont_draw_text(&renderer, english, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nSomething about a fast lazy 犬.", 0, 210, 20);
        RFont_renderer_set_color(&renderer, 0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text(&renderer, english, "RFont_draw_text(); ⌫§", 0, 240, 60);
        RFont_renderer_set_color(&renderer, 1.0f, 0.0f, 0, 1.0f);
        RFont_draw_text(&renderer, japanese, "テキスト例hola", 0, 300, 60);
        RGFW_window_swapBuffers_OpenGL(win);
    }

    RFont_font_free(&renderer, english);
    RFont_font_free(&renderer, japanese);
	RFont_renderer_free(&renderer);
    RGFW_window_close(win);
    return 0;
}
