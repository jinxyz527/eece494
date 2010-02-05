/*******************************************************
 *
 *  Main routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 ******************************************************/

#include "defs.h"

/* The following defines how many entries should be stored
   in the routing table at the start.  For some implementations
   of the routing table, this will have no effect on the 
   performance, while for others, it will.  */

#define NUMBER_ENTRIES_IN_ROUTING_TABLE 40000

/* The following indicate how many test cases we should
   run when performing timing measurements.  If you find it
   runs too slowly, you can make this smaller, or if you find
   it runs too fast to get accurate measurements, you can make
   this bigger */

#define NUMBER_LOOKUPS 50000

/* This structure is used by test bench to remember what
   is stored in the routing table.  It is used to ensure that
   only valid IP addreses are sent to the routing table.  Again,
   note that this is *not part of the routing table*, only part of
   the test bench */

typedef struct my_version_of_the_table_t {
   ip_address_t address;
   int port;
} my_version_of_the_table_t;


BOOL is_this_entry_in_the_table(my_version_of_the_table_t *table,
                                int table_size,
                                int n1,
                                int n2,
                                int n3,
                                int n4)
{
   int i;

   for(i=0;i<table_size;i++) {
      if ((n1 == table[i].address.n1) &&
          (n2 == table[i].address.n2) &&
          (n3 == table[i].address.n3) &&
          (n4 == table[i].address.n4)) {
         return(TRUE);
      }
   }
   return(FALSE);
}


main()
{
   int i;
   ip_address_t address;
   int port;
   int entry;
   double accum;
   struct timespec start_time, end_time;
   my_version_of_the_table_t my_version_of_the_table[NUMBER_ENTRIES_IN_ROUTING_TABLE];
   BOOL ok;

   /* Initalize the routing table by calling your routine */
   
   printf("Initializing the Routing Table\n");  
   cam_init();

   /* Now go through and set up the routing table.  */
   
   printf("Adding %d entries to the routing table\n", NUMBER_ENTRIES_IN_ROUTING_TABLE);

   for(i=0;i<NUMBER_ENTRIES_IN_ROUTING_TABLE;i++) {
 
      /* Choose an IP address randomly */

      ok = FALSE;
      while(!ok) {
         address.n1 = random()%256;
         address.n2 = random()%256;
         address.n3 = random()%256;
         address.n4 = random()%255;
         ok = !is_this_entry_in_the_table(my_version_of_the_table,i,
                                                     address.n1,
                                                     address.n2,
                                                     address.n3,
                                                     address.n4);
      }
 
      /* Choose an output port randomly */

      port = random()%4;

      /* Add this entry to the routing table by calling your routine */

      cam_add_entry(&address,port);

      /* Save the address locally to the testbench.  This will help us
         choose valid IP addresses during our tests */

      ip_address_copy( & address,
                       & (my_version_of_the_table[i].address));
      my_version_of_the_table[i].port = port;
   }

   /* Read the start time */

   if (clock_gettime(CLOCK_REALTIME,&start_time) == -1) {
       printf("Error reading clock\n");
       exit(0);
   } 

   /* Perform the actual test */

   printf("Performing %d lookups\n", NUMBER_LOOKUPS);

   for(i=0;i<NUMBER_LOOKUPS;i++) {

      /* Choose a valid IP address at random */

      entry = random()%NUMBER_ENTRIES_IN_ROUTING_TABLE;

      /* Call your routine to look up the entry in the routing table
         (this is the part that you want to be as fast as possible */

      port = cam_lookup_address(
              &(my_version_of_the_table[entry].address));

      /* Make sure your routine returned the right value */

      if (port != my_version_of_the_table[entry].port) {
         printf("Error: cam_lookup_address returned the wrong output port.  You really need to fix this.\n");
      }
   }

   /* Measure the end time */

   if (clock_gettime(CLOCK_REALTIME,&end_time) == -1) {
       printf("Error reading clock\n");
       exit(0);
   } 

   accum = (double)(end_time.tv_sec - start_time.tv_sec) +
       (double)(end_time.tv_nsec - start_time.tv_nsec)/1000000000.0;

   printf("Time difference: %f seconds\n",accum);
   printf("That is %f microseconds per lookup\n",
                 1000.0 * 1000.0 * accum / NUMBER_LOOKUPS);
                                     
   cam_free();
}
