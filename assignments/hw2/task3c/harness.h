
/*******************************************************
 *
 *  Harness test routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 *  This file contains the definitions for the test harness
 *  part of Assignment 2.  You probably don't need to change anything
 *  here, unless you want to change something for testing 
 *  purposes.  If you do change anything, make sure that when
 *  you are done it works with the original test harness, since
 *  that is what we will use to mark your code.
 *
 ******************************************************/


#define NUMBER_ENTRIES_IN_ROUTING_TABLE 20000  /* How many IP addresses
                                                  to load into the routing table */
#define NUMBER_PACKETS_TO_SEND 1000    /* How many packets to send during the test */
#define SEND_DENSITY 40              /* A Density of 100 means that a packet is
                                        sent through every input port every cycle.
                                        A Density of less than 100 indicates the
                                        percentage of cycles in which a packet
                                        is sent through each port.  The higher
                                        the number, the heavier the traffic,
                                        meaning you have to worry more about
                                        dropped packets */
#define HARNESS_SPEED (1000000000/10)  /* How fast the harness should run (in ns) */
#define HARNESS_DEBUG 0            /* 1 means debugging messages are printed */

/*------------------------------------------------------------*
 *                                                            *
 *  This routine initializes the harness data structures.     *
 *  It is called once before the start of the test.           *
 *                                                            *
 *------------------------------------------------------------*/

void harness_init();

/*------------------------------------------------------------*
 *                                                            *
 *  The test harness runs as a single thread.  This is the    * 
 *  thread routine.  The thread will be set up in main.c.     *
 *  The thread has two purposes: to send packets at random    *
 *  through the input ports, and extract packets from the     *
 *  output ports as they appear.  The thread keeps track of   *
 *  how many sent packets are recieved; from this, the number *
 *  of dropped packets can be calculated.                     *
 *                                                            *
 *------------------------------------------------------------*/

void *harness_thread_routine(void *arg);

/*------------------------------------------------------------*
 *                                                            *
 *  This routine frees the harness data structures.           *
 *  It is called once at the end of the test.                 *
 *                                                            *
 *------------------------------------------------------------*/

void harness_end();                       
