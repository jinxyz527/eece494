#include <stdio.h>
#include <pthread.h>


/**************************************************************
 *
 *  Example for EECE494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This simple example creates two threads.  The first (producer)
 *  sends data to the second (consumer).  Shared memory is used to communicate
 *  between the two threads.  Using shared memory, we can share
 *  whatever sort of data structure(s) we like.  Here share a single
 *  byte (a character) that stores a message being sent from the
 *  producer to the consumer, and a "full" flag which indicates whether
 *  that producer has something for the receiver. 
 *
 *  One of the interesting things to notice in this code is the
 *  way mutexes were used to ensure that the producer and consumer
 *  don't both try to access the shared data at the same time.
 *
 *  The program ends when the producer sends a "\0" to the consumer.
 *
 **************************************************************/


/* Define the structure type we will use for our shared
   data.  Note that one of the fields is of type
   pthread_mutex_t; this will be used as the mutex to ensure
   that only one thread can access this shared data at a time */

struct my_shared_data_t {
   pthread_mutex_t mutex;  /* The mutex itself */
   char message;            /* The message we will send */
   int full;                /* One if the consumer should read
                               the message.  Zero means that
                               there is nothing for the consumer
                               to read yet */
} my_shared_data_t;

/* Create an object to store our shared data.  This is global
   and can be accessed by either of the two threads. */

struct my_shared_data_t shared_data = 
      {PTHREAD_MUTEX_INITIALIZER,  /* Initial value for mutex */
      (char)0,                     /* Initial value for message */
      0};                          /* Initial value for "full" */


/*-------------------------------------------------------------
 *
 * This routine is the code that runs for the consumer thread.
 * Every second, it checks to see if there is a message for it
 * in the shared data object, and if so, retrieves it and prints
 * it on the screen.  Before accessing the shared data, the
 * appropriate mutex is locked to ensure no one else can access
 * the shared data.
 *
 *------------------------------------------------------------*/

void *consumer_thread_routine(void *arg)
{
   int done;
   char c;

   done = 0;
   while(!done) {
  
      /* Wait until no one else is accessing the shared
         data.  Question: what sort of problems might occur 
         if both the producer and receiver were accessing
         the shared data at the same time?  */

      pthread_mutex_lock(&shared_data.mutex);

      /* Now we know that no other thread is accessing
         the shared data, so we can go ahead and do whatever
         we want with it */

      if (shared_data.full) {
         shared_data.full = 0; 
         c = shared_data.message;
         printf("Received character: %c\n",c);
         if (c=='\0') done = 1;
      }

      /* Now we are done accessing the data, so we can
         unlock it.  This will allow other threads
         to access the shared data. */

      pthread_mutex_unlock(&shared_data.mutex);   
 
      /* Go to sleep for a while to let other threads
         to execute */

      sleep(1);
   }
}


/*------------------------------------------------------------
 *
 *  This is the code that is executed by the producer thread.
 *  It sends data through the shared memory space.  As with
 *  the consumer, the shared memory region is locked, so that
 *  two threads don't try to access the data at the same time.
 *
 *----------------------------------------------------------*/

void *producer_thread_routine(void *arg)
{
   int done;  /* non-zero when we have sent a \0 */
   int pos;   /* position within the message of the current character */
   static char *message_to_send = "EECE 494 is the best course ever";

   done = 0; 
   pos = 0;
   while(!done) {
  
      /* Wait until no one else is accessing the shared
         data.  Question: what sort of problems might occur 
         if both the producer and receiver were accessing
         the shared data at the same time?  */

      pthread_mutex_lock(&shared_data.mutex);

      /* Now we know that no other thread is accessing
         the shared data, so we can go ahead and do whatever
         we want with it */

      if (!shared_data.full) {         
         shared_data.full = 1;
         shared_data.message = message_to_send[pos];
         if (message_to_send[pos] == '\0') done = 1;
         pos++;
      }       

      /* Now we are done accessing the data, so we can
         unlock it.  This will allow other threads
         to access the shared data. */

      pthread_mutex_unlock(&shared_data.mutex);

      /* Go to sleep for a while to let other threads execute */

      sleep(1);
   }
}

/*------------------------------------------------------------
 *
 *  This is the main routine.  It simply sets up the two
 *  threads, then waits for them to finish (using the
 *  thread_join routine).  
 *
 *----------------------------------------------------------*/

main()
{
   pthread_t producer_thread_id;  /* id of the producer thread */
   pthread_t consumer_thread_id;  /* id of the consumer thread */
   int status;     /* Used to store the status of the pthread_create
                      and pthread_join routines.  If these routines fail,
                      this variable will be set to a non-zero value */

   /* Set up producer thread.  First, create the thread.
      Specify the "producer_thread_routine" as the function containing
      the code that this thread should run. */

   status = pthread_create(&producer_thread_id,
                           NULL,
                           producer_thread_routine,
                           NULL);
   if (status != 0) {
      printf("Error creating producer thread: status = %d\n",status);
      exit(0);
   }

   /* Now do NOT detach the thread, like we did in Example 2 */
   /* We don't detach the thread because we want to join to the
      thread later.  */


   /* Set up consumer thread.  First, create the thread.
      Specify the "consumer_thread_routine" as the function containing
      the code that this thread should run. */

   status = pthread_create(&consumer_thread_id,
                           NULL,
                           consumer_thread_routine,
                           NULL);
   if (status != 0) {
      printf("Error creating consumer thread: status = %d\n",status);
      exit(0);
   }


   /* Now, we can assume the threads are running.  There is
      nothing to do here except wait until the threads are
      done */

   status = pthread_join(producer_thread_id, NULL);
   if (status != 0) {
      printf("Error joining thread: status = %d\n",status);
      exit(0);
   }
   status = pthread_join(consumer_thread_id, NULL);
   if (status != 0) {
      printf("Error joining thread: status = %d\n",status);
      exit(0);
   }
}
