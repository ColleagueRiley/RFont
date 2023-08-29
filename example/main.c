#define RGFW_IMPLEMENTATION
#define RFONT_IMPLEMENTATION
#define RLGL_IMPLEMENTATION

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
#define GL_GLEXT_PROTOTYPES
#endif

#ifdef RFONT_RENDER_LEGACY
#define GRAPHICS_API_OPENGL_11
#endif

#ifdef RFONT_RENDER_RLGL
#include "rlgl.h"
#endif

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL) && defined(_WIN32)
#define GLAD_MALLOC malloc
#define GLAD_FREE free

#define GLAD_GL_IMPLEMENTATION
#include "glad.h"
#endif

#include "RFont.h"
#include "RGFW.h"


#include <stdbool.h>

int main(int argc, char **argv) {
    RGFW_setGLVersion(3, 3);
    
    RGFW_window* win = RGFW_createWindow((argc > 1) ? argv[1] : "window", 200, 200, 1000, 500, 0);

    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
    rlLoadExtensions((void*)RGFW_getProcAddress);        
    rlglInit(win->w, win->h);  

    rlDrawRenderBatchActive();      // Update and draw internal render batch
    #endif

    #if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL) && defined(_WIN32)
    gladLoadGL((GLADloadfunc)RGFW_getProcAddress);
    #endif

    RFont_init(win->w, win->h);
    RFont_font* font = RFont_font_init("DejaVuSans.ttf");
    RFont_font* japanese = RFont_font_init("DroidSansJapanese.ttf");

    bool running = true;

    while (running) {
        while(RGFW_window_checkEvent(win)) {   
            if (win->event.type == RGFW_quit) {
                running = false;
                break;
            }
        }

        glClearColor(100, 100, 100, 255);
        glClear(GL_COLOR_BUFFER_BIT);
        
        #ifdef RFONT_RENDER_RLGL
        rlColor4ub(255, 0, 0, 255);
        #else
        glColor4f(1.0f, 0, 0, 1.0f);
        #endif

        RFont_draw_text(font, "abcdefghijklmnopqrstuvwxyz\n1234567890@.<>,/?\\|[{]}", 0, 0, 60);
        RFont_draw_text(font, "`~!#$%^&*()_-=+", 0, 120, 60);
        RFont_draw_text(font, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nSomething about a fast lazy dog.", 0, 210, 20);

        RFont_draw_text(font, "RFont_draw_text(); ⌫", 0, 240, 60);
        RFont_draw_text(japanese, "テキスト例", 0, 300, 60);
        
        #if defined(RFONT_RENDER_RLGL)
        rlDrawRenderBatchActive();      // Update and draw internal render batch
        #endif


        RGFW_window_swapBuffers(win);
        
        #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
        rlSetFramebufferSize(win->w, win->h);  
        RFont_update_framebuffer(win->w, win->h);
        #endif
    }

    RFont_font_free(font);
    RFont_font_free(japanese);

    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
    rlglClose();
    #endif
    
    RGFW_window_close(win);

    return 0;
}