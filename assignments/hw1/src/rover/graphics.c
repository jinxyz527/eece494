
/*******************************************************
 *
 *  Graphics System Routines for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan 
 *
 *  This file contains the graphics= subsystem routines.
 *  Understanding this requires understanding some fairly
 *  subtle X-Windows programming issues, but you are free
 *  to look at it if you like.  The graphics window is
 *  set up, and then a thread is created to watch for
 *  Expose and Resize events.
 *
 ******************************************************/

#include "defs.h"
#include "rover.h"
#include "graphics.h"
#include "ext.h"

/* This thread is used to watch for Expose and Resize
   events  */

pthread_t graphics_thread_id;

/* This defines the ICON that will be used when the
   X-window is minimized by the user. */

int icon_bitmap_width = 20;
int icon_bitmap_height = 20;
static char icon_bitmap_bits[] = {
      0x60,0x00,0x01,0xb0,0x00,0x07,0x0c,0x03,0x00,0x04,0x04,0x00,
      0xc2,0x18,0x00,0x03,0x30,0x00,0x01,0x60,0x00,0xf1,0xdf,0x00,
      0xc1,0xf0,0x01,0x82,0x01,0x00,0x02,0x03,0x00,0x02,0x0c,0x00,
      0x02,0x38,0x00,0x04,0x60,0x00,0x04,0xe0,0x00,0x04,0x38,0x00,
      0x84,0x06,0x00,0x14,0x14,0x00,0x0c,0x34,0x00,0x00,0x00,0x00};

/* The following variables are used to communicate with the
   X-Windows system.  If you want to understand this, a good
   book to read is "Xlib Programming Manual" by Adrian Nye
   (it is avaible via Amazon or Chapters).  Learning X-windows
   programming will certainly not be part of this course. */

XWMHints *wm_hints;
XClassHint *class_hints;
char *window_name, *icon_name;
XTextProperty windowName, iconName;
XSizeHints *size_hints;
Pixmap icon_pixmap;
Pixmap rover_pixmap_lighton;
Pixmap rover_pixmap_lightoff;
Pixmap rover_pixmap_lighton_flash;
Pixmap rover_pixmap_lightoff_flash;
Pixmap martian_pixmap;
Window win;
int border_width = 8;
unsigned int width, height;
BOOL window_big_enough;
Display *display;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
int display_width, display_height;
GC gc;


/*--------------------------------------------------------------
 * These routines are routines that get a coordinate rescaled
 * to the size of the window.  This is required so that the
 * user can resize windows, etc.   Also, not all displays will
 * use the same window size 
 *--------------------------------------------------------------*/

int get_screenx(int x)
{
   return((x*width/MAX_REAL_COORD_X));
}
int get_screeny(int y)
{
   return((y*height/MAX_REAL_COORD_Y));
}
int get_rovercoordx(int coord)
{
   return(get_screenx(coord*ROVER_SIZE/ROVER_COORD));
}
int get_rovercoordy(int coord)
{
   return(get_screeny(coord*ROVER_SIZE/ROVER_COORD));
}
int get_martiancoordx(int coord)
{
   return(get_screenx(coord*MARTIAN_SIZE/MARTIAN_COORD));
}
int get_martiancoordy(int coord)
{
   return(get_screeny(coord*MARTIAN_SIZE/MARTIAN_COORD));
}
int get_martianrowx(int coord)
{
   int lp = LEFT_CLIFF_POSITION + 5*SURFACE_BREAKDIFF_X;
   int rp = RIGHT_CLIFF_POSITION - 5*SURFACE_BREAKDIFF_X;

   return(get_screenx(lp + (rp-lp)*coord/100));
}

/*--------------------------------------------------------------
 * The rover itself is never drawn to the window 
 * directly.  Instead, they are drawn to a Pixmap once at the
 * start, and then the pixmap can be copied to the main window
 * all at once.  This speeds up the graphics quite a bit.  Note
 * that four pixmaps are actually created: two with the light on
 * and two with the light off.
 *--------------------------------------------------------------*/

