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
*  John Yackovich
*******************************************************************************/

#include <include.h>
#include <ulib.h>
#include <stdlib.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <nrk_eeprom.h>
#include <basic_rf.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <nrk.h>
#include <nrk_events.h>
#include <nrk_timer.h>
#include <nrk_error.h>
#include <nrk_reserve.h>
#include <tdma_asap.h>
#include <tdma_asap_scheduler.h>


#define TDMA_TREEMAKE

#ifdef TDMA_TREEMAKE
#include <tdma_asap_tree.h>
#else
    #error "fail"
#endif


// general to collect tdma statistics
#define TDMA_STATS_COLLECT

#ifdef TDMA_STATS_COLLECT
#include <tdma_asap_stats.h>
#endif

#define TDMA_TEXT_DEBUG
//#define TDMA_TEXT_DEBUG_ALL

// to use some of the pin debug symbols, #define them as 
// which pin you want to use (on PORTA)

// raise debug pin for period that interrupts are disabled
// during _tdma_tx
//#define GPIO_INT_DEBUG DEBUG_1

// raise pin when a CORRECT minipacket is received (for testing only)
//#define MPKT_CORRECT_DEBUG DEBUG_0

// raise pin at beginning and end of delta slot 
#define DELTA_SIZE_DEBUG DEBUG_1

// jam and check
// raise pin during check period until end or jam is detected
// also raise pin while jamming
#define JAM_CHECK_DEBUG DEBUG_1

// oscillate DEBUG_0 at beginning of every slot
#define GPIO_NEWTMR_SLT_DEBUG DEBUG_0

// raise pin for actual TX
//#define GPIO_TX_DEBUG DEBUG_1

// raise pin when turning on radio to send minipacket
// oscillate on SFD
//#define GPIO_MPKT_RDOON DEBUG_1

// raise the pin during the transmitter's period of checking
// channel before the TX for slot stealing
//#define STEALCHECK_DEBUG DEBUG_1

// raise the MINIPKT_DEBUG pin to correspond with the value of the FIFO pin during minipkts
// DO NOT USE: This debug setting is a failure; it screws up the timing.
//#define MINIPKT_DEBUG DEBUG_0

// track FIFOP pin using DEBUG_1 for mini packet sends/recvs
//#define GPIO_NEWSS_DEBUG DEBUG_1

// Raise DEBUG_1 during times regarding SFD
//#define GPIO_NEWTMR_DEBUG
//#define GPIO_NEWTMR_DEBUG_MASTER

// raise DEBUG_1 for SFD times
//#define GPIO_SFD_DEBUG

// tests the mini packet results
//#define MPKT_TEST

#define LED_BASIC_DEBUG
//#define GPIO_BASIC_DEBUG

//#define GPIO_SYNC_DEBUG
//#define GPIO_SLOT_DEBUG
//#define LED_DELTA_DEBUG
//#define GPIO_DELTA_DEBUG
//#define TDMA_SYNC_DEBUG

uint32_t send_value=0x5555;
uint16_t mp_correct=0;
uint16_t mp_rcv=0;

#ifdef TDMA_SYNC_DEBUG
    uint16_t sync_count = 0;
    uint16_t resync_count = 0;
    uint16_t oos_count = 0;
#endif 

uint8_t tdma_init_done = 0;
uint8_t tdma_running=0;

uint32_t mac_address;
uint16_t my_addr16=-1;

uint16_t minipkt_id = 0;

// for autoack
static uint8_t pkt_got_ack = 0;


uint8_t tdma_channel   = TDMA_DEFAULT_CHANNEL;
int8_t tdma_cca_thresh = TDMA_DEFAULT_CCA_THRESH;

nrk_time_t current_time;
nrk_time_t slot_start_time;
nrk_time_t guard_time;
nrk_time_t period_delay_time;

tdma_slot_t sync_slot;
tdma_slot_t tdma_slot;
tdma_slot_t slots_since_wakeup;
int8_t n;

tdma_node_mode_t tdma_node_mode;

// method of building the schedule.
tdma_sched_method_t tdma_schedule_method=TDMA_SCHED_MANUAL;

tdma_schedule_entry_t next_schedule_entry;

// discrepancy between estimate using microseconds of slot time
// and actual nanosecond value
uint32_t slot_leftover_nanos;
// if the nanosecond-accurate value is MORE than the
// microsecond estimate, this will be 1.
uint8_t add_slot_difference;

nrk_time_t slot_length_time;

// Time of the last sync with a master
nrk_time_t sync_time;
nrk_time_t possible_sync_time;
nrk_time_t time_since_sync;

nrk_time_t sync_offset_time;
// freshness ctr
uint8_t tdma_time_token;

// time associated with beginning of current slot
nrk_time_t time_of_slot;

// Delta slot offset.  If I have lower priority on a TX slot
// I can wait to transmit
nrk_time_t delta_slot_offset;

// Expiration period for a node to be in sync
// If a node has not sync'd for this long, it will be considered
// out of sync and will not be able to tx until sync operation
static nrk_time_t sync_tolerance_time;

// flag to note whether node is in sync
uint8_t in_sync;

int8_t rx_buf_empty;
int8_t finished_tx;

uint8_t tdma_rx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t tdma_tx_buf[RF_MAX_PAYLOAD_SIZE];

uint8_t tdma_rx_data_ready;
uint8_t tdma_tx_data_ready;
uint16_t tdma_rx_slot;

// return the 16-bit version of my address
node_addr_t tdma_mac_get()
{
    if (my_addr16 == -1)
    {
        uint32_t mac;
        read_eeprom_mac_address(&mac);
        my_addr16 = (uint16_t)(mac & 0xFFFF);
    }

    return my_addr16;
}

tdma_node_mode_t tdma_my_mode_get()
{
    return tdma_node_mode;
}

int8_t _tdma_channel_check()
{

#ifdef TDMA_STATS_COLLECT
    stats_start_rdo();
#endif
    int8_t val;
    rf_polling_rx_on();
    // 8 symbol periods
    nrk_spin_wait_us(128);
    nrk_spin_wait_us(250);
    val=CCA_IS_1;
    //if(val) rf_rx_off(); 
    rf_rx_off();

#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif

    return val;
}

/**
 *  This is a callback if you require immediate response to a packet
 */

RF_RX_INFO *rf_rx_callback (RF_RX_INFO * pRRI)
{
    // Any code here gets called the instant a packet is received from the interrupt   
    return pRRI;
}

/*************
 * tdma_sync_status
 *
 * Checks whether this node is in sync
 * returns : 1 if sync'd, 0 otherwise
 * ***********/
uint8_t tdma_sync_status()
{
    return in_sync;
}
/*
int8_t tdma_encryption_set_ctr_counter(uint8_t *counter, uint8_t len)
{
if(len!=4 ) return NRK_ERROR;
rf_security_set_ctr_counter(counter);
   return NRK_OK;
}

int8_t tdma_tx_reserve_set( nrk_time_t *period, uint16_t pkts )
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

uint16_t tdma_tx_reserve_get()
{
#ifdef NRK_MAX_RESERVES
if(tx_reserve>=0)
  return nrk_reserve_get(tx_reserve);
else return 0;
#else
return 0;
#endif
}

*/

int8_t  tdma_auto_ack_disable() 
{
rf_auto_ack_disable();
return NRK_OK;
}

int8_t  tdma_auto_ack_enable() 
{
rf_auto_ack_enable();
return NRK_OK;
}

int8_t  tdma_addr_decode_disable() 
{
rf_addr_decode_disable();
return NRK_OK;
}

int8_t  tdma_addr_decode_enable() 
{
rf_addr_decode_enable();
return NRK_OK;
}

int8_t  tdma_addr_decode_dest_mac(uint16_t dest) 
{
tdma_rfTxInfo.destAddr=dest;
return NRK_OK;
}
/*

int8_t tdma_set_rf_power(uint8_t power)
{
if(power>31) return NRK_ERROR;
rf_tx_power(power);
return NRK_OK;
}
*/

int8_t tdma_set_cca_thresh(int8_t thresh)
{
  tdma_cca_thresh = thresh;
  return NRK_OK;
}

int8_t tdma_get_cca_thresh()
{
  return tdma_cca_thresh;
}

int8_t tdma_rx_pkt_check()
{
    return tdma_rx_data_ready;
}

int8_t tdma_tx_pkt_check()
{
    return tdma_tx_data_ready; 
}

int8_t tdma_wait_until_rx_pkt()
{
    nrk_sig_mask_t event;

    if(tdma_rx_pkt_check()==1) 
        return NRK_OK;

    nrk_signal_register(tdma_rx_pkt_signal); 
    event=nrk_event_wait (SIG(tdma_rx_pkt_signal));

    // Check if it was a time out instead of packet RX signal
    if((event & SIG(tdma_rx_pkt_signal)) == 0 ) return NRK_ERROR;
    else return NRK_OK;
}

int8_t tdma_wait_until_rx_or_tx()
{
    nrk_signal_register(tdma_rx_pkt_signal);
    nrk_signal_register(tdma_tx_pkt_done_signal);
    nrk_event_wait (SIG(tdma_rx_pkt_signal) | SIG(tdma_tx_pkt_done_signal));
    return NRK_OK;

}

int8_t tdma_wait_until_tx()
{
    nrk_signal_register(tdma_tx_pkt_done_signal);
    nrk_event_wait (SIG(tdma_tx_pkt_done_signal));
    return NRK_OK;
}


int8_t tdma_rx_pkt_set_buffer(uint8_t *buf, uint8_t size)
{
    if(buf==NULL) 
        return NRK_ERROR;

    tdma_rfRxInfo.pPayload = buf;
    tdma_rfRxInfo.max_length = size;
    rx_buf_empty=1;

    return NRK_OK;
}

