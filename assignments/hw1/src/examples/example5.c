#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <mqueue.h>  /* Warning: read comment below */

/**************************************************************
 *
 *  Example for EECE 494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  IMPORTANT NOTE:  This routine requires the message queue
 *  component of POSIX.4.   The ssh-linux machine DOES NOT HAVE
 *  THIS COMPONENT.  Thus, program will not compile on ssh-linux.
 *  To verify this, have a look at the output of Task 3 in 
 *  Assignment 1.  Hunt through the output to see if "message queues"
 *  are supported.  On ssh-linux, they are not.  On ssh, however,
 *  they are supported, so this will compile there.  If you are using
 *  your own version of linux, then it might compile or it might
 *  not, depending on whether you have this part of POSIX.4.
 *
 *  Since it doesn't work on ssh-linux, we won't be using message queues
 *  much in this course.  But the example is included here for completeness 
 *
 *
 *
 *  This simple example creates two processes.  The first (producer)
 *  sends data to the second (consumer).  POSIX.4 messages are used 
 *  to communicate between the two processes.   Recall that messages
 *  are sent via a message queue.  The main routine creates the
 *  message queue.  The parent process then sends data through the
 *  message queue, and the child recieves data from the message
 *  queue.  Compare this to the code in example4.c, and you will
 *  see that message queues are more suitable for this sort of thing
 *  than are signals. 
 * 
 *  In general, the consumer would do something exciting with the data;
 *  here, it just prints the data to the screen.  
 *
 *  The program ends when the producer sends a "\0" to the consumer.
 *
 **************************************************************/



/*---------------------------------------------------------------*/

/* This routine does the child processing.  It received data from
   the message queue, and prints the data to the screen.  When it
   receives a '\0', the child is done */

void do_child_processing(mqd_t mymq)
{
   int done;             /* Used to indicate if we are done */
   int received_prio;    /* The prioirity of the message we have
                            received */
   char received_char;   /* The character we have received */

   done = 0;   
   while(!done) {

      /* Receive a message.  If no message is available on the queue,
         this routine will block until one becomes available.  Note that
         it is possible to create a non-blocking queue by specifying
         O_NONBLOCK in the mq_open call (see below), but we won't
         worry about that. */

      mq_receive(mymq,            /* The message queue to wait on */
                &received_char,   /* Pointer to where the routine should
                                     put the data that it receives */
                1,                /* Size of the message we should receive
                                     in bytes */
                &received_prio);  /* Pointer to where the routine should
                                     put the priority of the message that
                                     it receives */
 
      printf("Received character: %c\n",received_char);
      done = (received_char == '\0');
   }
}



/*-------------------------------------------------------------------*/

/* Now we get to the parent processing.  The parent steps through 
   a message and sends it one character at a time.  */

/* This routine performs the core parent processing.  It cycles through
   all characters in the message to send, and calls the mq_send
   routine to actually send a message containing each character as 
   a payload.  Note that a message could be more than one byte wide; 
   we just do it this way here to be consistent with example4.c
   After a \0 has been sent, we are done.  */

void do_parent_processing(pid_t child_pid, mqd_t mymq)
{
   int pos;   /* What character we are currently sending */
   int done;  /* Non-zero when we are done */
   static char *message_to_send = "EECE 494 is the best course ever";

   pos = 0;     /* Start at first character */
   done = 0;    /* We are not done yet */
   while(!done) {    

      /* Send the signal using the above routine */

      mq_send(mymq,   /* The message queue we want to send a message on */
             &(message_to_send[pos]),  /* Pointer to the string we
                         want to send in our message queue. */
             1,       /* Number of characters we want to send in this message */
             0);      /* Priority of this message */

      /* If we just sent a \0, then we are done */

      if (message_to_send[pos] == '\0') {
         done = 1;
      }
      pos++;  /* skip to next character */

      /* We don't actually need to sleep here.  We could
         just queue up all the characters very quickly.
         I have included a sleep here just to make it
         easier to understand what is happening when you
         run the code.  You might want to try to take it
         out and verify that the program still works */

      sleep(1);
   }
}

/*------------------------------------------------------*/


/* Now the main routine.  */

main()
{
   pid_t child_pid;
   int status;
   struct mq_attr mq_attr;
   mqd_t mymq;
   
   /* Create the message queue.  We will create with with the mq_open
      function below.  To do this, we need to set up a structure of type
      mq_attr with various attributes that tell the mq_open what the
      message queue should look like.  */

   mq_attr.mq_maxmsg = 100;  /* The message queue will have 100 entries */
   mq_attr.mq_msgsize = 1;   /* The message queue will only be one byte
                                wide (note: this is unusual; usually you
                                would want wider messages) */
   mq_attr.mq_flags = 0;     /* Various flags that can be optionally set,
                                don't worry about these for now */

   /* Now, using the mq_attr structure, create the queue */

   mymq = mq_open("/steve",  /* The message queue name.  You can use
                                any name, but the name should start
                                with a '/' and not contain any other
                                '/'s after the first character.  */
                  O_CREAT | O_RDWR,   /* The first of these flags indicates
                                that the message queue should be created
                                if it does not exist.  The second flag
                                indicates that the queue can be used for
                                both reading and writing */
                  S_IRWXU,   /* Permissions: who can open this queue.  This
                                setting will be suitable for what you want
                                to do in your assignments */
                  &mq_attr); /* The set of attributes created above */

   /* Make sure the queue was successfully created */

   if ((int)mymq == -1) {
      printf("Error creating and opening message queue.\n"); 
      printf("I give up.\n");
   }

   /* Now create the child process, as in example1.c */

   child_pid = fork();
   
   if (child_pid == 0) {  
      do_child_processing(mymq);

      /* When the child is done, it should close the message queue */

      mq_close(mymq);

   } else {
      do_parent_processing(child_pid,mymq);     
 
      /* When the parent is done, it should close the message queue */

      mq_close(mymq);
 
      /* After everyone is done with the message queue, this routine 
         will destroy the queue.  If there are any processes that have
         not yet closed the queue, the destruction of the queue will wait
         until the other processes do close the queue.  You should always
         do this, because queues created during one run of the program
         will REMAIN IN THE SYSTEM unless you destroy them.  */

      mq_unlink(mymq);
   }
}
