#define RGFW_IMPLEMENTATION
#define RFONT_IMPLEMENTATION
#define RGL_IMPLEMENTATION
#define RGL_IMPLEMENTATION

#ifdef RFONT_RENDER_LEGACY
#define RGL_OPENGL_LEGACY
#endif

#ifdef RFONT_RENDER_ES3
#ifndef __EMSCRIPTEN__
#define RGFW_OPENGL_ES3
#endif

#include <GLES3/gl3.h>
#endif

#include "RGFW.h"

#ifdef RFONT_RENDER_RGL
#include "RGL.h"
#endif

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RGL) && !defined(RFONT_RENDER_ES3)
#define RGL_LOAD_IMPLEMENTATION

#include "ext/rglLoad.h"
#endif
#include "RFont.h"


#include <stdbool.h>

RFont_font* english;
RFont_font* japanese;

RFont_glyph glyphFallback(RFont_font* font, u32 codepoint, size_t size) {
   RFONT_UNUSED(font); RFONT_UNUSED(size);
   RFont_glyph g;

   if (font == japanese)
      g = RFont_font_add_codepointPro(english, codepoint, size, 0);
   else
      g = RFont_font_add_codepointPro(japanese, codepoint, size, 0);
   
   if (g.codepoint != 0 && g.size != 0) {
      printf("Codepoint found in fallback font\n");
      return g;
   } else {
      printf("Codepoint %s not found in fallback font either\n", RFont_codepoint_to_utf8(codepoint));
   }

   return (RFont_glyph){0, 0, 0, 0, (RFont_font*)0, 0, 0, 0, 0, 0, 0};
}

int main(int argc, char **argv) {
    #if !defined(RFONT_RENDER_LEGACY)
    RGFW_setGLVersion(RGFW_GL_CORE, 3, 3);
    #endif

    RGFW_window* win = RGFW_createWindow((argc > 1) ? argv[1] : "window", RGFW_RECT(200, 200, 1000, 500), 0);

    #if defined(RFONT_RENDER_RGL) && !defined(RFONT_RENDER_LEGACY)    
    rglInit((void*)RGFW_getProcAddress);    
    #endif

    #if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RGL) && !defined(RFONT_RENDER_ES3)
    if (RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress)) {
        printf("Failed to load OpenGL, defaulting to OpenGL 2\n");
        RFont_render_legacy(true);
    }
    #endif

    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    RFont_init(win->r.w, win->r.h);
    english = RFont_font_init("DejaVuSans.ttf");
    japanese = RFont_font_init("DroidSansJapanese.ttf");

    RFont_set_glyph_fallback_callback(glyphFallback);
    glViewport(0, 0, win->r.w, win->r.h);
    
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
        
        #if defined(RFONT_RENDER_RGL)
        rglRenderBatch();      // Update and draw internal render batch
        #endif

        RGFW_window_swapBuffers(win);
                
        RFont_update_framebuffer(win->r.w, win->r.h);
    }

    RFont_font_free(english);
    RFont_font_free(japanese);

    #if defined(RFONT_RENDER_RGL) && !defined(RFONT_RENDER_LEGACY)
    rglClose();
    #endif
    
    RGFW_window_close(win);

    return 0;
}
