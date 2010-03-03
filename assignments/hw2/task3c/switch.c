/*******************************************************
 *
 *  Switch Routines for EECE 494, Assignment 2.
 *
 *  Created by _________, University of British Columbia
 *
 *  This is where you will put your routines to implement
 *  the switch routines.  You should use the headers as
 *  supplied.  All your code for Assignment 2 will go in this
 *  file (and possibly in switch.h)
 *
 ******************************************************/

#include "defs.h"
#include "harness.h"
#define NUM_PORTS 4
#define LOOP_COUNT 5 

buf_queue * output_buffer ; 

// Sleep for the same time as the input/output threads sleep for. 
void sleep(){
	struct timespec delay_time, what_time_is_it;
	delay_time.tv_sec = 0;
	delay_time.tv_nsec = HARNESS_SPEED;
   	nanosleep(&delay_time, &what_time_is_it);
}

// Thread which will handle dequeing and re-enqueing based on the status
// and the flags for all ports in the output buffer
void *output_monitor_routine(void *arg){
	packet_t in_pkt ; 
	ip_address_t address ; 
	int dest_port=0 ; 
	int loop_count= 0 ;
	element_to_queue * element ; 
	
	while(!die){

		// Only care dequing if there are elements. 	
		if((output_buffer->size > 0)){
			// This will indeed dequeue the packet, but we may 
			// have to put it back if the port isn't ready. 
			
			queue_lock(output_buffer) ; 
			dequeue(output_buffer,&in_pkt) ; 
			queue_unlock(output_buffer) ; 

            // Fetch the IP & lookup destination port 
		    ip_address_copy(&(in_pkt.address),&address);
		    dest_port = cam_lookup_address(&address) ; 
		    if((dest_port != -1) && (dest_port < 4)){
				// Wait for the lock
		    	port_lock(&(out_port[dest_port])) ; 
				// If the flag is busy from the last write, then
				// we have to put the packet back in the queue and just
				// have to wait until we get to it again.
				if(out_port[dest_port].flag){
					element = calloc(1,sizeof(element_to_queue)) ; 
					packet_copy(&in_pkt,&(element->packet));
					
					queue_lock(output_buffer) ; 
					enqueue(element,output_buffer) ; 
					queue_unlock(output_buffer) ; 

					port_unlock(&(out_port[dest_port])) ;
					continue ; 
				}
				// Port ready to be written , so go ahead and write. 
				packet_copy(&in_pkt,&(out_port[dest_port].packet));    
			   	out_port[dest_port].flag = TRUE ; 
				port_unlock(&(out_port[dest_port])) ;
			}
		}
		// Make sure it tried to at least deque 5 elements, before we
		// make it sleep. 
		if(loop_count > LOOP_COUNT){
			loop_count = 0 ; 
			sleep() ; 
		}else
			loop_count++ ;
	}
}

void *switch_thread_routine(void *arg)
{
	BOOL recv_flag=FALSE ; 	
	packet_t in_pkt ; 
	ip_address_t address ; 
	int dest_port=0 ; 
	int i= 0 ;
	element_to_queue * element ; 

	while(!die){
		// Lock all 4 ports first ... 
		// This is a better way to do this, since this thread may get
		// preempted and switched back to the input port writer which
		// might override the existing messages. 
		for(i=0 ; i < NUM_PORTS ; i++){
			port_lock(&(in_port[i])) ; 
		}

		// Then try to unlock and check if packets are available. 
		// We are sure to have saved all elements in the queue once 
		// we've unlocked them
   		for(i=0 ;i < NUM_PORTS ; i++){
			if(in_port[i].flag == TRUE){
				packet_copy(&(in_port[i].packet),&in_pkt); 	
				in_port[i].flag = FALSE ; 
			}else{
				port_unlock(&(in_port[i])) ; 
				continue ; 
			}
			element = calloc(1,sizeof(element_to_queue)) ; 
			packet_copy(&in_pkt,&(element->packet));

			queue_lock(output_buffer) ; 
			enqueue(element,output_buffer) ; 
			queue_unlock(output_buffer) ; 

			port_unlock(&(in_port[i])) ; 
		}

   		// pause for a a little while and then check again
		sleep() ; 
   	}
	printf("All packets sent.. Exiting !\n") ; 
}


void switch_init()
{
	int status ; 
	pthread_t out_monitor_id ; 
	// Initialize the routing table
	cam_init() ; 

	// Initialize the output buffer
	output_buffer = calloc(1,sizeof(buf_queue));
	output_buffer->head = NULL ; 
	output_buffer->tail = NULL ;
	output_buffer->size = 0 ; 
	status = pthread_mutex_init(&(output_buffer->mutex),NULL);
	if(status != 0){
		printf("Error Initializing mutex ...\n") ; 
	}

	// Create the thread that will monitor the output buffer
	printf("Starting output buffer thread .\n"); 
	status = pthread_create(&out_monitor_id,NULL,output_monitor_routine,NULL);
	if(status != 0){
		printf("Error creating monitor thread.\n") ; 
	}
	status = pthread_detach(out_monitor_id);
	if(status != 0){
		printf("Error detaching monitor thread.\n") ; 
	}
	
}

void switch_add_entry(ip_address_t *address,
                      int port)
{
	// Initialize the routing table
	cam_add_entry(address,port) ; 
}

void switch_free()
{
	if(output_buffer->head != NULL){
	}
	if(output_buffer->tail != NULL){
	}
   free(output_buffer) ; 
}

// Given a header to the start of the queue and an element to queue
// Place the element at the end of the queue. 
void enqueue(element_to_queue * element,buf_queue * buffer){
	if(element != NULL){
		element->next = NULL ; 
	}
	if((buffer->head == NULL) && (buffer->size == 0)){
		buffer->head = element ; 
		buffer->tail = element ; 
		buffer->size = 1 ; 
	}else{
		buffer->tail->next = element ; 
		buffer->tail = element ; 	
		buffer->size++ ; 
	}
}

// Given a header to the start of the queue , store the packet in the 
// reference. 
void dequeue(buf_queue * buffer,packet_t * packet){
	element_to_queue * ptr_to_remove ; 
	if(buffer->head == NULL){
		return ; 
	}else{
		ptr_to_remove = buffer->head ; 
		packet_copy(&(buffer->head->packet),packet);
		buffer->head = buffer->head->next ; 
		if(buffer->head == NULL){
			buffer->tail == NULL ; 
		}
		buffer->size-- ; 
	}
	free(ptr_to_remove) ; 
}

// Make sure the queue is being either enqueued or dequeed at a time. 

void queue_lock(buf_queue *ptr){
	int status ; 
	status = pthread_mutex_lock(&(output_buffer->mutex));
	if(status != 0){
		printf("Error locking queue ...\n") ; 
	}
}

void queue_unlock(buf_queue *ptr){
	int status ; 
	status = pthread_mutex_unlock(&(output_buffer->mutex));
	if(status != 0){
		printf("Error unlocking queue ...\n") ; 
	}
}