nrk_time_t tdma_sync_time_get()
{
    return sync_time;
}

/* tdma_init 
** Called by an outside task to initialize the TDMA
** protocol.  Does not set up radio yet anymore.
*/

int8_t tdma_init (uint8_t chan)
{

#ifdef SFD_TEST
    sfd_to_slot_time = 25000;
#endif

    // In order to maintain nanosecond granularity, calculate how different the
    // microsecond estimate and nanosecond values of a slot are, then when
    // actually calculating the time, use this to factor in.

    uint32_t nanos_per_slot = TDMA_TICKS_PER_SLOT * (uint32_t) NANOS_PER_TICK;
    uint32_t nanos_slot_estimate  = (uint32_t) TDMA_SLOT_TIME_US * NANOS_PER_US;

    if (nanos_per_slot > nanos_slot_estimate)
    {
        slot_leftover_nanos = nanos_per_slot - nanos_slot_estimate;
        add_slot_difference = 1;
    }
    else
    {
        slot_leftover_nanos = nanos_slot_estimate - nanos_per_slot;
        add_slot_difference = 0;
    }   

    slot_length_time.secs = 0;
    slot_length_time.nano_secs = TDMA_TICKS_PER_SLOT * (uint32_t) NANOS_PER_TICK;

    sync_tolerance_time.secs = TDMA_SYNC_THRESHOLD_SECS;
    sync_tolerance_time.nano_secs = 0;

    tdma_rx_slot = 0;
    tdma_channel = chan;

    sync_offset_time.secs = 0;
    sync_offset_time.nano_secs = 1 * NANOS_PER_MS;  // TX delay
    nrk_time_add(&sync_offset_time, sync_offset_time, guard_time);

    tdma_rx_data_ready = 0;
    tdma_tx_data_ready = 0;
    tdma_time_token = 0;
    finished_tx = 0;

    read_eeprom_mac_address(&mac_address);
    
    my_addr16 = (mac_address & 0xFFFF);
    tdma_rx_pkt_signal=nrk_signal_create();
    if(tdma_rx_pkt_signal==NRK_ERROR)
    {
    nrk_kprintf(PSTR("TDMA ERROR: creating rx signal failed\r\n"));
    nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
    return NRK_ERROR;
    }
    tdma_tx_pkt_done_signal=nrk_signal_create();
    if(tdma_tx_pkt_done_signal==NRK_ERROR)
    {
    nrk_kprintf(PSTR("TDMA ERROR: creating tx signal failed\r\n"));
    nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
    return NRK_ERROR;
    }
    tdma_enable_signal=nrk_signal_create();
    if(tdma_enable_signal==NRK_ERROR)
    {
    nrk_kprintf(PSTR("TDMA ERROR: creating enable signal failed\r\n"));
    nrk_kernel_error_add(NRK_SIGNAL_CREATE_ERROR,nrk_cur_task_TCB->task_ID);
    return NRK_ERROR;
    }

    tdma_init_done = 1;
    return NRK_OK;
}

/* _tdma_setup()
** Internal setup function, sets up the radio and such
** (This is called AFTER doing scheduling)
*/

uint8_t _tdma_setup()
{

    // Set the one main rx buffer
    tdma_rfRxInfo.pPayload = tdma_rx_buf;
    tdma_rfRxInfo.max_length = RF_MAX_PAYLOAD_SIZE;

    // set the default tx buffer
    tdma_rfTxInfo.pPayload = tdma_tx_buf;
    tdma_rfTxInfo.length = TDMA_DATA_START;

    // Setup the cc2420 chip
    rf_init (&tdma_rfRxInfo, tdma_channel, 0xffff, my_addr16);
 
    //FASTSPI_SETREG(CC2420_RSSI, 0xE580); // CCA THR=-25
    //FASTSPI_SETREG(CC2420_TXCTRL, 0x80FF); // TX TURNAROUND = 128 us
    //FASTSPI_SETREG(CC2420_RXCTRL1, 0x0A56); 
    // default cca thresh of -45
    rf_set_cca_thresh(tdma_cca_thresh); 
    printf("tdma chan %d cca %d\r\n", tdma_channel, tdma_cca_thresh);

#ifdef TDMA_TEXT_DEBUG_ALL
    printf("TDMA Running. I am %d.\r\n" , my_addr16);
#endif

    return NRK_OK;
}

int8_t tdma_mode_set(tdma_node_mode_t mode)
{
    tdma_node_mode = mode;
    return NRK_OK;
}

int8_t tdma_tx_pkt(uint8_t * buf, uint8_t len)
{
    if (tdma_tx_data_ready == 1)
        return NRK_ERROR;

    tdma_tx_data_ready = 1;
    tdma_rfTxInfo.pPayload = buf;
    tdma_rfTxInfo.length = len;
    
    return NRK_OK;

}

int8_t tdma_rx_pkt_release(void)
{
    tdma_rx_data_ready=0;
    return NRK_OK;

}

uint8_t* tdma_rx_pkt_get (uint8_t *len, int8_t *rssi,uint8_t *slot)
{
if(tdma_rx_pkt_check()==0)
    {
    *len=0;
    *rssi=0;
    *slot=0;
    return NULL;
    }
  *len=tdma_rfRxInfo.length;
  *rssi=tdma_rfRxInfo.rssi;
  *slot=tdma_rx_slot;

return tdma_rfRxInfo.pPayload;
}

int8_t tdma_addr_decode_set_my_mac(uint16_t my_mac)
{
    rf_addr_decode_set_my_mac(my_mac);
    return NRK_OK;
}


/**
 * _tdma_rx()
 *
 * This is the low level RX packet function.  It will read in
 * a packet and buffer it in the link layer's single RX buffer.
 * This buffer can be checked with rtl_check_rx_status() and 
 * released with rtl_release_rx_packet().  If the buffer has not
 * been released and a new packet arrives, the packet will be lost.
 * This function is only called from the timer interrupt routine.
 *
 * Arguments: slot is the current slot that is actively in RX mode.
 */
