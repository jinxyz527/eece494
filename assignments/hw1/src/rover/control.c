
/*******************************************************
 *
 *  Skeleton files for the controller for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *  
 *	Modified by: _________________________________________
 *
 *  This file is where your code will go.
 *
 ******************************************************/


#include "defs.h"
#include "rover.h"
#include "ext.h"


void turn_on_light()
{
   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.light = TRUE;
   pthread_mutex_unlock(&rover_actuators.mutex);
}
void turn_off_light()
{
   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.light = FALSE;
   pthread_mutex_unlock(&rover_actuators.mutex);
}
void go_forward()
{
   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.forward = TRUE;
   pthread_mutex_unlock(&rover_actuators.mutex);
}
void take_picture()
{
   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.camera = TRUE;
   pthread_mutex_unlock(&rover_actuators.mutex);
}
void go_backward()
{
   pthread_mutex_lock(&rover_actuators.mutex);
   rover_actuators.forward = FALSE;
   pthread_mutex_unlock(&rover_actuators.mutex);
}
BOOL backward_sensor()
{
   BOOL retval;

   pthread_mutex_lock(&rover_sensors.mutex);
   retval = rover_sensors.left;
   pthread_mutex_unlock(&rover_sensors.mutex);
   return(retval);
}
BOOL life_sensor()
{
   BOOL retval;

   pthread_mutex_lock(&rover_sensors.mutex);
   retval = rover_sensors.life_sign;
   pthread_mutex_unlock(&rover_sensors.mutex);
   return(retval);
}
BOOL forward_sensor()
{
   BOOL retval;

   pthread_mutex_lock(&rover_sensors.mutex);
   retval = rover_sensors.right;
   pthread_mutex_unlock(&rover_sensors.mutex);
   return(retval);
}
BOOL alive_sensor()
{
   BOOL retval;

   pthread_mutex_lock(&rover_sensors.mutex);
   retval = rover_sensors.alive;
   pthread_mutex_unlock(&rover_sensors.mutex);
   return(retval);
}


/*--------------------------------------------------------------*/



/****************************************************************
 *
 * This routine is the thread that controls the flashing light 
 * All this thread does is turn on and off the light according 
 * to a predefined pattern, and sleeps for one second between 
 * each action.  This repeats until the alive_sensor() indicates 
 * that the rover is dead, at which time the thread exits.  
 *
 ****************************************************************/                                                         

void *flash_light_thread()
{
   /* Write some code here */
}
 
/*--------------------------------------------------------------*/

/****************************************************************
 *
 * This routine is the thread that controls the motor.
 * Every second, the thread checks the forward or backwards
 * sensor, and adjusts the direction accordingly.
 * This repeats until the alive_sensor() indicates 
 * that the rover is dead, at which time the thread exits.  
 *
 ****************************************************************/                                                         
 
void *movement_thread()
{
   /* Write some code here */
}



/*--------------------------------------------------------------*/

/****************************************************************
 *
 * This routine is the thread that controls the digital camera.
 * Every second, the thread checks the alive 
 * sensor, and takes a picture of a Martian is detected.
 * This repeats until the alive_sensor() indicates 
 * that the rover is dead, at which time the thread exits.  
 *
 ****************************************************************/                                                         
 
void *camera_thread()
{
   /* Write some code here */
}


 
/*--------------------------------------------------------------*/

/****************************************************************
 *
 *  This routine is the entry routine for your controller.
 *  It sets up the three threads, and then waits until the rover
 *  dies.  
 *
 ****************************************************************/                                                         

void your_code()
{
   /* Write some code here */
}

