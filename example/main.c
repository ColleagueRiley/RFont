#define RGFW_IMPLEMENTATION
#define RFONT_IMPLEMENTATION
#define RLGL_IMPLEMENTATION
#define RGL_IMPLEMENTATION

#ifdef RFONT_RENDER_LEGACY
#define GRAPHICS_API_OPENGL_11
#endif


#ifdef RFONT_RENDER_RLGL
#include "RGL.h"
#endif

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
#define GLAD_MALLOC malloc
#define GLAD_FREE free

#define GLAD_GL_IMPLEMENTATION

#include "ext/glad.h"
#endif


#include "RFont.h"

#include "RGFW.h"

#include <stdbool.h>

struct timeval GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv;
}

int main(int argc, char **argv) {
    RGFW_setGLVersion(3, 3);

    RGFW_window* win = RGFW_createWindow((argc > 1) ? argv[1] : "window", 200, 200, 1000, 500, 0);

    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)       
    rglInit(win->w, win->h, (void*)RGFW_getProcAddress);  

    //rlDrawRenderBatchActive();      // Update and draw internal render batch
    #endif

    #if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
    gladLoadGL((GLADloadfunc)RGFW_getProcAddress);
    #endif

    glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    RFont_init(win->w, win->h);
    //"../helvetica.ttf"
    RFont_font* font = RFont_font_init("../helvetica.ttf");
    RFont_font* japanese = RFont_font_init("DroidSansJapanese.ttf");

    bool running = true;

    while (running) {
        while(RGFW_window_checkEvent(win)) {   
            if (win->event.type == RGFW_quit) {
                running = false;
                break;
            }
        }

        glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RFont_set_color(0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text_spacing(font, "Adventure\nButton", 0, 0, 60, 0);
        
        //RFont_draw_text(font, "abcdefghijklmnopqrstuvwxyz\n1234567890@.<>,/?\\|[{]}", 0, 0, 60);

        RFont_draw_text_spacing(font, "`~!#$%^&*()_-=+", 0, 120, 60, 1.0f);
        RFont_set_color(1.0f, 0.0f, 0, 1.0f);
        RFont_draw_text(font, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nSomething about a fast lazy dog.", 0, 210, 20);
        RFont_set_color(0.0f, 1.0f, 0, 1.0f);
        RFont_draw_text(font, "RFont_draw_text(); ⌫§", 0, 240, 60);
        RFont_set_color(1.0f, 0.0f, 0, 1.0f);

        RFont_draw_text(font, RFont_fmt("FPS : %i", win->event.fps), 0, 300, 60);

        #if defined(RFONT_RENDER_RLGL)
        rglRenderBatch();
        //rlDrawRenderBatchActive();      // Update and draw internal render batch
        #endif

        RGFW_window_swapBuffers(win);
        
        #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
        rglSetFramebufferSize(win->w, win->h);  
        RFont_update_framebuffer(win->w, win->h);
        #endif
    }

    RFont_font_free(font);
    RFont_font_free(japanese);
    RFont_close();

    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
    rglClose();
    #endif
    
    RGFW_window_close(win);

    return 0;
}