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
*******************************************************************************/



#include <include.h>
#include <ulib.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <nrk.h>
#include <nrk_events.h>
#include <nrk_timer.h>
#include <nrk_error.h>
#include <nrk_reserve.h>
#include <bmac.h>
#include <nrk_cfg.h>


#ifndef BMAC_STACKSIZE
#define BMAC_STACKSIZE	128	
#endif

static nrk_task_type bmac_task;
static NRK_STK bmac_task_stack[BMAC_STACKSIZE];


//#define DEBUG
static uint32_t rx_failure_cnt;

static uint8_t tx_data_ready;
static uint8_t rx_buf_empty;
static uint8_t bmac_running;
static uint8_t pkt_got_ack;
static uint8_t g_chan;
static uint8_t is_enabled;

static nrk_time_t _bmac_check_period;

static uint8_t cca_active;
static int8_t tx_reserve;


/**
 *  This is a callback if you require immediate response to a packet
 */
RF_RX_INFO *rf_rx_callback (RF_RX_INFO * pRRI)
{
    // Any code here gets called the instant a packet is received from the interrupt   
    return pRRI;
}

int8_t bmac_encryption_set_ctr_counter(uint8_t *counter, uint8_t len)
{
if(len!=4 ) return NRK_ERROR;
rf_security_set_ctr_counter(counter);
   return NRK_OK;
}

int8_t bmac_tx_reserve_set( nrk_time_t *period, uint16_t pkts )
{

#ifdef NRK_MAX_RESERVES
// Create a reserve if it doesn't exist
if(tx_reserve==-1) tx_reserve=nrk_reserve_create();
if(tx_reserve>=0)
  return nrk_reserve_set(tx_reserve, period,pkts,NULL);
else return NRK_ERROR;
#else
return NRK_ERROR;
#endif
}

uint16_t bmac_tx_reserve_get()
{
#ifdef NRK_MAX_RESERVES
if(tx_reserve>=0)
  return nrk_reserve_get(tx_reserve);
else return 0;
#else
return 0;
#endif
}


int8_t  bmac_auto_ack_disable() 
{
rf_auto_ack_disable();
return NRK_OK;
}

int8_t  bmac_auto_ack_enable() 
{
rf_auto_ack_enable();
return NRK_OK;
}

int8_t  bmac_addr_decode_disable() 
{
rf_addr_decode_disable();
return NRK_OK;
}

int8_t  bmac_addr_decode_enable() 
{
rf_addr_decode_enable();
return NRK_OK;
}

int8_t bmac_addr_decode_set_my_mac(uint16_t my_mac)
{
rf_addr_decode_set_my_mac(my_mac);
return NRK_OK;
}

int8_t  bmac_addr_decode_dest_mac(uint16_t dest) 
{
bmac_rfTxInfo.destAddr=dest;
return NRK_OK;
}

int8_t bmac_rx_pkt_is_encrypted()
{
return rf_security_last_pkt_status();
}

int8_t bmac_encryption_set_key(uint8_t *key, uint8_t len)
{
  if(len!=16) return NRK_ERROR;
  rf_security_set_key(key);
  return NRK_OK;
}

int8_t bmac_encryption_enable()
{
  rf_security_enable();
  return NRK_OK;
}

int8_t bmac_encryption_disable()
{
  rf_security_disable();
  return NRK_OK;
}


int8_t bmac_set_rf_power(uint8_t power)
{
if(power>31) return NRK_ERROR;
rf_tx_power(power);
return NRK_OK;
}

void bmac_set_cca_active(uint8_t active)
{
cca_active=active;
}

int8_t bmac_set_cca_thresh(int8_t thresh)
{
  rf_set_cca_thresh(thresh); 
return NRK_OK;
}

int8_t bmac_set_channel(uint8_t chan)
{
if(chan>26) return NRK_ERROR;
g_chan=chan;
rf_init (&bmac_rfRxInfo, chan, 0xFFFF, 0x00000);
return NRK_OK;
}


int8_t bmac_wait_until_rx_pkt()
{
nrk_sig_mask_t event;

if(bmac_rx_pkt_ready()==1) return NRK_OK;

    nrk_signal_register(bmac_rx_pkt_signal); 
    event=nrk_event_wait (SIG(bmac_rx_pkt_signal));

// Check if it was a time out instead of packet RX signal
if((event & SIG(bmac_rx_pkt_signal)) == 0 ) return NRK_ERROR;
else return NRK_OK;
}

