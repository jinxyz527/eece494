
/*******************************************************
 *
 *  Rover Header File for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *
 *  This file is the header file for the Rover simulation
 *  routines.  
 *
 ******************************************************/


/* Some initial values you can change if you want */

#define INITIAL_FORWARD TRUE  /* Which direction we start in */
#define INITIAL_LIGHT FALSE   /* Whether the light starts on or off */
#define INITIAL_POSITION 400  /* Starting position of the rover */
#define MOVE_STEP 5           /* Speed of rover */

/* Prototypes for the functions in rover.c that routines
   in other files might want to call.  Only routines listed
   here will be visible outside rover.c */

void initialize_rover();
void run_rover_thread();

