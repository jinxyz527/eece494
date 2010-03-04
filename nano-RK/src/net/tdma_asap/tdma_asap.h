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


#ifndef _TDMA_H
#define _TDMA_H
#include <include.h>
#include <basic_rf.h>
#include <nrk.h>

/************************************************************************

TDMA-ASAP is a TDMA protocol featuring slot stealing and parallelism.
Scheduling can be done manually or determined via automatic tree creation.
Slot stealing can be performed among siblings in a tree when the owner of a
slot has nothing to transmit. TDMA-ASAP currently assumes data is propogated
from leaf to root, while time synchronization packets are sent from root
to children periodically. 

This implementation currently provides one RX and one TX buffer, and works with
16-bit addresses.

************************************************************************/
// for debugging only
//#define SFD_TEST
#ifdef SFD_TEST
    uint16_t sfd_to_slot_time;
#endif

// uncomment if doing timing tests 
//#include "testslotstride.h"

// owners of TX slots will not wait for an acknowledgement 
// of their mini packet
//#define TDMA_OWNER_NO_ACK

#define TDMA_MASTER_MODE
#define TDMA_SLAVE_MODE

// TICK-INDEPENDENT TIMES

    #define TDMA_RX_WAIT_MS       		2 //shooting for 2
    #define TX_GUARD_TIME_US            1000

    #define TDMA_DELTA_SLOT_US 300

    // if I'm doing the testslotsride.h stuff
    #ifndef SLOTSTRIDETESTS 
        #define TDMA_SLOTS_PER_CYCLE 100
    #endif

    #define TDMA_SYNC_THRESHOLD_SECS	10

// BIG CHANGE:  I'm making the slot time different so that the TOTAL slot time is equal to a microsecond value that
// is equally divisible by the OS tick time (977 uS on the atmega1281).

// Note: The times such as delta slots don't matter for this because we're using the high speed timer to time these.

    // For reference:
    // NANOS_PER_TICK      976563
    // US_PER_TICK         977 
    // TICKS_PER_SEC       1024 

    // 8ms is normal, but we'll do 8 ticks of os clock instead
    // 10 delta slots * 800uS each = 8 ms, or 8000 uS
    // 15632 uS is 16 ticks, 16609 is 17 ticks
    // 16609 - 8000 = 8609 uS

    //#define TDMA_SLOT_DATA_TIME_US  8609
    //#define TDMA_TICKS_PER_SLOT 17

    //CHANGED 
    // 3000 

    // commented out 13 Jan 2010
    //#define TDMA_SLOT_DATA_TIME_US  8724

    #define TDMA_TICKS_PER_SLOT 12


    // exact value is 16601571 nanos
    //#define TDMA_SLOT_DATA_TIME_US  8601

    #define TDMA_SLOT_TIME_US		((((NUM_DELTA_SLOTS * TDMA_DELTA_SLOT_US)) )\
                                     + TDMA_SLOT_DATA_TIME_US)


    // optimization added feb 15: if none of my children
    // have to TX I will sleep for the rest of my RX slots from children in the cycle
    // NOTE: this is implemented as still waking up but sleeping immediately again if
    // I don't need to be awake
//#define TDMA_SLEEP_OPT


// END TDMA TIMES


// THESE ARE DEPENDENT ON THE DELTA SLOT LENGTH AND QUANTITY

// 10 delta slots, 600 us each, / .125 uS tick
// (in ticks)
//#define DATA_SLOT_START 48000

// these are times based on the atmega1281 speed of
// 7.3728 ticks per us

//#define DATA_SLOT_START_TICKS 44237

// commented out 9/18/09 to offset the entire slot to shorten delta slots
//#define DATA_SLOT_START_TICKS 58982
//CHANGED

// commented out 13 Jan 2010: Good for 9 children, 10 delta slots
//#define DATA_SLOT_START_TICKS 22118
//#define SFD_TO_SLOT_TIME 48703
//#define TDMA_SLOT_DATA_TIME_US  8724
//#define TREE_MAX_CHILDREN 9


// New times: for 10 children, 11 delta slots
//#define DATA_SLOT_START_TICKS 24331
//#define SFD_TO_SLOT_TIME 46844
//#define TDMA_SLOT_DATA_TIME_US  8424
//#define TREE_MAX_CHILDREN 10

// New times: for 10 children, 12 delta slots
#define DATA_SLOT_START_TICKS 26544
#define SFD_TO_SLOT_TIME 44732
//#define SFD_TO_SLOT_TIME 49736
#define TDMA_SLOT_DATA_TIME_US  8124
#define TREE_MAX_CHILDREN 10

// 192 uS
//#define DELTA_SLOT_OFFSET 1416

