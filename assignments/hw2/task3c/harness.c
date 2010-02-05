
/*******************************************************
 *
 *  Harness test routine for EECE 494, Assignment 2
 *
 *  Sathish Gopalakrishnan & Steve Wilton
 *  University of British Columbia
 *
 *  This file contains the definitions for the test harness
 *  part of Assignment 2.  You probably don't need to change anything
 *  here, unless you want to change something for testing 
 *  purposes.  If you do change anything, make sure that when
 *  you are done it works with the original test harness, since
 *  that is what we will use to mark your code.
 *
 ******************************************************/

#include "defs.h"

/* This data structure is used to store a copy of what is
   in the routing table.  This is simply an artifact of the 
   testing procedure.  It is used only to make sure we send
   valid packets into the switch, and to make sure the packets
   come out the right place.  To repeat: this is NOT part of
   the switch, it is only used to test the switch */

typedef struct my_version_of_the_table_t {
   ip_address_t address;   /* IP Address */
   int port;               /* Port corresponding to this IP address */
} my_version_of_the_table_t;

my_version_of_the_table_t my_version_of_the_table[NUMBER_ENTRIES_IN_ROUTING_TABLE];


/* Each packet is assigned a sequence number, so that the test harness can
   better match the packets coming out of the switch to the packets that it
   has sent into the switch.  The sequence number is actually encoded as
   part of the payload, although your switch will never look inside the
   payload.  The first packet sent in each port is sequence number 0, the
   second packet sent in each port is sequence number 1, etc.  */

/* The following array indicates the largest sequence number sent down
   each input port */

int sequence_numbers[4];

/* There is one entry in this 2D array for each message sent.  The first
   index indicates the input port into which the message was sent, and
   the second index indicates the sequence number of that message.  The
   value of each element in the array is TRUE if the message was received
   and FALSE if it wasn't.  So, for example, received_message[2][5] is
   TRUE if the message with sequence number 5 sent into input port 2 was
   actually received at the output of the switch.  This is used to calculate
   how many packets were dropped */

BOOL received_message[4][NUMBER_PACKETS_TO_SEND];


/*=========================================================================*
 *                                                                         *
 *  The following are local routines that are used only within this file.  *
 *                                                                         *
 *=========================================================================*/


/*-------------------------------------------------------------------------*
 *                                                                         *
 * This is like the sleep command, but it sleeps for only                  *
 * HARNESS_SPEED nanoseconds (this constant is defined in harness.h        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void short_sleep()
{
   struct timespec delay_time, what_time_is_it;

   delay_time.tv_sec = 0;
   delay_time.tv_nsec = HARNESS_SPEED;

   nanosleep(&delay_time, &what_time_is_it);
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 *  This routine sends a message into a specified input port.  The         *
 *  destination IP address is chosen randomly from among all the           *
 *  IP addresses that have been previously loaded into the table.          *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void send_message(int input_port)
{
   int p;             /* Destination IP address */
   packet_t packet;   /* Packet that will be sent */

   /* Choose the destination IP address randomly */

   p = random()%NUMBER_ENTRIES_IN_ROUTING_TABLE;

   /* Lock the appropriate input port */

   port_lock(&(in_port[input_port]));

   /* Copy the destination IP address to the "packet" variable */

   ip_address_copy(&(my_version_of_the_table[p].address), &(packet.address));

   /* Compute the payload (your switch ignores the value inside the
      payload, but the test harness uses it to figure out where this
      message came from */

   packet.payload = input_port +
                    4*my_version_of_the_table[p].port + 
                    16*sequence_numbers[input_port];

   /* Update the sequence number corresponding to this input port (so the
      next packet sent to this input port has a sequence number that is one
      larger than the sequence number of this packet */

   (sequence_numbers[input_port])++;

   /* Copy the packet to the port */

   packet_copy(&packet,
               &(in_port[input_port].packet));

   /* Set the flag to indicate that the port has a packet */

   in_port[input_port].flag = TRUE;

   /* Unlock the port */

   port_unlock(&(in_port[input_port]));

   if (HARNESS_DEBUG) {
      printf(" Sending message on port %d\n",input_port);
      packet_print(&packet);
   }
}

/*-------------------------------------------------------------------------*
 *                                                                         *
 *  This routine receives a message from a specific output port, if there  *
 *  is one.  If a message is received, it is removed from the port, and    *
 *  the local stats within the test harness are updated to indicate that   *
 *  this packet is arrived.  This routine also makes sure that the packet  *
 *  has arrived on the CORRECT port.                                       *
 *                                                                         *
 *  The routine returns a TRUE if a message was received, and FALSE        *
 *  otherwise                                                              *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static BOOL receive_message_if_there_is_one(int output_port)
{
   packet_t packet;   /* Used to store received packet */
   int seq_num;       /* Sequence number of the received packet */
   int to_port;       /* Port where packet should have gone to */
   int from_port;     /* Port where packet came from */
   int payload;       /* Payload of the received packet */
   BOOL retval;       /* The value we will return (TRUE or FALSE) */

   /* Lock the output port */
 
   port_lock(&(out_port[output_port]));

   /* If the output port flag is high, that means a packet is there. */

   if (out_port[output_port].flag) {

      retval = TRUE;

      /* Set the flag to false */

      out_port[output_port].flag = FALSE;

      /* Copy the packet into our local storage */
      
      packet_copy(&(out_port[output_port].packet), &packet);

      /* Extract the payload */

      payload = packet.payload;

      if (HARNESS_DEBUG) {
         printf("   received packet on port %d\n",output_port);
         packet_print(&packet);
      }

      /* Use the payload to make sure that this packet has come out of
         the right port */

      from_port = payload & 3;
      to_port = (payload / 4) & 3; 
      seq_num = (payload / 16);

      if (to_port != output_port) {
         printf("Error: received packet on port %d when it should have come on port %d\n",
                     output_port, to_port);
         printf("Discarding packet... this is probably an error you should fix\n");
      } else {

         /* If it has come out of the right port, extract the sequence number 
            from the payload and set the appropriate 'received_message'  (see above) */

         received_message[from_port][seq_num] = TRUE;
      }
   }else {
      retval = FALSE;
   }

   /* Unlock the port, because we are done */

   port_unlock(&(out_port[output_port]));   

   return(retval);
}




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

