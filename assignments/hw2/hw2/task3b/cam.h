/*******************************************************
 *
 *  Routing Table Routines for EECE 494, Assignment 2.
 *
 *  Created by _________, University of British Columbia
 *
 *  This file contains the prototypes for the routing
 *  table routines that you will create.  In addition,
 *  you can add any type information or anything else
 *  to this file that you want.  All the material you
 *  should be in this file and cam.c.
 *  You won't need to modify any of the other other files.
 *
 ******************************************************/


void cam_init();
void cam_add_entry(ip_address_t *address, int port);
int cam_lookup_address(ip_address_t *address);
void cam_free();
