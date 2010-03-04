/***************************************************************
*                            NanoRK CONFIG                     *
***************************************************************/

#ifndef __nrk_cfg_h	
#define __nrk_cfg_h

#define NRK_REPORT_ERRORS
// NRK_REPORT_ERRORS will cause the kernel to print out information about
// missed deadlines or reserve violations

#define NRK_HALT_ON_ERROR
// NRK_HALT_ON_ERRORS will cause the kernel to freeze on errors so that
// it is easier to see debugging messages.

//#define NRK_HALT_AND_LOOP_ON_ERROR
// NRK_HALT_AND_LOOP_ON_ERRORS will cause the kernel to freeze on errors but
// unlike NRK_HALT_ON_ERROR, the kernel panic msg will continously be printed out.

#define NRK_STACK_CHECK
// NRK_STACK_CHECK adds a little check to see if the bottom of the stack
// has been over written on all suspend calls

//#define NRK_NO_POWER_DOWN
// Leave NRK_NO_POWER_DOWN define in if the target can not wake up from sleep
// because it has no asynchronously clocked

// Disable common errors when connected to programmer
#define IGNORE_BROWN_OUT_ERROR
#define IGNORE_EXT_RST_ERROR

#define NRK_MAX_TASKS                   3
// Max number of tasks in your application
// Be sure to include the idle task
// Making this the correct size will save on BSS memory which
// is both RAM and ROM...
                           

#define NRK_TASK_IDLE_STK_SIZE          128   // Idle task stack size min=32 
#define NRK_APP_STACKSIZE               128 
#define NRK_KERNEL_STACKSIZE            128 

#define NRK_MAX_RESOURCE_CNT            1
// NRK_MAX_RESOURCE_CNT defines the number of semaphores in the system.
// If you don't use any semaphores, set this to 0.  Be sure that libraries
// you are using do not require semaphores.  These should be stated in any
// documenation that comes with them.

#endif
