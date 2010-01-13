
/*******************************************************
 *
 *  Skeleton files for the controller for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *  
 *	Modified by: Jason Poon
 *
 *  This file is where your code will go.
 *
 ******************************************************/


#include "defs.h"
#include "rover.h"
#include "ext.h"

#define light_dash() \
    ({turn_on_light();\
      sleep(2);\
      turn_off_light();\
      sleep(1);})
#define light_dot() \
    ({ turn_on_light();\
       sleep(1);\
       turn_off_light();\
       sleep(1);})
typedef struct sensor_value_t {
    pthread_mutex_t mutex;
    BOOL sensorValue;
} sensor_value_t;

sensor_value_t is_alive = {
    PTHREAD_MUTEX_INITIALIZER,
    TRUE 
};

pthread_t create_thread(void *);
void monitor_sensor(sensor_value_t, BOOL (*) ());

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
     BOOL isAlive;

     while(1) {
        pthread_mutex_lock(&is_alive.mutex);
        isAlive = is_alive.sensorValue;
        pthread_mutex_unlock(&is_alive.mutex);

        if (isAlive) {
            light_dot();
            light_dot();
            light_dot();
            light_dot();
            sleep(1);
            light_dot();
            sleep(1);
            light_dot();
            light_dash();
            light_dot();
            light_dot();
            sleep(1);
            light_dot();
            light_dash();
            light_dash();
            light_dot();
            sleep(1);
        } else {
            break;
        }

        sleep(1);
    }
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
    BOOL isAlive;

    while(1) {
        pthread_mutex_lock(&is_alive.mutex);
        isAlive = is_alive.sensorValue;
        pthread_mutex_unlock(&is_alive.mutex);

        if (isAlive) {
            if (backward_sensor()) {
                go_forward();
            } 
            if (forward_sensor()) {
                go_backward();
            }
        } else {
            break;
        }

        sleep(1);
    }
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
    BOOL isAlive;

    while(1) {
        pthread_mutex_lock(&is_alive.mutex);
        isAlive = is_alive.sensorValue;
        pthread_mutex_unlock(&is_alive.mutex);

        if (isAlive) {
            if(life_sensor()) take_picture();
        } else {
            break;
        }

        sleep(1);
    }
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
    create_thread(camera_thread);
    create_thread(movement_thread);
    create_thread(flash_light_thread);

    while(1) {
        monitor_sensor(is_alive, &alive_sensor);
    }

}

pthread_t create_thread(void *func)
{
    pthread_t thread_id;
    int status;
    
    status = pthread_create(&thread_id,
                           NULL,
                           func,
                           NULL);
    if (status != 0) {
        printf("Error creating thread: status = %d\n",status);
        exit(0);
    }
    
    status = pthread_detach(thread_id);
    if (status != 0) {
        printf("Error detaching thread: status = %d\n",status);
        exit(0);
    }

    return thread_id;
}

void monitor_sensor(sensor_value_t sensor, BOOL (*sensor_func) ()) {
    pthread_mutex_lock(&sensor.mutex);
    is_alive.sensorValue= (*sensor_func)();
    pthread_mutex_unlock(&is_alive.mutex);

    sleep(1);
 
}
