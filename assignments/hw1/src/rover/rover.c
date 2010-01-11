
/*******************************************************
 *
 *  Rover simulation routines for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan 
 *
 *  This file contains all the routines for the rover simulation.
 *  As far as the assignment goes, you don't really need to look in
 *  this file, but if you do, you might find it interesting.
 *
 ******************************************************/


#include "defs.h"
#include "rover.h"
#include "graphics.h"
#include "ext.h"

/* The rover controller runs as a separate p-thread.  Once we
   talk about p-threads in class, you will understand why this
   is a good thing.  Basically, it means that the rover simulator
   can operate "concurrently" with the controller and graphics
   subsystem */

pthread_t rover_thread_id;


/*--------------------------------------------------------------
 *
 *  This is where all the exciting stuff happens in the rover
 *  simulator.  It is a simple thread that loops forever (or
 *  until the rover is dead).  Each time through the loop, it
 *  checks the actuators, and updates the sensors and status.
 *  Then it sleeps for a second, and repeats.  You wouldn't call
 *  this routine directly from anywhere; instead, the routine is
 *  used as an argument to the pthread_create routine below.
 *  Thus, it runs as its own thread.
 *
 *--------------------------------------------------------------*/

void *rover_thread()
{
   BOOL done = FALSE;
   BOOL must_redraw_position;
   BOOL must_redraw_light;
   BOOL must_show_crash_animation;
   BOOL must_update_flash;
   BOOL crash_left;
   BOOL life;
   int cnt;
   struct timespec delay_time, what_time_is_it;

   delay_time.tv_sec = 0;
   delay_time.tv_nsec = 1000000000/2; 
   life = FALSE;

   while(!done) {

      must_redraw_light = FALSE;
      must_redraw_position = FALSE;
      must_show_crash_animation = FALSE;
      must_update_flash = TRUE;

      /* First, we will need access to the status of the rover.
         Obtain the lock; this way no one can attempt to read it
         while we are possibly modifying the values.  The need for
         this sort of mutual exclusion will become clear later in
         the course */

      pthread_mutex_lock(&rover_status.mutex);

      /* Now obtain a lock for the actuators.  Again, this means
         that no one else can modify the actuators while we
         are dealing with them */

      pthread_mutex_lock(&rover_actuators.mutex);

      /* Check the camera actuator.  If it is true, set FLASH
         to true, and set a flag so we know we have to redraw
         the rover during this time step.  Also, turn off the
         camera actuator */

      if (rover_actuators.camera) {
         rover_actuators.camera = FALSE;
         rover_status.flash = TRUE;
         must_update_flash = TRUE;
      } 

      /* If the martian is on, consider turning it off */
      
      if (rover_status.martian_on) {
         life = TRUE;
         if (random()%100 < 10) {
            rover_status.martian_on = FALSE;
            must_redraw_position = TRUE;
            life = FALSE;
	 }
      } else {
         life = FALSE;
         if (random()%100 < 10) {
            rover_status.martian_on = TRUE;
            rover_status.martian_position = 40+(random()%40);
            must_redraw_position = TRUE;
            life = TRUE;
	 }
      }

      /* Check the light actuator.  If it doesn't match the
         current status of the light, change the current status
         of the light, and set a flag so we know we have to
         redraw the rover during this time step */

      if (rover_status.light != rover_actuators.light) {
         rover_status.light = rover_actuators.light;
         must_redraw_light = TRUE;
      }

      /* Check the status of the "forward" actuator.
         If it is true, move the rover forward.  If it is
         false, move the rover backwards.  In either case,
         set a flag indicating we have to redraw the rover. */

      if (rover_actuators.forward) { 
         rover_status.location += MOVE_STEP;      
      } else {
         rover_status.location -= MOVE_STEP;
      }
      must_redraw_position = TRUE;

      /* Remember where we are.  We do this so that we can free the
         actuator mutex as soon as possible */

      cnt = rover_status.location;

      /* Unlock the actuator mutex so that others can use it. */

      pthread_mutex_unlock(&rover_actuators.mutex);

      /* Now obtain a lock for the sensors.  Again, this means
         that no one else can read the sensors while we
         are dealing with them */

      pthread_mutex_lock(&rover_sensors.mutex);

      /* See if we are close to a cliff (either forwards or backwards).
         Set the appropriate sensors appropriately */

      rover_sensors.right = (BOOL)
                (cnt >= RIGHT_CLIFF_POSITION - SENSOR_WARNING);
      rover_sensors.left = (BOOL)
                (cnt <= LEFT_CLIFF_POSITION + SENSOR_WARNING + ROVER_SIZE);

      /* Now check our status.  If we are too near the edge of the
         cliff, we are dead.  */

      if (cnt >= RIGHT_CLIFF_POSITION - 3*SURFACE_BREAKDIFF_X + ROVER_SIZE) {
         rover_sensors.alive = FALSE;
         must_show_crash_animation = TRUE;
         crash_left = FALSE;
      }

      if (cnt <= LEFT_CLIFF_POSITION + 3*SURFACE_BREAKDIFF_X) {
         rover_sensors.alive = FALSE;
         must_show_crash_animation = TRUE;
         crash_left = TRUE;
      }

      rover_sensors.life_sign = life;


      /* Unlock the sensors so that others can access them */

      pthread_mutex_unlock(&rover_sensors.mutex);

      /* Unlock the rover status information, so others can 
         access it.  In particular, the graphics routines will
         need to access this information. */

      pthread_mutex_unlock(&rover_status.mutex);



      /* If the light or position has changed, redraw the rover */

      if (must_redraw_position ||
          must_redraw_light ||
          must_update_flash) update_graphics_rover();

      if (must_show_crash_animation) {
         rover_off_cliff(crash_left);
         done = TRUE;
      }

      /* Go to sleep for 1 second.  This will give other threads
         a chance to run */

      nanosleep(&delay_time, &what_time_is_it);
   }
}


