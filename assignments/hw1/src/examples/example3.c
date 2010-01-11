#include <stdio.h>
#include <pthread.h>


/**************************************************************
 *
 *  Example for EECE 494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This is exactly the same as example 2, except here,
 *  the threads don't run forever.  The difference is that
 *  the main program will now use the "pthread_join" routine
 *  to wait for the child to be done.  Because of this, the
 *  main program can not "detach" the thread once it is 
 *  created (remember, once you detach a thread, you can not
 *  join to it... we talked about this in class).  
 *
 **************************************************************/


void *thread_routine(void *arg)
{
   int i;

   for(i=0;i<5;i++) {
      printf("ECE\n");
      sleep(3);
   }
   printf("The child is done\n");
}


main()
{
   pthread_t thread_id;
   int i;

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

   /* Now do NOT detach the thread, like we did in Example 2 */
   /* We don't detach the thread because we want to join to the
      thread later.  */

   /* Now, we can assume the thread is running.  Do the
      parent processing.  Note that could, instead, create
      another child process to do this processing, and then
      the parent routine wouldn't really have to do anything */

   for(i=0;i<3;i++) {
      printf("UBC\n");
      sleep(2);
   }

   printf("The parent is done and is waiting for the child\n");

   /* Now wait until the child thread is done. */

   status = pthread_join(thread_id, NULL);
   if (status != 0) {
      printf("Error joining thread: status = %d\n",status);
      exit(0);
   }

   /* Now we know both the child and parent are done so
      we can exit */
}
