
/*******************************************************
 *
 *  Main routine for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *
 *  This file contains the entry routine for your assignment.
 *  It simply sets up the graphics window, starts
 *  the rover task running, and then calls your loop.
 *
 ******************************************************/

#include "defs.h"
#include "graphics.h"
#include "rover.h"
#include "ext.h"

/* These are global variables that are used throughout the program.
   They are defined as global variables because we want to use
   shared memory to communicate between tasks.  Other possibilities
   will be described later in the course */

rover_status_t rover_status = {PTHREAD_MUTEX_INITIALIZER,0};
rover_actuators_t rover_actuators = {PTHREAD_MUTEX_INITIALIZER,FALSE,FALSE};
rover_sensors_t rover_sensors = {PTHREAD_MUTEX_INITIALIZER,FALSE,FALSE};

main()
{
   initialize_rover();   
   set_up_graphics();
   run_rover_thread();
   run_graphics_thread();
   your_main_loop();
   end_graphics();

   printf("The UBC Rover has been destroyed.\n");
   printf("Please insert another $820,000,000 into the coin slot and try again.\n");
}
