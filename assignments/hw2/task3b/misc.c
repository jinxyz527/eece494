
/*******************************************************
 *
 *  Misc routines for EECE 494, Assignment 2
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

