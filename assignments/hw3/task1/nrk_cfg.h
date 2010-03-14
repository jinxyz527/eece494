/***************************************************************
*                            NanoRK CONFIG                     *
***************************************************************/

#ifndef __nrk_cfg_h	
#define __nrk_cfg_h

#define NRK_REPORT_ERRORS

#define NRK_HALT_AND_LOOP_ON_ERROR

#define NRK_STACK_CHECK

#define IGNORE_BROWN_OUT_ERROR
#define IGNORE_EXT_RST_ERROR

#define NRK_MAX_TASKS                   5
                           
#define NRK_MAX_RESOURCE_CNT            5

#define NRK_TASK_IDLE_STK_SIZE          128   // Idle task stack size min=32 
#define NRK_APP_STACKSIZE               256 
#define NRK_KERNEL_STACKSIZE            128 

#endif