void _tdma_rx (uint16_t slot)
{
    uint8_t heard_sender, n,v;
    uint16_t tmp_slot;
    int8_t bits_left;
    uint32_t mini_pkt_val;
    uint16_t hst_rx_timeout, hst_rx_wait;
    uint8_t mpkt_sender;
    uint16_t mpkt_rcvid;

    //nrk_kprintf(PSTR("Calling tdma_rx()\r\n"));
    heard_sender = 0;


    /*
    ** Slot stealing RX procedure
    ** -- LISTEN for delta slots
    ** --   AT DETECT, start to jam channel 
    ** -- RX at begin of real slot 
    */

    #ifdef GPIO_DELTA_DEBUG
        nrk_gpio_set (NRK_DEBUG_1);
    #endif

    #if defined(SLOT_STEALING) && defined(STEAL_G)
        // wait a slot guard time to offset everything
        while(_nrk_high_speed_timer_get() < DELTA_SLOT_OFFSET);

        // receive the mini-packet
        bits_left= 15;
        mini_pkt_val = 0;

        // wait for first bit to arrive
        nrk_gpio_raw_direction(DDRC,FIFO,NRK_PIN_INPUT);

        //_rf_rx_set_unbuffered(); // put the receiver in serial mode, wait for sfd
#ifdef TDMA_STATS_COLLECT
        stats_start_rdo();      
#endif
        rf_rx_set_serial();
        rf_rx_on();


    #endif

        // CHECKPOINT: Beginning of slot (or later)

        //_nrk_high_speed_timer_stop();
        //_nrk_high_speed_timer_reset();
        //_nrk_high_speed_timer_start();

    uint16_t timeout = DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET;

#ifdef SLOT_STEALING
// if this is a rxsync slot, skip everything in the slot stealing section
if (next_schedule_entry.priority != 1)
{
    while(_nrk_high_speed_timer_get() < timeout)
    {

    // If we're using new slot stealing, the master will wait for a fifop
    // pin raise from a serial packet, rather than just signal noise
    // the hope here is that two packets sent will collide and result in no stolen slot
    #ifdef STEAL_G
        if (SFD_IS_1)
        {
            #ifdef DELTA_SIZE_DEBUG
                PORTA |= BM(DELTA_SIZE_DEBUG);
            #endif
            #ifdef GPIO_NEWSS_DEBUG
                PORTA &= ~BM(DEBUG_1);
            #endif
    #else
            //if (!heard_sender && !(v = _tdma_channel_check())) // if channel activity
    #endif

    #ifdef STEAL_G

            while (!(PINE & 128)); // same thing, wait til fifop raise
            DISABLE_GLOBAL_INT();

            #ifdef GPIO_NEWSS_DEBUG
                PORTA |= BM(GPIO_NEWSS_DEBUG);
            #endif

            while (bits_left != -1)
            {
                mini_pkt_val |= FIFO_IS_1;

            #ifdef MINIPKT_DEBUG
                if (FIFO_IS_1)
                    PORTA |= BM(MINIPKT_DEBUG);
            #endif
                
                mini_pkt_val <<= 1;
                bits_left--;

                while(FIFOP_IS_1);
                #ifdef GPIO_NEWSS_DEBUG
                    PORTA &= ~BM(GPIO_NEWSS_DEBUG);
                #endif

                while(!FIFOP_IS_1);
                #ifdef GPIO_NEWSS_DEBUG
                    PORTA |= BM(GPIO_NEWSS_DEBUG);
                #endif

            #ifdef MINIPKT_DEBUG
                PORTA &= ~BM(MINIPKT_DEBUG);
            #endif
            }
            mini_pkt_val >>=1;
            #ifdef GPIO_NEWSS_DEBUG
                PORTA &= ~BM(GPIO_NEWSS_DEBUG);
            #endif

            #ifdef DELTA_SIZE_DEBUG
                PORTA &= ~BM(DELTA_SIZE_DEBUG);
            #endif
            ENABLE_GLOBAL_INT();

            // reset the radio so a new packet could be received
            //rf_rx_off();
            //rf_data_mode();
            //rf_rx_set_serial();
                    
#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif

    //PORTA |= BM(DEBUG_1);
                    
#ifdef TDMA_STATS_COLLECT
/*
    // get the time of the mini packet arrival
    nrk_time_get(&current_time);
    nrk_time_compact_nanos(&current_time);
    // bottom 4 bits are the address
    // add 1 to adjust (cause nodes go 1-16 instead of 0-15
    mpkt_sender = (mini_pkt_val & 0x0F)+1;
    
    // top 12 bits are the event_id
    mpkt_rcvid = (mini_pkt_val >> 4);
*/
#endif

    //PORTA &= ~BM(DEBUG_1);

                // register minipacket as rcv
                //if (mini_pkt_val == send_value)
                /*
                if (mini_pkt_val == 0xAAAA || mini_pkt_val == 0x5555)
                {
                    mp_correct++;
                    #ifdef MPKT_CORRECT_DEBUG
                        PORTA |=   BM(MPKT_CORRECT_DEBUG);
                        PORTA &=  ~BM(MPKT_CORRECT_DEBUG);
                    #endif
                }
                mp_rcv++;
                */

                //printf("minipkt from %lu= %lu %u, %u\r\n", mini_pkt_val, send_value, mp_correct, mp_rcv);
                //printf("minipkt from 0x%02lX %u, %u\r\n", mini_pkt_val, mp_correct, mp_rcv);

#ifdef TDMA_STATS_COLLECT
    stats_start_rdo();
#endif
                //rf_rx_on();

                //make the new timeout the end of the current delta slot
                //timeout = TDMA_DELTA_SLOT_TICKS * ((_nrk_high_speed_timer_get() / TDMA_DELTA_SLOT_TICKS)+1);
                //Changed 13 Jan 2010 : don't wait any longer than you have to

                timeout = 0;

    #else
                rf_test_mode();
                rf_carrier_on();
    #endif
                heard_sender = 1;
                //nrk_spin_wait_us(10);

    #ifdef GPIO_DELTA_DEBUG
                nrk_gpio_clr (NRK_DEBUG_1);
                nrk_gpio_set (NRK_DEBUG_1);
    #endif
    #ifdef LED_DELTA_DEBUG
                nrk_led_set  (RED_LED);
    #endif

        }
    } // while waiting for delta slots to end

    rf_carrier_off();
    rf_rx_off();
    #ifdef TDMA_STATS_COLLECT
        stats_stop_rdo();
    #endif

    // if I heard exactly one sender, then I will jam the channel
    // to notify potential sender and deter any losers.
    #ifdef STEAL_G

        // if priority is 1, it's a sync slot so we still want to listen.
        // (1 is not a real priority with RX, just a way of telling)
        if (heard_sender!= 1 && next_schedule_entry.priority!=1) 
        {
            // No one sent a signal; I'm not expecting a packet,
            // so I can finish and sleep.
            #ifdef TDMA_TEXT_DEBUG_ALL
                nrk_kprintf(PSTR("No sender heard\r\n"));
            #endif

            //nrk_spin_wait_us(100);

            //_rf_rx_set_buffered();
            rf_carrier_off();
            rf_rx_off();
            rf_data_mode();
            return;
        }

    
        // time to jam, because I heard a minipacket
#ifdef TDMA_PREAMBLE_HANDSHAKE

        if (next_schedule_entry.priority!=1)
        {
            rf_tx_set_serial();
            //rf_set_preamble_length(4); // send 2 more bytes to make it equal to minipacket size

            nrk_gpio_raw_direction(DDRC, FIFO, NRK_PIN_OUTPUT);
            // keep sending preambles repeatedly

            // MOD_CCA 13 jan 2010 : Just throw on the radio in serial mode and let it roll
    #ifdef TDMA_STATS_COLLECT
                stats_start_rdo();
    #endif
            rf_carrier_on();

        #ifdef JAM_CHECK_DEBUG
            PORTA |= BM(JAM_CHECK_DEBUG);
        #endif

            // this timing needs perfected
            //while (_nrk_high_speed_timer_get() < (DATA_SLOT_START_TICKS-TDMA_DELTA_SLOT_TICKS+500))
            while (_nrk_high_speed_timer_get() < (DATA_SLOT_START_TICKS+DELTA_SLOT_OFFSET))
            {

                /*
                rf_carrier_on();
                while (!SFD_IS_1 || !FIFOP_IS_1);
                //nrk_spin_wait_us(64);
                //nrk_spin_wait_us(128);
                
                nrk_high_speed_timer_wait(0,TDMA_DELTA_SLOT_TICKS * ((_nrk_high_speed_timer_get() / TDMA_DELTA_SLOT_TICKS)+1));
                rf_carrier_off();
                */

            } 

    #ifdef TDMA_STATS_COLLECT
                stats_stop_rdo();
    #endif

            // MOD_CCA 13 Jan 2010
            rf_carrier_off();

        #ifdef JAM_CHECK_DEBUG
            PORTA &= ~BM(JAM_CHECK_DEBUG);
        #endif

            //rf_set_preamble_length(2); // 3 bytes, 802.15.4 compliant

            nrk_gpio_raw_direction(DDRC,FIFO,NRK_PIN_INPUT);
            rf_rx_off();
            rf_data_mode();
        }
        else
        {
            // if it's a sync packet expected, dont' bother jamming channel.
            nrk_high_speed_timer_wait(0, DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET);
        }
#else // no preamble handshake
    #ifdef TDMA_STATS_COLLECT
        stats_start_rdo();
    #endif
        rf_test_mode();
        rf_carrier_on();

        #ifdef JAM_CHECK_DEBUG
            PORTA |= BM(JAM_CHECK_DEBUG);
        #endif

        nrk_high_speed_timer_wait(0, DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET);

        rf_carrier_off();
        rf_data_mode();
    #ifdef TDMA_STATS_COLLECT
        stats_stop_rdo();
    #endif

#endif


        #ifdef JAM_CHECK_DEBUG
            PORTA &= ~BM(JAM_CHECK_DEBUG);
        #endif
    #endif // ifdef STEAL_G

    // this won't work now
//  #ifdef MPKT_TEST
//          if (heard_sender == 0)
//          {
//              nrk_kprintf(PSTR("NOMPKT\r\n"));
//          }
//          else if (heard_sender == 1 && (mini_pkt_val != 0x5555 && mini_pkt_val != 0xAAAA))
//          {
//              nrk_kprintf(PSTR("COLLISION\r\n"));
//          }
//          else if (heard_sender > 1)
//          {
//              nrk_kprintf(PSTR("MULTIPLE\r\n"));
//          }
//          else
//          {
//              printf("PKTOK %d\r\n", mini_pkt_val);
//          }
//  #endif // ifdef MPKT_TEST

    #ifdef GPIO_DELTA_DEBUG
        nrk_gpio_clr (NRK_DEBUG_1);
    #endif
    #ifdef LED_DELTA_DEBUG
        nrk_led_clr  (RED_LED);
    #endif


    #ifdef GPIO_BASIC_DEBUG
        nrk_gpio_set(NRK_DEBUG_1);
    #endif

    // NEW: wait for a litle bit to make sure I don't catch some of my
    // own signal.
    //nrk_spin_wait_us(500);

}
else // if a rxsync slot
{
    nrk_high_speed_timer_wait(0,timeout);
}
#else
    nrk_high_speed_timer_wait(0,timeout);
#endif // if SLOT_STEALING

    // Data portion of slot has started; start receive.

    //_rf_rx_set_buffered();
    //rf_flush_rx_fifo();

#ifdef TDMA_STATS_COLLECT
    stats_start_rdo();
#endif

    rf_data_mode();
    //tdma_auto_ack_enable();
    //tdma_addr_decode_set_my_mac(tdma_mac_get()); 
    //tdma_addr_decode_enable();
    rf_polling_rx_on ();

     n = 0;


        // Set a limit for the high speed timer to represent a RX timeout
        _nrk_high_speed_timer_reset();
        _nrk_high_speed_timer_start();

        // 1000 us per ms, 8 ticks per us
        hst_rx_timeout = (uint16_t) TDMA_RX_WAIT_MS * 1000 * 8;
        
        while (!SFD_IS_1)
        //while ((n = rf_rx_check_fifop()) == 0)
        {
            hst_rx_wait = _nrk_high_speed_timer_get();

            if (hst_rx_wait > hst_rx_timeout)
            {
#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif
                rf_rx_off ();
#ifdef GPIO_BASIC_DEBUG
                nrk_gpio_clr(NRK_DEBUG_1);
#endif
#ifdef LED_BASIC_DEBUG
                nrk_led_clr(GREEN_LED);
#endif

                // FIXME: add this back later
                // rtl_debug_dropped_pkt();
                //nrk_gpio_clr(NRK_DEBUG_1);
                //printf("Leaving tx  %lu :  %lu\r\n",current_time.secs,current_time.nano_secs);
#ifdef TDMA_TEXT_DEBUG
                nrk_kprintf(PSTR("norx\r\n"));
#endif

#if defined(TDMA_STATS_COLLECT) && defined(SLOT_STEALING)
                //sprintf(mpkt_log[mpkt_log_curbuf++], "DELR %u %u %u %lu %lu", mpkt_sender, mpkt_rcvid, my_addr16,
                //        current_time.secs, current_time.nano_secs);
#endif
                return;
            }
        }

    #ifdef LED_BASIC_DEBUG
        nrk_led_set(GREEN_LED);
    #endif

        // CHECKPOINT: Just received SFD
        _nrk_high_speed_timer_reset();
        _nrk_high_speed_timer_start();

#ifdef GPIO_NEWTMR_DEBUG
        PORTA &= ~BM(DEBUG_1); 
        PORTA |= BM(DEBUG_1);
        PORTA &= ~BM(DEBUG_1); // make sure pin is down
#endif

        // wait for fifop
        // XXX no error checking
        do
        {}
        while((n = rf_rx_check_fifop()) == 0);

        possible_sync_time = current_time;
        nrk_time_compact_nanos(&possible_sync_time);
     
#ifdef GPIO_BASIC_DEBUG
        nrk_gpio_clr(NRK_DEBUG_1);
#endif
        
#ifdef GPIO_BASIC_DEBUG
        nrk_gpio_set(NRK_DEBUG_1);
#endif

        uint8_t rx_timeout = _nrk_os_timer_get() + 4;
        if (n != 0) 
        {
            n = 0;
            // Packet on its way
            while ((n = rf_polling_rx_packet ()) == 0) 
            {
                if (_nrk_os_timer_get() > rx_timeout) 
                {
                    #ifdef TDMA_TEXT_DEBUG
                        nrk_kprintf( PSTR("rxtimeout\r\n") );
                    #endif
                    break;          // huge timeout as failsafe
                }
            }
        } 


#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif

    rf_rx_off ();

#ifdef GPIO_BASIC_DEBUG
    nrk_gpio_clr(NRK_DEBUG_1);
#endif

    if (n == 1)
    {

        uint8_t tmp_token = tdma_rfRxInfo.pPayload[TDMA_TIME_TOKEN] & 0x7F;

        // check the received time token against current one
        // if it's newer, sync to it.
        // token loops around, so allow some slack
        if (tdma_node_mode != TDMA_MASTER && (tmp_token > tdma_time_token || 
                    (tdma_time_token > 110 && tmp_token < 10 )))
        {   
            
            //we have a valid packet, so sync to it
            nrk_time_sub(&sync_time, possible_sync_time, sync_offset_time);
            nrk_time_compact_nanos(&sync_time);
            //sync_time = possible_sync_time;
            //tdma_rx_slot = slot;
            tmp_slot = tdma_rfRxInfo.pPayload[TDMA_SLOT];
            tmp_slot <<= 8;
            tmp_slot |= tdma_rfRxInfo.pPayload[TDMA_SLOT + 1];

#ifdef TDMA_SYNC_DEBUG
            resync_count++;

            #ifdef SFD_TEST
            printf("RSYNC %d %d %d %u\r\n",
                    my_addr16, resync_count, tmp_token, sfd_to_slot_time);
            #else
            //printf("RSYNC %d %d %d %lu %lu\r\n", 
            //        my_addr16, resync_count, tmp_token, sync_time.secs, sync_time.nano_secs);
            //nrk_kprintf(PSTR("RSYNC\r\n"));
            #endif


            
#endif

            nrk_led_toggle(ORANGE_LED);
            sync_slot = tmp_slot;

            //update my token
            tdma_time_token = tmp_token;

            // lastly, wait a time to exactly the next slot
            // then reset the OS timer so that
            // its tick lines up with a start of a slot.
            //uint8_t ost = _nrk_os_timer_get();
            _nrk_os_timer_stop();
            _nrk_os_timer_reset();

            //DISABLE_GLOBAL_INT();
            //_nrk_os_timer_set(5); // I don't know what the hell this is for
#ifdef SFD_TEST
            nrk_high_speed_timer_wait(0, sfd_to_slot_time);
#else
            nrk_high_speed_timer_wait(0, SFD_TO_SLOT_TIME-DELTA_SLOT_OFFSET);
#endif

            _nrk_os_timer_reset();
            _nrk_os_timer_start();

#ifdef GPIO_NEWTMR_DEBUG
            PORTA &= ~BM(DEBUG_1); 
            PORTA |= BM(DEBUG_1);
            PORTA &= ~BM(DEBUG_1); // make sure pin is down
#endif

            //ENABLE_GLOBAL_INT();
            nrk_time_get(&sync_time);
            nrk_time_compact_nanos(&sync_time);

            sync_slot+=1;
            if (sync_slot >= TDMA_SLOTS_PER_CYCLE)
                sync_slot=0;

            // we waited until the next slot
            time_of_slot = sync_time;
            tdma_slot = sync_slot;

        }
        else
        {
            //nrk_kprintf(PSTR("Not syncing, time token is old\r\n"));
        }
    
        //if (rx_callback != NULL)
            //rx_callback (slot);

        // check if packet is an explicit time sync packet
        if((tdma_rfRxInfo.pPayload[TDMA_TIME_TOKEN]&0x80)==0)
        {
            // if we got a good packet, send the signal to
            // the application.  Shouldn't need to check rx
            // mask here since this should only get called by real
            // rx slot.
            tdma_rx_data_ready = 1;
            tdma_rx_slot = slot;
            // moved to before sleep
            //nrk_event_signal (tdma_rx_pkt_signal);
        }
        else
        { 
            // If it is an explicit time sync packet, then release it
            // so it doesn't block a buffer...
            //nrk_kprintf( PSTR("got explicit sync\r\n") );
            tdma_rx_pkt_release(); 
        }
    } 
#ifdef TDMA_TEXT_DEBUG
    else 
        printf( "RX: Error = %d\r\n",(int8_t)n );           
#endif

#ifdef LED_BASIC_DEBUG
    nrk_led_clr (GREEN_LED);
#endif

#if defined(TDMA_STATS_COLLECT) && defined(SLOT_STEALING)
                //sprintf(mpkt_log[mpkt_log_curbuf++], "DELR %u %u %u %lu %lu", mpkt_sender, mpkt_rcvid, my_addr16,
                //        current_time.secs, current_time.nano_secs);
#endif

    //printf("mp %02lX\r\n",mini_pkt_val);
    //printf("minipkt from 0x%02lX %u, %u\r\n", mini_pkt_val, mp_correct, mp_rcv);

} // END TDMA_RX