void draw_rover_to_pixmap(Pixmap *p, BOOL lighton, BOOL flash)
{
   XSetForeground(display,gc,WhitePixel(display,screen_num));
   XFillRectangle(display,*p,gc,0,0,get_rovercoordx(ROVER_COORD)+2, 
                                    get_rovercoordy(ROVER_COORD)+2);
   XSetForeground(display,gc,BlackPixel(display,screen_num));

   /* The body of the rover */

   XFillRectangle(display,*p,gc,
                  get_rovercoordx(60), get_rovercoordy(150),
                  get_rovercoordx(180), get_rovercoordy(88));

   /* Camera Flash */

   if (flash) {
      XDrawLine(display,*p,gc,
                get_rovercoordx(58),
                get_rovercoordy(200),
                get_rovercoordx(0),
                get_rovercoordy(200));
      XDrawLine(display,*p,gc,
                get_rovercoordx(58),
                get_rovercoordy(200),
                get_rovercoordx(0),
                get_rovercoordy(150));
      XDrawLine(display,*p,gc,
                get_rovercoordx(58),
                get_rovercoordy(200),
                get_rovercoordx(0),
                get_rovercoordy(238));
      XDrawLine(display,*p,gc,
                get_rovercoordx(242),
                get_rovercoordy(200),
                get_rovercoordx(300),
                get_rovercoordy(200));
      XDrawLine(display,*p,gc,
                get_rovercoordx(242),
                get_rovercoordy(200),
                get_rovercoordx(300),
                get_rovercoordy(150));
      XDrawLine(display,*p,gc,
                get_rovercoordx(242),
                get_rovercoordy(200),
                get_rovercoordx(300),
                get_rovercoordy(238));
   }

   /* Wheels.  For some reason, I can't get DrawArc to work, 
      so our wheels are square */

   XFillRectangle(display,*p,gc,
                  get_rovercoordx(120), get_rovercoordy(240),
                  get_rovercoordx(60), get_rovercoordy(60));
   XFillRectangle(display,*p,gc,
                  get_rovercoordx(50), get_rovercoordy(240),
                  get_rovercoordx(60), get_rovercoordy(60));
   XFillRectangle(display,*p,gc,
                  get_rovercoordx(190), get_rovercoordy(240),
                  get_rovercoordx(60), get_rovercoordy(60));

   /* Sensor lines */

   XDrawLine(display,*p,gc,
             0,
             get_rovercoordy(90),
             get_rovercoordx(60),
             get_rovercoordy(90));
   XDrawLine(display,*p,gc,
             get_rovercoordx(300),
             get_rovercoordy(90),
             get_rovercoordx(240),
             get_rovercoordy(90));
   XDrawLine(display,*p,gc,
             get_rovercoordx(60),
             get_rovercoordy(90),
             get_rovercoordx(60),
             get_rovercoordy(150));
   XDrawLine(display,*p,gc,
             get_rovercoordx(240),
             get_rovercoordy(90),
             get_rovercoordx(240),
             get_rovercoordy(150));

   /* Light stand */

   XDrawLine(display,*p,gc,
             get_rovercoordx(150),
             get_rovercoordy(30),
             get_rovercoordx(150),
             get_rovercoordy(150));
   
   /* Light */

   if (lighton) {
      XFillRectangle(display,*p,gc,
                  get_rovercoordx(130), get_rovercoordy(30),
                  get_rovercoordx(40), get_rovercoordy(40));

      XDrawLine(display,*p,gc,
             get_rovercoordx(150),
             get_rovercoordy(20),
             get_rovercoordx(150),
             get_rovercoordy(0));
      XDrawLine(display,*p,gc,
             get_rovercoordx(120),
             get_rovercoordy(50),
             get_rovercoordx(110),
             get_rovercoordy(50));
      XDrawLine(display,*p,gc,
             get_rovercoordx(180),
             get_rovercoordy(50),
             get_rovercoordx(190),
             get_rovercoordy(50));

      XDrawLine(display,*p,gc,
             get_rovercoordx(120),
             get_rovercoordy(20),
             get_rovercoordx(110),
             get_rovercoordy(10));
      XDrawLine(display,*p,gc,
             get_rovercoordx(180),
             get_rovercoordy(20),
             get_rovercoordx(190),
             get_rovercoordy(10));
      XDrawLine(display,*p,gc,
             get_rovercoordx(120),
             get_rovercoordy(80),
             get_rovercoordx(110),
             get_rovercoordy(90));
      XDrawLine(display,*p,gc,
             get_rovercoordx(180),
             get_rovercoordy(80),
             get_rovercoordx(190),
             get_rovercoordy(90));
   }
}


/*--------------------------------------------------------------
 * The Martian itself is never drawn to the window 
 * directly.  Instead, they are drawn to a Pixmap once at the
 * start, and then the pixmap can be copied to the main window
 * all at once.  This speeds up the graphics quite a bit.  
 *--------------------------------------------------------------*/

