#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

/**************************************************************
 *
 *  Example for EECE 494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This simple example creates two processes.  The first (producer)
 *  sends data to the second (consumer).  Signals are used to communicate
 *  between the two processes.  Recall that there are two types of
 *  Signals: POSIX.1 signals and POSIX.4 signals.  The latter have a
 *  number of advantages, including the fact that POSIX.4 signals can
 *  carry a small data payload.   We will use this payload to
 *  transfer the data.  Each signal will carry one character (note: 
 *  a POSIX.4 signal can actually carry 4 bytes, but we will only
 *  use 1 byte [i.e. one character] per signal).
 * 
 *  In general, the consumer would do something exciting with the data;
 *  here, it just prints the data to the screen.  That is still 
 *  pretty exciting, I guess.
 *
 *  The program ends when the producer sends a "\0" to the consumer.
 *
 *  This code is pretty ugly.  One of the things I want you to
 *  get from this is that, although signals *can* be used to 
 *  send data, they are really cumbersome for doing this.  Signals
 *  are fine for informing a process that something bad is happened,
 *  but as a communication mechanism for data, they are not a good choice.
 *  If you are communicating between processes, you could consider
 *  using "messages" instead (another example).  If you are using
 *  threads, you can simply use shared memory.
 *
 **************************************************************/



/*---------------------------------------------------------------*/

/*  The first two routines do the child processing.  All the child
    does is receive characters and print them to the screen.  There
    are two routines.  The first routine, signal_handler_code is 
    where all the excitement is.  This routine is set up to 
    automatically be called whenever a certain signal is received.
    This signal has a one-character payload; this routine 
    extracts that one-character payload and prints it to the 
    screen. */

void signal_handler_code(int signo, siginfo_t *info, void *ignored)
{
   char payload;

   payload = (char) (info->si_value.sival_int);
   printf("Character received: %c\n",payload);   
}

void do_child_processing()
{
   while(1) {
      sleep(1);  /* Nothing to do, because all the work is 
                    done by the signal handler */
   }
}



/*-------------------------------------------------------------------*/

/* Now we get to the parent processing.  The parent steps through 
   a message and sends it one character at a time.  */

/* This routine sends a certain signal to another process.  There
   are many different signal types that we can send.  The signals
   that are reserved for our use (in Posix.4) are SIGRTMIN to
   SIGRTMAX (the number of signals in there depends on your system).
   We will arbitrarily decide to use signal SIGRTMIN. */

void send_signal(pid_t destination_pid, char payload)
{
   /* The sigval structure contains a field for storing
      the payload of a message.  A payload can either be
      a 32-bit integer, or a 32-bit pointer.  We will use
      it as a 32-bit integer.  Since we are only going to
      send an 8-bit payload, only 8 of these 32 bits will 
      actually be used */

   union sigval sval;

   /* This variable is used to indicate whether an error
      occurs in the sigqueue routine */

   int status;   

   /* Set the payload value */
   sval.sival_int = (int)payload;

   /* Send the signal itself.  The first parameter of this routine
      is of type pid_t and indicates the pid of the destination process.
      The second parameter is the signal to send (as explained above,
      we arbitrarily choose to send signal SIGRTMIN).  The final
      value is the payoad that goes along with the signal.  */

   status = sigqueue(destination_pid, SIGRTMIN, sval);

   /* Make sure the sigqueue function worked properly */

   if (status < 0) {
      printf("Error in function sigqueue.  Could not send a signal to the child\n");
      printf("I'm giving up\n");
      exit(0);
   }
}


/* This routine performs the core parent processing.  It cycles through
   all characters in the message to send, and calls the above
   routine to actually send a signal containing each character as 
   a payload.  After a \0 has been sent, we are done.  */

void do_parent_processing(pid_t child_pid)
{
   int pos;   /* What character we are currently sending */
   int done;  /* Non-zero when we are done */
   static char *message_to_send = "EECE 494 is the best course ever";

   pos = 0;     /* Start at first character */
   done = 0;    /* We are not done yet */
   while(!done) {    

      /* Send the signal using the above routine */

      send_signal(child_pid, message_to_send[pos]);

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
   struct sigaction sa;
   int status;

   /* We are going to tell the operating system that whenever signal 
      SIGRTMIN is received (we arbitrarily chose this signal as 
      explained above) occurs.  We will do this soon, with a call to 
      sigaction.   That routine, however, requires that we set up
      some parameters in a structure of type sigaction (yes, the
      structure and the function have the same name.  This doesn't
      confuse the compiler, but it might confuse you.  Don't let it.
      One is a structure where we store information for the routine
      to use, and the other is the operating system routine itself */

   /* First, indicate what routine we should call whenever the
      signal occurs.  Indicate this by storing the routine address
      in the appropriate field within the sa data structure */

   sa.sa_sigaction = signal_handler_code;  /* Name of routine to call when
                                              this signal occurs */
   
   /* At this point, we can optionally tell the operating system to
      "mask out" (disallow) certain signals whenever the call-back
      routine is called.  We really don't care about that here
      so just set the set of masked signals to the empty set. */
  
   sigemptyset(&sa.sa_mask);
 
   /* We can optionally set a number of flags to further indicate
      how a particular signal should be handled.  The only one we
      care about here is SA_SIGINFO.  When this is specified, if
      multiple signals are sent before the receiver can process 
      them, they are queued up.  If you don't specify this, if you
      send a second signal before the first one is serviced, the
      first one will never be serviced.  */

   sa.sa_flags = SA_SIGINFO;

   /* Now that we have set up the sa structure, we can call the 
      routine to actually tell the operating system
      what we want to do when SIGRTMIN signal is received.  */

   status = sigaction(SIGRTMIN, &sa, NULL);
   if (status != 0) {
       printf("Error in routine sigaction while setting up signal handler\n");
       printf("I give up\n");
       exit(0);
   }

   /* Ok, now the operating system knows what to do when the SIGRTMIN
      signal happens.  Now we can use "fork" to create our child process.
      This is like example1.c */

   child_pid = fork();
   
   if (child_pid == 0) {  
      do_child_processing();
   } else {
      do_parent_processing(child_pid);
      kill(child_pid,SIGKILL);
   }
}
