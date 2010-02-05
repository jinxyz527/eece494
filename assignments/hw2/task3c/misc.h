
/*******************************************************
 *
 *  Main routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 *  This file contains miscellaneous structures that might
 *  be useful in your assignment.  
 *
 *  The following structures are defined here.
 *
 *    ip_address_t 
 *    port_t 
 *    packet_t
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


/*------------------------------------------------------------*
 *                                                            *
 *  This structure stores a packet.  A packet consists of     *
 *  a destination IP address and a payload.  In our           *
 *  implementation, the payload is just an integer (how       *
 *  boring)                                                   *
 *                                                            *
 *------------------------------------------------------------*/

typedef struct packet_t {
   ip_address_t address;   /* IP Address this packet is destined for */
   int payload;            /* Payload of the packet */
} packet_t;


/*------------------------------------------------------------*
 *                                                            *
 *  This structure stores a single input or output port.      *
 *  A port has three fields.  The first is the mutex.  This   *
 *  allows us to enforce mutual exclusion for this object.    *
 *  The second parameter is a packet; when a packet is sent   *
 *  through a port, it is stored here.  The final field is    *
 *  a flag that indicates whether the port contains a valid   *
 *  packet or not.  The assignment handout contains more      *
 *  details on how this structure is used.                    *
 *                                                            *
 *------------------------------------------------------------*/

typedef struct port_t {
   pthread_mutex_t mutex;  /* The mutex */
   packet_t packet;   /* Packet in this port */
   BOOL flag;   /* Whether the port contains a valid packet or not */
} port_t;


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

/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to print a variable of type        *
 * ip_address_t.  Note that, like the previous routine,    *
 * a pointer to the variable is passed rather than the     *
 * variable itself.                                        *
 *                                                         *
 *---------------------------------------------------------*/

void ip_address_print( ip_address_t *address_ptr);

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
                   packet_t *dest_packet_ptr);

/*---------------------------------------------------------*
 *                                                         *
 * This routine is used to print a variable of type        *
 * packet_t.  Note that, like the previous routine,        *
 * a pointer to the variable is passed rather than the     *
 * variable itself.                                        *
 *                                                         *
 *---------------------------------------------------------*/

void packet_print( packet_t *packet_ptr);

/*---------------------------------------------------------*
 *                                                         *
 *  This routine should be called once at the start of     *
 *  the program for each port (it is called in main.c).    *
 *  The reason for this is that mutexes have to be         *
 *  initialized in an unlocked state.                      *
 *                                                         *
 *---------------------------------------------------------*/

void port_init(port_t *port_ptr);

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

void port_lock(port_t *port_ptr);

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

void port_unlock(port_t *port_ptr);




                     
