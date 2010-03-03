
/*******************************************************
 *
 *  Main routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 *  This file contains miscellaneous routines that might
 *  be useful in your assignment.  You don't need to use
 *  any of them.  In particular, this file provides routines
 *  that will allow easy manipulations of variables of
 *  the following types:
 *
 *    ip_address_t 
 *    port_t 
 *    packet_t
 *
 *  Each of these types are defined in misc.h.  You should
 *  really start by reading misc.h
 *
 ******************************************************/

#include "defs.h"




/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to copy one variable of type       *
 * ip_address_t to another variable of type ip_address_t.  *
 * Note that both the source and destination parameters    *
 * are passed by REFERENCE.  In other words, addresses are *
 * passed to the two variables rather than the variables   *
 * themselves.                                             *
 *                                                         *
 * To use this routine, you might imagine code that looks  *
 * something like this:                                    *
 *    ip_address_t  s, d;                                  *
 *    ..                                                   *
 *    ip_address_copy ( &s, &d )                           *
 *                                                         *
 * The ampersand is used to pass the address of s and d    *
 * rather than s and d themselves.  If this confuses       *
 * you, see any C book for a more detailed explanation.    *
 *                                                         *
 *---------------------------------------------------------*/

void ip_address_copy( ip_address_t *source_address_ptr,
                      ip_address_t *dest_address_ptr)
{
   /* Error check on the input parameters */

   if ((source_address_ptr == (ip_address_t *)NULL) ||
       (dest_address_ptr == (ip_address_t *)NULL)) {
      printf("Error in ip_address_copy\n");
      exit(0);
   }
 
   /* Copy all the fields individually */

   dest_address_ptr->n1 = source_address_ptr->n1;
   dest_address_ptr->n2 = source_address_ptr->n2;
   dest_address_ptr->n3 = source_address_ptr->n3;
   dest_address_ptr->n4 = source_address_ptr->n4;
}



/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to print a variable of type        *
 * ip_address_t.  Note that, like the previous routine,    *
 * a pointer to the variable is passed rather than the     *
 * variable itself.                                        *
 *                                                         *
 *---------------------------------------------------------*/

void ip_address_print( ip_address_t *address_ptr)
{
   /* Error check the input parameter */

   if (address_ptr == (ip_address_t *)NULL) {
      printf("Error in ip_address_print\n");
      exit(0);
   }
 
   /* Print the variable */

   printf("%d.%d.%d.%d\n",address_ptr->n1,
                          address_ptr->n2,
                          address_ptr->n3,
                          address_ptr->n4);
}



/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to copy one variable of type       *
 * packet_t to another variable of type packet_t.          *
 * Note that both the source and destination parameters    *
 * are passed by REFERENCE.  In other words, addresses are *
 * passed to the two variables rather than the variables   *
 * themselves.                                             *
 *                                                         *
 * To use this routine, you might imagine code that looks  *
 * something like this:                                    *
 *    packet_t  s, d;                                      *
 *    ..                                                   *
 *    packet_copy ( &s, &d )                               *
 *                                                         *
 * The ampersand is used to pass the address of s and d    *
 * rather than s and d themselves.  If this confuses       *
 * you, see any C book for a more detailed explanation.    *
 *                                                         *
 *---------------------------------------------------------*/

void packet_copy( packet_t *source_packet_ptr,
                   packet_t *dest_packet_ptr)
{
   /* Error check on the input parameters */

   if ((source_packet_ptr == (packet_t *)NULL) ||
       (dest_packet_ptr == (packet_t *)NULL)) {
      printf("Error in packet_copy\n");
      exit(0);
   }

   /* Copy the fields.  To copy the address field,
      use the ip_address_copy routine defined above */

   ip_address_copy( &(source_packet_ptr->address),
                    &(dest_packet_ptr->address));
   dest_packet_ptr->payload = source_packet_ptr->payload;
}


/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to print a variable of type        *
 * packet_t.  Note that, like the previous routine,        *
 * a pointer to the variable is passed rather than the     *
 * variable itself.                                        *
 *                                                         *
 *---------------------------------------------------------*/

void packet_print( packet_t *packet_ptr)
{
 
   /* Error check input parameter */

   if (packet_ptr == (packet_t *)NULL) {
      printf("Error in packet_print\n");
      exit(0);
   }

   /* Print the packet (which contains an address and a payload */

   printf("      Address: ");
   ip_address_print(&(packet_ptr->address));
   printf("      Payload: %d\n",packet_ptr->payload);
}



/*---------------------------------------------------------*
 *                                                         *
 *  This routine should be called once at the start of     *
 *  the program for each port (it is called in main.c).    *
 *  The reason for this is that mutexes have to be         *
 *  initialized in an unlocked state.                      *
 *                                                         *
 *---------------------------------------------------------*/

void port_init(port_t *port_ptr)
{
   int status;

   port_ptr->flag = FALSE;
   status = pthread_mutex_init(&(port_ptr->mutex),NULL);
   if (status != 0) {
      printf("Error initializing mutex in port data structure\n");
      exit(0);
   }
}

/*-----------------------------------------------------------*
 *                                                           *
 *  This routine locks the mutex associated with a port.     *
 *  It is called by the harness, and will most certainly be  *
 *  called by your routine.  You want to lock the mutex      *
 *  before you read or write to the port, to enforce mutual  *
 *  exclusion.  After calling this routine, make sure you    *
 *  call port_unlock.                                        *
 *                                                           *
 *  Note that a pointer to a port_t structure is passed      *
 *  as a parameter rather than the port_t structure itself.  *
 *  For examples of how to call this routine, have a look at *
 *  the code in harness.c                                    *
 *                                                           *
 *-----------------------------------------------------------*/

void port_lock(port_t *port_ptr)
{
  int status;

   status = pthread_mutex_lock(&(port_ptr->mutex));   
   if (status != 0) {
      printf("Error locking mutex in port data structure\n");
      exit(0);
   }
}


/*-----------------------------------------------------------*
 *                                                           *
 *  This routine unlocks the mutex associated with a port.   *
 *  It is called by the harness, and will most certainly be  *
 *  called by your routine.  
 *                                                           *
 *  Note that a pointer to a port_t structure is passed      *
 *  as a parameter rather than the port_t structure itself.  *
 *  For examples of how to call this routine, have a look at *
 *  the code in harness.c                                    *
 *                                                           *
 *-----------------------------------------------------------*/

void port_unlock(port_t *port_ptr)
{
   int status;

   status = pthread_mutex_unlock(&(port_ptr->mutex));   
   if (status != 0) {
      printf("Error unlocking mutex in port data structure\n");
      exit(0);
   }
}
                     

