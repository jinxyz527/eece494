/* 
 * Baselide code for EECE 494, Assignment 4
 * Sathish Gopalakrishnan & Steve Wilton
 *
 * This code is the baseline DCT code. You should go through this 
 * code carefully and understand how it works, before proceeding 
 * with Tasks 1 and 2 of this assignment.
 *
 */

/* This includes the math library.  This is needed for the cos()
   function.  Note that when you compile, you have to use the -lm 
   flag, as described in the assignment handout */

#include <math.h>
#include <stdio.h>

/* Some definitions that will be useful */

#define PI 3.1416
#define ONEOVERSQ2 0.707107

static const int cos_table[8][8] = {
    { 32768,  32138,  30273,  27245,  23170,  18204,  12539,   6392 },
    { 32768,  27245,  12539,  -6392, -23170, -32138, -30273, -18204 },
    { 32768,  18204, -12539, -32138, -23170,   6392,  30273,  27245 },
    { 32768,   6392, -30273, -18204,  23170,  27245, -12539, -32138 },
    { 32768,  -6392, -30273,  18204,  23170, -27245, -12539,  32138 },
    { 32768, -18204, -12539,  32138, -23170,  -6392,  30273, -27245 },
    { 32768, -27245,  12539,   6392, -23170,  32138, -30273,  18204 },
    { 32768, -32138,  30273, -27245,  23170, -18204,  12539,  -6392 }
};

/* This is the figure that we will encode (the original spatial-domain
   array.  If you want to see what the figure is, you can draw it out
   as an 8x8 picture; low values are dark and high values are light.
   If you do this, you will see it is kind of a happy face */

static const short img[8][8] = {0,1,30,40,40,30,1,0,
                                0,35,10,8,8,10,35,0,
                                40,10,150,0,155,15,50,
                                40,0,0,0, 0,0,0,40,
                                40,0,202,10,20,203,0,40,
                                42,0,10,202,202,12,10,50,
                                0,35,8,0, 0,3,40,0,
                                0,1,40,30,30,40,2,0};

/* This is where the results will be placed.  Notice that we are
   using global variables here.  Although you learned that this is
   always a bad idea, for simple embedded systems, global variables are
   common.  */

static float f[8][8];
static float g[8][8];


/*  This function computes C as described in the notes.  Note that there
    is an extra negative sign here.  The notes are wrong; this code is
    right.  By the way, if you are not familiar with the == operator,
    the following code returns (-ONEOVERSQ2) if i is zero, and 1.0 
    otherwise; you could write this with an if statement, but this 
    is a common C shortcut. */

static float C(int i)
{
   return( (i==0) ? (-ONEOVERSQ2) : 1.0);
}


/* This is where the guts of the DCT algorithm is implemented.
   It implements the F(u,v) function as in the notes.
   The implementation is slightly different than that in the
   slides (I think it is clearer the way I do it below).  */

static float FDCT_baseline(int u, 
                  int v)
{
   float sum;
   int x, y;

   sum = 0.0;
  
   /* Calculate everything after the summation sign. */

   for(x=0;x<8;x++) {
      for(y=0;y<8;y++) {
         sum += img[x][y] *
                cos((PI*(2*x+1)*u)/16) *
                cos((PI*(2*y+1)*v)/16);
      }
   }
 
   /* Multiply this by C(u) and C(v) and 0.25 and return the
      result -- the result is F(u,v) */

  return( 0.25 * C(u) * C(v) * sum);
}



static float FDCT(int u, 
                  int v)
{
   float sum;
   int x, y;

   sum = 0.0;
  
   /* Calculate everything after the summation sign. */

   for(x=0;x<8;x++) {
      for(y=0;y<8;y++) {
         sum += img[x][y] *
                cos_table[x][u]/32768.0 *
                cos_table[y][v]/32768.0;
      }
   }
 
   /* Multiply this by C(u) and C(v) and 0.25 and return the
      result -- the result is F(u,v) */

  return( 0.25 * C(u) * C(v) * sum);
}


/*  The main program.  This calculates f[u][v] for each value of
    u and v (there are 64 different values calcuated).  To make 
    timing measurements easier, the 64 values are calculated 10,000
    times; of course, the same value is obtained each time. */

main()
{
   int u,v,it;
   float accuracy = 0;

   for(it=0;it<10000;it++) {

      for(u=0;u<8;u++) {
         for(v=0;v<8;v++) {
            /* Calculate both FDCT using both the table and the math functions */
            f[u][v] = FDCT(u,v);
            g[u][v] = FDCT_baseline(u,v);
	 }
      }
   }


   /* Calculate loss in accuracy in using the look-up table */
   for(u=0;u<8;u++) {
      for(v=0;v<8;v++) {
         printf("%f ",f[u][v]);
         accuracy += pow(f[u][v] - g[u][v], 2);
      }
      printf("\n");
   }
   accuracy = accuracy/64;
   printf("Loss in accuracy: %.10e", accuracy);
}


