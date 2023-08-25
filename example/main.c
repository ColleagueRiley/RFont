#define RGFW_IMPLEMENTATION
#define RFONT_IMPLEMENTATION
#define RLGL_IMPLEMENTATION

#if !defined(RFONT_RENDER_LEGACY) && !defined(RFONT_RENDER_RLGL)
#define GL_GLEXT_PROTOTYPES
#endif

#ifdef RFONT_RENDER_RLGL
#include "rlgl.h"
#define glColor4ub rlColor4ub
#endif

#include "RGFW.h"

#include "RFont.h"

#include <stdbool.h>

int main(int argc, char **argv) {
    RGFW_setGLVersion(3, 3);
    
    RGFW_window* win = RGFW_createWindow((argc > 1) ? argv[1] : "window", 200, 200, 500, 500, 0);

    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
    rlLoadExtensions((void*)RGFW_getProcAddress);        
    rlglInit(win->w, win->h);  

    rlDrawRenderBatchActive();      // Update and draw internal render batch
    #endif

    RFont_init(500, 500);

    RFont_font* font = RFont_font_init("/usr/share/fonts/TTF/DejaVuSans.ttf");

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
        
        glColor4ub(255, 0, 0, 255);
        RFont_draw_text(font, "c", 200, 100, 50);

        #if defined(RFONT_RENDER_RLGL)
        rlDrawRenderBatchActive();      // Update and draw internal render batch
        #endif


        RGFW_window_swapBuffers(win);
    }


    #if defined(RFONT_RENDER_RLGL) && !defined(RFONT_RENDER_LEGACY)
    rlglClose();
    #endif
    
    RGFW_window_close(win);

    return 0;
}