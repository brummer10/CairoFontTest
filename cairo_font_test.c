

#include <stdio.h>
#include <string.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// gcc -g cairo_font_test.c  -lX11 `pkg-config --cflags --libs cairo` -o cairo_font_test 

// resize window
typedef struct {
    double x;
    double y;
    double x1;
    double y1;
} re_scale;

typedef struct {
    Display* display;
    Drawable win;
    Atom wm_delete_window;
    XEvent event;
    long event_mask;
    int w;
    int h;

    cairo_t *cr;
    cairo_surface_t *surface;
    re_scale rescale;
    int width;
    int height;

} viewport;

// redraw the window
static void expose(viewport *v) {

    // push and pop to avoid any flicker (offline drawing)
    cairo_push_group (v->cr);

    // scale window to user request
    cairo_scale (v->cr, v->rescale.x, v->rescale.y);

    // draw background
    cairo_set_source_rgba (v->cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(v->cr,0, 0, v->w, v->h);
    cairo_fill(v->cr);

    cairo_set_source_rgba (v->cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_font_size (v->cr, 21);

    cairo_select_font_face (v->cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (v->cr, 10, 30);
    cairo_show_text(v->cr, "This should be Sans");
    cairo_new_path (v->cr);

    cairo_select_font_face (v->cr, "Serif", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (v->cr, 10, 60);
    cairo_show_text(v->cr, "This should be Serif");
    cairo_new_path (v->cr);

    cairo_select_font_face (v->cr, "Roboto Condensed", CAIRO_FONT_SLANT_NORMAL,
                               CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (v->cr, 10, 90);
    cairo_show_text(v->cr, "This should be Roboto Condensed");
    cairo_new_path (v->cr);

    cairo_select_font_face (v->cr, "Roboto", CAIRO_FONT_SLANT_ITALIC,
                               CAIRO_FONT_WEIGHT_NORMAL);
    cairo_move_to (v->cr, 10, 120);
    cairo_show_text(v->cr, "This should be Roboto Italic");
    cairo_new_path (v->cr);

    // re-scale to origin
    cairo_scale (v->cr, v->rescale.x1, v->rescale.y1);

    cairo_pop_group_to_source (v->cr);

    // finally paint to window
    cairo_paint (v->cr);
}

static void resize_event(viewport *v) {
    // get new size
    v->width = v->event.xconfigure.width;
    v->height = v->event.xconfigure.height;
    // resize cairo surface
    cairo_xlib_surface_set_size( v->surface, v->width, v->height);
    // calculate scale factor
    v->rescale.x  = (double)v->width/v->w;
    v->rescale.y  = (double)v->height/v->h;
    // calculate re-scale factor
    v->rescale.x1 = (double)v->w/v->width;
    v->rescale.y1 = (double)v->h/v->height;
}

int main(int argc, char* argv[])
{
    #ifdef FONTCONFIG_PATH
    fprintf(stderr, "%s\n", FONTCONFIG_PATH);
    #else
    fprintf(stderr, "FONTCONFIG_PATH not defined\n");
    #endif
    viewport v;
    v.display = XOpenDisplay(NULL);
    v.w = 320;
    v.h = 130;

    v.win = XCreateWindow(v.display, DefaultRootWindow(v.display), 0, 0, v.w,v.h, 0,
                        CopyFromParent, InputOutput, CopyFromParent, CopyFromParent, 0);

    v.event_mask = StructureNotifyMask|ExposureMask;

    XSelectInput(v.display, v.win, v.event_mask);

    v.wm_delete_window = XInternAtom(v.display, "WM_DELETE_WINDOW", 0);
    XSetWMProtocols(v.display, v.win, &v.wm_delete_window, 1);

    const char* title = "Font test";
    XStoreName(v.display, v.win, title);
    XChangeProperty(v.display, v.win, XInternAtom(v.display, "_NET_WM_NAME", False),
        XInternAtom(v.display, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char*)title, strlen(title));

    v.surface = cairo_xlib_surface_create
        (v.display, v.win,DefaultVisual(v.display, DefaultScreen (v.display)), v.w, v.h);
    v.cr = cairo_create(v.surface);

    XMapWindow(v.display, v.win);

    int keep_running = 1;

    while (keep_running) {
        XNextEvent(v.display, &v.event);

        switch(v.event.type) {
            case ConfigureNotify:
                // configure event, we only check for resize events here
                resize_event(&v);
            break;
            case Expose:
                // only redraw on the last expose event
                if (v.event.xexpose.count == 0) {
                    expose(&v);
                }
            break;
            case ClientMessage:
                if (v.event.xclient.message_type == XInternAtom(v.display, "WM_PROTOCOLS", 1) &&
                   (Atom)v.event.xclient.data.l[0] == XInternAtom(v.display, "WM_DELETE_WINDOW", 1))
                    keep_running = 0;
                break;
            default:
                break;
        }
    }

    cairo_destroy(v.cr);
    cairo_surface_destroy(v.surface);
    XDestroyWindow(v.display, v.win);
    XCloseDisplay(v.display);
    return 0;
}
