
/*******************************************************
 *
 *  Global External File for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *
 *  This routine contains external references to the global
 *  variables in the system.  Global variables are used because
 *  we want to use shared memory to communicate between tasks
 *  (other methods will be discussed in class).  These variables
 *  are all defined in main.c, but by including this file, 
 *  other threads can access the variables directly.
 *
 ******************************************************/


typedef struct rover_actuators_t {
   pthread_mutex_t mutex;
   BOOL forward;
   BOOL light;
   BOOL camera;
} rover_actuators_t;

typedef struct rover_sensors_t {
   pthread_mutex_t mutex;
   BOOL left;
   BOOL right;
   BOOL alive;
   BOOL life_sign;
} rover_sensors_t;

typedef struct rover_status_t {
   pthread_mutex_t mutex;
   int location;
   BOOL light;
   BOOL flash;
   BOOL martian_on;
   int martian_position;
} rover_status_t;


extern rover_status_t rover_status;
extern rover_actuators_t rover_actuators;
extern rover_sensors_t rover_sensors;