/**
 * _tdma_tx()
 *
 * This function is the low level TX function.
 * It is only called from the timer interrupt and fetches any
 * packets that were set for a particular slot by rtl_tx_packet().
 *
 * Arguments: slot is the active slot set by the interrupt timer.
 */
void _tdma_tx (uint16_t slot)
{
    int8_t explicit_tsync;
    uint32_t g;
    int8_t bits_left;
    int8_t v;
    //uint16_t dest_tmp;


    if (tdma_sync_status () == 0)
        return;                 // don't tx if you aren't sure you are in sync

    //if (tx_callback != NULL)
     ////   tx_callback (slot);
    // Copy the element from the smaller vector of TX packets
    // to the main TX packet

    
    tdma_rfTxInfo.pPayload[TDMA_SLOT] = (-1 >> 8);
    tdma_rfTxInfo.pPayload[TDMA_SLOT + 1] = (-1 & 0xFF);

    // This clears the explicit sync bit
    tdma_rfTxInfo.pPayload[TDMA_TIME_TOKEN]= -1; 
    explicit_tsync=0;

    // If it is an empty packet set explicit sync bit
    if(tdma_rfTxInfo.length==TDMA_DATA_START )
    {
        explicit_tsync=1;
        tdma_rfTxInfo.pPayload[TDMA_TIME_TOKEN]|= 0x80;
    } 
    else
    {
       // printf("Sending packet content: %s\r\n", &tx_buf[TDMA_DATA_START]);

    }
     

    /*
    ** The slot stealing procedure is this...
    ** -- SENSE the channel for the number of delta slots equal to my priority
    ** -- CHECK if I have detected activity.  If yes, then call the whole thing off.
    ** -- FLOOD the channel for the remainder of delta slots.
    ** -- WAIT the standard TX Guard Time
    ** -- TRANSMIT my packet
    */

#ifdef SLOT_STEALING

if (next_schedule_entry.type == TDMA_TX_PARENT)
{

    // If it's currently past my delta slot already, I can't transmit.
    //if (nrk_time_cmp(current_time, next_delta_offset_time) > 0)
    if (_nrk_high_speed_timer_get() > (TDMA_DELTA_SLOT_TICKS * (next_schedule_entry.priority+1)))
    {
        // I'm late!  Forget it

        nrk_time_get(&current_time);
        nrk_time_compact_nanos(&current_time);
        printf("Late for TX: %lu %lu %lu %lu\r\n",
            current_time.secs, current_time.nano_secs, time_of_slot.secs, time_of_slot.nano_secs);

        rf_rx_off();
        return;
    }
    
    // if I'm a potential stealer, I need to sense the channel to see if it's
    // busy, meaning either the sender or receiver is jamming it.
    // Right now, if it's past the start of my delta slot, just checks once.
    if (next_schedule_entry.priority > 0)
    {
#ifdef STEALCHECK_DEBUG
        PORTA |= BM(STEALCHECK_DEBUG);
        //PORTA |= BM(DEBUG_1);
#endif

#ifdef TDMA_PREAMBLE_HANDSHAKE
    #ifdef TDMA_STATS_COLLECT
        stats_start_rdo();
    #endif
        //rf_rx_on();
        // MOD_CCA 13 Jan 2010: 
        rf_set_cca_mode(2);
        rf_polling_rx_on();
        nrk_spin_wait_us(128); // 8 symbol periods
#else
    #ifdef TDMA_STATS_COLLECT
        stats_start_rdo();
    #endif
        rf_polling_rx_on();
        nrk_spin_wait_us(128); // 8 symbol periods
#endif

        g = 0;
        do
        {
            g++;
            //nrk_kprintf(PSTR("Checking Channel\r\n"));
#ifdef TDMA_PREAMBLE_HANDSHAKE
            //if (SFD_IS_1)
            // MOD_CCA 13 Jan 2010: use CCA (modified method)
            if (!CCA_IS_1)
#else
            //if (!_tdma_channel_check()) // if channel activity
            if (!CCA_IS_1)
#endif
            {
                // Detected channel activity
#ifdef TDMA_TEXT_DEBUG
                nrk_kprintf(PSTR("nosteal\r\n"));
#endif

#ifdef TDMA_STATS_COLLECT
                steal_backoff_log();
#endif
                rf_rx_off();
#if defined(TDMA_PREAMBLE_HANDSHAKE) && defined(TDMA_STATS_COLLECT)
                stats_stop_rdo();
#elif defined(TDMA_STATS_COLLECT) // no preamble, but still collecting stats
                stats_stop_rdo();
#endif
                nrk_led_set(GREEN_LED);
                rf_set_cca_mode(1);
                return;
            }
        }
        while (_nrk_high_speed_timer_get() < (TDMA_DELTA_SLOT_TICKS * (next_schedule_entry.priority * 2)));
#ifdef STEALCHECK_DEBUG
        PORTA &= ~BM(STEALCHECK_DEBUG);
        //PORTA &= ~BM(DEBUG_1);
#endif
    }

#if defined(TDMA_PREAMBLE_HANDSHAKE) && defined(TDMA_STATS_COLLECT)
    stats_stop_rdo();
#endif
    // done checking channel
#ifdef TDMA_PREAMBLE_HANDSHAKE
    // MOD_CCA 13 Jan 2010
    rf_set_cca_mode(1);
#endif

    rf_rx_off();

#ifdef GPIO_DELTA_DEBUG
    nrk_gpio_set (NRK_DEBUG_1);
#endif

#ifdef LED_DELTA_DEBUG
    nrk_led_set (RED_LED);
#endif

#ifdef STEAL_G

    // set the radio into serial TX mode, which will send a SFD that
    // the parent can recognize.
    rf_tx_set_serial();
    
#ifdef DELTA_SIZE_DEBUG
    PORTA |= BM(DELTA_SIZE_DEBUG);
#endif

#else
    // Just do jamming, no packet
    rf_test_mode();
#endif

#ifdef STEAL_G
    // wait for FIFOP pin to raise, then we'll transmit a bit sequence to identify myself
    nrk_gpio_raw_direction(DDRC, FIFO, NRK_PIN_OUTPUT);

    bits_left = 15;

#ifdef TDMA_STATS_COLLECT
    // XXX this only works with nodes up to 16
    // lowest 4 bits is the node addr
    uint32_t minipkt = (my_addr16 - 1) & 0x0F; 

    // upper 12 bits are minipacket ID, starting from 0, which will increment.
    minipkt |= (minipkt_id << 4);
    minipkt_id++;
#else
    uint32_t minipkt = my_addr16;
#endif


    //uint32_t minipkt = 0x5555;
    //uint32_t minipkt = send_value = (send_value ==  0x5555 ? 0xAAAA : 0x5555);

    /*
    #ifdef GPIO_TX_DEBUG
        PORTA |= BM(DEBUG_1);
        PORTA &= ~BM(DEBUG_1);
    #endif
    */

/* THIS BLOCK OF CODE IS VERY TIME SENSITIVE! SENDING UNBUFFERED SERIAL OUTPUT */

#ifdef GPIO_MPKT_RDOON
    PORTA |= BM(GPIO_MPKT_RDOON);
#endif

#ifdef TDMA_STATS_COLLECT
    stats_start_rdo();
#endif

    rf_carrier_on(); // enable TX
#ifdef GPIO_MPKT_RDOON
    PORTA &= ~BM(GPIO_MPKT_RDOON);
#endif

    while (!SFD_IS_1);

#ifdef GPIO_MPKT_RDOON
    PORTA |= BM(GPIO_MPKT_RDOON);
    PORTA &= ~BM(GPIO_MPKT_RDOON);
#endif
    while (bits_left != -1)
    {

        if (minipkt & 0x00008000)
        {
            PORTC |= BM(FIFO);
            #ifdef MINIPKT_DEBUG
                PORTA |= BM(MINIPKT_DEBUG);
            #endif
        }
        else
        {
            PORTC &= ~BM(FIFO);
        }
        minipkt <<=1;
        bits_left--;

        while(!FIFOP_IS_1);
        #ifdef GPIO_NEWSS_DEBUG
            PORTA |= BM(GPIO_NEWSS_DEBUG);
        #endif

        while(FIFOP_IS_1);
        #ifdef GPIO_NEWSS_DEBUG
            PORTA &= ~BM(GPIO_NEWSS_DEBUG);
        #endif

        #ifdef MINIPKT_DEBUG
            PORTA &= ~BM(MINIPKT_DEBUG);
        #endif

    }
    
    // Important: Can't shut off the radio yet! 
    // Wait for the last bit to send first.
    nrk_spin_wait_us(5);

    // Makes sure the pin is set to input again!! Otherwise this will screw up recvs 
    nrk_gpio_raw_direction(DDRC,FIFO,NRK_PIN_INPUT);
    rf_carrier_off();

#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif

#ifdef DELTA_SIZE_DEBUG
    PORTA &= ~BM(DELTA_SIZE_DEBUG);
#endif
/* END TIME CRITICAL BLOCK */

    //printf("mpkt %lu\r\n", minipkt);
    //printf("mpkt %lu\r\n", send_value);

    #ifdef TDMA_STATS_COLLECT
        // submit the minipacket log using sprintf
        if (next_schedule_entry.type == TDMA_TX_PARENT)
        {
            nrk_time_get(&current_time);
            nrk_time_compact_nanos(&current_time);
        }
    #endif
    
#else // if ndef STEAL_G
    rf_carrier_on();
#endif


    g = 0;
    // jam for my entire delta slot, no longer

    // stop jamming
    rf_carrier_off();


#ifdef GPIO_DELTA_DEBUG
    nrk_gpio_clr (NRK_DEBUG_1);
#endif

#ifdef LED_DELTA_DEBUG
    nrk_led_clr (RED_LED);
#endif

//both will have been awake from here on

/*
#ifdef GPIO_TX_DEBUG 
    PORTA |= BM(DEBUG_1);
    PORTA &= ~BM(DEBUG_1);
#endif
*/

#ifdef GPIO_INT_DEBUG
    PORTA |= BM(GPIO_INT_DEBUG);
#endif
    
    //DISABLE_GLOBAL_INT();

    v = 0;

    // Don't do any of the following if I own this slot or if it's a txsync.  I'm just going to assume the parent heard me.
#ifdef TDMA_OWNER_NO_ACK
    // omit listening to parent for acknolwedgement of minipacket if I own the slot
    if (next_schedule_entry.type == TDMA_TX_PARENT && next_schedule_entry.priority > 0)
#else
    // owner slots will wait for acknowledgement
    if (next_schedule_entry.type == TDMA_TX_PARENT)
#endif
    {
        // first make sure it's the end of my delta slot 
       //nrk_high_speed_timer_wait(0, (DELTA_SLOT_OFFSET + (TDMA_DELTA_SLOT_TICKS * ((next_schedule_entry.priority*2) + 1))));

#ifdef TDMA_PREAMBLE_HANDSHAKE
    #ifdef TDMA_STATS_COLLECT
        stats_start_rdo();
    #endif
        // MOD_CCA 13 Jan 2010
        //rf_rx_set_serial();
        //rf_rx_on();
        rf_set_cca_mode(2); // set CCA mode to check if receiving valid 802.15 data.
        rf_polling_rx_on();
        nrk_spin_wait_us(128); // 8 symbol periods
#ifdef JAM_CHECK_DEBUG
    PORTA |= BM(JAM_CHECK_DEBUG);
#endif
        while (_nrk_high_speed_timer_get() < (DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET) &&
            (v = CCA_IS_1));
            //(v = !(SFD_IS_1)));
#else // no preamble handshake
    #ifdef TDMA_STATS_COLLECT
        stats_start_rdo();
    #endif
        rf_polling_rx_on();
        nrk_spin_wait_us(128);
        // wait while we are still before the data slot start time and the channel is free
#ifdef JAM_CHECK_DEBUG
    PORTA |= BM(JAM_CHECK_DEBUG);
#endif
        while (_nrk_high_speed_timer_get() < (DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET) &&
            //(v = _tdma_channel_check()));
            (v = CCA_IS_1));
    #ifdef TDMA_STATS_COLLECT
        stats_stop_rdo();
    #endif
#endif

        // if channel was clear all the way, master is 
        // NOT giving the OK to send the packet
        if (v)
        {
            //nrk_kprintf(PSTR("ClrChan\r\n"));
    #ifdef JAM_CHECK_DEBUG
        PORTA &= ~BM(JAM_CHECK_DEBUG);
    #endif
    #ifdef TDMA_TEXT_DEBUG
           printf("noack %d p%d\r\n", next_schedule_entry.slot, next_schedule_entry.priority);
    #endif
            nrk_led_set(GREEN_LED);
    #ifdef TDMA_PREAMBLE_HANDSHAKE
        // MOD_CCA 13 Jan 2010
        rf_set_cca_mode(1);
    #endif
    #if defined(TDMA_PREAMBLE_HANDSHAKE) && defined(TDMA_STATS_COLLECT)
        stats_stop_rdo();
    #endif
            rf_rx_off();
            rf_data_mode();
    #ifdef TDMA_STATS_COLLECT
            // submit the minipacket log using sprintf
            /*
            if (next_schedule_entry.type == TDMA_TX_PARENT)
            {
                sprintf(mpkt_log[mpkt_log_curbuf++], "DELT %u %u %u %lu %lu", my_addr16, minipkt_id-1, my_parent,
                current_time.secs, current_time.nano_secs);
            }
            */
    #endif
            return;
        }

#ifdef TDMA_PREAMBLE_HANDSHAKE
    #ifdef TDMA_STATS_COLLECT
        stats_stop_rdo();
    #endif
        rf_set_cca_mode(1);
        rf_rx_off();
        rf_data_mode();
#endif
    } // if tx parent and (maybe) prio > 0


#ifdef JAM_CHECK_DEBUG
    PORTA &= ~BM(JAM_CHECK_DEBUG);
#endif

} // if slot is TDMA_TX_PARENT
#endif // IF SLOT STEALING
    // otherwise, we are good to go :)
    nrk_high_speed_timer_wait(0, DATA_SLOT_START_TICKS + DELTA_SLOT_OFFSET);

/*
#ifdef GPIO_TX_DEBUG 
    PORTA |= BM(DEBUG_1);
    PORTA &= ~BM(DEBUG_1);
#endif
*/

#ifdef GPIO_NEWTMR_DEBUG_MASTER
    // reset the hst, set it to 8 prescaler so we can measure to the end of the slot
    _nrk_high_speed_timer_reset();
    TCCR1B= BM(CS11); // start with a prescaler of 8
    GTCCR |= BM(PSRSYNC);
    //_nrk_high_speed_timer_reset();
    //_nrk_high_speed_timer_start();
#endif

    _nrk_high_speed_timer_reset();
    _nrk_high_speed_timer_start();
    //TCCR1B = BM(CS11);
    //GTCCR |= BM(PSRSYNC);

    //tdma_auto_ack_enable();
    //tdma_addr_decode_set_my_mac(tdma_mac_get()); 
    //tdma_addr_decode_dest_mac(tdma_tree_parent_get());  // 0xFFFF is broadcast
    //tdma_addr_decode_enable();


    // wait the guard time.
    do {} while (_nrk_high_speed_timer_get() < (TX_GUARD_TIME_US * 8));

    _nrk_high_speed_timer_reset();
    _nrk_high_speed_timer_start();

#ifdef GPIO_BASIC_DEBUG
    nrk_gpio_set (NRK_DEBUG_1);
#endif

#ifdef LED_BASIC_DEBUG
    nrk_led_set(RED_LED);
#endif

    // Don't fill packet til here
    tdma_rfTxInfo.pPayload[TDMA_SLOT] = (slot >> 8);
    tdma_rfTxInfo.pPayload[TDMA_SLOT + 1] = (slot & 0xFF);

    // This clears the explicit sync bit
    tdma_rfTxInfo.pPayload[TDMA_TIME_TOKEN]= tdma_time_token; 


#ifdef GPIO_TX_DEBUG
    PORTA |= BM(GPIO_TX_DEBUG);
#endif


    // Transmit the packet (finally!)
    rf_data_mode();
#ifdef TDMA_STATS_COLLECT
    stats_start_rdo();
#endif
 
    //rf_rx_off();
    
    rf_tx_packet (&tdma_rfTxInfo);

#ifdef TDMA_STATS_COLLECT
    stats_stop_rdo();
#endif
    rf_rx_off();

    //if (!pkt_got_ack)
    //    printf("af %u\r\n", tdma_rfTxInfo.destAddr);

    // restore dest addr at end
    //if (explicit_tsync)
    //    tdma_rfTxInfo.destAddr = dest_tmp;

#ifdef TDMA_STATS_COLLECT
    if (next_schedule_entry.priority > 0)
        steal_log();
#endif

    //ENABLE_GLOBAL_INT();

#ifdef GPIO_INT_DEBUG
    PORTA &= ~BM(GPIO_INT_DEBUG);
#endif

#ifdef GPIO_TX_DEBUG
    PORTA &= ~BM(GPIO_TX_DEBUG);
#endif

#ifdef GPIO_BASIC_DEBUG
    nrk_gpio_clr (NRK_DEBUG_1);
#endif

    if (!explicit_tsync)
    {
        // if not a sync packet sent, notify that data was sent
        // and be ready to accept more
        finished_tx = 1;
        tdma_tx_data_ready = 0;
        // moved to just before sleep
        //nrk_event_signal (tdma_tx_pkt_done_signal);
    }

#ifdef LED_BASIC_DEBUG
    nrk_led_clr (RED_LED);
#endif

    #ifdef TDMA_STATS_COLLECT
            // submit the minipacket log using sprintf
            /*
            if (next_schedule_entry.type == TDMA_TX_PARENT)
            {
                sprintf(mpkt_log[mpkt_log_curbuf++], "DELT %u %u %u %lu %lu", my_addr16, minipkt_id-1, my_parent,
                current_time.secs, current_time.nano_secs);
            }
            */
    #endif

}  // END TDMA_TX

