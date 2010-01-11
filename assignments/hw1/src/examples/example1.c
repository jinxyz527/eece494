#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/**************************************************************
 *
 *  Example for EECE 494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This simple example creates two processes.  The first (parent)
 *  process prints out the word "UBC" every 3 seconds (8 times)
 *  while the second (child) prints out the name "ECE" every 7 seconds (3 times).
 * 
 *  Note there is something unsafe about this.  Can you figure
 *  out what it is?  Hint: consider the case when two processes
 *  try to print to the screen at exactly the same time.  What
 *  might happen?  The answer is that it depends on the implementations
 *  of the printf routine.  In general, you should provide locks
 *  so that only one process is printing at the same time.
 *  We haven't done that here, because we want this example to
 *  be as simple as possible to illustrate the use of multiple
 *  processes.  Other examples will show how to handle this
 *  sort of thing more safely.
 *
 **************************************************************/

main()
{
   pid_t child_pid;
   int i;   /* Question: This variable i is used in two for loops:
               one in the child code and one in the parent code.  Is
               there a conflict?  Should we have two variables?  Why
               or why not?   This would be a great test question. */

   child_pid = fork();
   
   if (child_pid == 0) {
   
      /* I am the child.  Start executing the child processing here. */

      for(i=0;i<8;i++) {
         printf("ECE\n");
         sleep(3);
      }
      printf("The child is exiting now\n");
   
   } else {

      /* I am the parent.  Start executing the parent processing here */

      for(i=0;i<3;i++) {
         printf("UBC\n");
         sleep(7);
      }
      printf("The parent is exiting now\n");
   }
}
