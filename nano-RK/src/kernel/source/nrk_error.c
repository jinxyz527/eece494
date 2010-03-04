/******************************************************************************
*  Nano-RK, a real-time operating system for sensor networks.
*  Copyright (C) 2007, Real-Time and Multimedia Lab, Carnegie Mellon University
*  All rights reserved.
*
*  This is the Open Source Version of Nano-RK included as part of a Dual
*  Licensing Model. If you are unsure which license to use please refer to:
*  http://www.nanork.org/nano-RK/wiki/Licensing
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, version 2.0 of the License.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  Contributing Authors (specific to this file):
*  Anthony Rowe
*  Zane Starr
*  Anand Eswaren
*******************************************************************************/
 
#include <nrk_error.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_task.h>
#include <nrk_cfg.h>
#include <nrk_timer.h>
#ifdef NRK_LOG_ERRORS
#include <nrk_eeprom.h>
static uint8_t error_cnt;
#endif

void pause();
void blink_morse_code_error( uint8_t number );


void _nrk_errno_set (NRK_ERRNO error_code) 
{
  nrk_cur_task_TCB->errno = error_code;
} 

uint8_t nrk_errno_get () 
{
  return nrk_cur_task_TCB->errno;
}

#ifdef NRK_LOG_ERRORS
void _nrk_log_error(uint8_t error_num, uint8_t error_task)
{
  
   // 1) Load error cnt and add 1
   error_cnt=nrk_eeprom_read_byte(NRK_ERROR_EEPROM_INDEX);
   error_cnt++;
   if(error_cnt==255) error_cnt=0;
   // 2) write error
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6),error_num);
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6+1),error_task);
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6+2),(nrk_system_time.secs>>24)&0xff);
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6+3),(nrk_system_time.secs>>16)&0xff);
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6+4),(nrk_system_time.secs>>8)&0xff);
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX+1+((uint16_t)error_cnt*6+5),(nrk_system_time.secs)&0xff);
   // 3) write error cnt back
   nrk_eeprom_write_byte(NRK_ERROR_EEPROM_INDEX,error_cnt);
}
#endif

void nrk_error_add (uint8_t n) 
{
  error_num = n;
  error_task = nrk_cur_task_TCB->task_ID;

#ifdef NRK_LOG_ERRORS
_nrk_log_error(error_num, error_task);
#endif

#ifdef NRK_REPORT_ERRORS
    nrk_error_print ();
#endif  /*  */
} 

void nrk_kernel_error_add (uint8_t n, uint8_t task) 
{
  error_num = n;
  error_task = task;
  uint8_t i;
  uint8_t t;
  
#ifdef NRK_LOG_ERRORS
_nrk_log_error(error_num, error_task);
#endif

#ifdef NRK_REPORT_ERRORS
    nrk_error_print ();
  
#endif  /*  */

#ifdef NRK_SOFT_REBOOT_ON_ERROR
#ifdef NRK_WATCHDOG
         nrk_watchdog_disable();
#endif
asm volatile("jmp 0x0000\n\t" ::);
#endif

#ifdef NRK_REBOOT_ON_ERROR
  // wait for watchdog to kick in
  if(n!=NRK_WATCHDOG_ERROR && n!=NRK_BOD_ERROR && n!=NRK_EXT_RST_ERROR)
  {
  nrk_watchdog_enable();
  nrk_int_disable(); 
  while(1);
  }
#endif
    
#ifdef NRK_HALT_ON_ERROR
    while (1)
     {
    for(i=0; i<20; i++ )
    {
    nrk_led_set (2);
    nrk_led_clr (3);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    nrk_led_set (3);
    nrk_led_clr (2);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    }
    nrk_led_clr (3);
    nrk_led_clr (2);
    blink_morse_code_error( task );
    blink_morse_code_error( n );
    }
  
#endif  /*  */
 



} 


uint8_t nrk_error_get (uint8_t * task_id, uint8_t * code) 
{
  if (error_num == 0)
    return 0;
  *code = error_num;
  *task_id = error_task;
  return 1;
}

