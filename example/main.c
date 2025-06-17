#define RFONT_IMPLEMENTATION

#ifdef RFONT_RENDER_ES3
#ifndef __EMSCRIPTEN__
#define RGFW_OPENGL_ES3
#endif

#include <GLES3/gl3.h>
#endif

#define RFONT_C89
#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_ES3)
#define RGL_LOAD_IMPLEMENTATION
#include "ext/rglLoad.h"
#endif

#define RGFWDEF
#define RGFW_C89
#include "RGFW.h"

#define RFONT_INT_DEFINED
#include "RFont.h"

RFont_font* english;
RFont_font* japanese;

RFont_glyph glyphFallback(RFont_font* font, u32 codepoint, size_t size) {
    RFont_glyph g;
    RFONT_UNUSED(font); RFONT_UNUSED(size);

   if (font == japanese)
      g = RFont_font_add_codepoint_ex(english, codepoint, size, 0);
   else
      g = RFont_font_add_codepoint_ex(japanese, codepoint, size, 0);
   
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
    RGFW_rect rect = {200, 200, 1000, 500};
    RGFW_window* win;
    #if !defined(RFONT_RENDER_LEGACY)
        RGFW_setGLHint(RGFW_glProfile, RGFW_glCore);
        RGFW_setGLHint(RGFW_glMinor, 3);
        RGFW_setGLHint(RGFW_glMajor, 3);
    #endif
    
    win = RGFW_createWindow((argc > 1) ? argv[1] : "window", rect, 0);

    #if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_ES3)
    if (RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress)) {
        printf("Failed to load OpenGL, defaulting to OpenGL 2\n");
        RFont_render_legacy(1);
    }
    #endif

    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    RFont_init((size_t)win->r.w, (size_t)win->r.h);
    english = RFont_font_init("DejaVuSans.ttf");
    japanese = RFont_font_init("DroidSansJapanese.ttf");

    RFont_set_glyph_fallback_callback(glyphFallback);
    glViewport(0, 0, (size_t)win->r.w, (size_t)win->r.h);
    
    while (RGFW_window_shouldClose(win) == 0) {
 
        while(RGFW_window_checkEvent(win)) {   
            if (win->event.type == RGFW_quit) {
                break;
            }
        }

        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        RFont_set_color(0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text(english, "abcdefghijklmnopqrstuvwxyz\n1234567890@.<>,/?\\|[{]}", 0, 0, 60);
        RFont_draw_text_spacing(english, "`~!#$%^&*()_-=+", 0, 120, 60, 1.0f);
        RFont_set_color(1.0f, 0.0f, 0, 1.0f);
        RFont_draw_text(english, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nSomething about a fast lazy 犬.", 0, 210, 20);
        RFont_set_color(0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text(english, "RFont_draw_text(); ⌫§", 0, 240, 60);
        RFont_set_color(1.0f, 0.0f, 0, 1.0f);
        RFont_draw_text(japanese, "テキスト例hola", 0, 300, 60);
        
        RGFW_window_swapBuffers(win);
                
        RFont_update_framebuffer((size_t)win->r.w, (size_t)win->r.h);
    }

    RFont_font_free(english);
    RFont_font_free(japanese);
    
    RGFW_window_close(win);
    return 0;
}