void draw_martian_to_pixmap(Pixmap *p)
{
   XSetForeground(display,gc,WhitePixel(display,screen_num));
   XFillRectangle(display,*p,gc,0,0,get_martiancoordx(MARTIAN_COORD)+2, 
                                    get_martiancoordy(MARTIAN_COORD)+2);
   XSetForeground(display,gc,BlackPixel(display,screen_num));

   /* The body of the martian */

   XFillRectangle(display, *p, gc,
                  get_martiancoordx(0), get_martiancoordy(0),
                  get_martiancoordx(70), get_martiancoordy(300));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(230), get_martiancoordy(0),
                  get_martiancoordx(70), get_martiancoordy(300));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(120), get_martiancoordy(0),
                  get_martiancoordx(70), get_martiancoordy(240));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(70), get_martiancoordy(120),
                  get_martiancoordx(60), get_martiancoordy(60));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(70), get_martiancoordy(0),
                  get_martiancoordx(60), get_martiancoordy(60));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(180), get_martiancoordy(120),
                  get_martiancoordx(60), get_martiancoordy(60));
   XFillRectangle(display, *p, gc,
                  get_martiancoordx(180), get_martiancoordy(0),
                  get_martiancoordx(60), get_martiancoordy(60));
}


/*--------------------------------------------------------------
 * This routine is used to set the "graphics context".  The
 * graphics context contains information for the windowing system
 * regarding colours, line styles, etc.
 *--------------------------------------------------------------*/

void getGC(Window win, GC *gc)
{
   XGCValues values;

   *gc = XCreateGC(display, win, 0,&values);

   XSetForeground(display,*gc,BlackPixel(display,screen_num));
   XSetBackground(display,*gc,WhitePixel(display,screen_num));
   XSetLineAttributes(display,*gc,0,LineSolid,CapRound,JoinRound);
}

/*--------------------------------------------------------------
 *  This is a routine that can be called from an external file.
 *  It connects to the X-server, does all sorts of initialization,
 *  and then opens the window.   This should be called once at
 *  the start of the simulation.
 *--------------------------------------------------------------*/

void set_up_graphics()
{

   /* Attempt to connect to the server.  If the DISPLAY variable is
      not properly set, an error will occur here */

   display = XOpenDisplay(display_name);
   if (display == (Display *)NULL) {
      printf("Error connecting to display name=%s\n",XDisplayName(display_name));
      exit(0);
   } else {
      printf("Connected to display name=%s\n",XDisplayName(display_name));
   }

   /* Remember the screen number, and the height and width of the
      display we have just connected to */

   screen_num = DefaultScreen(display);
   screen_ptr = DefaultScreenOfDisplay(display);

   display_width = DisplayWidth(display,screen_num);
   display_height = DisplayHeight(display,screen_num);
   printf("Screen size: %d x %d\n",display_width, display_height);

   /* Now, calculate the height and width of the window we should
      create, based on the height and width of the physical screen */

   width = (int)((double)display_width / (double)1.2);
   height = width * MAX_REAL_COORD_Y / MAX_REAL_COORD_X;

   /* Create the window */

   win = XCreateSimpleWindow(display,
                             RootWindow(display,screen_num),
                             0, 0, width, height,
                             border_width,
                             BlackPixel(display,screen_num),
                             WhitePixel(display,screen_num));

   /* Create a pixmap for the Icon that is used whenever the
      window is minimized */

   icon_pixmap = XCreateBitmapFromData(display,
                             win,
                             icon_bitmap_bits,
                             icon_bitmap_width,
                             icon_bitmap_height); 

   /* Create two pixmaps that will be used to store a drawing of
      the rover with its light on and its light off */

   rover_pixmap_lighton = XCreatePixmap(display,win,width, height, 
                                DefaultDepth(display,screen_num));

   rover_pixmap_lightoff = XCreatePixmap(display,win,width, height, 
                                DefaultDepth(display,screen_num));

   rover_pixmap_lighton_flash = XCreatePixmap(display,win,width, height, 
                                DefaultDepth(display,screen_num));

   rover_pixmap_lightoff_flash = XCreatePixmap(display,win,width, height, 
                                DefaultDepth(display,screen_num));

   martian_pixmap = XCreatePixmap(display,win,width, height, 
                                DefaultDepth(display,screen_num));

   /* Some other obvious window parameters */

   window_name = "The UBC Rover";
   icon_name = "EECE 494";

   /* We need to allocate memory for parameters that we will
      pass to XSetWMProperties */

   size_hints = XAllocSizeHints();
   if (size_hints == (XSizeHints *)NULL) {
      printf("Error allocating memory\n"); exit(0);
   }
 
   wm_hints = XAllocWMHints();
   if (wm_hints == (XWMHints *)NULL) {
      printf("Error allocating memory\n"); exit(0);
   }
 
   class_hints = XAllocClassHint();
   if (class_hints == (XClassHint *)NULL) {
      printf("Error allocating memory\n"); exit(0);
   }

   /* Now set the properties of the window that we are opening */

   size_hints->flags = PPosition | PSize | PMinSize;
   size_hints->min_width = 300;
   size_hints->min_height = 200;
   window_big_enough = TRUE;
   XStringListToTextProperty(&window_name, 1, &windowName);
   XStringListToTextProperty(&icon_name, 1, &iconName);
   wm_hints->initial_state = NormalState;
   wm_hints->input = False; 
   wm_hints->icon_pixmap = icon_pixmap;
   wm_hints->flags = StateHint | IconPixmapHint | InputHint;
   class_hints->res_name = "Steve";
   class_hints->res_class = "Steve";

   XSetWMProperties(display, win,
                    &windowName, &iconName,
                    NULL, 0, size_hints,
                    wm_hints, class_hints);

   /* What events will we care about?  In this case, we only
      want to know if the user resizes the window, or if the window
      becomes visible for some reason */

   XSelectInput(display, win,
                ExposureMask | StructureNotifyMask);   

   getGC(win, &gc);

   draw_rover_to_pixmap(&rover_pixmap_lighton,TRUE,FALSE);
   draw_rover_to_pixmap(&rover_pixmap_lightoff,FALSE,FALSE);
   draw_rover_to_pixmap(&rover_pixmap_lighton_flash,TRUE,TRUE);
   draw_rover_to_pixmap(&rover_pixmap_lightoff_flash,FALSE,TRUE);

   draw_martian_to_pixmap(&martian_pixmap);

   /* Finally!  Draw the window */

   XMapWindow(display,win);
   XFlush(display);
}