int8_t nrk_error_print () 
{
  int8_t t,i;
  if (error_num == 0)
    return 0;

   #ifdef NRK_HALT_ON_ERROR
     nrk_int_disable ();
       #ifdef NRK_WATCHDOG
         nrk_watchdog_disable();
       #endif
   #endif 

   #ifndef NRK_REBOOT_ON_ERROR
      nrk_int_disable ();
   #endif 


#ifdef NRK_HALT_AND_LOOP_ON_ERROR 
    nrk_int_disable ();
   #ifdef NRK_WATCHDOG
      nrk_watchdog_disable();
   #endif


  while (1)
     {
    
#endif  
    
    nrk_kprintf (PSTR ("*NRK ERROR("));
    printf ("%d", error_task);
    nrk_kprintf (PSTR ("): "));
    if (error_num > NRK_NUM_ERRORS)
      error_num = NRK_UNKOWN;
    switch (error_num)
       {
    case NRK_STACK_TOO_SMALL:
      nrk_kprintf (PSTR ("Stack was not defined as large enough!"));
      break;
    case NRK_STACK_OVERFLOW:
      nrk_kprintf (PSTR ("Task Stack Overflow"));
      break;
    case NRK_INVALID_STACK_POINTER:
      nrk_kprintf (PSTR ("Invalid Stack Pointer"));
      break;
    case NRK_RESERVE_ERROR:
      nrk_kprintf (PSTR ("Reserve Error in Scheduler"));
      break;
    case NRK_RESERVE_VIOLATED:
      nrk_kprintf (PSTR ("Task Reserve Violated"));
      break;
    case NRK_WAKEUP_MISSED:
      nrk_kprintf (PSTR ("Scheduler Missed Wakeup"));
      break;
    case NRK_DUP_TASK_ID:
      nrk_kprintf (PSTR ("Duplicated Task ID"));
      break;
    case NRK_BAD_STARTUP:
      nrk_kprintf (PSTR ("Unexpected Restart"));
      break;
    case NRK_STACK_SMASH:
      nrk_kprintf (PSTR ("Idle or Kernel Stack Overflow"));
      break;
    case NRK_EXTRA_TASK:
      nrk_kprintf (PSTR ("Extra Task started, is nrk_cfg.h ok?"));
      break;
    case NRK_LOW_VOLTAGE:
      nrk_kprintf (PSTR ("Low Voltage"));
      break;
    case NRK_SEG_FAULT:
      nrk_kprintf (PSTR ("Unhandled Interrupt Vector"));
      break;
    case NRK_TIMER_OVERFLOW:
      nrk_kprintf (PSTR ("Timer Overflow"));
      break;
    case NRK_SW_WATCHDOG_ERROR:
      nrk_kprintf (PSTR ("SW Watchdog Restart"));
      break;
    case NRK_WATCHDOG_ERROR:
      nrk_kprintf (PSTR ("Watchdog Restart"));
      break;
    case NRK_DEVICE_DRIVER:
      nrk_kprintf (PSTR ("Device Driver Error"));
      break;
    case NRK_UNIMPLEMENTED:
      nrk_kprintf (PSTR ("Kernel function not implemented"));
      break;
    case NRK_SIGNAL_CREATE_ERROR:
      nrk_kprintf (PSTR ("Failed to create Signal"));
      break;
    case NRK_SEMAPHORE_CREATE_ERROR:
      nrk_kprintf (PSTR ("Failed to create Semaphore"));
      break;
    case NRK_BOD_ERROR:
      nrk_kprintf (PSTR ("Brown Out Detect"));
      break;
    case NRK_EXT_RST_ERROR:
      nrk_kprintf (PSTR ("External Reset"));
      break;
    default:
      nrk_kprintf (PSTR ("UNKOWN"));
      }
    putchar ('\r');
    putchar ('\n');

#ifdef NRK_SOFT_REBOOT_ON_ERROR
#ifdef NRK_WATCHDOG
         nrk_watchdog_disable();
#endif

GTCCR=0;
ASSR=0;

OCR0B=0;
OCR0A=0;
TCNT0=0;
TCCR0B=0;
TCCR0A=0;

EIMSK=0;
EIFR=0;
PCIFR=0;


OCR3B =0;
OCR3A =0;
TCNT3 =0;
TCCR3B=0;
TCCR3A=0;
TIFR3=0;
TIMSK3=0;


OCR2B =0;
OCR2A =0;
TCNT2 =0;
TCCR2B=0;
TCCR2A=0;
TCCR2A=0;
TIFR2=0;
TIMSK2=0;

//nrk_int_disable(); 
asm volatile(	"ldi r28,0xFF\n\t" 
		"out __SP_L__, r28\n\t" 
		"ldi r29,0x21\n\t" 
		"out __SP_H__, r29\n\t" 
		"clr r0\n\t" 
		"clr r1\n\t" 
		"clr r2\n\t" 
		"clr r3\n\t" 
		"clr r4\n\t" 
		"clr r5\n\t" 
		"clr r6\n\t" 
		"clr r7\n\t" 
		"clr r8\n\t" 
		"clr r9\n\t" 
		"clr r10\n\t" 
		"clr r11\n\t" 
		"clr r12\n\t" 
		"clr r13\n\t" 
		"clr r14\n\t" 
		"clr r15\n\t" 
		"clr r16\n\t" 
		"clr r17\n\t" 
		"clr r18\n\t" 
		"clr r19\n\t" 
		"clr r20\n\t" 
		"clr r21\n\t" 
		"clr r22\n\t" 
		"clr r23\n\t" 
		"clr r24\n\t" 
		"clr r25\n\t" 
		"clr r26\n\t" 
		"clr r27\n\t" 
		"clr r28\n\t" 
		"clr r29\n\t" 
		"clr r30\n\t" 
		"clr r31\n\t" 
		"cli\n\t" 
		"out __SREG__, r0\n\t" 
		"jmp __ctors_end\n\t" 
		::);
#endif

#ifdef NRK_REBOOT_ON_ERROR
  // wait for watchdog to kick in
  if(error_num!=NRK_WATCHDOG_ERROR && error_num!=NRK_BOD_ERROR && error_num!=NRK_EXT_RST_ERROR)
  {
  nrk_watchdog_enable();
  nrk_int_disable(); 
  while(1);
  }
#endif



//t=error_num;
#ifdef NRK_HALT_AND_LOOP_ON_ERROR
    for(i=0; i<20; i++ )
    {
    nrk_led_set (2);
    nrk_led_clr (3);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    nrk_led_set (3);
    nrk_led_clr (2);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    }

   nrk_led_clr(2);
   nrk_led_clr(3);
   blink_morse_code_error( error_task );
   blink_morse_code_error( error_num);
}
  
#endif  /*  */
    
#ifdef NRK_HALT_ON_ERROR
    while (1)
     {
    for(i=0; i<20; i++ )
    {
    nrk_led_set (2);
    nrk_led_clr (3);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    nrk_led_set (3);
    nrk_led_clr (2);
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (1000);
    }
    nrk_led_clr (3);
    nrk_led_clr (2);
    blink_morse_code_error( error_task );
    blink_morse_code_error( error_num);
    }
  
#endif  /*  */
    error_num = 0;
  return t;
}

