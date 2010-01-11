#include <stdio.h>
#include <pthread.h>


/**************************************************************
 *
 *  Example for EECE 494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This simple example creates two threads.  The first (parent)
 *  process prints out the word "UBC" every 3 seconds,
 *  while the second (child) prints out the name "ECE" every 7 seconds.
 * 
 *  Note there is something unsafe about this.  Can you figure
 *  out what it is?  Hint: consider the case when two threads
 *  try to print to the screen at exactly the same time.  What
 *  might happen?  The answer is that it depends on the implementations
 *  of the printf routine.  In general, you should provide locks
 *  so that only one thread is printing at the same time.
 *  We haven't done that here, because we want this example to
 *  be as simple as possible to illustrate the use of multiple
 *  threads.  Other examples will show how to handle this
 *  sort of thing more safely.
 *
 **************************************************************/


void *thread_routine(void *arg)
{
   while(1) {
      printf("ECE\n");
      sleep(3);
   }
}


main()
{
   pthread_t thread_id;

   /* This is used to store status information from the
      pthread_create and the pthread_detach routines */

   int status;

   /* Set up child thread.  First, create the thread.
      Specify the "thread_routine" as the function containing
      the code that this thread should run. */

   status = pthread_create(&thread_id,
                           NULL,
                           thread_routine,
                           NULL);
   if (status != 0) {
      printf("Error creating thread: status = %d\n",status);
      exit(0);
   }

   /* Now detach the thread.  As described in class, this is very
      important.  If we don't do it, then when the main program
      exits, the threads will stay alive, consuming system resources */

   status = pthread_detach(thread_id);
   if (status != 0) {
      printf("Error detaching thread: status = %d\n",status);
      exit(0);
   }

   /* Now, we can assume the thread is running.  Do the
      parent processing.  Note that we could, instead, create
      another child process to do this processing, and then
      the parent routine wouldn't really have to do anything */

   while(1) {
      printf("UBC\n");
      sleep(7);
   }
}