tdma_slot_t _get_slots_past_wakeup()
{
    tdma_slot_t ssw = 0;

    nrk_time_t time_since_last_wakeup;
    nrk_time_t result;

    result = time_of_slot;
    //result.secs = 0;
    //result.nano_secs = 0;

    nrk_time_get(&time_since_last_wakeup);
    
    if (time_since_last_wakeup.nano_secs == time_of_slot.nano_secs &&
            time_since_last_wakeup.secs == time_of_slot.secs)
        return 0;

    // difference between beginning of last wakeup slot and now
    // if the sub fails, this probably means that the OS thinks it's before the slot time, but it's not.
    if (nrk_time_sub(&time_since_last_wakeup, time_since_last_wakeup, time_of_slot) == NRK_ERROR)
        return 0;

    nrk_time_compact_nanos(&time_since_last_wakeup);

    //printf("tslw %lu %lu tos %lu %lu sl  %lu %lu\r\n", time_since_last_wakeup.secs, time_since_last_wakeup.nano_secs,
    //    time_of_slot.secs, time_of_slot.nano_secs, slot_length_time.secs, slot_length_time.nano_secs);

    while(nrk_time_sub(&time_since_last_wakeup, time_since_last_wakeup, slot_length_time) != NRK_ERROR)
    {
        nrk_time_compact_nanos(&time_since_last_wakeup);
        ssw++;
    }

    return ssw;
}


