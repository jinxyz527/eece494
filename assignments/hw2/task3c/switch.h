
/*******************************************************
 *
 *  Switch Routines for EECE 494, Assignment 2
 *
 *  Created by _________, University of British Columbia
 *
 *  This file contains the prototypes for the switch 
 *  routines that you will create.  In addition,
 *  you can add any type information or anything else
 *  to this file that you want.  All the material you
 *  add for Assignment 2 should be in this file and switch.c.
 *  You won't need to modify any of the other other files.
 *
 ******************************************************/

void switch_init();

void switch_add_entry(ip_address_t *address,
                      int port);

void *switch_thread_routine(void *arg);
void *output_monitor_routine(void *arg);

void switch_free();                      

typedef struct element_to_queue{
	packet_t packet ; 
	struct element_to_queue *next ; 
} element_to_queue;

typedef struct buf_queue{
	struct element_to_queue *head ;
	struct element_to_queue *tail ;
	int size ; 
	pthread_mutex_t mutex ; 
} buf_queue;

void enqueue(element_to_queue * element,buf_queue * buffer);
void dequeue(buf_queue * buffer,packet_t *packet);
void queue_lock(buf_queue *ptr) ; 
void queue_unlock(buf_queue *ptr) ; 
