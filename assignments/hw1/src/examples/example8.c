
#include <stdio.h>
#include <time.h>



/*************************************************************************
 *
 *  Example for EECE494
 *  University of British Columbia, ECE
 *  Sathish Gopalakrishnan & Steve Wilton
 *
 *  This example shows how to use the timer to measure a time interval.
 *  This is probably the simplest example of them all.
 *
 *  The program measures the time to bubblesort a 1000-element list.
 *  It actually performs the measurement 5 times, and prints the
 *  time taken by each.  In an ideal world, all 5 measurements
 *  would be exactly the same.  Is that the case here?  Try it 
 *  and see!
 *
 *************************************************************************/


/*----------------------------------------------------------
 *
 *  This routine simply initializes the array with random
 *  numbers.  Each entry is a random number between 0 and 999.
 *
 *----------------------------------------------------------*/

void initialize_array(int num, int *array)
{
   int i;

   /* This routine sets the seed of the random number
      generator.  By setting it to a known constant, we can
      ensure that we get the same "random" sequence each of 
      the 5 times we go through the main loop.  This is important
      because if we want to compare the run times for each of the
      five sorting attempts, we should make sure that each sorting
      attempt is doing exactly the same amount of work */

   srandom(0);
   
   for(i=0;i<num;i++) {
      array[i] = random()%1000;
   }
}


/*----------------------------------------------------------
 *
 *  This routine performs a bubble sort on the array.
 *
 *----------------------------------------------------------*/

void bubblesort_array(int num, int *array)
{
   int i,j,tmp;

   for(i=0;i<num;i++) {
      for(j=i;j<num;j++) {
         if (array[i] > array[j]) {
	    tmp = array[j];
	    array[i] = array[j];
	    array[j] = tmp;
	 }
      }
   }
}


/*-------------------------------------------------------
 *
 *  This is the main program where all the action is.  
 *  It initializes an array and sorts it, measuring the
 *  time do perform the sorting.  It does this 5 times 
 *
 *--------------------------------------------------------*/

#define ARRAY_SIZE 1000  /* Number of elements in the array */

main()
{
   struct timespec start_time;  /* Clock value just before sorting */
   struct timespec end_time;    /* Clock value just after sorting */
   long secs_diff, ns_diff, diff;  /* Used to compute time difference */
   int array[ARRAY_SIZE];       /* The array itself */
   int i; 


   for(i=0;i<5; i++) {

      initialize_array(ARRAY_SIZE, array);
      
      /* Record the start time */

      clock_gettime(CLOCK_REALTIME, &start_time);

      /* Compute the factorial */

      bubblesort_array(ARRAY_SIZE, array);
   
      /* Record the end time */

      clock_gettime(CLOCK_REALTIME, &end_time);
 
      /* Compute the difference between the start time and 
         the end time */  

      secs_diff = (long)(end_time.tv_sec) - (long)(start_time.tv_sec);
      ns_diff = end_time.tv_nsec - start_time.tv_nsec;
      diff = secs_diff * 1000000000 + ns_diff;
 
      printf("time=%d ns\n",diff);
   }
}