/********
 * New function: The time returned is a time that is relative to the current
 * slot start time.  This should make it possible to compute a time exact to the
 * nanosecond that we should wake up.
********/

nrk_time_t gnw_relative(nrk_time_t slot_start_time, tdma_slot_t current_slot, tdma_slot_t wakeup_slot)
{
    nrk_time_t time_until_wakeup;  // time between "now" and next wakeup.
    nrk_time_t wakeup_time; // absolute time of wakeup
    uint16_t slots_to_wakeup;

    
    if (current_slot < wakeup_slot)
    {
        slots_to_wakeup = (wakeup_slot - current_slot);
    }
    else
    {
        slots_to_wakeup =  (TDMA_SLOTS_PER_CYCLE - current_slot) + wakeup_slot;
    }


    // nanosecond granularity
    // this will overflow, sadly, if next wakeup is more than 4.25secs away
//#if (TDMA_SLOTS_PER_CYCLE * TDMA_SLOT_TIME_US) < (4 * US_PER_SEC)
//    uint32_t time_until_wakeup_ns = slots_to_wakeup * TDMA_TICKS_PER_SLOT * NANOS_PER_TICK;
//    time_until_wakeup.secs      = time_until_wakeup_ns / NANOS_PER_SEC;
//    time_until_wakeup.nano_secs = time_until_wakeup_ns % NANOS_PER_SEC;
//#else
    // hell if I know
    //#warning Cycle length is too long. Less granularity being used.
    // microsecond granularity
    uint32_t time_until_wakeup_us = slots_to_wakeup * ((uint32_t)TDMA_SLOT_TIME_US);

    time_until_wakeup.secs = (time_until_wakeup_us / US_PER_SEC);
    time_until_wakeup.nano_secs = (time_until_wakeup_us % US_PER_SEC) * (uint32_t) NANOS_PER_US;

    nrk_time_t time_difference;
    time_difference.secs = 0;
    time_difference.nano_secs = slots_to_wakeup * slot_leftover_nanos;

    //printf("b4 %lu\r\n", time_until_wakeup.nano_secs);
    
    if (add_slot_difference)
    {
        //printf("add %lu\r\n", time_difference.nano_secs);
        nrk_time_add(&time_until_wakeup, time_until_wakeup, time_difference);
        
    }
    else
    {
        //printf("sub %lu\r\n", time_difference.nano_secs);
        nrk_time_sub(&time_until_wakeup, time_until_wakeup, time_difference);
        nrk_time_compact_nanos(&time_until_wakeup);
    }
//#endif
    //printf("sub %lu\r\n", time_until_wakeup.nano_secs);
        
    nrk_time_add(&wakeup_time, slot_start_time, time_until_wakeup);
    
    return wakeup_time;

}

int8_t tdma_schedule_method_set(tdma_sched_method_t m)
{
    tdma_schedule_method = m;
    return NRK_OK;
}


/*************************************
 * tdma_nw_task
 * ************************************
 */
