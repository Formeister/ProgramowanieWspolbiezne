#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/X.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <math.h>


#define NTS 1000000000.0


int newWindow(char *address){

    Display *display;
    int screen;
    Visual *visual;
    int depth;
    XSetWindowAttributes windowattributes;
    Window window;
    Colormap colormap;
    XColor color1, color2, dummy;
    GC gc;
    double x;
    XEvent event;
    double timer = 0;

    struct timespec timer_start, timer_stop;

    display = XOpenDisplay(address);

    if (display == NULL){
        fprintf(stderr, "Nie można otworzyć wyświetlacza.\n");
        exit(1);
    }

    screen = DefaultScreen(display);
    visual = DefaultVisual(display, screen);
    depth = DefaultDepth(display, screen);

    windowattributes.background_pixel = XWhitePixel(display, screen);
    windowattributes.override_redirect = False;

    window = XCreateWindow(display,XRootWindow(display, screen),
            100, 100, 500, 500, 10, depth, InputOutput,
            visual, CWBackPixel|CWOverrideRedirect, &windowattributes); 

    XSelectInput(display, window,ExposureMask|KeyPressMask);
    
    colormap = DefaultColormap(display, screen);

    XAllocNamedColor(display, colormap, "cyan", &color1, &dummy);
    XAllocNamedColor(display, colormap, "red", &color2, &dummy);
    XMapWindow(display, window);
    gc = DefaultGC(display, screen);

    //Obsługa protokołu WM_DELETE_WINDOW
    Atom WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False); 
    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

    while(1){
        XNextEvent(display, &event);
	switch (event.type){
	    case Expose:
		XSetForeground(display, gc, color1.pixel);
                XFillRectangle(display, window, gc, 100, 100, 310, 310);
                XSetForeground(display, gc, color2.pixel);

		for(x = 100; x <= 175; x+=5){
                   XFillRectangle(display, window, gc, x, 275-x, 10, 10);
 		}
		for(x = 175; x <= 250; x+=5){
                   XFillRectangle(display, window, gc, x, x-75, 10, 10);
 		}
		for(x = 250; x <= 325; x+=5){
                   XFillRectangle(display, window, gc, x, 425-x, 10, 10);
 		}
		for(x = 325; x <= 400; x+=5){
                   XFillRectangle(display, window, gc, x, x-225, 10, 10);
 		}
		for(x = 100; x <= 250; x+=5){
                   XFillRectangle(display, window, gc, x, (3*x)/2+25, 10, 10);
 		}
		for(x = 250; x <= 400; x+=5){
                   XFillRectangle(display, window, gc, x, 775-(3*x/2), 10, 10);
 		}

                XFlush(display);
       
                clock_gettime(CLOCK_REALTIME, &timer_start);
                break;      
 
            case KeyPress:
                XCloseDisplay(display);
                clock_gettime(CLOCK_REALTIME, &timer_stop);
                timer = timer_stop.tv_sec + timer_stop.tv_nsec/NTS
                    - (timer_start.tv_sec + timer_start.tv_nsec/NTS);
                printf("Wyłączono okno na ekranie %s poprzez naciśniecie dowolnego klawisza po %.1f sekundach.\n", address, timer);
                exit(0);

            case ClientMessage:
                XCloseDisplay(display);
                clock_gettime(CLOCK_REALTIME, &timer_stop);
                timer = timer_stop.tv_sec + timer_stop.tv_nsec/NTS
                    - (timer_start.tv_sec + timer_start.tv_nsec/NTS);
                printf("Wyłączono okno na ekranie %s poprzez zamknięcie po %.1f sekundach.\n", address, timer);
                exit(0);
    	}
    }
}

int main(int argc, char *argv[]){

    int i;

    //Sprawdzanie argumentów
    if (argc < 2){
        printf("Błędna liczba argumentów (%d), spodziewane argumenty: adres_użytkownika adres_użytkownika...\n", argc-1);	
        exit(1);
    }

    for (i = 0; i < argc - 1; i++){
        if (fork() == 0){
            newWindow(argv[i + 1]);
        }
    }

    for (i = 0; i < argc - 1; i++){
        if (wait(NULL) == -1){
            perror("Błąd wait");
        }
    }
    
    return 0;
}
