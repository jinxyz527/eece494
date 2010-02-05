

/*******************************************************
 *
 *  Main routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 ******************************************************/

#include "defs.h"

/* Shared variables among all threads */

port_t in_port[4];
port_t out_port[4];
BOOL die;

main()
{
   int i,status;
   pthread_t switch_thread_id;
   pthread_t harness_thread_id;

   printf("Initializing ports\n");

   for(i=0;i<4;i++) {
      port_init( &(in_port[i]));
      port_init( &(out_port[i]));
   }

   printf("Initializing switch\n");

   switch_init();

   printf("Initializing harness\n");
  
   harness_init();

   die = FALSE;

   printf("Starting switch thread\n");
   status = pthread_create(&switch_thread_id,
                           NULL,
                           switch_thread_routine,
                           NULL);

   if (status != 0) {
      printf("Error creating switch thread\n");
      exit(0);
   }

   status = pthread_detach(switch_thread_id);
   if (status != 0) {
      printf("Error detaching switch thread\n");
      exit(0);
   }


   printf("Starting harness thread\n");
   status = pthread_create(&harness_thread_id,
                           NULL,
                           harness_thread_routine,
                           NULL);

   if (status != 0) {
      printf("Error creating harness thread\n");
      exit(0);
   }

   status = pthread_detach(harness_thread_id);
   if (status != 0) {
      printf("Error detaching harness thread\n");
      exit(0);
   }

   while (!die) {
      sleep(1);
   }

   printf("Main program is now done\n");

   harness_end();   
   switch_free();   
}
