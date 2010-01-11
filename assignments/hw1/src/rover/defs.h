
/*******************************************************
 *
 *  Global Header File for EECE 494
 *
 *  University of British Columbia, ECE
 *	Steve Wilton & Sathish Gopalakrishnan
 *
 *  This file contains definitions that everyone might need.
 *
 ******************************************************/


/*-------------------------------------------------
 * 
 * These are some standard definitions.  Including
 * them here means that the include statements don't 
 * have to be repeated in each file.
 *
 *-------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>

#include <pthread.h>


/*-----------------------------------------------------
 * The following describes the BOOL type,
 * which is represented by a CHAR (1 byte).  
 * Legal values for a variable of this type
 * are TRUE and FALSE 
 *----------------------------------------------------*/

#define BOOL char
#define TRUE ((char)1)
#define FALSE ((char)0)