/*--------------------------------------------------------------
 *  This routine is used to draw the Martian landscape on the
 *  screen.  It is called immediately after the window is opened,
 *  and then only after the window is resized or made visible.
 *  If you want to make a more interesting lanscape, you could 
 *  change this code.
 *--------------------------------------------------------------*/

void update_landscape(BOOL breakleft, BOOL breakright)
{
   int y_surface;
   int y_break1;
   int y_break2;
   int y_bottom;
   int xleft0,xleft1,xleft2,xleft3;
   int xright0,xright1,xright2,xright3;
  
   y_surface = get_screeny(SURFACE_Y);
   y_break1 = get_screeny(SURFACE_BREAK2_Y) ;
   y_break2 = get_screeny(SURFACE_BREAK1_Y);
   y_bottom = height;
   xleft0 = get_screenx(LEFT_CLIFF_POSITION);   
   xleft1 = get_screenx(LEFT_CLIFF_POSITION+SURFACE_BREAKDIFF_X);   
   xleft2 = get_screenx(LEFT_CLIFF_POSITION+3*SURFACE_BREAKDIFF_X);   
   xleft3 = get_screenx(LEFT_CLIFF_POSITION+5*SURFACE_BREAKDIFF_X);   
   xright0 = get_screenx(RIGHT_CLIFF_POSITION);   
   xright1 = get_screenx(RIGHT_CLIFF_POSITION-SURFACE_BREAKDIFF_X);   
   xright2 = get_screenx(RIGHT_CLIFF_POSITION-3*SURFACE_BREAKDIFF_X);   
   xright3 = get_screenx(RIGHT_CLIFF_POSITION-5*SURFACE_BREAKDIFF_X);   

   XSetForeground(display,gc,WhitePixel(display,screen_num));
   XFillRectangle(display,win,gc,0,0,width,height);
   XSetForeground(display,gc,BlackPixel(display,screen_num));

   XDrawLine(display,win,gc,xleft3,y_bottom,xleft2,y_break2);
   if (!breakleft) {
      XDrawLine(display,win,gc,xleft2,y_break2,xleft1,y_break1);
      XDrawLine(display,win,gc,xleft1,y_break1,xleft0,y_surface);
      XDrawLine(display,win,gc,xleft0,y_surface,xleft2,y_surface);
   } else {
      XDrawLine(display,win,gc,xleft2,y_break2,xleft2,y_surface);
   }
   XDrawLine(display,win,gc,xleft2,y_surface,xright2,y_surface);

   if (!breakright) {
      XDrawLine(display,win,gc,xright2,y_break2,xright1,y_break1);
      XDrawLine(display,win,gc,xright1,y_break1,xright0,y_surface);
      XDrawLine(display,win,gc,xright0,y_surface,xright2,y_surface);
   } else {
      XDrawLine(display,win,gc,xright2,y_break2,xright2,y_surface);
   }
   XDrawLine(display,win,gc,xright3,y_bottom,xright2,y_break2);
}