void blink_dash()
{
uint8_t i;
   nrk_led_set (1); 
   pause(); 
   pause(); 
   pause(); 
   nrk_led_clr(1);
   pause(); 
}

void blink_dot()
{
   nrk_led_set(1); 
   pause(); 
   nrk_led_clr(1);
   pause(); 
}


void blink_morse_code_error( uint8_t number )
{
uint8_t i;
char str[3];

sprintf( str,"%d",number );

for(i=0; i<strlen(str); i++ )
{
switch( str[i])
{
case '0': blink_dash(); blink_dash(); blink_dash(); blink_dash(); blink_dash(); break;
case '1': blink_dot(); blink_dash(); blink_dash(); blink_dash(); blink_dash(); break;
case '2': blink_dot(); blink_dot(); blink_dash(); blink_dash(); blink_dash(); break;
case '3': blink_dot(); blink_dot(); blink_dot(); blink_dash(); blink_dash(); break;
case '4': blink_dot(); blink_dot(); blink_dot(); blink_dot(); blink_dash(); break;
case '5': blink_dot(); blink_dot(); blink_dot(); blink_dot(); blink_dot(); break;
case '6': blink_dash(); blink_dot(); blink_dot(); blink_dot(); blink_dot(); break;
case '7': blink_dash(); blink_dash(); blink_dot(); blink_dot(); blink_dot(); break;
case '8': blink_dash(); blink_dash(); blink_dash(); blink_dot(); blink_dot(); break;
case '9': blink_dash(); blink_dash(); blink_dash(); blink_dash(); blink_dot(); break;
}
pause();
pause();
pause();
}

}

void pause()
{
volatile uint8_t t;
    for (t = 0; t < 100; t++)
      nrk_spin_wait_us (2000);
}
