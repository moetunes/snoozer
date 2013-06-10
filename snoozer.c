#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#define TABLENGTH(X)    (sizeof(X)/sizeof(*X))

typedef struct {
    unsigned long color;
    GC gc;
} Theme;

static unsigned long getcolor(const char* color);
static void chime();
static void *waiting();
static void add_snooze();

//USER DEFINES
static unsigned int REPEATS = 10;    // replay the chime
static unsigned int SNOOZETIME = 5;  // snooze time
static char *CHIME = "/home/pnewm/progs/c.files/snooze/chime.wav";
static char *FONTNAME = "-*-dina-*-r-*";

static unsigned int screen, width, height, running,text_width,text_width1;
static int alarmhour, alarmminutes;
static const char *themecolors[] = { "#992200", "#004466","#555555", "#009955", "#bbbbbb", };
static Display *dis;
static Window root, mainwin, snoozewin, quitwin;
static 	XFontStruct *font;

static char text[20];

static Theme theme[TABLENGTH(themecolors)];

unsigned long getcolor(const char* color) {
    XColor c;
    Colormap map = DefaultColormap(dis,screen);

    if(!XAllocNamedColor(dis,map,color,&c,&c)) {
        puts("\033[0;31mError parsing color!");
        return 1;
    }
    return c.pixel;
}

void chime() {
    unsigned int i;
    int a_pipe[2];
    pipe(a_pipe);
    
    for(i=0;i<REPEATS;++i) {
        if(running < 1) {
            if(vfork() == 0) {
                close(a_pipe[1]);
                dup2(a_pipe[0], STDERR_FILENO);
                execl("/usr/bin/aplay", "aplay", "-c 1", CHIME, NULL);
            }
            wait(0);
        } else
            return;
    }
}

void *waiting() {
	struct tm tm;
	time_t time_value;
    while(running < 1) {
        time_value = time(0);
        tm = *localtime(&time_value);
        if(tm.tm_hour == alarmhour && tm.tm_min == alarmminutes) {
            sprintf(text,"WAKE UP YA BASTARD !!");
            XSetWindowBackground(dis,quitwin,theme[0].color);
            XClearWindow(dis, quitwin);
            XDrawString(dis, quitwin, theme[4].gc,
   			 (((width/3)*2)/2-(text_width/2)), (height/8)+font->ascent, text, strlen(text));
            XFlush(dis);
            chime();
        }
        sleep(15);
    }
    return 0;
}

void add_snooze() {
    running = 1;
    alarmminutes += SNOOZETIME;
    if(alarmminutes >= 60) {
        alarmminutes -= 60;
        alarmhour += 1;
        if(alarmhour > 24) alarmhour -= 24;
    }
    XSetWindowBackground(dis,quitwin,theme[3].color);
    XClearWindow(dis, quitwin);
    if(alarmminutes < 10)
        sprintf(text, "Alarm set for %d:0%d", alarmhour, alarmminutes);
    else
        sprintf(text, "Alarm set for %d:%d", alarmhour, alarmminutes);
    XDrawString(dis, quitwin, theme[4].gc,
     (((width/3)*2)/2-(text_width/2)), (height/8)+font->ascent, text, strlen(text));
    XFlush(dis);
    sleep(5);
    running = 0;
}

int main(int argc, char ** argv){
    if (argc > 3 || argc < 2) {
        fputs("USAGE: snooze <hour> <minutes>\n    e.g. snooze 4 30\n         snooze 16 30\n", stderr);
        return 1;
    }
    running = 0;

    alarmhour = atoi(argv[1]);
    alarmminutes = atoi(argv[2]);

    if(alarmminutes < 10)
        sprintf(text, "Alarm set for %d:0%d", alarmhour, alarmminutes);
    else
        sprintf(text, "Alarm set for %d:%d", alarmhour, alarmminutes);

	pthread_t pth;
	pthread_create(&pth,NULL,waiting,NULL);

	unsigned int i;
	XEvent ev;
	XGCValues values;

    char text1[20];
    sprintf(text1, "Snooze for %d more minutes", SNOOZETIME);

	dis = XOpenDisplay(NULL);
	if (!dis) {fputs("unable to connect to display", stderr);return 7;}

	screen = DefaultScreen(dis);
	root = RootWindow(dis,screen);

	font = XLoadQueryFont(dis, FONTNAME);
	if (!font) {
		fprintf(stderr, "unable to load preferred font: %s using fixed", FONTNAME);
		font = XLoadQueryFont(dis, "fixed");
	}

	values.line_width = 2;
	values.line_style = LineSolid;
	values.font = font->fid;
    for(i=0;i<5;++i) {
        theme[i].color = getcolor(themecolors[i]);
        values.foreground = theme[i].color;
        //values.background = theme[i].color;
        theme[i].gc = XCreateGC(dis, root, GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
	}
	width = (XDisplayWidth(dis, screen)/4);
	height = (XDisplayHeight(dis, screen)/4);

	mainwin = XCreateSimpleWindow(dis, root,width*3-20,height*3-20,
      width,height,0,0,theme[2].color);
	snoozewin = XCreateSimpleWindow(dis, mainwin, width/6,(2*height/4)+10,
	  (width/3)*2,height/4,1,0,theme[1].color);
	quitwin = XCreateSimpleWindow(dis, mainwin,width/6,height/4,
	  (width/3)*2,height/4,1,0,theme[3].color);

	text_width = XTextWidth(font, text, strlen(text));
	text_width1 = XTextWidth(font, text1, strlen(text1));

	XStoreName(dis, mainwin, "SNOOZER");
	XSelectInput(dis, mainwin, ButtonPressMask|StructureNotifyMask|ExposureMask );
	XMapWindow(dis, mainwin);
	XMapSubwindows(dis, mainwin);

	while(1){
		XNextEvent(dis, &ev);
		switch(ev.type){
		case Expose:
   			if(ev.xexpose.count > 0) break;
   			XDrawString(dis, quitwin, theme[4].gc,
   			 (((width/3)*2)/2-(text_width/2)), (height/8)+font->ascent, text, strlen(text));
   			XDrawString(dis, snoozewin, theme[4].gc,
   			 (((width/3)*2)/2-(text_width1/2)), (height/8)+font->ascent, text1, strlen(text1));
			break;
		case ConfigureNotify:
			if (width != ev.xconfigure.width
					|| height != ev.xconfigure.height) {
				width = ev.xconfigure.width;
				height = ev.xconfigure.height;
				XMoveResizeWindow(dis, snoozewin, width/6,(2*height/4)+10,
				 (width/3)*2, height/4);
				XMoveResizeWindow(dis, quitwin, width/6,height/4,
				  (width/3)*2,height/4);
			}
			break;
		case ButtonPress:
			if(ev.xbutton.subwindow == snoozewin) {
			    add_snooze();
			}
			if(ev.xbutton.subwindow == quitwin) {
    			running = 1;
    			pthread_cancel(pth);
    			for(i=0;i<5;++i)
	    		    XFreeGC(dis, theme[i].gc);
		    	XCloseDisplay(dis);
			    return 0;
			}
		}
	}
}