/*=========================================================================*
 *                                                                         *
 *  The following are the exported routines from this file.                *
 *                                                                         *
 *=========================================================================*/



/*------------------------------------------------------------*
 *                                                            *
 *  This routine initializes the harness data structures.     *
 *  It is called once before the start of the test.  The main *
 *  thing that it does is initalize the routing table within  *
 *  the switch.                                               *
 *                                                            *
 *------------------------------------------------------------*/

void harness_init()
{
   int i,j, port;
   ip_address_t address;
   BOOL ok;

   if (HARNESS_DEBUG) {   
      printf("  setting up switch with routing table with %d entries\n", 
                NUMBER_ENTRIES_IN_ROUTING_TABLE);
   }

   /* Randomly fill the routing table within the switch */

   for(i=0;i<NUMBER_ENTRIES_IN_ROUTING_TABLE;i++) {

      /* Make up a random IP address */

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

      /* Pick a random port corresponding to this address */

      port = random()%4;

      /* Call your switch routine to add this entry to the routing table */

      switch_add_entry(&address,port);

      /* Store it in the harness's local table, which we will use when
         deciding what messages to send, and determining if the sent messages
         arrive out of the correct port */

      ip_address_copy( &address,
                       &(my_version_of_the_table[i].address));
      my_version_of_the_table[i].port = port;
   }

   /* Set all the sequence numbers to 0, and initialize the
      received_message array entries to FALSE */

   for(i=0;i<4;i++) {
      sequence_numbers[i] = 0;
      for(j=0;j<NUMBER_PACKETS_TO_SEND;j++) {
         received_message[i][j] = FALSE;
      }
   }
}


/*------------------------------------------------------------*
 *                                                            *
 *  The test harness runs as a single thread.  This is the    * 
 *  thread routine.  The thread will be set up in main.c.     *
 *  The thread has two purposes: to send packets at random    *
 *  through the input ports, and extract packets from the     *
 *  output ports as they appear.  The thread keeps track of   *
 *  how many sent packets are recieved; from this, the number *
 *  of dropped packets can be calculated.                     *
 *                                                            *
 *------------------------------------------------------------*/

void *harness_thread_routine(void *arg)
{
   int num_sent;
   int i;
   BOOL done, received;

   done = FALSE; 
   num_sent = 0;
   while(!done) {
      printf("In harness routine: Number sent=%d\n", num_sent);
      
 
      /* Consider sending a message on each port */

      for(i=0;i<4;i++) {
         if ( (num_sent < NUMBER_PACKETS_TO_SEND) &&
              (random()%100 < SEND_DENSITY)) {
            send_message(i);
            num_sent ++;
         }
      }

      short_sleep();

      /* Check all output ports and see if there is
         a packet we should extract */

      received = FALSE;
      for(i=0;i<4;i++) {
         received |= receive_message_if_there_is_one(i);
      }

      /* We exit the loop when we have sent all of our
         packets and we have not received any packets */

      done = (received == FALSE) &&  
             (num_sent >= NUMBER_PACKETS_TO_SEND);

   }

   /* We are done, so set the DIE global variable.  The switch
      thread will see this and die */

   die = TRUE;
}


/*------------------------------------------------------------*
 *                                                            *
 *  This routine frees the harness data structures.           *
 *  It is called once at the end of the test.                 *
 *  This routine also prints out the stats about how many     *
 *  messages were received and how many were dropped.         *
 *                                                            *
 *------------------------------------------------------------*/

void harness_end()
{
   int i,j;
   int num_packets_sent;
   int num_packets_received;


   num_packets_sent = 0;
   num_packets_received = 0;

   /* Look at each input port */

   for(i=0;i<4;i++) {
 
      /* Go through all messages that were sent into this 
         input port.  For each, indicate whether the message
         was received or not.  */

      for(j=0;j<sequence_numbers[i];j++) {
         num_packets_sent ++;
         printf("port %d, seq %d  ",i,j);
         if (received_message[i][j]) { 
            num_packets_received++;
            printf("received\n");
	 } else {
            printf("not received\n");
	 }
      }
   } 

   printf("Total packets sent: %d\n",num_packets_sent);
   printf("Total packets received: %d\n",num_packets_received);
   printf("Total packets dropped: %d\n", num_packets_sent- num_packets_received);
   printf("Percent packets dropped: %f\%\n", 100*
               (float)(num_packets_sent-num_packets_received)/(num_packets_sent));
}
