#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

/****************************************************************
 *
 * Assinment2 Task1 EECE494
 * This program analyzes the schedulability of a given task set 
 * using the deadline monotonic scheduling policy.
 *
****************************************************************/


int total_tasks;
double task[100][3] ;

bool test1(void);
bool test2(void);
bool test3(void);

main()
{
		struct timespec start_time;  /* Clock value just before test1 */
    struct timespec end_time_1;  /* Clock value just after test1 & before test2 */
		struct timespec end_time_2;  /* Clock value just after test2 & before test3 */   
		struct timespec end_time_3;	 /* Clock value just after test2 */
		float secs_diff_1, ns_diff_1, diff_1;   
    float secs_diff_2, ns_diff_2, diff_2;   
		float secs_diff_3, ns_diff_3, diff_3;  
		float sum_1=0; float avg_1=0 ;
		float sum_2=0; float avg_2=0 ;
		float sum_3=0; float avg_3=0 ;

		int task_num;
    int i, j, n;
    double C, D, P;
    bool ans1, ans2, ans3;
    bool third = NULL;
		
		/* Get task sets from user*/
    printf ("%s\n", "Entre the number of tasks");
    scanf("%d" , &total_tasks);

    for (i=0; i < total_tasks; i++ )
		{
		task_num = i+1;
		printf ( "Entre parameters(execution time, reletive deadline, period) for task %i	\n" ,task_num);
		scanf ("%lf %lf %lf" , &task[i][0],&task[i][1],&task[i][2]);
		}
  
		/*Conditions to perform each tests*/
    for (j=0; j < total_tasks; j++)
		{           
			/*Compare relative deadline to the period*/
			if( task[j][1] <= task[j][2]  )		
			{
			third=true;
			}
			else  {
			third=false;
			break;
			}
		}

		/*Measure time interval 1000 times for each of test*/
		for (n=0; n < 1000 ; n++)
		{

		/* Record the start time for test1 */
		clock_gettime(CLOCK_REALTIME, &start_time);   
    
		ans1=test1();  
		
		/* Record the end time for test1 and start time for  test2*/
		clock_gettime(CLOCK_REALTIME, &end_time_1);     

		ans2=test2();
    
    /* Record the end time for test2 and start time for test3 */
		clock_gettime(CLOCK_REALTIME, &end_time_2);

		ans3=test3();

    /* Record the end time for test3 */
		clock_gettime(CLOCK_REALTIME, &end_time_3);    

		/* Test1:Compute the difference between the start time and the end time */  
      secs_diff_1 = (long)(end_time_1.tv_sec) - (long)(start_time.tv_sec);
      ns_diff_1 = end_time_1.tv_nsec - start_time.tv_nsec;
      diff_1 = secs_diff_1 * 1000000000 + ns_diff_1;
      sum_1 = diff_1 + sum_1;

		/* Test2:Compute the difference between the start time and the end time */  
      secs_diff_2 = (long)(end_time_2.tv_sec) - (long)(end_time_1.tv_sec);
      ns_diff_2 = end_time_2.tv_nsec - end_time_1.tv_nsec;
      diff_2 = secs_diff_2 * 1000000000 + ns_diff_2;
			sum_2 = diff_2+ sum_2;

		/* Test3:Compute the difference between the start time and the end time */  
      secs_diff_3 = (long)(end_time_3.tv_sec) - (long)(end_time_2.tv_sec);
      ns_diff_3 = end_time_3.tv_nsec - end_time_2.tv_nsec;
      diff_3 = secs_diff_3 * 1000000000 + ns_diff_3;
      sum_3 = diff_3 + sum_3;
		}
	
	/* Finding the average time for each test*/
	avg_1 = (sum_1/1000.00)/1000000.00;
	avg_2 = (sum_2/1000.00)/1000000.00;
	avg_3 = (sum_3/1000.00)/1000000.00;

	
	/* Liu& Layland and Hyperbolic bound not applicable if relative deadline smaller than period*/
	if(third==true)
	{   
	printf( "Liu and Layland bound: not applicable (time: 0 ms)\n");
	printf( "Hyperbolic bound: not applicable (time: 0 ms)\n");
  
		if (ans3==true){
		printf( "Response time analysis: schedulable (time: %4.6f ms)\n",avg_3);
		}       
		else {
		printf( "Response time analysis: not schedulable (time: %4.6f ms)\n",avg_3);
		}
	}

	/* Liu& Layland applicable*/	
	else if (third==false)
	{
		if (ans1==true){
		printf( "Liu and Layland bound: schedulable (time: %4.6f ms)\n",avg_1);
		}
		else {
		printf( "Liu and Layland bound: not schedulable (time: %4.6f ms)\n",avg_1);	
		}

		if (ans2==true){
		printf( "Hyperbolic bound: schedulable (time: %4.6f ms)\n",avg_2);
		}
		else {
		printf( "Hyperbolic bound: not schedulable (time: %4.6f ms)\n",avg_2);
		}
		printf( "Response time analysis: not applicable (time: 0 ms)\n");
	}
}


//Liu & Layland Bound   

bool test1()
{
   
		double sum=0;
		double u_bound=0;           
		double base =2.00;
		double exp = 1.00/total_tasks; 
		int i;   
   
		for(i=0; i<total_tasks; i++)
		{   
		sum = (task[i][0]/task[i][2])+ sum;
		}
  	u_bound = total_tasks*(pow(2.00,exp)-1);
		if(sum <= u_bound)
    {return true;}   
    else {
		return false;}
}


//Hyperbolic Bound

bool test2(void)
{   
    double H=1;
    double h_bound=2;       
    int i;   
    for(i=0; i<total_tasks; i++)
    {   
    H = (1+(task[i][0]/task[i][2]))*H;
    }
		if(H <= h_bound)
    {return true;}   
    else 
		{return false;}
}


//Response Time(exact) Test

bool test3(void)
{
		int R,I;
		int i,j;
    for( i = 0; i<total_tasks; i++)
    {
    I=0;
				do{
						R = I + task[i][0];
						for( j = 1; j<total_tasks; j++)
						{
						  I = ceil((R/task[j][2])* task[j][0]) + I;
						}
                
				}while ((1+ task[i][0]) > R);
        
			if (R > task[i][1])
			{
			return false;
			}
			else{
			return true;
			}
    }   
}
