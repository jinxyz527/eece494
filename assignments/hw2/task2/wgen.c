#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

/*********************************************************************

	Assignment #2 - Task 2 - "Generating synthetic workloads"

	Overview: This program uses an optimal (of two) scheme to generate task
			  utilizations with a fixed U (utilization). It should have a
			  more uniform distribution of utilization compared to another
			  scheme (which was also implemented but not included in this
			  file. The synthetic task sets are generated with random parameters
			  (execution times and periods).
        Author: Regina So (62104047)
        Last modified: 2010/2/12

**********************************************************************/

//initializations here
int n_tasks;

main(){
  double u_total;
  int i;
  int Pmax;

  printf ("%s", "Enter the number of tasks: ");
    scanf("%d" , &n_tasks);



    double randNum[n_tasks];

	printf ("%s", "Enter the fixed utilization: ");
    scanf("%f" , &u_total);

    printf ("%s", "Enter the maximum period: ");
    scanf("%d" , &Pmax);

	//testing values before sending it in for rand()
   //printf("n_tasks:%d u_total:%d\n",n_tasks, u_total);

  srand(time(NULL));

  double tempMax = u_total;
  //printf("temp: %f u_total: %f n_tasks: %f\n",tempMax, u_total);
  randNum[n_tasks];
  randNum[0]=0.0;
  double sum=0;

  for(i=1;i<n_tasks;i++)
    {
      randNum[i] = ((double)rand() / ((double)(RAND_MAX) + (double)(1)));
      printf("U%3d is: %5f\n",i,randNum[i]);
      tempMax = tempMax - randNum[i];
      sum = randNum[i]+sum;
      printf("sum is %f\n",sum);
    }
  randNum[n_tasks] = u_total - sum;
  printf("U%3d is: %5f\n",i,randNum[n_tasks]);

	return 0;
}