void tdma_nw_task ()
{

tdma_stats_setup(nrk_get_pid());

period_delay_time.secs = 1;
period_delay_time.nano_secs = 0;

printf("tdma %d\r\n",nrk_get_pid());

#ifdef LED_BASIC_DEBUG
    nrk_led_set(GREEN_LED);
#endif

// wait until another task calls tdma_init
do
{
    nrk_wait(period_delay_time);
#ifdef TDMA_TEXT_DEBUG_ALL
    nrk_kprintf(PSTR("Wait on tdma_init\r\n"));
#endif
}
while (!tdma_init_done);

if (tdma_schedule_method == TDMA_SCHED_TREE)
{
    // Tree creation done here
    // Sanity check to make sure the tree code is included (by def TDMA_TREEMAKE),
    // otherwise fail out.
#ifdef TDMA_TREEMAKE
    tdma_tree_create(tdma_channel);
    #ifdef TDMA_STATS_COLLECT
        nrk_time_get(&tree_creation_time);
    #endif
#else
    nrk_kprintf(PSTR("TDMA_SCHED_TREE specified, but TDMA_TREEMAKE not defined.\r\n"));
    nrk_halt();
#endif
}

_tdma_setup();

tdma_running=1;
tdma_slot = 0;

if (tdma_node_mode == TDMA_MASTER)
{
    in_sync = 1;
    next_schedule_entry = tdma_schedule_get_next(0);

    sync_slot = 0;

    nrk_time_get(&sync_time);
    nrk_time_compact_nanos(&sync_time);
    time_of_slot = sync_time;
}

#ifdef TDMA_TEXT_DEBUG_ALL
    printf("TDMA Started\r\n");
#endif

#ifdef LED_BASIC_DEBUG
    nrk_led_clr(GREEN_LED);
#endif

    // main wakeup loop
    while (1)
    {
        if (tdma_node_mode == TDMA_SLAVE)
        {
            while (!in_sync)
            {
                //nrk_kprintf(PSTR("Waiting for sync\r\n"));
                nrk_led_clr(BLUE_LED);
                nrk_led_set(RED_LED);

                //poll for incoming packet
                tdma_rfRxInfo.pPayload[TDMA_SLOT] = (-1 >> 8);
                tdma_rfRxInfo.pPayload[TDMA_SLOT + 1] = (-1 & 0xFF);

                tdma_rfRxInfo.max_length = RF_MAX_PAYLOAD_SIZE;
                rf_rx_off();
                rf_data_mode();
                rf_polling_rx_on();

                sync_time.secs = 0;
                sync_time.nano_secs = 0;
                
                // do the nasty crap that RT-Link does

                while (!SFD_IS_1)
                {
                }

#ifdef GPIO_SFD_DEBUG
    PORTA |= BM(DEBUG_1);
    PORTA &= ~BM(DEBUG_1);
#endif


                // reset the high speed timer here, at the SFD
                _nrk_high_speed_timer_reset();
                _nrk_high_speed_timer_start();

                //nrk_kprintf(PSTR("GotSYNCSFD\r\n"));

#ifdef GPIO_NEWTMR_DEBUG
        PORTA &= ~BM(DEBUG_1); 
        PORTA |= BM(DEBUG_1);
        PORTA &= ~BM(DEBUG_1); // make sure pin is down
#endif

                do 
                {
                    nrk_time_get(&sync_time);
                }
                while (rf_rx_check_fifop() == 0);

                if (sync_time.secs == 0 && sync_time.nano_secs == 0)
                    nrk_kprintf(PSTR("SYNC TIME IS 0. THERE'S A PROBLEM\r\n"));

#ifdef GPIO_SYNC_DEBUG
                nrk_gpio_set(NRK_DEBUG_1);
                nrk_gpio_set(NRK_DEBUG_0);
#endif

                // calculate the sync time based off of the fifop
                nrk_time_compact_nanos(&sync_time);
                nrk_time_sub(&sync_time, sync_time, sync_offset_time);
                nrk_time_compact_nanos(&sync_time);


                //packet coming
                while ((n = rf_polling_rx_packet()) == 0)
                {
                }

                rf_rx_off();

                if (n != 1)
                {
#ifdef TDMA_TEXT_DEBUG_ALL
                    printf("FAILED recv: n = %d\r\n", n);
#endif
                    continue;
                }

#ifdef GPIO_SYNC_DEBUG
                nrk_gpio_clr(NRK_DEBUG_1);
#endif
                

                //read the slot we're going to assign
                //printf("Recv : %s\r\n", tdma_rfRxInfo.pPayload);
                //sscanf(tdma_rfRxInfo.pPayload,"%16"PRIx32"", &sync_slot);
                
                sync_slot = tdma_rfRxInfo.pPayload[TDMA_SLOT];
                sync_slot <<=8;
                sync_slot |= tdma_rfRxInfo.pPayload[TDMA_SLOT + 1];

                // I need to make sure this packet is real, and not just noise caused by
                // the master. This will probably happen the first time I see a sync packet because
                // of the minipackets
                tdma_rx_pkt_release();

                //printf("Sync slot = %d\r\n", sync_slot);
                //printf("Token = %d\r\n", tdma_time_token);

                if (sync_slot < 0 || sync_slot > TDMA_SLOTS_PER_CYCLE)
                {
                    printf("BAD SYNC %d\r\n", sync_slot);
                    continue;
                }

                uint8_t tmp_token;
                tmp_token = 0x7F & tdma_rfRxInfo.pPayload[TDMA_TIME_TOKEN];

                if ((tmp_token <= tdma_time_token) && 
                    !(tdma_time_token > 110 && tmp_token < 10 ))
                {   
                    printf("bad token %d %d\r\n", tdma_time_token, tmp_token);
                    continue;
                } 
                tdma_time_token = tmp_token; //0x7F & tdma_rfRxInfo.pPayload[TDMA_TIME_TOKEN];

#ifdef TDMA_SYNC_DEBUG
                sync_count++;
                printf("SYNC %d %d %lu %lu %d\r\n",
                         my_addr16, sync_count, 
                         sync_time.secs, sync_time.nano_secs, sync_slot);
#endif
                
#ifdef GPIO_SYNC_DEBUG
                nrk_gpio_clr(NRK_DEBUG_0);
#endif

                //next_schedule_entry = tdma_schedule_get_next(sync_slot);

#ifdef GPIO_SLOT_DEBUG
                nrk_gpio_set(NRK_DEBUG_0);
#endif

                // lastly, wait a time to exactly the next slot, then reset the OS timer so that
                // its tick lines up with a start of a slot.
                //uint8_t ost = _nrk_os_timer_get();
                _nrk_os_timer_stop();
                _nrk_os_timer_reset();

                //DISABLE_GLOBAL_INT();

#ifdef SFD_TEST
                nrk_high_speed_timer_wait(0, sfd_to_slot_time);
#else
                nrk_high_speed_timer_wait(0, SFD_TO_SLOT_TIME-DELTA_SLOT_OFFSET);
#endif

                _nrk_os_timer_reset();
                _nrk_os_timer_start();

#ifdef GPIO_NEWTMR_DEBUG
                PORTA &= ~BM(DEBUG_1); // make sure pin is down
                PORTA |= BM(DEBUG_1);
                PORTA &= ~BM(DEBUG_1); 
#endif

                //ENABLE_GLOBAL_INT();
                nrk_time_get(&sync_time);
                nrk_time_compact_nanos(&sync_time);

                sync_slot+=1;
                if(sync_slot >= TDMA_SLOTS_PER_CYCLE)
                    sync_slot = 0;

                // we waited until the next slot
                time_of_slot = sync_time;
                tdma_slot = sync_slot;
                next_schedule_entry = tdma_schedule_get_next(sync_slot);

                nrk_led_clr(RED_LED);
                in_sync = 1;

            } //while not in sync

            nrk_led_set(BLUE_LED);

            //printf("sync slot is %d, next wakeup on slot %d\r\n", sync_slot, next_schedule_entry.slot);

            nrk_time_get(&current_time);
            nrk_time_compact_nanos(&current_time);
            
            //printf("Sync time: %lu : %lu, Current time  %lu :  %lu\r\n",
                    //sync_time.secs, sync_time.nano_secs,
                    //current_time.secs,current_time.nano_secs);

            nrk_time_sub(&time_since_sync, current_time, sync_time);
            nrk_time_compact_nanos(&time_since_sync);
    
           /* printf("Time since sync: %lu : %lu, tolerance  %lu : %lu\r\n",
                    time_since_sync.secs, time_since_sync.nano_secs,
                    sync_tolerance_time.secs, sync_tolerance_time.nano_secs);
                    */
            //printf("tss: %lu,%lu."
            //printf("sync tol: %lu,%lu\r\n", 
            //        sync_tolerance_time.secs, sync_tolerance_time.nano_secs );

            //printf("Time since sync, %lu, %lu\r\n", time_since_sync.secs,
            //                                        time_since_sync.nano_secs);
            if (nrk_time_cmp(time_since_sync, sync_tolerance_time) > 0)
            {
#ifdef TDMA_SYNC_DEBUG
                oos_count++;
                nrk_time_get(&current_time);
                nrk_time_compact_nanos(&current_time);
                printf("OOS %d %d %lu %lu\r\n",
                        my_addr16, oos_count,
                        current_time.secs, current_time.nano_secs);

#endif
                // sync time has expired.  Go into re-sync mode
                in_sync = 0;
                //tdma_time_token = 0;
                continue;
            }

        } // end if TDMA_SLAVE

        nrk_time_get(&current_time);
        nrk_time_compact_nanos(&current_time);

        // in the event we were awake longer than 1 slot, we want to adjust the time
        // accordingly
        slots_since_wakeup = _get_slots_past_wakeup();

        tdma_slot += slots_since_wakeup;

        // add x slot-lengths to the slot time
        while (slots_since_wakeup > 0)
        {
            nrk_time_add(&time_of_slot, time_of_slot, slot_length_time);
            slots_since_wakeup--;
        }
        
        //time_of_slot = get_time_of_next_wakeup(time_of_slot, 
        //                            next_schedule_entry.slot, sync_slot, sync_time);

        time_of_slot = gnw_relative(time_of_slot, tdma_slot, next_schedule_entry.slot);
        nrk_time_compact_nanos(&time_of_slot);

        //time_of_slot.nano_secs +=  (uint32_t)5 * (uint32_t)16601 * (uint32_t) NANOS_PER_US;
        //nrk_time_compact_nanos(&time_of_slot);

        //printf("%d -> %d, nw %lu %lu\r\n", tdma_slot, next_schedule_entry.slot, time_of_slot.secs, time_of_slot.nano_secs);
#ifdef GPIO_SLOT_DEBUG
        nrk_gpio_clr(NRK_DEBUG_0);
#endif

        //printf("wait %lu  %lu  %lu  %lu\r\n", 
        //    current_time.secs, current_time.nano_secs, time_of_slot.secs, time_of_slot.nano_secs);

        // because of 100us scheduler, let's make this 100us early
        /*
        if (time_of_slot.nano_secs < 100000)
        {
            time_of_slot.nano_secs =  NANOS_PER_SEC - (100000 - time_of_slot.nano_secs);
            time_of_slot.secs -= 1;
        }
        else
        {
            time_of_slot.nano_secs -= 100000;
        }
        */

        // Signal any events, if they have occurred
        if (tdma_rx_pkt_check() != 0)
        {
            nrk_event_signal (tdma_rx_pkt_signal);
        }
        else if (finished_tx != 0)
        {
            finished_tx = 0;
            nrk_event_signal (tdma_tx_pkt_done_signal);
        }

#ifdef GPIO_NEWTMR_SLT_DEBUG
        PORTA &= ~BM(GPIO_NEWTMR_SLT_DEBUG);
#endif

        //nrk_led_clr(BLUE_LED);
        //printf("%lu %lu\r\n", time_of_slot.secs, time_of_slot.nano_secs);

        // sleep
        nrk_wait_until(time_of_slot);

/*
// THIS IS JUST SOMETHING I'M TRYING OUT 

        tdma_slot_t slots_to_wakeup;

    if (tdma_slot < next_schedule_entry.slot)
    {
        slots_to_wakeup = (next_schedule_entry.slot - tdma_slot);
    }
    else
    {
        slots_to_wakeup =  (TDMA_SLOTS_PER_CYCLE - tdma_slot) + next_schedule_entry.slot;
    }
        nrk_wait_until_next_n_periods(slots_to_wakeup);

// END TRYING OUT 
*/

    // FIXME THIS IS JUST FOR DEBUGGGINGGNGNGNGNGGGGGGG!!!!!!!!!!!!!!!!!!!!!!!!!!!@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!!!
    //next_schedule_entry.type = 50;


        // Start the high speed timer for measuring into slot times.
        _nrk_high_speed_timer_reset();
        _nrk_high_speed_timer_start();


        if (tdma_node_mode == TDMA_MASTER)
            nrk_time_get(&possible_sync_time);

        //nrk_led_set(BLUE_LED);
        // the current slot is when I was supposed to wake up
        tdma_slot = next_schedule_entry.slot;

#ifdef GPIO_NEWTMR_SLT_DEBUG
        PORTA |= BM(GPIO_NEWTMR_SLT_DEBUG);
#endif

#ifdef GPIO_SLOT_DEBUG
        nrk_gpio_set(NRK_DEBUG_0);
#endif
        nrk_led_clr(BLUE_LED);

        if (next_schedule_entry.type == TDMA_RX) // RX
        {
            _tdma_rx(next_schedule_entry.slot);

/*
            if (tdma_rx_pkt_check() == 0)
            {
                // this will have to be removed
                if (next_schedule_entry.priority == 1)
                {
                    // I was supposed to get a sync packet
#ifdef TDMA_SYNC_DEBUG
                    printf("NOSYNC %d\r\n",my_addr16);
#endif
                    
                }
                else
                {
#ifdef TDMA_TEXT_DEBUG
                    nrk_kprintf(PSTR("NO RX\r\n"));
#endif
                }

            }
*/

            /*
            if (tdma_rx_pkt_check() != 0)
                printf("Received: %s\r\n", &tdma_rfRxInfo.pPayload[TDMA_DATA_START]);
            tdma_rx_pkt_release();
            */
        }
        else if (tdma_node_mode == TDMA_MASTER && (next_schedule_entry.type == TDMA_TX_CHILD))
        {
            // time to send a sync packet
           // printf("Master send, time token is %d, slot is %d\r\n", tdma_time_token, next_schedule_entry.slot);

            
            tdma_tx_buf[TDMA_DATA_START] = '\0';
            
            tdma_rfTxInfo.pPayload = tdma_tx_buf;
            tdma_rfTxInfo.length = TDMA_DATA_START;


            // This needs to be BEFORE the TX! Otherwise it becomes after the guard time and
            // screws everything up!!
            // also, make it the time of the slot so that it doesn't offset by some amount either.
            //sync_time = possible_sync_time;

            //_nrk_os_timer_reset();
            //_nrk_os_timer_start();
    
            //nrk_time_get(&sync_time);
            //nrk_time_compact_nanos(&sync_time);
            //sync_time = current_time;

            _tdma_tx(next_schedule_entry.slot);

#ifdef GPIO_NEWTMR_DEBUG_MASTER
            // at this point, the high speed timer is running in prescaler 8 mode.
            // it was started at the beginning of the data slot (past the delta slots)
            // (prescaler 8 oscillates at 1us)
            //DISABLE_GLOBAL_INT();

            PORTA &= ~BM(DEBUG_1); 
            PORTA |= BM(DEBUG_1);
            PORTA &= ~BM(DEBUG_1); 

            do {} while (_nrk_high_speed_timer_get() < ((uint16_t)TDMA_SLOT_DATA_TIME_US));

            //_nrk_os_timer_reset();
            //_nrk_os_timer_start();

            // this pin raise should happen at the master's opinion of the end of the slot.
            PORTA &= ~BM(DEBUG_1); 
            PORTA |= BM(DEBUG_1);
            PORTA &= ~BM(DEBUG_1); 

            //ENABLE_GLOBAL_INT();
            //_nrk_high_speed_timer_reset();
            //_nrk_high_speed_timer_start();
            // do something funny here.  The timer hasn't ticked yet most likely so it will still think
            // it is the same slot when it is the next.
            // remedy this manually.  This won't ruin the get_slots_past_wakeup function call.
            //tdma_slot+=1;
            //nrk_time_add(&time_of_slot, time_of_slot, slot_length_time);


            //printf("spw %d\r\n", _get_slots_past_wakeup());
#endif
            _nrk_high_speed_timer_reset();
            _nrk_high_speed_timer_start();

            //sync_slot = next_schedule_entry.slot;


            tdma_time_token++;
            if (tdma_time_token > 127)
                tdma_time_token = 0;

        }
        else if (tdma_node_mode == TDMA_SLAVE)
        {
            if (next_schedule_entry.type == TDMA_TX_CHILD)
            {
                // send a sync packet (use the existing buffer but do not modify it)
                uint8_t tmp_length = tdma_rfTxInfo.length; 
                tdma_rfTxInfo.length = TDMA_DATA_START;
                _tdma_tx(next_schedule_entry.slot); // send sync
                tdma_rfTxInfo.length = tmp_length;
#ifdef TDMA_TEXT_DEBUG_ALL
                nrk_kprintf(PSTR("Sent sync\r\n"));
#endif
            }
            else if (next_schedule_entry.type==TDMA_TX_PARENT)
            {
                if (tdma_tx_data_ready)
                {
                    // send a real packet
                    _tdma_tx(next_schedule_entry.slot);
#ifdef TDMA_TEXT_DEBUG
                    if (!tdma_tx_data_ready)
                    {
                        if (next_schedule_entry.priority == 0)
                        #ifdef TDMA_TEXT_DEBUG_ALL
                            nrk_kprintf(PSTR("nrmtx\r\n"))
                        #endif
                            ;
                        else 
                            printf("txsteal %d\r\n", next_schedule_entry.priority);
                        //printf("Sent packet on slot %d\r\n", next_schedule_entry.slot);
                    }
#endif
                }
#ifdef TDMA_TEXT_DEBUG_ALL
                else
                    nrk_kprintf(PSTR("Nothing to send\r\n"));
#endif
            }
#ifdef TDMA_TEXT_DEBUG_ALL
            else
                nrk_kprintf(PSTR("Slv: invalid slot type\r\n"));
#endif
        }
#ifdef TDMA_TEXT_DEBUG_ALL
        else
            nrk_kprintf(PSTR("Mstr: invalid slot type\r\n"));
#endif

        next_schedule_entry = tdma_schedule_get_next(next_schedule_entry.slot);
    }
}



int8_t tdma_started()
{
    return tdma_running;
}

void tdma_task_config ()
{
    nrk_task_set_entry_function( &tdma_task, tdma_nw_task);
    nrk_task_set_stk( &tdma_task, tdma_task_stack, TDMA_STACK_SIZE);
    tdma_task.prio = TDMA_TASK_PRIORITY;
    tdma_task.FirstActivation = TRUE;
    tdma_task.Type = BASIC_TASK;
    tdma_task.SchType = PREEMPTIVE;
    tdma_task.period.secs = 0;
    tdma_task.period.nano_secs = 0;
    //tdma_task.period.secs = 0;
    //tdma_task.period.nano_secs = TDMA_TICKS_PER_SLOT * NANOS_PER_TICK;
    tdma_task.cpu_reserve.secs = 0;      // tdma reserve , 0 to disable
    tdma_task.cpu_reserve.nano_secs = 0;
    tdma_task.offset.secs = 0;
    tdma_task.offset.nano_secs = 0;
    nrk_activate_task (&tdma_task);
}
