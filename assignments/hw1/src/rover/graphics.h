
/*******************************************************
 *
 *  Graphics Header File for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *
 *  This routine contains coordinates describing the Mars
 *  landscape as well as the graphical view of the rover.
 *  In addition, prototypes for some routines in graphics.c
 *  are provided.
 *
 ******************************************************/


/* All coordinates below are relative to these dimensions.
   Note that changing these does not affect the size of the
   window on the screen.  */

#define MAX_REAL_COORD_X 1000
#define MAX_REAL_COORD_Y 500

/* Define the Martian Landscape.  */

#define SURFACE_Y 300             /* Y coordinate of the surface */
#define SURFACE_BREAK1_Y 340      /* Y coordinate of one break of a cliff */
#define SURFACE_BREAK2_Y 360      /* Y coordinate of other break of cliff */
#define SURFACE_BREAKDIFF_X 10    /* X size of cliff break */
#define LEFT_CLIFF_POSITION 200   /* Left cliff position */
#define RIGHT_CLIFF_POSITION 800  /* Right cliff position */

/* Define the Graphical view of the Rover */

#define SENSOR_WARNING 0         /* How close to the cliff you need to
                                     be before sensor notices the cliff */
#define ROVER_SIZE 70             /* Size of the Rover on the screen */
#define ROVER_COORD 300           /* Rover is drawn according to this 
                                     coordinate size (don't change this) */


/* Define the Graphical view of the Martian */

#define MARTIAN_SIZE 50             /* Size of the Rover on the screen */
#define MARTIAN_COORD 300           /* Rover is drawn according to this 
                                     coordinate size (don't change this) */


/* Prototypes for the functions in graphics.c that routines
   in other files might want to call.  Only the routines listed
   here will be visible outside graphics.c */

void set_up_graphics();
void run_graphics_thread();
void end_graphics();
void update_graphics_rover();
void rover_off_cliff(BOOL left);