//400, or about 54 us, seems to work. (radio takes about 42 uS to boot)
#define DELTA_SLOT_OFFSET 400
//#define DELTA_SLOT_OFFSET 0


#define TDMA_DELTA_SLOT_TICKS 2212


//#define TX_TO_SFD 2976
//#define SLOT_TO_SFD (TX_GUARD_TIME_US+TX_TO_SFD)

// the official RT_Link value
//#define SFD_TO_SLOT_TIME 32192


// what should be reasonable,  calculated from difference
// between TDMA and RTL:

// TDMA: 8609 data portion, 1000 gt
// RTL : 8793 data portion,  542 gt

// difference of 642 us, * frequency = 4733 ticks difference
// RTL is 27750, so 27750 + 4733 = 32483
//#define SFD_TO_SLOT_TIME 32483
//#define SFD_TO_SLOT_TIME 48703


#define TDMA_DEFAULT_CCA_THRESH        (-35) 
#define TDMA_DEFAULT_CHANNEL           (18) 
                       
#define TDMA_MAX_PKT_SIZE		116

// normal
#define TDMA_STACK_SIZE 	    	512

#define TDMA_TASK_PRIORITY		20 

// max number of children/neighbors to keep track of
//#define TREE_MAX_CHILDREN 9
#define NUM_DELTA_SLOTS (TREE_MAX_CHILDREN+2)
#define TREE_MAX_NEIGHBORS 15 

// Whether to do Slot stealing
// right now, omitting slot stealing doesn't result in many
// optimizations during RX slots. Potential stealers will
// NOT wake up for stealable slots, but the microslot times will
// still be observed.
#define SLOT_STEALING

//#define TDMA_PREAMBLE_HANDSHAKE

// use the new form where mini packets are received instead of channel jam
// by receiver
#define STEAL_G

// addr is 2 bytes
#define ADDR_SIZE 2

// Fill this in with all packet fields.
typedef enum {
	// Global slot requires 10 bits to hold 1024 slots
	// hence it uses 2 bytes.
	TDMA_SLOT=0,
	// This token is used to ensure that a node never
	// synchronizes off of a node that has an older sync 
	// value.
	TDMA_TIME_TOKEN=2,
	TDMA_DATA_START=3
} tdma_pkt_field_t;

typedef struct {
	int8_t length;
	uint8_t *pPayload;
} TDMA_TX_INFO;

// node type
typedef enum {
	TDMA_MASTER,
	TDMA_SLAVE
} tdma_node_mode_t;

typedef enum {
    TDMA_SCHED_TREE,
    TDMA_SCHED_MANUAL
} tdma_sched_method_t;

tdma_node_mode_t tdma_my_mode_get();

// Structures
nrk_task_type tdma_task;
NRK_STK tdma_task_stack[TDMA_STACK_SIZE];

RF_RX_INFO tdma_rfRxInfo;
RF_TX_INFO tdma_rfTxInfo; 

// Signals
nrk_sig_t tdma_rx_pkt_signal;
nrk_sig_t tdma_tx_pkt_done_signal;
nrk_sig_t tdma_enable_signal;

nrk_sig_t tdma_get_tx_done_signal();
nrk_sig_t tdma_get_rx_pkt_signal();

// Initialization
void tdma_task_config ();
int8_t tdma_started();
int8_t tdma_init(uint8_t chan);

// Config
int8_t tdma_set_cca_thresh(int8_t thresh);
int8_t tdma_get_cca_thresh();
int8_t tdma_rx_pkt_set_buffer(uint8_t *buf, uint8_t size);
int8_t tdma_schedule_method_set(tdma_sched_method_t m);
int8_t tdma_mode_set(tdma_node_mode_t mode);
int8_t tdma_set_channel(uint8_t chan);

// tree-related
uint8_t tdma_tree_level_get();
uint16_t tdma_mac_get();

// RX and TX
int8_t tdma_tx_pkt_check();
int8_t tdma_rx_pkt_check();

uint8_t *tdma_rx_pkt_get(uint8_t *len, int8_t *rssi, uint8_t *slot);
int8_t tdma_tx_pkt(uint8_t *buf, uint8_t len);
int8_t tdma_rx_pkt_release(void);
int8_t tdma_set_rf_power(uint8_t power);

// Waiting
int8_t tdma_wait_until_rx_or_tx();
int8_t tdma_wait_until_tx();
int8_t tdma_wait_until_rx_pkt();

int8_t  tdma_auto_ack_disable();
int8_t  tdma_auto_ack_enable();
int8_t  tdma_addr_decode_disable();
int8_t  tdma_addr_decode_enable();
int8_t  tdma_addr_decode_dest_mac(uint16_t dest);
int8_t  tdma_addr_decode_set_my_mac(uint16_t dest);

#endif