int8_t bmac_rx_pkt_set_buffer(uint8_t *buf, uint8_t size)
{
if(buf==NULL) return NRK_ERROR;
    bmac_rfRxInfo.pPayload = buf;
    bmac_rfRxInfo.max_length = size;
    rx_buf_empty=1;
return NRK_OK;
}

int8_t bmac_init (uint8_t chan)
{
    bmac_running=0;
    tx_reserve=-1;
    cca_active=true;
    rx_failure_cnt=0;
    #ifdef NRK_SW_WDT
	#ifdef BMAC_SW_WDT_ID
	
	_bmac_check_period.secs=30;
	_bmac_check_period.nano_secs=0;
	nrk_sw_wdt_init(BMAC_SW_WDT_ID, &_bmac_check_period, NULL );
	nrk_sw_wdt_start(BMAC_SW_WDT_ID);
	#endif
    #endif



    _bmac_check_period.secs=0;
    _bmac_check_period.nano_secs=BMAC_DEFAULT_CHECK_RATE_MS*NANOS_PER_MS;
    bmac_rx_pkt_signal=nrk_signal_create();
    if(bmac_rx_pkt_signal==NRK_ERROR)
	{
	nrk_kprintf(PSTR("BMAC ERROR: creating rx signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
	}
    bmac_tx_pkt_done_signal=nrk_signal_create();
    if(bmac_tx_pkt_done_signal==NRK_ERROR)
	{
	nrk_kprintf(PSTR("BMAC ERROR: creating tx signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
	}
    bmac_enable_signal=nrk_signal_create();
    if(bmac_enable_signal==NRK_ERROR)
	{
	nrk_kprintf(PSTR("BMAC ERROR: creating enable signal failed\r\n"));
	nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
	return NRK_ERROR;
	}

     
    tx_data_ready=0;
    // Set the one main rx buffer
    rx_buf_empty=0;
    bmac_rfRxInfo.pPayload = NULL;
    bmac_rfRxInfo.max_length = 0;

    // Setup the cc2420 chip
    rf_init (&bmac_rfRxInfo, chan, 0xffff, 0);
    g_chan=chan;
 
    FASTSPI_SETREG(CC2420_RSSI, 0xE580); // CCA THR=-25
    FASTSPI_SETREG(CC2420_TXCTRL, 0x80FF); // TX TURNAROUND = 128 us
    FASTSPI_SETREG(CC2420_RXCTRL1, 0x0A56); 
    // default cca thresh of -45
    //rf_set_cca_thresh(-45); 
    rf_set_cca_thresh(-45); 
    bmac_running=1;
    is_enabled=1;
    return NRK_OK;
}

int8_t bmac_tx_pkt_nonblocking(uint8_t *buf, uint8_t len)
{
  if(tx_data_ready==1) return NRK_ERROR;
  tx_data_ready=1;
  bmac_rfTxInfo.pPayload=buf;
  bmac_rfTxInfo.length=len;
return NRK_OK;
}

nrk_sig_t bmac_get_rx_pkt_signal()
{
   nrk_signal_register(bmac_rx_pkt_signal); 
return(bmac_rx_pkt_signal);
}

nrk_sig_t bmac_get_tx_done_signal()
{
   nrk_signal_register(bmac_tx_pkt_done_signal); 
return(bmac_tx_pkt_done_signal);
}


int8_t bmac_tx_pkt(uint8_t *buf, uint8_t len)
{
uint32_t mask;
if(tx_data_ready==1) return NRK_ERROR;
// If reserve exists check it
#ifdef NRK_MAX_RESERVES
if(tx_reserve!=-1)
	{
	if( nrk_reserve_consume(tx_reserve)==NRK_ERROR ) { 
		return NRK_ERROR;
		}
	}
#endif
nrk_signal_register(bmac_tx_pkt_done_signal); 
tx_data_ready=1;
bmac_rfTxInfo.pPayload=buf;
bmac_rfTxInfo.length=len;
#ifdef DEBUG
nrk_kprintf( PSTR("Waiting for tx done signal\r\n"));
#endif
mask=nrk_event_wait (SIG(bmac_tx_pkt_done_signal));
if(mask==0) nrk_kprintf( PSTR("BMAC TX: Error calling event wait\r\n"));
if((mask&SIG(bmac_tx_pkt_done_signal))==0) nrk_kprintf( PSTR("BMAC TX: Woke up on wrong signal\r\n"));
if(pkt_got_ack) return NRK_OK;
return NRK_ERROR;
}


uint8_t *bmac_rx_pkt_get(uint8_t *len, int8_t *rssi)
{

  if(bmac_rx_pkt_ready()==0) 
	{
	*len=0;
	*rssi=0;
	return NULL;
	}
  *len=bmac_rfRxInfo.length;
  *rssi=bmac_rfRxInfo.rssi;
  return bmac_rfRxInfo.pPayload;
}

int8_t bmac_rx_pkt_ready(void)
{
return (!rx_buf_empty);
}

int8_t bmac_rx_pkt_release(void)
{
    rx_buf_empty=1;
return NRK_OK;
}

void bmac_disable()
{
  is_enabled=0;
  rf_power_down();
}

void bmac_enable()
{
  is_enabled=1;
  rf_power_up();
  nrk_event_signal (bmac_enable_signal);
}


void bmac_nw_task ()
{
int8_t v;
int8_t e;
uint8_t backoff;
nrk_sig_mask_t event;

while(bmac_started()==0) nrk_wait_until_next_period();

//register the signal after bmac_init has been called
v=nrk_signal_register(bmac_enable_signal); 
if(v==NRK_ERROR) nrk_kprintf( PSTR("Failed to register signal\r\n"));
backoff=0;
    while (1) {
    #ifdef NRK_SW_WDT
	#ifdef BMAC_SW_WDT_ID
	nrk_sw_wdt_update(BMAC_SW_WDT_ID);
	#endif
    #endif
	if(is_enabled ) { 
	v=1;
	if(rx_buf_empty==1) v=_bmac_channel_check();
	// If the buffer is full, signal the receiving task again.
	else e=nrk_event_signal (bmac_rx_pkt_signal);
	// bmac_channel check turns on radio, don't turn off if
	// data is coming.
		if(v==0)
			{
			if(_bmac_rx()==1)
			  {
				e=nrk_event_signal (bmac_rx_pkt_signal);
				//if(e==NRK_ERROR) {
				//	nrk_kprintf( PSTR("bmac rx pkt signal failed\r\n"));
				//	printf( "errno: %u \r\n",nrk_errno_get() );
				//}
			  }
			  //else nrk_kprintf( PSTR("Pkt failed, buf could be corrupt\r\n" ));
			} 
		if(/*rx_buf_empty==1 &&*/ tx_data_ready==1)
			{
				rf_rx_off(); 
				_bmac_tx();
			}
	//do {
		nrk_wait(_bmac_check_period); 
	//	if(rx_buf_empty!=1)  nrk_event_signal (bmac_rx_pkt_signal);
	//} while(rx_buf_empty!=1);
	} else {
		event=0;
		do {
		v=nrk_signal_register(bmac_enable_signal); 
    		event=nrk_event_wait (SIG(bmac_enable_signal));
		} while((event & SIG(bmac_enable_signal))==0);
	}
	//nrk_wait_until_next_period();
	}

}


int8_t bmac_set_rx_check_rate(nrk_time_t period)
{
if(period.secs==0 && period.nano_secs < BMAC_MIN_CHECK_RATE_MS*NANOS_PER_MS)
	return NRK_ERROR;
_bmac_check_period.secs=period.secs;
_bmac_check_period.nano_secs=period.nano_secs;
return NRK_OK;
}

int8_t bmac_started()
{
return bmac_running;
}

int8_t _bmac_channel_check()
{
int8_t val;
rf_polling_rx_on();
nrk_spin_wait_us(250);
val=CCA_IS_1;
if(val) rf_rx_off(); 
return val;

}

int8_t _bmac_rx()
{
int8_t n;
uint8_t cnt;

	rf_set_rx (&bmac_rfRxInfo, g_chan);
        rf_polling_rx_on ();
	cnt=0;
	while ((n = rf_rx_check_fifop()) == 0)
	{
	cnt++;
	nrk_wait(_bmac_check_period);
	if(cnt>2) { 
			#ifdef DEBUG
			printf( "rx timeout 1 %d\r\n",cnt );
			#endif
			if(rx_failure_cnt<65535) rx_failure_cnt++;
			rf_rx_off();
			return 0;
			} 
	}
        if (n != 0) {
                n = 0;
                // Packet on its way
                cnt=0;
                while ((n = rf_polling_rx_packet ()) == 0) {
                cnt++;
                nrk_spin_wait_us(100);
                if (cnt > 50) { 
			#ifdef DEBUG
			printf( "rx timeout 2\r\n" ); 
			#endif
			rx_failure_cnt++;
			rf_rx_off(); 
			return 0;
			}        
                }
        }
        rf_rx_off();
        if (n == 1) {
                // CRC and checksum passed
    		rx_buf_empty=0;
		#ifdef DEBUG
                printf( "BMAC: SNR= %d [",bmac_rfRxInfo.rssi );
                for(uint8_t i=0; i<bmac_rfRxInfo.length; i++ )
                        printf( "%c", bmac_rfRxInfo.pPayload[i]);
                printf( "]\r\n" );
		#endif
		return 1;
        } else 
	{
	#ifdef DEBUG 
	printf( "CRC failed!\r\n" );
	#endif 
	rx_failure_cnt++;
	return 0; 
	}
rx_failure_cnt++;
return 0;
}


uint16_t bmac_rx_failure_count_get()
{
  return rx_failure_cnt;
}

uint8_t bmac_rx_failure_count_reset()
{
  rx_failure_cnt=0;
return NRK_OK;
}

int8_t _bmac_tx()
{
uint8_t v,backoff, backoff_count;
uint16_t b;

#ifdef DEBUG
nrk_kprintf( PSTR("_bmac_tx()\r\n"));
#endif
if(cca_active)
{

// Add random time here to stop nodes from synchronizing with eachother
b=_nrk_time_to_ticks(&_bmac_check_period);
b=b/((rand()%10)+1);
//printf( "waiting %d\r\n",b );
nrk_wait_until_ticks(b);
//nrk_wait_ticks(b);

	backoff_count=1;
	do{
	v=_bmac_channel_check();
	rf_rx_off(); 
	if(v==1) break;
	// Channel is busy
	backoff=rand()%(_b_pow(backoff_count));
			#ifdef DEBUG
			printf( "backoff %d\r\n",backoff );
			#endif
//	printf( "backoff %d\r\n",backoff );
	nrk_wait_until_next_n_periods(backoff);
	backoff_count++;
	if(backoff_count>6) backoff_count=6; // cap it at 64	
	b=_nrk_time_to_ticks(&_bmac_check_period);
	b=b/((rand()%10)+1);
//	printf( "waiting %d\r\n",b );
	nrk_wait_until_ticks(b);
//	nrk_wait_ticks(b);

	} while(v==0);
}

	rf_test_mode();
	rf_carrier_on(); 
	nrk_wait(_bmac_check_period);
	//nrk_wait_until_next_period();
	rf_carrier_off(); 
	rf_data_mode();
	// send packet
	rf_rx_off();
	pkt_got_ack=rf_tx_packet (&bmac_rfTxInfo);
	rf_rx_off(); 	
tx_data_ready=0;
nrk_event_signal (bmac_tx_pkt_done_signal);
return NRK_OK;
}

uint8_t _b_pow(uint8_t in)
{
uint8_t i;
uint8_t result;
if(in<=1) return 1;
if(in>7) in=6; // cap it at 128 
result=1;
for(i=0; i<in; i++ )
  result=result*2;
return result;
}


void bmac_task_config ()
{
    nrk_task_set_entry_function( &bmac_task, bmac_nw_task);
    nrk_task_set_stk( &bmac_task, bmac_task_stack, BMAC_STACKSIZE);
    bmac_task.prio = BMAC_TASK_PRIORITY;
    bmac_task.FirstActivation = TRUE;
    bmac_task.Type = BASIC_TASK;
    bmac_task.SchType = PREEMPTIVE;
    bmac_task.period.secs = 0;
    bmac_task.period.nano_secs = BMAC_MIN_CHECK_RATE_MS * NANOS_PER_MS;
    bmac_task.cpu_reserve.secs = 5;      // bmac reserve , 0 to disable
    bmac_task.cpu_reserve.nano_secs = 0;
    bmac_task.offset.secs = 0;
    bmac_task.offset.nano_secs = 0;
    #ifdef DEBUG
    printf( "bmac activate\r\n" );
    #endif 
    nrk_activate_task (&bmac_task);
}