/*--------------------------------------------------------------
 * This routine is used to update the graphics rover on the screen.
 * It is needed whenever the rover moves, whenever the light changes
 * state, or when an Expose or Resize event occurs.  Note that the
 * rover is not actually drawn using primitive commands; instead,
 * the pixmap that was created ealier is copied to the window
 * (this speeds things up)
 *--------------------------------------------------------------*/

void update_graphics_rover()
{
   int position;
   int martian_position;
   BOOL martian_on;
   BOOL lighton;
   BOOL flash;

   /* Get the position of the rover.  Make sure we have
      exlusive access to this variable so we can be sure it
      doesn't change while we are in the middle of accessing it.
      Also, get the current status of the light */

   pthread_mutex_lock(&rover_status.mutex);
   position = rover_status.location;
   martian_position = rover_status.martian_position;
   martian_on = rover_status.martian_on;
   lighton = rover_status.light;
   flash = rover_status.flash;
   if (flash) {
      rover_status.flash = FALSE;
      if (martian_on) {
         printf("Clicked a martian.  Good work.\n");
         rover_status.martian_on = FALSE;
      } else {
         printf("No martian.  You wasted a picture.\n");
      }
   }
   pthread_mutex_unlock(&rover_status.mutex);

   /* Erase the old rover */

   XClearArea(display,win,
              0,
              get_screeny(SURFACE_Y-ROVER_SIZE),
              width,
              get_screeny(ROVER_SIZE),FALSE);

   /* Copy the new rover to the window */

   if (flash) {               
      XCopyArea(display, 
             lighton?rover_pixmap_lighton_flash:rover_pixmap_lightoff_flash, 
             win, gc, 
             0,0,
             get_screenx(ROVER_SIZE), 
             get_screeny(ROVER_SIZE),
             get_screenx(position-ROVER_SIZE),
             get_screeny(SURFACE_Y-ROVER_SIZE));
   } else {
      XCopyArea(display, 
             lighton?rover_pixmap_lighton:rover_pixmap_lightoff, 
             win, gc, 
             0,0,
             get_screenx(ROVER_SIZE), 
             get_screeny(ROVER_SIZE),
             get_screenx(position-ROVER_SIZE),
             get_screeny(SURFACE_Y-ROVER_SIZE));

   /* Erase the old martian */

   XClearArea(display,win,
              get_martianrowx(0),
              get_screeny(SURFACE_Y+MARTIAN_SIZE),
              get_martianrowx(100)-get_martianrowx(0),
              get_screeny(MARTIAN_SIZE),FALSE);

   }

   /* Draw the martian */
   if (martian_on) {

      XCopyArea(display, 
             martian_pixmap,
             win, gc, 
             0,0,
             get_screenx(MARTIAN_SIZE), 
             get_screeny(MARTIAN_SIZE),
             get_martianrowx(martian_position)-MARTIAN_SIZE,
             get_screeny(SURFACE_Y+MARTIAN_SIZE));

   }
   XFlush(display);
}

/*--------------------------------------------------------------
 * This routine is used to update both the rover and the landscape.
 * It is only needed after a Resize or Expose event (most of the
 * time, the update_graphics_rover routine should be called directly
 * since the landscape never changes)
 *--------------------------------------------------------------*/

void update_graphics()
{
   update_landscape(FALSE, FALSE);
   update_graphics_rover();
}


/*--------------------------------------------------------------
 * A simple animation for when the rover goes off a cliff.
 *--------------------------------------------------------------*/