/*--------------------------------------------------------------
 *  
 *  The following routine simply sets initial values for all the
 *  actuators, sensors, and rover status variables.  It is called
 *  once at the start of the simulation.  Since the status, 
 *  actuators, and sensors are part of a mutex, we need to 
 *  obtain the lock before modifying them, and free the lock after
 *  modifying them.  This makes sure that no other thread is 
 *  able to read or write to these values while we are changing
 *  them.  It probably isn't strictly necessary here, but it is
 *  good practice anyway.
 *
 *--------------------------------------------------------------*/

void initialize_rover()
{
   /* Set the initial status of the rover */

   pthread_mutex_lock(&rover_status.mutex);
   rover_status.location = INITIAL_POSITION;
   rover_status.light = INITIAL_LIGHT;
   rover_status.martian_position = 50;
   rover_status.martian_on = FALSE;
   rover_status.flash = FALSE;
   pthread_mutex_unlock(&rover_status.mutex);

   /* Set the initial status of all the actuators */

   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.forward = INITIAL_FORWARD;
   rover_actuators.light = INITIAL_LIGHT;
   rover_actuators.camera = FALSE;
   pthread_mutex_unlock(&rover_actuators.mutex);

   /* Set the initial status of all the sensors */

   pthread_mutex_lock(&rover_sensors.mutex);
   rover_sensors.right = FALSE;
   rover_sensors.left = FALSE;
   rover_sensors.alive = TRUE;
   rover_sensors.life_sign = FALSE;
   pthread_mutex_unlock(&rover_sensors.mutex);
}


/*--------------------------------------------------------------
 *
 *  This routine runs the rover Thread.  It simply starts the 
 *  thread and then "detaches" it.  Detaching it is necessary
 *  so that when the program ends, the thread doesn't continue
 *  chewing up system resources.  It does mean, however, that
 *  one can not control this thread any more.
 *
 *--------------------------------------------------------------*/

void run_rover_thread()
{
   int status;

   /* Start the thread going.  The routine "rover_thread" (defined
      above) will start executing as a new thread.  If the creation
      of the thread fails, and error will be printed, and the program
      is over.  The variable rover_thread_id will be used to
      refer to this thread. */

   status = pthread_create(&rover_thread_id,
              NULL,
              rover_thread,
              NULL);
   if (status != 0) {
      printf("Error creating rover thread: status = %d\n",status);
      exit(0);
   }

   /* Now detach the thread.  The thread still runs, but when the
      program ends, the thread will die and not retain any
      system resources. */

   status = pthread_detach(rover_thread_id);
   if (status != 0) {
      printf("Error detaching rover thread: status = %d\n",status);
      exit(0);
   }
}
