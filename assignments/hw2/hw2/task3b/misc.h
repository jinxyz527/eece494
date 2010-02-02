
/*******************************************************
 *
 *  Misc routines for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 *  This file contains miscellaneous structures that might
 *  be useful in your assignment 
 *
 *  The following structures are defined here:
 *
 *    ip_address_t 
 *
 *  In addition, function prototypes are provided for some
 *  routines that manipulate these structures that you might
 *  find useful.  The routines themselves can be found in
 *  misc.c
 *
 ******************************************************/


/*------------------------------------------------------------*
 *                                                            *
 *  This structure stores a single destination IP address.    *
 *  An IP address contains four integers.                     *
 *                                                            *
 *------------------------------------------------------------*/

typedef struct ip_address_t {
   int n1;  /* First integer */
   int n2;  /* Second integer */
   int n3;  /* Third integer */
   int n4;  /* Fourth integer */
} ip_address_t;

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
                      ip_address_t *dest_address_ptr);



                     