void rover_off_cliff(BOOL left)
{
   int position, yval, cnt;
   BOOL lighton;
   struct timespec delay_time, what_time_is_it;

   delay_time.tv_sec = 0;
   delay_time.tv_nsec = 1000000000/60; 

   pthread_mutex_lock(&rover_status.mutex);
   position = rover_status.location;
   lighton = rover_status.light;
   pthread_mutex_unlock(&rover_status.mutex);

   update_landscape(left, !left);   
   XClearArea(display,win,
              0,
              get_screeny(SURFACE_Y-ROVER_SIZE),
              width,
              get_screeny(ROVER_SIZE),FALSE);

   cnt = 0;
   for(yval = get_screeny(SURFACE_Y-ROVER_SIZE); yval < height; yval=yval+MOVE_STEP) {
      if ((cnt++) % 2 == 0) {
        XCopyArea(display, 
               lighton?rover_pixmap_lighton:rover_pixmap_lightoff, 
               win, gc, 
               0,0,
               get_screenx(ROVER_SIZE), 
               get_screeny(ROVER_SIZE),
               get_screenx(position-ROVER_SIZE),
               yval);
      } else {
        XCopyArea(display, 
               lighton?rover_pixmap_lighton_flash:rover_pixmap_lightoff_flash, 
               win, gc, 
               0,0,
               get_screenx(ROVER_SIZE), 
               get_screeny(ROVER_SIZE),
               get_screenx(position-ROVER_SIZE),
               yval);

      }
      XFlush(display);
      nanosleep(&delay_time,&what_time_is_it);
      XClearArea(display,win,
              get_screenx(position-ROVER_SIZE),
              0,
              get_screenx(ROVER_SIZE),
              height,FALSE);
   }
}


/*--------------------------------------------------------------
 * This routine runs as a separate thread.  It simply waits for 
 * an event from X, and responds to it when it arrives.  The only
 * two events we care about are an Expose event or a Resize event.
 *--------------------------------------------------------------*/

void *graphics_thread()
{
   XEvent report;

   while(1) {
  
      /* Wait for an event.  This is non-blocking */

      XNextEvent(display, &report);

      /* Do something based on the type of the event */

      switch(report.type) {

         /* If it was an expose event, the window is now
            visible (when it wasn't before).  In this case,
            we need to redraw all the graphics in the window */

         case Expose:
            if (report.xexpose.count != 0) break;
            if (window_big_enough) {
               update_graphics();
	    }
            break;

         /* If it was a resize event, we need to calcuate the
            new size of the window.  We then need to not only
            redraw all the graphics, but re-create the rover
            pixmaps for their new size */

         case ConfigureNotify:
            width = report.xconfigure.width;
            height = report.xconfigure.height;
            if ((width < size_hints->min_width) ||
                (height < size_hints->min_height)) {
               window_big_enough = FALSE;
	    } else {
               window_big_enough = TRUE;               
 
               /* Re-create rover and martians pixmaps */

               draw_rover_to_pixmap(&rover_pixmap_lighton,TRUE,FALSE);
               draw_rover_to_pixmap(&rover_pixmap_lightoff,FALSE,FALSE);
               draw_rover_to_pixmap(&rover_pixmap_lighton_flash,TRUE,TRUE);
               draw_rover_to_pixmap(&rover_pixmap_lightoff_flash,FALSE,TRUE);

               draw_martian_to_pixmap(&martian_pixmap);

               /* Update all the graphics */

               update_graphics();
	    }
            break;
         default: 
			;
            /* Ignore any other events */
      }
   }
 
   /* We never actually return, but this keeps the
      compiler happy */

   return( (void *)NULL);
}


/*--------------------------------------------------------------
 *
 *  This routine runs the Graphics Thread.  It simply starts the 
 *  thread and then "detaches" it.  Detaching it is necessary
 *  so that when the program ends, the thread doesn't continue
 *  chewing up system resources.  It does mean, however, that
 *  one can not control this thread any more.
 *
 *--------------------------------------------------------------*/

void run_graphics_thread()
{
   int status;

   /* Create the thread.  The routine "graphics_thread" (defined
      above) will be run in the new thread. */

   status = pthread_create(&graphics_thread_id,
              NULL,
              graphics_thread,
              NULL);
   if (status != 0) {
      printf("Error creating graphics thread: status = %d\n",status);
      exit(0);
   }
 
   /* Detatch the thread */

   status = pthread_detach(graphics_thread_id);
   if (status != 0) {
      printf("Error detaching graphics thread: status = %d\n",status);
      exit(0);
   }
}


/*--------------------------------------------------------------
 *
 *  This routine simply closes the X-window
 *
 *--------------------------------------------------------------*/

void end_graphics()
{
   XCloseDisplay(display);
}
