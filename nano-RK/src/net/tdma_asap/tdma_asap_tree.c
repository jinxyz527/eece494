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
*
*  tdma_asap_tree.c
*  Contains functions to create a tree for communication and
*  relay of data packets
*******************************************************************************/

#include <include.h>
#include <ulib.h>
#include <stdlib.h>
#include <tdma_asap_tree.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <avr/eeprom.h>
#include <nrk_eeprom.h>

// MUST be defined.
#define TDMA_TREE_DEBUG 1

#define FIRST_HEAR_TREEMAKE

#ifndef TDMA_TREE_DEBUG
  #error Tree debug verbosity not defined
#endif

#define TDMA_TREE_STATIC
// this will go away when I update
//#define TDMA_TREE_LIMIT_BY_CCA

// in tdma_asap.c
extern uint8_t tdma_init_done;
extern int8_t _tdma_channel_check();
extern tdma_node_mode_t tdma_node_mode;
extern int8_t tdma_get_cca_thresh();

// PRIVATE STUFF
uint8_t tree_init_done=0;
uint8_t tree_made=0;

uint8_t child_has_color;
uint8_t info_buf_size;

// A node's view of its part of the tree
uint8_t subtree_info[SUBTREE_INFO_SIZE];
uint8_t subtree_info_size;

// Master: to color graph must know depth of tree
uint8_t depth_of_tree;

// my children
uint8_t child_info[CHILD_INFO_SIZE];
uint8_t child_info_size;

// neighbors
node_addr_t neighbor_info[TREE_MAX_NEIGHBORS];
int8_t neighbor_rssi[TREE_MAX_NEIGHBORS];
uint8_t num_neighbors;

// offset while parsing the child info
uint8_t child_info_offset;

nrk_time_t cur_time;
nrk_time_t backoff_time;

void _tdma_tree_init(uint8_t chan)
{
    uint32_t mac;

    read_eeprom_mac_address(&mac);
    tree_addr16 = (mac & 0xFFFF);
    srand(tree_addr16);

    // set up the Tree RX buffer
    tree_rfRxInfo.max_length = TREE_BUF_SIZE;
    tree_rfRxInfo.pPayload = tree_rx_buf;

    rf_set_cca_thresh(tdma_get_cca_thresh());
    rf_init(&tree_rfRxInfo, chan, 0xffff, tree_addr16);

    num_children = 0;
    num_neighbors = 0;
    child_info_size = 0;
    subtree_info_size = 0;
    depth_of_tree = 0;
    
    child_info_offset = 0;
#if TDMA_TREE_DEBUG>=2
    printf("CIO: %d\r\n", child_info_offset);
#endif

    for (int8_t i = 0; i < DIM; i++)
        sensorsInfo[i].isDead = 1;

    tree_init_done = 1;
}

/* Obtain a static tree parent */
uint8_t _tdma_tree_static_parent()
{
    return sensorsInfo[tree_addr16].parent;
}

uint8_t _tdma_tree_static_level()
{
    return sensorsInfo[tree_addr16].level;
}

// fill the sensorsInfo with the static array
// set the parents and children
// this is for the CIC only, obviously

uint8_t _tdma_tree_static_create()
{
    uint8_t i,j;
    uint8_t hops, tmplv, tmpnode;

    // set of parents represents the tree.

    sensorsInfo[0 ].isDead = 1; // doesn't exist
    sensorsInfo[1 ].isDead = 0;
        sensorsInfo[1 ].parent = 0; // have no parent
    sensorsInfo[2 ].isDead = 0;
        sensorsInfo[2 ].parent = 1;
    sensorsInfo[3 ].isDead = 0;
        sensorsInfo[3 ].parent = 14;
    sensorsInfo[4 ].isDead = 0;
        sensorsInfo[4 ].parent = 1;
    sensorsInfo[5 ].isDead = 0;
        sensorsInfo[5 ].parent = 14;
    sensorsInfo[6 ].isDead = 0;
        sensorsInfo[6 ].parent = 5;
    sensorsInfo[7 ].isDead = 0;
        sensorsInfo[7 ].parent = 11;
    sensorsInfo[8 ].isDead = 0;
        sensorsInfo[8 ].parent = 1;
    sensorsInfo[9 ].isDead = 0;
        sensorsInfo[9 ].parent = 12;
    sensorsInfo[10].isDead = 0;
        sensorsInfo[10].parent = 8;
    sensorsInfo[11].isDead = 0;
        sensorsInfo[11].parent = 1;
    sensorsInfo[12].isDead = 0;
        sensorsInfo[12].parent = 16;
    sensorsInfo[13].isDead = 0;
        sensorsInfo[13].parent = 10;
    sensorsInfo[14].isDead = 0;
        sensorsInfo[14].parent = 1;
    sensorsInfo[15].isDead = 0;
        sensorsInfo[15].parent = 9;
    sensorsInfo[16].isDead = 0;
        sensorsInfo[16].parent = 6;

    // because node 16 is down
    /*
    sensorsInfo[16].isDead = 1;
        sensorsInfo[16].parent = 0;
    sensorsInfo[9 ].isDead = 0;
        sensorsInfo[9 ].parent = 15;
    sensorsInfo[12].isDead = 0;
        sensorsInfo[12].parent = 9;
    sensorsInfo[15].isDead = 0;
        sensorsInfo[15].parent = 8;
    */


    for (i = 0; i < 17; i++)
    {
        sensorsInfo[i].address = i;
        sensorsInfo[i].level = 0xFF;
    }

    // master is at level 0
    sensorsInfo[1].level = 0;

    // fill the children arrays according to the tree structure
    for (i = 1; i < 17; i++)
    {
        if (!sensorsInfo[i].isDead)
        {
            // find the level
            // do this by searching up from my node (following parent
            // links) until I find a node whose level has been determined.
            // then I decide my own level based on how many links I am from them.
            tmplv = sensorsInfo[i].level; // will be 0xFF if not determined yet
            tmpnode = i;
            hops = 0; // # of hops from me to known level node

            while (tmplv == 0xFF)
            {
                hops++;
                tmpnode = sensorsInfo[tmpnode].parent;
                tmplv = sensorsInfo[tmpnode].level;
            }

            sensorsInfo[i].level = hops+tmplv;
        
            // set depth of tree to be maximum level of any node
            if (depth_of_tree < sensorsInfo[i].level)
                depth_of_tree = sensorsInfo[i].level;

            // loop through sensors and add all nodes with children that have me
            // as their parent
            for (j = 1; j < 17; j++)
            {
                if (i!=j)
                {
                    if (sensorsInfo[j].parent == i)
                    {
                        sensorsInfo[i].children[sensorsInfo[i].totalChildren] = j;
                        sensorsInfo[i].totalChildren++;
                    }
                }
            }
        } // if not dead
    }

    // fill in my children array
    for (i = 0; i < sensorsInfo[tree_addr16].totalChildren; i++)
    {
        child_addrs[num_children] = sensorsInfo[tree_addr16].children[i];
        num_children++;
    }

    // and my level and parent
    my_level  = sensorsInfo[tree_addr16].level;
    my_parent = sensorsInfo[tree_addr16].parent;

    if (sensorsInfo[tree_addr16].isDead)
    {
        nrk_terminate_task();
    }
}

uint8_t tdma_tree_made()
{
    return tree_made;
}

// Returns my level of the tree
uint8_t tdma_tree_level_get()
{
    if (tree_made == 0)
        return NRK_ERROR;

    return my_level;
}

node_addr_t tdma_tree_parent_get()
{
    if (tree_made == 0)
    {
        return -1;
    }
    return my_parent;
}

uint8_t tdma_tree_is_leaf()
{
    if (tree_made == 0)
        return NRK_ERROR;

    return (num_children == 0);
}

void _tdma_tree_backoff()
{
    uint16_t b;
    uint8_t r;
    
    // OPTION 1: USE NRK_WAIT_UNTIL
    nrk_time_get(&cur_time);
    backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
    backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
    
    nrk_time_add(&backoff_time,backoff_time,cur_time);
    nrk_time_compact_nanos(&backoff_time);
    nrk_wait_until(backoff_time);

/*
    // OPTION 2: USE NRK_SPIN_WAIT_US
    b=(TREE_BACKOFF_TIME_MS * NANOS_PER_MS)/(((uint16_t)rand()%100)+1);
    nrk_spin_wait_us(b);

    // OPTION 3: USE THE HST
    // backoff some number of ticks between 655 and 65500, in steps of 655
    b=(TREE_BACKOFF_TIME_TICKS) * ((rand()%100)+1);

    #if TDMA_TREE_DEBUG>=2
        printf("bk %u\r\n", b);
    #endif

    // wait some random number of times less than 10
    for (r = 0; r < (rand()%10)+1; r++)
        nrk_high_speed_timer_wait(0,b);
*/
}

// assign colors to:
// some set (1 or more) of TX slots specified to be to the parent (data)
// some set (1 or more) of TX slots to be to children  (time sync/ actuation)
tdma_slot_t _tdma_tree_color_to_slot(tdma_color_t color, tdma_slot_type_t type)
{
    if (type == TDMA_TX_PARENT)
    {
        // this is defined arbitrarily.  Right now it's just 2 * color
        // for send to parent slot and send to child slot is 2 * color + 1
        return (COLOR_SLOT_STRIDE*2)*color + COLOR_SLOT_STRIDE;
    }
    else if (type == TDMA_TX_CHILD)
    {
        return (COLOR_SLOT_STRIDE*2)*color;
    }
    else
    {
        return -1;
    }
}

int8_t _tdma_tree_add_neighbor(node_addr_t neighbor_addr, int8_t rssi)
{
    for (uint8_t i = 0; i < num_neighbors; i++)
    {
        if (neighbor_info[i] == neighbor_addr)
        {
            // Let's be pessimistic.  If the neighbor has been
            // recorded before, let's record the minimum RSSI value
            // of the last record and this new one. 
            if (neighbor_rssi[i] > rssi)
                neighbor_rssi[i] = rssi;

            return NRK_ERROR;
        }
    }
    if (num_neighbors < TREE_MAX_CHILDREN)
    {
        neighbor_info[num_neighbors] = neighbor_addr;
        neighbor_rssi[num_neighbors] = rssi;
        num_neighbors++;
        return NRK_OK;
    }

    return NRK_ERROR;
}


// Here, I have to add the child, but also know the size of the array
// they are sending me with their own child info

int8_t _tdma_tree_add_child(uint8_t * new_child, uint8_t new_child_size)
{
    //printf("in add child: %d %d\r\n", 
    //                                            new_child[0],
    //                                            new_child[1]);

    memcpy(&child_info[child_info_size], new_child, new_child_size);

    uint16_t child_num_print = child_info[child_info_size];
    child_num_print <<= 8;
    child_num_print |= child_info[child_info_size+1];

#if TDMA_TREE_DEBUG>=3
    printf("ADDCHILD %d, %d, %d\r\n", child_info[child_info_size],
        child_info[child_info_size+1], child_num_print);
#endif

    child_info_size += new_child_size;
    //num_children++;
    return NRK_OK;
}


int8_t _tdma_tree_add_child_to_list(node_addr_t child_addr)
{
    for (uint8_t i = 0; i < num_children; i++)
    {
        if (child_addrs[i] == child_addr)
            return NRK_ERROR;
    }
    if (num_children < TREE_MAX_CHILDREN)
    {
        child_addrs[num_children++] = child_addr;
        return NRK_OK;
    }

    return NRK_ERROR;
}

/*
int8_t _tdma_tree_build_subtree()
{
    memcpy(subtree_info[subtree_info_size], tree_addr16, ADDR_SIZE);
    subtree_info_size+= ADDR_SIZE;

    subtree_info[subtree_info_size++] = num_children;

    memcpy(subtree_info[subtree_info_size], child_info, (num_children * ADDR_SIZE));
    subtree_info_size += num_children * ADDR_SIZE;

    subtree_info[subtree_info_size++] = num_neighbors;
    memcpy(subtree_info[subtree_info_size], neighbor_info, (num_neighbors * ADDR_SIZE));
    subtree_info_size += num_neighbors * ADDR_SIZE;
}
*/

// Here we fill the info array (a tx buffer) with my address,
// and info about all children I've collected and neighbors

int8_t _tdma_tree_build_subtree(uint8_t * info_arr)
{
    // might want to do this with shifting instead

    info_arr[info_buf_size]   = (tree_addr16 >> 8);
    info_arr[info_buf_size+1] = (tree_addr16 & 0xFF);

    //memcpy(info_arr[info_buf_size], tree_addr16, ADDR_SIZE);

#if TDMA_TREE_DEBUG>=2
    printf("my addr to send: %d %d\r\n", info_arr[info_buf_size],
            info_arr[info_buf_size+1]);
#endif


    info_buf_size += ADDR_SIZE;

    info_arr[info_buf_size++] = num_children;

    memcpy(&info_arr[info_buf_size], child_info, child_info_size);
    info_buf_size += child_info_size;

    info_arr[info_buf_size++] = num_neighbors;

    // flip the dang bits
    for (uint8_t i = 0; i < num_neighbors; i++)
    {
        info_arr[info_buf_size] = neighbor_info[i] >> 8;
        info_arr[info_buf_size+1] = neighbor_info[i] & 0xFF;
        //memcpy(&info_arr[info_buf_size], neighbor_info, (num_neighbors * ADDR_SIZE));
        
        info_buf_size+=2;
    }

    return NRK_OK;
    //info_buf_size += num_neighbors * ADDR_SIZE;
}


// Send out packet to everyone to become their parent
void _tdma_tree_send_mktree()
{
    int8_t v;
    uint8_t i;
    //uint16_t b;

    // send out a packet to everyone
    //tree_rfTxInfo.pPayload[0] = TREE_MAKE;
    //tree_rfTxInfo.pPayload[1] = my_level;
    tree_tx_buf[0] = TREE_MAKE;
    tree_tx_buf[1] = my_level;

    tree_rfTxInfo.pPayload = tree_tx_buf;
    //tree_rfTxInfo.length = TREE_TX_BUF_SIZE;
    tree_rfTxInfo.length = 2;

    // do things the bmac way sorta, back off for a random period
    // of time
    
    //b=((uint32_t) TREE_BACKOFF_TIME_MS*1000)/(((uint32_t)rand()%100)+1);
    //nrk_spin_wait_us(b);

    _tdma_tree_backoff();

    for (i = 0; i < CSMA_SEND_REPEAT; i++)
    {
        while (!(v = _tdma_channel_check()))
        {
            /*
            rf_rx_off();
            
            nrk_time_get(&cur_time);
            nrk_time_compact_nanos(&cur_time);

            backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
            backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
            nrk_time_add(&backoff_time, backoff_time, cur_time);
            nrk_time_compact_nanos(&backoff_time);

            nrk_wait_until(backoff_time);
            */
            _tdma_tree_backoff();
        }

        rf_rx_off();
        rf_tx_packet( &tree_rfTxInfo );
    }

#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("MKTREE\r\n"));
#endif
}

// Code to wait for children to notify me I am their parent
void _tdma_tree_get_children()
{
    uint8_t n;
    uint8_t expired = 0;
    node_addr_t child_addr;
    node_addr_t dest_addr = 0;
    uint8_t message_type;

#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("WFR\r\n"));
#endif

    nrk_time_t wait_time;

    nrk_time_get(&cur_time);
    nrk_time_compact_nanos(&cur_time);

    uint16_t my_wait_time_ms = TREE_WAIT_TIME_MS / (my_level+1);
    wait_time.secs = my_wait_time_ms / 1000;
    wait_time.nano_secs = (my_wait_time_ms % 1000) * NANOS_PER_MS;

    nrk_time_compact_nanos(&wait_time);
    nrk_time_add(&wait_time, wait_time, cur_time);
    nrk_time_compact_nanos(&wait_time);



    while (!expired)
    {
        rf_polling_rx_on();
        //while (rf_rx_check_fifop() == 0)
        //    ;
        while (((n=rf_polling_rx_packet()) == 0) && !expired)
        {
            nrk_time_get(&cur_time);
            nrk_time_compact_nanos(&cur_time);

            if (nrk_time_cmp(cur_time, wait_time) >= 0)
                expired = 1;
        }

        //make sure a valid packet 
        if (n == 0)
        {
#if TDMA_TREE_DEBUG>=2
            nrk_kprintf(PSTR("WFR: Timeout\r\n"));
#endif
            continue;
        }
        else if (n != 1)
        {
#if TDMA_TREE_DEBUG>=2
            nrk_kprintf(PSTR("WFR: bad rcv\r\n"));
#endif
            continue;
        }

        rf_rx_off();

        // packet is here, check for right type
        message_type = tree_rfRxInfo.pPayload[PKT_TYPE];
        //printf("wfr:msg type: %d\r\n",message_type);

        if (message_type != TREE_NOTIFY)
        {
            //nrk_kprintf(PSTR("  msg type wrong. \r\n"));
            continue;
        }

        // Extract the destination address.  If it's for me, I have to
        // do parental things (like take the children and forward them.
        // Otherwise, I add the address to my neighbor list and turn a blind eye.
        dest_addr = tree_rfRxInfo.pPayload[DEST_ADDR];
        dest_addr <<= 8;
        dest_addr |= tree_rfRxInfo.pPayload[DEST_ADDR+1];

	
	//printf("ta16 %d\r\n", tree_addr16);
        //printf("Dest addr : %d \r\n", dest_addr);
        if (dest_addr == tree_addr16)
        {
                
                // open the present! This child has sent
                // me all of its subtree information
                uint8_t child_pkt_size = tree_rfRxInfo.pPayload[SIZE_OF_CHILDREN];
                
                // add this child
                child_addr = tree_rfRxInfo.srcAddr;

                //extract 

                // unless I've heard from this child,
                // add it to my list
                if (_tdma_tree_add_child_to_list(child_addr) == NRK_OK)
                {
#if TDMA_TREE_DEBUG>=1
                    printf("addchild %d, %d %d rssi %d\r\n", child_addr, 
                                tree_rfRxInfo.pPayload[MY_ADDR],
                                tree_rfRxInfo.pPayload[MY_ADDR],
                                tree_rfRxInfo.rssi);
#endif

                    _tdma_tree_add_child_to_list(child_addr);
                    _tdma_tree_add_child(&tree_rfRxInfo.pPayload[MY_ADDR], child_pkt_size);
                    //_tdma_tree_add_child(child_addr);
                }
                else
                {
                    //nrk_kprintf(PSTR("Child already added.\r\n"));
                }
        }
        else
        {
#if TDMA_TREE_DEBUG>=2
            printf("%d nbr %d rssi %d\r\n", tree_addr16, child_addr, tree_rfRxInfo.rssi);
#endif
            _tdma_tree_add_neighbor(dest_addr, tree_rfRxInfo.rssi);
        }

        nrk_time_get(&cur_time);
        nrk_time_compact_nanos(&cur_time);
        
        if (nrk_time_cmp(cur_time, wait_time) >= 0)
            expired = 1;
    }
    
}

// After I receive a packet, I want to notify the sender that
// I am now their child

void _tdma_tree_notify_parent()
{
    int8_t v;
    //uint16_t b;

    /* My packet will look something like this ....
     *  0   : Tree build packet
     *  1-2 : Destination address (parent)
     *  3   : My level
     *  4   : Size of the following information block:
     *  5-6 : My address
     *  7   : My number of children
     *  8...: My children addresses etc
     */
        
    tree_tx_buf[PKT_TYPE] = TREE_NOTIFY;
    //tree_tx_buf[1] will contain size...
    tree_tx_buf[MY_LEVEL] = my_level;

    info_buf_size = MY_ADDR; // so far...

    // put in my address
    // NEVERMIND: Done in _tdma_tree_build_subtree
    tree_tx_buf[DEST_ADDR] = (my_parent >> 8);
    tree_tx_buf[DEST_ADDR+1] = (my_parent & 0xFF);

    // FIXME Bad practice here, should count from start

    // add my address, the children I've collected, and my neighbors
    _tdma_tree_build_subtree(tree_tx_buf);

    tree_tx_buf[SIZE_OF_CHILDREN] = info_buf_size - MY_ADDR;

#if TDMA_TREE_DEBUG>=2
    printf("send parent Type %d\r\n", tree_tx_buf[PKT_TYPE]);
#endif

    tree_rfTxInfo.pPayload = tree_tx_buf;
    //tree_rfTxInfo.length = TREE_TX_BUF_SIZE;
    tree_rfTxInfo.length = info_buf_size;
    
    // I thought this might be a good idea to try at first,
    // but the hardware recognition seems to be kind of funky
    // if not within a (very close) distance.  Straaaange yes?
    //tree_rfTxInfo.destAddr = my_parent;

    
    // Send out packet multiple times to notify.
    for (uint8_t i = 0; i < CSMA_SEND_REPEAT; i++)
    {

            /*
            b=(TREE_BACKOFF_TIME_MS*1000)/(((uint32_t)rand()%10)+1);
            nrk_spin_wait_us(b);
            */

            _tdma_tree_backoff();

            while (!(v = _tdma_channel_check())) //channel busy
            {
                /*
#if TDMA_TREE_DEBUG>=2
                nrk_kprintf(PSTR("Backoff\r\n"));
#endif
                //back off some set time, then check the channel again
                nrk_time_get(&cur_time);
                nrk_time_compact_nanos(&cur_time);

                backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
                backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
                nrk_time_add(&backoff_time, backoff_time, cur_time);
                nrk_time_compact_nanos(&backoff_time);

                nrk_wait_until(backoff_time);
                */
                _tdma_tree_backoff();
            }

            rf_rx_off();
            rf_tx_packet( &tree_rfTxInfo );
    }
}



void _tdma_tree_print_sensors()
{
    int8_t i;
    for (i = 0; i < DIM; i++)
    {
        if (sensorsInfo[i].isDead)
        {
            printf("Sensor %d is dead\r\n",i);
        }
        else
        {
            printf("  addr %d par: %d lvl: %d ch: %d clr: %d\r\n",
                sensorsInfo[i].address,
                sensorsInfo[i].parent,
                sensorsInfo[i].level,
                sensorsInfo[i].totalChildren,
                sensorsInfo[i].color);
            //printf("  color: %d\r\n", sensorsInfo[i].color);
                //graphColors[sensorsInfo[i].color]);
                //graphColors[colors_used_level[ sensorsInfo[i].level  ]
                                              //[ sensorsInfo[i].color] ] );

        }

    }

}

void _tdma_tree_discover_neighbors()
{
    nrk_time_t neighbor_dsc_time;
    nrk_time_t next_send_time;
    nrk_time_t ct;
    uint16_t nbr_addr;
    uint8_t n,v;


    neighbor_dsc_time.secs = TREE_NBR_DISCOVERY_MS / 1000;
    neighbor_dsc_time.nano_secs = (uint32_t)(TREE_NBR_DISCOVERY_MS % 1000) * NANOS_PER_MS;
    nrk_time_compact_nanos(&neighbor_dsc_time);

    nrk_time_get(&ct);
    nrk_time_add(&neighbor_dsc_time, neighbor_dsc_time, ct);

#if TDMA_TREE_DEBUG>=1
    nrk_kprintf(PSTR("get nbrlist\r\n"));
#endif

    // prepare the nbr packet.
    // 3 bytes
    // byte 1: CTL for NBR
    // bytes 2-3: my node address.
    tree_tx_buf[0] = TREE_NBR;
    tree_tx_buf[1] = (tree_addr16 >> 8);
    tree_tx_buf[2] = (tree_addr16 & 0xFF);

    tree_rfTxInfo.pPayload = tree_tx_buf;
    tree_rfTxInfo.length   = 3;

    do
    {
        // RX until send time
        nrk_time_get(&ct);

        next_send_time.secs      = 0;
        next_send_time.nano_secs = (TREE_BACKOFF_TIME_MS*NANOS_PER_MS)/
                                    (((uint32_t)rand()%10)+1);
        nrk_time_compact_nanos(&next_send_time);
        nrk_time_add(&next_send_time, next_send_time, ct);

        // if our next send is going to be outside the final timeout,
        // just do the send at that timeout and then quit.
        if (nrk_time_cmp(next_send_time, neighbor_dsc_time) > 0)
            next_send_time = neighbor_dsc_time;

        do 
        {
            rf_polling_rx_on();
            
            do
            {
                n = rf_polling_rx_packet();
                nrk_time_get(&ct);
            }
            while (n == 0 && nrk_time_cmp(ct, next_send_time) < 0);

            rf_rx_off();

            if (n == 1)
            {
                if (tree_rfRxInfo.pPayload[0] == TREE_NBR)
                {
                    nbr_addr = tree_rfRxInfo.pPayload[1];
                    nbr_addr <<= 8;
                    nbr_addr |= tree_rfRxInfo.pPayload[2];
                    if (_tdma_tree_add_neighbor(nbr_addr, tree_rfRxInfo.rssi) == NRK_OK)
    #if TDMA_TREE_DEBUG>=1
                        printf("Have nbr %d %d %d\r\n", tree_addr16, nbr_addr, tree_rfRxInfo.rssi);
    #else
			;
    #endif
                }
                else
                {
    #if TDMA_TREE_DEBUG>=2
                    nrk_kprintf(PSTR("TD: Wrong pkt type\r\n"));
    #endif
                }

            }
        } while (nrk_time_cmp(ct, next_send_time) < 0);


        while (!(v = _tdma_channel_check())) //channel busy
        {
            /*
#if TDMA_TREE_DEBUG>=2
            nrk_kprintf(PSTR("Backoff\r\n"));
#endif
            //back off some set time, then check the channel again
            nrk_time_get(&cur_time);

            backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
            backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
            nrk_time_add(&backoff_time, backoff_time, cur_time);
            nrk_time_compact_nanos(&backoff_time);

            nrk_wait_until(backoff_time);
            */
            _tdma_tree_backoff();
        }

        rf_rx_off();
        rf_tx_packet( &tree_rfTxInfo );
        

    } while (nrk_time_cmp(ct, neighbor_dsc_time) < 0);

}


#ifdef TDMA_MASTER_MODE
/* SCHEDULE PACKET FORMAT 
** Requires that sensorsInfo is filled with useful information
**
**  0     : TREE_SCHEDULE (control value)
**  1     : number of nodes in schedule
**  2     : number of this sched packet
**  3-4   : Addr of first node
**  5-6   : Slot with priority 0 (my slot)
**  7-8   : Stealable slot with priority 1
**  9-14  : Other stealable slots p1 to p_{max_children}
**  15-16 : TX slot to children (sync packet)
*/

void _tdma_tree_send_schedule()
{
    int8_t i,j,v;
    //uint16_t b;
    uint8_t pkt_offset;
    uint8_t live_sensors;
    int8_t priority;
    uint8_t sched_pkt_num;
    uint8_t total_sched_pkts;
    uint8_t nodes_per_packet;

    live_sensors = 0;

    for (i = 0; i < DIM; i++)
    {
        if (!sensorsInfo[i].isDead)
            live_sensors++;
    }

    tree_rfTxInfo.pPayload = tree_tx_buf;
    tree_rfTxInfo.pPayload[0] = TREE_SCHEDULE;
    //tree_rfTxInfo.pPayload[1] will have # of live nodes
    
#if TDMA_TREE_DEBUG>=2
    printf("CASS: sensors %d\r\n", live_sensors);
#endif
    tree_rfTxInfo.pPayload[1] = live_sensors;

    // new stuff
    sched_pkt_num = 1;
    tree_rfTxInfo.pPayload[2] = sched_pkt_num;
    
    // size: control + # nodes + # packets + each node's size
    //total_sched_pkts = (live_sensors * (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2)))
     //                    / (RF_MAX_PAYLOAD_SIZE-3);

    // how many nodes' schedule entries can we fit into a single packet
    nodes_per_packet = (TREE_BUF_SIZE-3) / 
                        (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2));

    total_sched_pkts = live_sensors / nodes_per_packet;

    if ((live_sensors % nodes_per_packet) > 0)
        total_sched_pkts++;

//    if (((live_sensors * (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2)))
//                 % (RF_MAX_PAYLOAD_SIZE-3))  > 0)
//            total_sched_pkts++;

#if TDMA_TREE_DEBUG>=2
    printf("CASS: pkts %d\r\n", total_sched_pkts);
#endif
    pkt_offset = 3;

    for (i = 0; i < DIM; i++)
    {
        if (!sensorsInfo[i].isDead)
        {
            // deteremine if we can fit the next node in the schedule
            if (pkt_offset + ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2)  >= TREE_BUF_SIZE)
            {
                // We need a new packet
                tree_rfTxInfo.length = pkt_offset;

                for (uint8_t r = 0; r < CSMA_SEND_REPEAT; r++)
                {
                    /*
                    b=(TREE_BACKOFF_TIME_MS*1000)/(((uint32_t)rand()%100)+1);
                    nrk_spin_wait_us(b);
                    */
                    _tdma_tree_backoff();

                    while (!(v = _tdma_channel_check()))
                    {
                        /*
                        rf_rx_off();
                        nrk_time_get(&cur_time);

                        backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
                        backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
                        nrk_time_add(&backoff_time, backoff_time, cur_time);
                        nrk_time_compact_nanos(&backoff_time);

                        nrk_wait_until(backoff_time);
                        */
                        _tdma_tree_backoff();
                    }

                    rf_rx_off();
                    rf_tx_packet( &tree_rfTxInfo );               
                }

#if TDMA_TREE_DEBUG>=1
                printf("Sent pkt %d/%d\r\n", sched_pkt_num, total_sched_pkts);
#endif

                nrk_spin_wait_us(5000);
                
                // first two bytes remain the same: Replace the third and move on.
                tree_rfTxInfo.pPayload[2] = ++sched_pkt_num;
                pkt_offset = 3;
            }
            
            //insert its color into p0
            
#if TDMA_TREE_DEBUG>=2
            printf(" Slot %d for %d, offset %d\r\n", 
                _tdma_tree_color_to_slot(sensorsInfo[i].color,TDMA_TX_PARENT), i,pkt_offset);
#endif

            // put in the address
            tree_rfTxInfo.pPayload[pkt_offset] = (sensorsInfo[i].address >> 8);
            tree_rfTxInfo.pPayload[pkt_offset+1] = (sensorsInfo[i].address & 0xFF);

            pkt_offset+=2;

            // put in their priority 0 slot
            tree_rfTxInfo.pPayload[pkt_offset] =
                (_tdma_tree_color_to_slot(sensorsInfo[i].color,TDMA_TX_PARENT) >> 8);
            tree_rfTxInfo.pPayload[pkt_offset+1] = 
                (_tdma_tree_color_to_slot(sensorsInfo[i].color,TDMA_TX_PARENT) & 0xFF);

            priority = 1;

            // NEW PRIORITY METHOD.  Loop around list and add in order
            for (j = ((i+1) % DIM);
                 j!=i && priority < TREE_MAX_CHILDREN;
                 j=((j+1) % DIM))
            {
                if ((!sensorsInfo[j].isDead) && sensorsInfo[i].parent == sensorsInfo[j].parent)
                {
                    tree_rfTxInfo.pPayload[pkt_offset+(2*priority)] = 
                        (_tdma_tree_color_to_slot(sensorsInfo[j].color,TDMA_TX_PARENT) >> 8);
                    tree_rfTxInfo.pPayload[pkt_offset+(2*priority)+1] = 
                        (_tdma_tree_color_to_slot(sensorsInfo[j].color,TDMA_TX_PARENT) & 0xFF);
                    priority++;
                }
            }     

#if TDMA_TREE_DEBUG>=2
            printf("Priority %d\r\n", priority);
#endif
            // fill other "Stealable" slot entries with -1 to say they are not so
            for (j = priority; j < TREE_MAX_CHILDREN; j++)
            {
                //printf("Encoded into %d\r\n", pkt_offset+(2*j));
                tree_rfTxInfo.pPayload[pkt_offset+(2*j)] = (((int16_t) -1) >> 8);
                tree_rfTxInfo.pPayload[pkt_offset+(2*j)+1] = (((int16_t) -1) & 0xFF);
            }
            
            pkt_offset += TREE_MAX_CHILDREN * 2;

            // Put in their tx slot to children, last
            tree_rfTxInfo.pPayload[pkt_offset] = 
                 (_tdma_tree_color_to_slot(sensorsInfo[i].color, TDMA_TX_CHILD) >> 8);
            tree_rfTxInfo.pPayload[pkt_offset+1] = 
                 (_tdma_tree_color_to_slot(sensorsInfo[i].color, TDMA_TX_CHILD) & 0xFF);

            pkt_offset += 2;

#if TDMA_TREE_DEBUG>=2
            printf("pkt size %d\r\n", pkt_offset);
#endif
        }
    }

    if (pkt_offset > 3)
    {    
        //send last schedule packet
        tree_rfTxInfo.length = pkt_offset;

        for (uint8_t r = 0; r < CSMA_SEND_REPEAT; r++)
        {
            _tdma_tree_backoff();
            /*
            b=(TREE_BACKOFF_TIME_MS*1000)/(((uint32_t)rand()%100)+1);
            nrk_spin_wait_us(b);
            */

            while (!(v = _tdma_channel_check()))
            {
                /*
                rf_rx_off();
                nrk_time_get(&cur_time);

                backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
                backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
                nrk_time_add(&backoff_time, backoff_time, cur_time);
                nrk_time_compact_nanos(&backoff_time);

                nrk_wait_until(backoff_time);
                */
                _tdma_tree_backoff();
            }

            rf_rx_off();
            rf_tx_packet( &tree_rfTxInfo );               
        }
#if TDMA_TREE_DEBUG>=1
        printf("Sent pkt %d/%d\r\n", sched_pkt_num, total_sched_pkts);
#endif
    }

    //nrk_spin_wait_us(50000);
    
    // first two bytes remain the same: Replace the third and move on.
    //tree_rfTxInfo.pPayload[2] = sched_pkt_num++;
    //pkt_offset = 3;


    /* Pruint8_t packet
    nrk_kprintf(PSTR("PACKET: "));
    for (uint8_t offset = 0; offset < pkt_offset; offset++)
    {
        printf("%x ", tree_rfTxInfo.pPayload[offset]);
    }
    nrk_kprintf(PSTR("\r\n"));
    */

} // END CASS

// Master's function to obtain its schedule via a filled sensorsInfo structure
void _tdma_tree_master_get_schedule()
{
    uint8_t i, child_index;
    for (i = 0; i < DIM; i++)
    {
        if (!sensorsInfo[i].isDead)
        {
            if (sensorsInfo[i].address  == tree_addr16)
            {
                // if me, add my slots
                //tdma_schedule_add(_tdma_tree_color_to_slot(sensorsInfo[i].color, TDMA_TX_PARENT), TDMA_TX, 0);
                tdma_schedule_add(_tdma_tree_color_to_slot(sensorsInfo[i].color, TDMA_TX_CHILD) , TDMA_TX_CHILD, 0);
            }
            else
            {
                for (child_index = 0; child_index < num_children; child_index++)
                {
                    if (sensorsInfo[i].address == child_addrs[child_index])
                    {
                        //is an immediate child;  add RX slots
                        tdma_schedule_add(_tdma_tree_color_to_slot(sensorsInfo[i].color, TDMA_TX_PARENT), TDMA_RX, -1);
                    }
                }
            }
        }
    }
}

// only the master should need to do this
// This takes each child info packet and interprets it to map to the
// sensorsInfo information array

void _tdma_tree_parse(uint8_t * info_arr, uint8_t * offset,
                         uint8_t level, node_addr_t parent)
{
    uint8_t i;
    uint8_t n_children, n_neighbors;
    node_addr_t child, neighbor;

    if (level > depth_of_tree)
        depth_of_tree = level;

#if TDMA_TREE_DEBUG>=2
    printf("Level %d cio %d\r\n", level, *offset); 
#endif
    child = info_arr[*offset];
    child <<= 8;
    child |= info_arr[(*offset)+1];
    
    n_children = info_arr[(*offset)+2];

#if TDMA_TREE_DEBUG>=2
    printf("Child %d, has %d children, parent %d\r\n", child, n_children,parent);
#endif

    //sensorsInfo[child].totalChildren = n_children;
    sensorsInfo[child].address = child;
    sensorsInfo[child].totalChildren = 0;
    sensorsInfo[child].isDead        = 0;
    sensorsInfo[child].level         = level;
    sensorsInfo[child].parent        = parent;
    
    sensorsInfo[ parent ].children[ sensorsInfo[parent].totalChildren++ ] = child;

    Neighbors[child][parent] = 1;
    Neighbors[parent][child] = 1;

    (*offset) += 3;
#if TDMA_TREE_DEBUG>=2
    printf("cio after add 3: %d\r\n",  *offset);
#endif
    
    for (i = 0; i < n_children; i++)
    {
        _tdma_tree_parse(info_arr, offset, level+1, child);
    }
    
    n_neighbors = info_arr[*offset];
    (*offset)++;
    
    for (i = 0; i < n_neighbors; i++)
    {
        neighbor = info_arr[*offset];
        neighbor <<= 8;
        neighbor |= info_arr[(*offset)+1];

#if TDMA_TREE_DEBUG>=2
        printf("Child %d has neighbor %d\r\n", child, neighbor);
#endif
        (*offset)+=2;
        Neighbors[child][neighbor] = 1;
        Neighbors[neighbor][child] = 1;
    }

}



// Slave's function to obtain schedule
// This includes waiting for the packet, parsing it
// and adding slots
void _tdma_tree_obtain_schedule()
{
    int8_t n;
    uint8_t v;
    //uint32_t b;
    uint8_t i, j, pkt_offset, child_index;
    uint8_t message_type;
    uint8_t num_nodes;
    node_addr_t current_node_addr;
    tdma_slot_t new_slot;
    uint8_t have_schedule = 0;
    uint8_t current_pkt   = 0;
    uint8_t pkts_to_expect = 0;
    uint8_t pkts_received = 1;
    uint8_t nodes_per_packet;

    tree_rfRxInfo.max_length = TREE_BUF_SIZE;
    tree_rfRxInfo.pPayload = tree_rx_buf; 

    // Wait for a packet containing the schedule.
    
#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("Wait for schedule\r\n"));
#endif

    while (!have_schedule)
    {
    
        tree_rfRxInfo.pPayload[0] = 0;
        message_type = 0;
        rf_polling_rx_on();
            
        while ((n=rf_polling_rx_packet()) == 0)
            ; 

        rf_rx_off();

        if (n != 1)
        {
#if TDMA_TREE_DEBUG>=2
            nrk_kprintf(PSTR("SCHED: Invalid recv\r\n"));
#endif
            continue;
        }


        message_type = tree_rfRxInfo.pPayload[0];

        if (message_type != TREE_SCHEDULE)
        {
#if TDMA_TREE_DEBUG>=2
            nrk_kprintf(PSTR("SCHED: Wrong msg type\r\n"));
#endif
            continue;
        }

        // We have a valid tree packet, begin parsing
        num_nodes = tree_rfRxInfo.pPayload[1];
        current_pkt = tree_rfRxInfo.pPayload[2];

        nodes_per_packet = (TREE_BUF_SIZE-3) / 
                         (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2));

        pkts_to_expect = num_nodes / nodes_per_packet;

        if ((num_nodes % nodes_per_packet) > 0)
            pkts_to_expect++;

        // if this is first packet, we can find how many to expect
        /*
        pkts_to_expect = (num_nodes * (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2)))
                         / (RF_MAX_PAYLOAD_SIZE-3);
        if (((num_nodes * (ADDR_SIZE + ((TREE_MAX_CHILDREN+1) * 2)))
                         % (RF_MAX_PAYLOAD_SIZE-3))  > 0)
            pkts_to_expect++;
        */

        if (current_pkt != pkts_received)
        {
#if TDMA_TREE_DEBUG>=2
            printf("Recv %d need %d\r\n", 
                current_pkt, pkts_received);
#endif
            //if (current_pkt > pkts_received)
            //    nrk_halt();
            continue;
        }

        pkts_received++;

#if TDMA_TREE_DEBUG>=1
        printf("Got pkt %d/%d. I'm %d\r\n", current_pkt,pkts_to_expect, tree_addr16);
#endif


        if (current_pkt == pkts_to_expect)
        {
#if TDMA_TREE_DEBUG>=2
            printf("Have all pkts %d lv %d\r\n", tree_addr16, my_level);
#endif
            have_schedule = 1;
        }

    ///PRINT WHOLE PACKET
    /*
    nrk_kprintf(PSTR("PACKET: "));
    for (uint8_t offset = 0; offset < (tree_rfRxInfo.pPayload[1] * ((DIM+1) * ADDR_SIZE)) + 2; offset++)
    {
        printf("%x ", tree_rfRxInfo.pPayload[offset]);
    }
    nrk_kprintf(PSTR("\r\n"));
    */
    ///ENDPRINT

        //parse schedule
       

#if TDMA_TREE_DEBUG>=2
        printf("Num Nodes: %d\r\n", num_nodes);
#endif

        pkt_offset = 3; //

        for (i = 0;
             i < num_nodes && 
                ((pkt_offset + (TREE_MAX_CHILDREN+1) * 2) < TREE_BUF_SIZE &&
                 (pkt_offset + (TREE_MAX_CHILDREN+1) * 2) < tree_rfRxInfo.length
                 )
              ; i++)
        {
            current_node_addr = tree_rfRxInfo.pPayload[pkt_offset]; 
            current_node_addr <<= 8;
            current_node_addr |= tree_rfRxInfo.pPayload[pkt_offset+1]; 

#if TDMA_TREE_DEBUG>=2
            printf("OS: Got node %d. I'm %d offset %d\r\n",
                   current_node_addr, tree_addr16, pkt_offset);
#endif

            pkt_offset += 2;

            if (current_node_addr == tree_addr16)
            {
#if TDMA_TREE_DEBUG>=2
                nrk_kprintf(PSTR("It's for me\r\n"));
#endif
                for (j = 0; j < TREE_MAX_CHILDREN; j++)
                {
                    new_slot = tree_rfRxInfo.pPayload[pkt_offset];
                    new_slot <<= 8;
                    new_slot |= tree_rfRxInfo.pPayload[pkt_offset+1];

#if TDMA_TREE_DEBUG>=2
                    printf("Slot %d at offset %d\r\n", new_slot, pkt_offset);
#endif
                    if (new_slot > TDMA_SLOTS_PER_CYCLE)
                    {
#if TDMA_TREE_DEBUG>=2
                        printf("IGNORING SLOT: %d\r\n", new_slot);
#endif
                    }
                    // If SLOT_STEALING is enabled, add all priorities.
                    // otherwise, just add the 0 priority slots (my TX slots)
#ifdef SLOT_STEALING_SCHED
                    else if (new_slot >= 0)
                        tdma_schedule_add(new_slot, TDMA_TX_PARENT, j);
#else
                    else if (new_slot >= 0 && j == 0)
                        tdma_schedule_add(new_slot, TDMA_TX_PARENT, j);
#endif

                    pkt_offset+=2;
                }

                // add my TX_CHILD slot.  Priority can be 0 because slot
                // stealing among time packets is pretty impractical.
                new_slot = tree_rfRxInfo.pPayload[pkt_offset];
                new_slot <<= 8;
                new_slot |= tree_rfRxInfo.pPayload[pkt_offset+1];
            
                if (new_slot >= 0)
                    tdma_schedule_add(new_slot, TDMA_TX_CHILD, 0);

                pkt_offset+=2;

            }
            else if (current_node_addr == my_parent)
            {
                // add the parent's TX to child slot to my RX slots
                // This is my rxsync slot.

                // seek past the priority slots
                pkt_offset += ((TREE_MAX_CHILDREN) * ADDR_SIZE);

                new_slot = tree_rfRxInfo.pPayload[pkt_offset];
                new_slot <<= 8;
                new_slot |= tree_rfRxInfo.pPayload[pkt_offset+1];

                if (new_slot >= 0)
                    tdma_schedule_add(new_slot, TDMA_RX, 1); //p1 to distinguish that it is sync

                pkt_offset+=2;
            }
            else  // wasn't my address or parent's
            {
                // check if any child in my child array matches this node address
                for (child_index = 0; child_index < num_children; child_index++)
                {
                    if (child_addrs[child_index] == current_node_addr) // this refers to my child
                    {
                        new_slot = tree_rfRxInfo.pPayload[pkt_offset];
                        new_slot <<= 8;
                        new_slot |= tree_rfRxInfo.pPayload[pkt_offset+1];
                        // add this as a RX
#if TDMA_TREE_DEBUG>=2
                        printf("Slot %d\r\n", new_slot);
#endif
                        if (new_slot >= 0)
                            tdma_schedule_add(new_slot, TDMA_RX, -1);
                        
                        break;
                    }
                }
                // skip past priority slots and TX_CHILD slot
                pkt_offset+= ((TREE_MAX_CHILDREN) * ADDR_SIZE) + 2;
            }
        }
        // end of this packet. Time to send.
#if TDMA_TREE_DEBUG>=2
        printf("Me %d send offset %d\r\n", tree_addr16, pkt_offset);
#endif

        memcpy(tree_tx_buf, tree_rfRxInfo.pPayload, pkt_offset);
        tree_rfTxInfo.pPayload = tree_tx_buf;

        //printf("Type %d\r\n", tree_rfTxInfo.pPayload[0]);
        // length is predefined based on number of sensors
        tree_rfTxInfo.length = pkt_offset;// (tree_rfRxInfo.pPayload[1] * DIM) + 2;

        // Send out packet multiple times to notify.
        for (uint8_t i = 0; i < CSMA_SEND_REPEAT; i++)
        {

            /*
            b=((uint32_t)TREE_BACKOFF_TIME_MS*1000)/(((uint32_t)rand()%10)+1);
            nrk_spin_wait_us(b);
            */
            _tdma_tree_backoff();

            while (!(v = _tdma_channel_check())) //channel busy
            {
                /*
                //back off some set time, then check the channel again
                nrk_time_get(&cur_time);

                backoff_time.secs = TREE_BACKOFF_TIME_MS / 1000;
                backoff_time.nano_secs = (TREE_BACKOFF_TIME_MS % 1000) * NANOS_PER_MS;
                nrk_time_add(&backoff_time, backoff_time, cur_time);
                nrk_time_compact_nanos(&backoff_time);

                nrk_wait_until(backoff_time);
                */
                _tdma_tree_backoff();
            }

            rf_rx_off();
            rf_tx_packet( &tree_rfTxInfo );
        }

    }
}
#endif // TDMA_SLAVE

#ifdef TDMA_MASTER_MODE

// colors each node differently
/*
void discrete_color()
{
    uint8_t i,j;

    j = 0;
   
    for ( i = 0; i < DIM; i++)
    {
        if (!sensorsInfo[i].isDead)
        {
            sensorsInfo[i].color = j++;
        }
    }
}
*/

void _tdma_tree_color_discrete()
{
    // uses a discrete mapping of colors to nodes, but
    // schedules such that children always TX before parent.
    uint8_t colors = 0;
    int8_t lvl,j;

    for (lvl = depth_of_tree; lvl >= 0; lvl--)
    {
        for (j = 0; j < DIM; j++)
        {
            if (sensorsInfo[j].level == lvl)
            {
                sensorsInfo[j].color = colors++;
            }
        }
    }
}

#ifdef TDMA_SLOT_SPREADING
void _tdma_tree_schedule_spread()
{
    sensorsInfo[0].color = 0;
    sensorsInfo[1].color = 15;
    sensorsInfo[2].color = 2;
    sensorsInfo[3].color = 4;
    sensorsInfo[4].color = 5;
    sensorsInfo[5].color = 13;
    sensorsInfo[6].color = 12;
    sensorsInfo[7].color = 7;
    sensorsInfo[8].color = 11;
    sensorsInfo[9].color = 1;
    sensorsInfo[10].color = 10;
    sensorsInfo[11].color = 8;
    sensorsInfo[12].color = 3;
    sensorsInfo[13].color = 9;
    sensorsInfo[14].color = 14;
    sensorsInfo[15].color = 0;
    sensorsInfo[16].color = 6;
}
#endif

// Master code
// I send out to everyone without waiting

int8_t _tdma_tree_make_master()
{
#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("in MTM\r\n"));
#endif

    if (tree_init_done == 0)
        return NRK_ERROR;


    my_level = 0;
#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("Will build tree\r\n"));
#endif
    _tdma_tree_send_mktree(); 
    _tdma_tree_get_children();
    
    child_info_offset = 0;
    
    sensorsInfo[tree_addr16].address = tree_addr16;
    sensorsInfo[tree_addr16].level = my_level;
    sensorsInfo[tree_addr16].isDead = 0;
    sensorsInfo[tree_addr16].parent = 0;
    //sensorsInfo[tree_addr16].totalChildren = num_children;
    sensorsInfo[tree_addr16].totalChildren = 0;

#if TDMA_TREE_DEBUG>=2
    printf( "I have %d children, cio is %d\r\n", num_children, child_info_offset);
#endif
    for (uint8_t i = 0; i < num_children; i++)
        _tdma_tree_parse(child_info, &child_info_offset, my_level+1, tree_addr16);
    
#if TDMA_TREE_DEBUG>=2
    printf("mismatch: cio: %d, cis: %d\r\n", 
           child_info_offset, child_info_size);
#endif

#ifdef DISCRETE_COLORING
    _tdma_tree_color_discrete();
#else
    color_graph();
#endif
    //discrete_color();

#if TDMA_TREE_DEBUG>=1
    nrk_kprintf(PSTR("-SENSORS-\r\n\r\n"));
    _tdma_tree_print_sensors();
#endif

    tree_rfTxInfo.pPayload = tree_tx_buf;
    _tdma_tree_send_schedule();
    _tdma_tree_master_get_schedule();
#if TDMA_TREE_DEBUG>=1
    tdma_schedule_print();
#endif

    tree_made=1;
    return NRK_OK;

}


#ifdef TDMA_TREE_STATIC
/*
** Abbreviated function to create the tree from an already
** determined source
*/

int8_t _tdma_tree_static_make_master()
{
    // fill the sensorsInfo structure with topology information
    _tdma_tree_static_create();
    // figure out the colors of the nodes
#ifdef TDMA_SLOT_SPREADING
    _tdma_tree_schedule_spread();
#else
    _tdma_tree_color_discrete();
#endif

    // show the topology/colors
#if TDMA_TREE_DEBUG>=1
    nrk_kprintf(PSTR("-SENSORS-\r\n\r\n"));
    _tdma_tree_print_sensors();
#endif

    tree_rfTxInfo.pPayload = tree_tx_buf;
    _tdma_tree_send_schedule();
    _tdma_tree_master_get_schedule();

#if TDMA_TREE_DEBUG>=1
    tdma_schedule_print();
#endif

    tree_made=1;
    return NRK_OK;
}
#endif //TDMA_TREE_STATIC

#endif // TDMA_MASTER


// Slave code
//
// wait for a packet, then call the packet's owner my parent
#ifdef TDMA_SLAVE_MODE

int8_t _tdma_tree_make_slave()
{
    uint16_t n;
    uint8_t message_type;
    uint8_t part_of_tree = 0;
    
    if (tree_init_done == 0)
        return NRK_ERROR;

    nrk_led_set(BLUE_LED);
#if TDMA_TREE_DEBUG>=2
    nrk_kprintf(PSTR("Beginning slave make tree operation\r\n"));
#endif

    while (!part_of_tree)
    {
            rf_polling_rx_on();

            while ((n=rf_polling_rx_packet()) == 0)
                ;

            //make sure a valid packet 
            if (n != 1)
            {
#if TDMA_TREE_DEBUG>=2
                nrk_kprintf(PSTR("MKTREE_SLV: Invalid recv\r\n"));
#endif
                rf_rx_off();
                continue;
            }

            rf_rx_off();

            // packet is here, check for right type
            message_type = tree_rfRxInfo.pPayload[0];

#if TDMA_TREE_DEBUG>=2
            printf("mkslvtree: Msg type %d\r\n", message_type);
#endif
            if (message_type != TREE_MAKE)
            {
#if TDMA_TREE_DEBUG>=2
                nrk_kprintf(PSTR("MKTREE_SLV: msg type wrong. \r\n"));
#endif
                continue;
            }

#ifdef TDMA_TREE_LIMIT_BY_CCA
            // If the sender's rssi is too low, we will decide 
            // not to add it as my parent
            if (tree_rfRxInfo.rssi < tdma_get_cca_thresh())
            {
#if TDMA_TREE_DEBUG>=2
                printf("RSSI low %d\r\n", tree_rfRxInfo.rssi);
#endif
                continue;
            }
#endif


#if defined(BEST_RSSI_TREEMAKE)
            /* Add to list of neighbors */
            _tdma_tree_add_neighbor(tree_rfRxInfo.srcAddr, tree_rfRxInfo.rssi);


    #if TDMA_TREE_DEBUG>=2
            printf("Hear from %d rssi %d\r\n", tree_rfRxInfo.srcAddr, tree_rfRxinfo.rssi);
    #endif
            /* Keep receiving until timeout */
            /* Pick the best RSSI out of all, decide to make them my parent */
            /* The rest will remain neighbors */
#elif defined(FIRST_HEAR_TREEMAKE)
            // Here is a very basic tree make policy -- Simply accept the first
            // node I hear from to be my parent.

            my_parent = tree_rfRxInfo.srcAddr;
            my_level = tree_rfRxInfo.pPayload[1]+1;
#else 
    #error No method of tree make defined.  Need BEST_RSSI_TREEMAKE or FIRST_HEAR_TREEMAKE defined.
#endif
        
#if TDMA_TREE_DEBUG>=1
        printf("My parent/level: %d/%d\r\n", my_parent, my_level);
#endif


        _tdma_tree_send_mktree();
        nrk_led_set(GREEN_LED);
        _tdma_tree_get_children();
        _tdma_tree_notify_parent();
        
        nrk_led_set(RED_LED);
        _tdma_tree_obtain_schedule();

        part_of_tree = 1;

#if TDMA_TREE_DEBUG>=1
        nrk_kprintf(PSTR("Have Sched\r\n"));
#endif
    }

#if TDMA_TREE_DEBUG>=1
    tdma_schedule_print();
#endif

    tree_made=1;
    return NRK_OK;
}

#ifdef TDMA_TREE_STATIC
/*
** Abbreviated function to create tree from already
** determined source, and obtain the node's schedule
*/
int8_t _tdma_tree_static_make_slave()
{
    _tdma_tree_static_create();
    _tdma_tree_obtain_schedule();

#if TDMA_TREE_DEBUG>=1
    tdma_schedule_print();
#endif
    tree_made = 1;
    return NRK_OK;
}

#endif //TDMA_TREE_STATIC


#endif //TDMA_SLAVE_MODE


int8_t tdma_tree_create(uint8_t chan)
{
    int8_t ret;

    // go into neighbor discovery mode, where everyone
    // sends and receives packets
    _tdma_tree_init(chan);

    nrk_led_clr(BLUE_LED);
    nrk_led_clr(GREEN_LED);
    nrk_led_clr(RED_LED);

    nrk_led_set(ORANGE_LED);
    _tdma_tree_discover_neighbors();

    if (tdma_node_mode != TDMA_MASTER &&
            tdma_node_mode != TDMA_SLAVE)
        ret = NRK_ERROR;

    if (tdma_node_mode == TDMA_MASTER)
    {
        // wait a bit, to make sure everyone else is in tree creation mode
        nrk_time_get(&cur_time);
        cur_time.secs += 5;
        nrk_wait_until(cur_time);

#ifdef TDMA_MASTER_MODE
    #ifdef TDMA_TREE_STATIC
        ret = _tdma_tree_static_make_master();
    #else
        ret = _tdma_tree_make_master();
    #endif
#endif

    }
    else if (tdma_node_mode == TDMA_SLAVE)
    {
#ifdef TDMA_SLAVE_MODE
    #ifdef TDMA_TREE_STATIC
        ret = _tdma_tree_static_make_slave();
    #else
        ret = _tdma_tree_make_slave();
    #endif
#endif
    }

    nrk_led_clr(ORANGE_LED);
    nrk_led_clr(BLUE_LED);
    nrk_led_clr(GREEN_LED);
    nrk_led_clr(RED_LED);
    
    tdma_init_done = 0;
    return ret;
}


/*
 * Graph coloring algorithm as used in TDMA-ASAP simulation
 */

#if 0
void color_graph()
{
	uint8_t count = 0;
	uint8_t nodesAlive = 0;
	uint8_t color = 0;
	uint8_t xyid;
    uint8_t x;
	int8_t current_level;	
	//uint8_t test = 0;
	//GCMCounter = 0;
	//GCMCounterSteal_C = 0;

	/*strcpy(graphColors[0], "red");
	strcpy(graphColors[1], "blue");
	strcpy(graphColors[2], "green");
	strcpy(graphColors[3], "yellow");
	strcpy(graphColors[4], "gray");
	strcpy(graphColors[5], "orange");
	strcpy(graphColors[6], "purple");
	strcpy(graphColors[7], "brown");
	strcpy(graphColors[8], "pink");
	strcpy(graphColors[9], "navy");
	strcpy(graphColors[10], "darkgreen");
	strcpy(graphColors[11], "gold");
	strcpy(graphColors[12], "lightgray");
	strcpy(graphColors[13], "darkorange");
	strcpy(graphColors[14], "magenta");
	strcpy(graphColors[15], "beige");
	strcpy(graphColors[16], "crimson");
	strcpy(graphColors[17], "skyblue");
	strcpy(graphColors[18], "forestgreen");
	strcpy(graphColors[19], "lightyellow");
	strcpy(graphColors[20], "slategray");
	strcpy(graphColors[21], "orchid");
	strcpy(graphColors[22], "peru");
	strcpy(graphColors[23], "tomato");
	strcpy(graphColors[24], "midnightblue");
	strcpy(graphColors[25], "limegreen");
	strcpy(graphColors[26], "yellowgreen");
	strcpy(graphColors[27], "violet");
	strcpy(graphColors[28], "tan");
	strcpy(graphColors[29], "maroon");
    */
	
for(uint8_t j = 0; j < DIM; j++)
{
	if (sensorsInfo[j].isDead)
	{
#if TDMA_TREE_DEBUG>=2
		printf("Node: %d is Dead\r\n", j);
#endif
	}
}

////Test - Pruint8_t Tree
//for(uint8_t i = 0; i <= depth_of_tree[1]; i++)
//{
//	printf("Level: %d, ", i);
//	for(uint8_t j = 0; j < DIM; j++)
//	{
//		if (i == sensorsInfo[j].level)
//			printf("Node: %d , parent: %d ", sensorsInfo[j].xy, sensorsInfo[j].parent);
//	}
//	printf("\n");
//}
	
	//Fill uncolored_node_list
	for(int8_t j = depth_of_tree; j >= 0 ; j--)
	{
		for(int8_t i = 0; i < DIM; i++)	
		{
			if(sensorsInfo[i].level == j && !sensorsInfo[i].isDead)
			{
				uncolored_node_list[0][count] = i;
				uncolored_node_list[1][count] = j;
				count++;	
			}
		}
	}
nodesAlive = count;
	
//printf("\nVerify uncolored node list\n");
//printf("Count: %d\n", count);
//for(uint8_t i = 0; i < count; i++)
//{
//	printf("uncolored node list: %d %d\n", uncolored_node_list[0][i], uncolored_node_list[1][i]);	
//}


	
//	printf("Verifying sort\n");		
//	for (uint8_t i = 0; i < DIM; i++)
//	{
//		if (i > 0)
//		{
//			if(uncolored_node_list[1][i] > uncolored_node_list[1][i -1])
//			{
//				printf("\n\nWE HAVE A PROBLEM\n\n");
//			}
//		}
//	}
//	printf("Nodes sorted okay\n");
	
	

	//Determine how many slots we need for this tree				
	//Initialize each sensor's color and the ableToSteal flags
	for(uint8_t i = 0; i < DIM; i++)
	{
		sensorsInfo[i].color = -1;

//Robert Cleric commented out 5/10/07		
//		if (sensorsInfo[i].level != -1)
//		{
/*
			if (steal_C)
			{
				for (uint8_t j = 0; j < MAX_SIBLINGS; j++)
				{
					sensorsInfo[i].ableToSteal[j] = -1;
					sensorsInfo[i].ableToSteal[j] = -1;
				}
			}
*/
//		}
	}
//**printf("\nRegular coloring, used for steal_C, steal_G, ZMAC\n");
	//Pruint8_t Number of nodes per level to get an idea of structure
//**	printf("Number of nodes on each level\n");
//**	for(uint8_t j = 0; j <= depth_of_tree[1]; j++)
//**	{
//**		for(uint8_t i = 0; i < DIM; i++)
//**		{
//**			if (j == sensorsInfo[i].level)
//**				test++;
//**		}
//**		printf("Level:, %d, # Nodes:, %d\n" , j, test);
//**		test = 0;
//**	}
//**	printf("\n");

	//Start at the bottom and work your way up
	for(int8_t j = depth_of_tree; j >= 0 ; j--)
	{
//printf("J: %d  nodesAlive:%d\n", j, nodesAlive);
		uint8_t first_node_for_level = 1;
		uint8_t more_to_color_on_this_level = 1;
		color = 0;	//This is the first color
		count = 0;
		while(more_to_color_on_this_level)
		{	
			current_level = -1;
			for(uint8_t i = 0; i < nodesAlive; i++)
			{
				//if(sensorsInfo[i].level != -1)
				//{
//	printf("uncolored_node_list[1][%d]: %d\n", i, uncolored_node_list[1][i]);
					if (uncolored_node_list[1][i] == j)
					{
	
//	printf("Level: %d Node: %d made it in loop\n", j, i);
						current_level = uncolored_node_list[1][i];
						uncolored_node_list[1][i] = -1;
						xyid = uncolored_node_list[0][i];
					
						//This node is on the current level
						if(first_node_for_level)
						{
							//This is the first node so just assign it the first color
                            
                            // JOHN:  ^^^ WHAAAAAT?  That's not a graph coloring algorithm!!!
                            // Begin JCY

                            do
                            {
                                    child_has_color = 0;
                                    for (x = 0; x < sensorsInfo[xyid].totalChildren; x++)
                                    {
#if TDMA_TREE_DEBUG>=3
                                        printf("Testing for matches: %d\r\n", x);
                                        printf("color: %d, child %d \r\n", 

                                        color, sensorsInfo[ sensorsInfo[xyid].children[x] ].color);
#endif
                                        if (color == sensorsInfo[ sensorsInfo[xyid].children[x] ].color)
                                        {
#if TDMA_TREE_DEBUG>=3
                                            nrk_kprintf(PSTR("Color matches\r\n"));
#endif
                                            child_has_color = 1;
                                        }
                                    }
                                    if(child_has_color)
                                        color++;
                            } while (child_has_color);
                            //if (sensorsInfo[xyid].parent != 0 && 
                            //    sensorsInfo[ sensorsInfo[xyid].parent ].color == color)


                            // End JCY
#if TDMA_TREE_DEBUG>=3
                            printf("SETTING COLOR OF NODE %d to %d\r\n", xyid, color);
#endif
							sensorsInfo[xyid].color = color;
							//Add this node id to the colored node list
							colored_node_list[count] = xyid;
							count++;
							first_node_for_level = 0;
						
							//GCMCounter++;
						}
						else
						{
							//This is not the first node 
							//We need to determine which other nodes can go 
							//into this color
							//Loop through each node that is colored to see if
							//this node can be the same color
							uint8_t okay_to_add_node = 1;
							for(uint8_t k = 0; k < count; k++)
							{
								//Robert Cleric modified 8/6/06
								if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list[k]].parent
                                  || Neighbors[xyid][sensorsInfo[colored_node_list[k]].parent] 
                                  || Neighbors[sensorsInfo[xyid].parent][colored_node_list[k]])	
								{
									//It is not okay to add this node
									okay_to_add_node = 0;																				
								}
								//GCMCounter++;
							}

                            // BEGIN JCY
                            // Also, make sure none of children are using the same color, dammit!
                            for (x = 0; x < sensorsInfo[xyid].totalChildren; x++)
                            {
#if TDMA_TREE_DEBUG>=3
                                printf("Testing colors %d %d\r\n", color,
                                    sensorsInfo[ sensorsInfo[xyid].children[x] ].color);
#endif
                                if (color == sensorsInfo[ sensorsInfo[xyid].children[x] ].color)
                                {
                                    okay_to_add_node = 0;
                                }

                            }
                            // and parent ...
                            if (color == sensorsInfo[ sensorsInfo[xyid].parent ].color)
                            {
                                okay_to_add_node = 0;
                            }

                            // END JCY
							if (okay_to_add_node)
							{
								//It is okay to color this node the current color
#if TDMA_TREE_DEBUG>=3
            printf("SETTING COLOR OF NODE %d to %d\r\n", xyid, color);
#endif
								sensorsInfo[xyid].color = color;
								//Add this node id to the colored node list
								colored_node_list[count] = xyid;
								count++;
							
								//Now that the node has been colored, check to see if it is able to steal any slots
								/*if (steal_C)
								{
									//Make sure that this is not the first color
									//If it is, I am the in the first slot and have nothing to steal 
									if(sensorsInfo[sensorsInfo[xyid].parent].totalChildren > 1)
									{
										for(uint8_t m = color - 1; m >= 0; m--)
										{
											//Check to see that if my sibling did not exist
											//would I then be able to send?
										
											bool okayToProceed = false;
											//First I need to know if I have a sibling that is color m
											for(uint8_t p = 0; p < sensorsInfo[sensorsInfo[xyid].parent].totalChildren; p++)
											{
												if(sensorsInfo[sensorsInfo[sensorsInfo[xyid].parent].children[p]].color == m)
												{
													//I also need to know if that sibling is within range
													if(Neighbors[xyid][sensorsInfo[sensorsInfo[xyid].parent].children[p]])
														okayToProceed = true;
												}
											}
										
											if(okayToProceed)
											{		
												//If I am going to try to steal slot i, I must first make sure
												//that I am able to steal slot i + 1 or I cannot gaurantee that 
												//I will be able to steal slot i.
												if(m == (color-1))
												{
													//This can be implemented much more efficiently
													//Loop through each node on this level
													//and check to see if the current node can be scheduled with m
													bool canSteal = true;
													for(uint8_t n = 0; n < DIM; n++)
													{
														if(!sensorsInfo[n].isDead && sensorsInfo[n].level == j && sensorsInfo[n].color == m && (sensorsInfo[n].parent != sensorsInfo[xyid].parent))
														{
															//This node is on the right level and the color we are looking for
															if(Neighbors[xyid][sensorsInfo[n].parent])	
															{
																//It is not okay to steal this slot
																canSteal = false;																				
															}				
														
														}
														//GCMCounterSteal_C++;
													}
													if(canSteal)
													{
														sensorsInfo[xyid].ableToSteal[m] = 1;  //I can steal
													}
													else
														sensorsInfo[xyid].ableToSteal[m] = 0;	//I can't steal
												}
												else if(sensorsInfo[xyid].ableToSteal[m+1] == 1) //This condition limits the nodes that can be stolen but I do not see a way around it
												{
													//This can be implemented much more efficiently
													//Loop through each node on this level
													//and check to see if the current node can be scheduled with m
													bool canSteal = true;
													for(uint8_t n = 0; n < DIM; n++)
													{
														if(!sensorsInfo[n].isDead && sensorsInfo[n].level == j && sensorsInfo[n].color == m && (sensorsInfo[n].parent != sensorsInfo[xyid].parent))
														{
															//This node is on the right level and the color we are looking for
															if(Neighbors[xyid][sensorsInfo[n].parent])	
															{
																//It is not okay to steal this slot
																canSteal = false;																				
															}				
															
														}
														//GCMCounterSteal_C++;
													}
													if(canSteal)
													{
														sensorsInfo[xyid].ableToSteal[m] = 1;
													}
													else
														sensorsInfo[xyid].ableToSteal[m] = 0;
												}
												else
												{
													//GCMCounterSteal_C++;
													break;
												}
											}
										}
									}
								}*/
							}
							else
							{
								//Since this node could not be colored, it must be added back
								//into the uncolored node list
								uncolored_node_list[1][i] = current_level;
							}
						}		
					}
					//else
						//GCMCounter++;
				//}		
			}
			color++; //Increment to next color to use
			count = 0; //reset count for new color
			if(current_level == -1)
			{
				more_to_color_on_this_level = 0;
				colors_used[j] = color -1;
				colors_used_level[j][color-2] = color -2;
			}
		}
	}
}
#endif
#if 0
void color_graph2()
{
	uint8_t count = 0;
	uint8_t color = 0;
	uint8_t xyid;
	uint8_t current_level;	
    uint8_t x;
	
	uint8_t total_colors_used2 = 0;
	
////Test - Pruint8_t Tree
//for(uint8_t i = 0; i <= depth_of_tree[1]; i++)
//{
//	printf("Level: %d, ", i);
//	for(uint8_t j = 0; j < DIM; j++)
//	{
//		if (i == sensorsInfo[j].level)
//			printf("Node: %d , parent: %d ", sensorsInfo[j].xy, sensorsInfo[j].parent);
//	}
//	printf("\n");
//}
	
	//Fill uncolored_node_list
	for(uint8_t j = depth_of_tree; j >= 0 ; j--)
	{
		for(uint8_t i = 0; i < DIM; i++)	
		{
			if(sensorsInfo[i].level == j && !sensorsInfo[i].isDead)
			{
				uncolored_node_list[0][count] = i;
				uncolored_node_list[1][count] = j;
				count++;
			}
		}
	}

uint8_t nodesAlive = count;
//	printf("Verifying sort\n");		
//	for (uint8_t i = 0; i < DIM; i++)
//	{
//		if (i > 0)
//		{
//			if(uncolored_node_list[1][i] > uncolored_node_list[1][i -1])
//			{
//				printf("\n\nWE HAVE A PROBLEM\n\n");
//			}
//		}
//	}


	//Determine how many slots we need for this tree				
	//Initialize each sensor's color and ableToSteal flags
	for(uint8_t i = 0; i < DIM; i++)
	{
		sensorsInfo[i].color2 = -1;
		sensorsInfo[xyid].schedWithDiffLevel = -1;
//		if (steal_C)
//		{
//			for (uint8_t j = 0; j < MAX_SIBLINGS; j++)
//			{
//				sensorsInfo[i].ableToSteal[j] = false;
//			}
//		}
	}
			
//	//Pruint8_t Number of nodes per level to get an idea of structure
//	printf("\nNumber of nodes on each level\n");
//	for(uint8_t j = 0; j <= depth_of_tree[1]; j++)
//	{
//		for(uint8_t i = 0; i < DIM; i++)
//		{
//			if (j == sensorsInfo[i].level)
//				test++;
//		}
//		printf("Level: %d, # Nodes: %d\n" , j, test);
//		test = 0;
//	}
//	printf("\n");

	//Start at the bottom and work your way up
	for(uint8_t j = depth_of_tree; j >= 0 ; j--)
	{
		uint8_t first_node_for_level = 1;
		uint8_t more_to_color_on_this_level = 1;
		color = 0;	//This is the first color
		count = 0;
		while(more_to_color_on_this_level)
		{	
			current_level = -1;
			for(uint8_t i = 0; i < nodesAlive; i++)
			{	
				//if (sensorsInfo[i].level != -1)
				//{
					if (uncolored_node_list[1][i] == j)
					{
						current_level = uncolored_node_list[1][i];
						uncolored_node_list[1][i] = -1;
						xyid = uncolored_node_list[0][i];
					
						//This node is on the current level
						if(first_node_for_level)
						{
							//This is the first node so just assign it the first color
							sensorsInfo[xyid].color2 = color;
							//Add this node id to the colored node list
							colored_node_list2[count] = xyid;
							count++;
							first_node_for_level = 0;
						}
						else
						{
							//This is not the first node 
							//We need to determine which other nodes can go 
							//into this color
							//Loop through each node that is colored to see if
							//this node can be the same color
							uint8_t okay_to_add_node = 1;
							for(uint8_t k = 0; k < count; k++)
							{
								//Robert Cleric modified 8/6/06
								if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list2[k]].parent || Neighbors[xyid][sensorsInfo[colored_node_list2[k]].parent] || Neighbors[sensorsInfo[xyid].parent][colored_node_list2[k]] )	
								{
									//It is not okay to add this node
									okay_to_add_node = 0;																				
								}
							}
							if (okay_to_add_node)
							{
								//It is okay to color this node the current color
								sensorsInfo[xyid].color2 = color;
								//Add this node id to the colored node list
								colored_node_list2[count] = xyid;
								count++;								
							}
							else
							{
								//Since this node could not be colored, it must be added back
								//into the uncolored node list
								uncolored_node_list[1][i] = current_level;
							}
						}		
					}
				//}//new
			}
			color++; //Increment to next color to use
			count = 0; //reset count for new color
			if(current_level == -1)
			{
				more_to_color_on_this_level = 0;
				colors_used2[j] = color -1;				
				colors_used_level2[j][color-2] = color -2;
			}
		}
		
		//Optimization 2
		//Optimization 2 - after this level has been colored go up
		//some levels and see if there are any nodes with no children
		//that can also be scheduled with these nodes.	
			
		//Loop through each node on the levels i - 1, i - 2, i -3 
		uint8_t levelsToGoUp = 0;
		uint8_t count2 = 0;
		
		if (j == 0)
			levelsToGoUp = 0;
		else if (j==1)
			levelsToGoUp = 1;
		else if (j==2)
			levelsToGoUp = 2;
		else
			levelsToGoUp = 3;

		//for(uint8_t m = 0; m < levelsToGoUp; m++)
		for(uint8_t k = 0; k < (colors_used2[j]); k++)
		{
			current_level = -1;
			count2 = 0;
			//Make a list of colored nodes
			for(uint8_t p = 0; p < DIM; p++)
			{
				if(sensorsInfo[p].color2 == k && sensorsInfo[p].level == j && !sensorsInfo[p].isDead)
				{
					colored_node_list[count2] = p;
					count2++;				
				}
			}
			//for(uint8_t k = 0; k < (colors_used2[j]); k++)
			for(uint8_t m = 0; m < levelsToGoUp; m++)
			{				
				for(uint8_t i = 0; i < DIM; i++)
				{
					if (sensorsInfo[i].level != -1 && !sensorsInfo[i].isDead)
					{
						if (uncolored_node_list[1][i] == (j-(m+1)))
						{
							xyid = uncolored_node_list[0][i];
							//Check to see if this node has no children
							if(sensorsInfo[xyid].totalChildren == 0)
							{						
								current_level = uncolored_node_list[1][i];
								uncolored_node_list[1][i] = -1;						
								//Determine if other nodes can go 
								//into this color
								//Loop through each node on level i that is colored to see if
								//this node can be one of those colors
						
								uint8_t okay_to_add_node = 1;
								uint8_t tempNode;
								//Check each node
								for (uint8_t n = 0; n < count2; n++)
								{
									//See if the node is the proper color
									//and if it is on the proper level								
									//Robert Cleric modified 8/6/06
									//Check to see if the current node can be scheduled with the node that is already colored
									if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list[n]].parent || Neighbors[xyid][sensorsInfo[colored_node_list[n]].parent] || Neighbors[sensorsInfo[xyid].parent][colored_node_list[n]] )										
									{
										//It is not okay to add this node
										okay_to_add_node = 0;																				
									}	
									tempNode = colored_node_list[n];				
								}
								if (okay_to_add_node)
								{
									//It is okay to color this node the current color
									sensorsInfo[xyid].color2 = sensorsInfo[tempNode].color2;
									sensorsInfo[xyid].schedWithDiffLevel = j;
									colored_node_list[count2] = xyid;
									count2++;
								}
								if(!okay_to_add_node)
								{							
									//Since this node could not be colored, it must be added back
									//into the uncolored node list
									uncolored_node_list[1][i] = current_level;
								}
							}
						}
					}//New
				}
			}
		}
	}

}
//*****************************************************************************************
/* color_graph4, uses optimization */
//*****************************************************************************************

void color_graph4()
{
	uint8_t count = 0;
	uint8_t color = 0;
	uint8_t xyid;
	uint8_t current_level;	
	
	uint8_t total_colors_used4 = 0;
    uint8_t x;
	
	
	//Fill uncolored_node_list
	for(uint8_t j = depth_of_tree; j >= 0 ; j--)
	{
		for(uint8_t i = 0; i < DIM; i++)	
		{
			if(sensorsInfo[i].level == j && !sensorsInfo[i].isDead)
			{
				uncolored_node_list[0][count] = i;
				uncolored_node_list[1][count] = j;
				count++;
			}
		}
	}
uint8_t nodesAlive = count;
	//Determine how many slots we need for this tree				
	//Initialize each sensor's color and ableToSteal flags
	for(uint8_t i = 0; i < DIM; i++)
	{
		sensorsInfo[i].color4 = -1;
		sensorsInfo[xyid].schedWithDiffLevel4 = -1;
//		if (steal_C)
//		{
//			for (uint8_t j = 0; j < MAX_SIBLINGS; j++)
//			{
//				sensorsInfo[i].ableToSteal[j] = false;
//			}
//		}
	}
			

	//Start at the bottom and work your way up
	for(uint8_t j = depth_of_tree; j >= 0 ; j--)
	{
		uint8_t first_node_for_level = 1;
		uint8_t more_to_color_on_this_level = 1;
		color = 0;	//This is the first color
		count = 0;
		while(more_to_color_on_this_level)
		{	
			current_level = -1;
			for(uint8_t i = 0; i < nodesAlive; i++)
			{	
				//if(sensorsInfo[i].level != -1)
				//{
					if (uncolored_node_list[1][i] == j)
					{
						current_level = uncolored_node_list[1][i];
						uncolored_node_list[1][i] = -1;
						xyid = uncolored_node_list[0][i];
					
						//This node is on the current level
						if(first_node_for_level)
						{
							//This is the first node so just assign it the first color


                            // JOHN:  ^^^ WHAAAAAT?  That's not a graph coloring algorithm!!!
                            // Begin JCY

                            do
                            {
                                    child_has_color = 0;
                                    for (x = 0; x < sensorsInfo[xyid].totalChildren; x++)
                                    {
                                        printf("Testing for matches: %d\r\n", x);
                                        printf("color: %d, child %d \r\n", 
                                        color, sensorsInfo[ sensorsInfo[xyid].children[x] ].color);
                                        if (color == sensorsInfo[ sensorsInfo[xyid].children[x] ].color)
                                        {
                                            nrk_kprintf(PSTR("Color matches\r\n"));
                                            child_has_color = 1;
                                        }
                                    }
                                    if(child_has_color)
                                        color++;
                            } while (child_has_color);
                            //if (sensorsInfo[xyid].parent != 0 && 
                            //    sensorsInfo[ sensorsInfo[xyid].parent ].color == color)


                            // End JCY

							sensorsInfo[xyid].color4 = color;
							//Add this node id to the colored node list
							colored_node_list4[count] = xyid;
							count++;
							first_node_for_level = 0;
						}
						else
						{
							//This is not the first node 
							//We need to determine which other nodes can go 
							//into this color
							//Loop through each node that is colored to see if
							//this node can be the same color
							uint8_t okay_to_add_node = 1;
							for(uint8_t k = 0; k < count; k++)
							{	
								//Robert Cleric modified 8/6/06
								if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list4[k]].parent || 
                                        Neighbors[xyid][sensorsInfo[colored_node_list4[k]].parent] || 
                                        Neighbors[sensorsInfo[xyid].parent][colored_node_list4[k]] )	
								{
									//It is not okay to add this node
									okay_to_add_node = 0;																				
								}
							}


                            // BEGIN JCY
                            // Also, make sure none of children are using the same color, dammit!
                            for (x = 0; x < sensorsInfo[xyid].totalChildren; x++)
                            {
                                printf("Testing colors %d %d\r\n", color,
                                    sensorsInfo[ sensorsInfo[xyid].children[x] ].color);
                                if (color == sensorsInfo[ sensorsInfo[xyid].children[x] ].color)
                                {
                                    okay_to_add_node = 0;
                                }

                            }
                            // and parent ...
                            if (color == sensorsInfo[ sensorsInfo[xyid].parent ].color)
                            {
                                okay_to_add_node = 0;
                            }

                            // END JCY


                            
							if (okay_to_add_node)
							{
								//It is okay to color this node the current color
								sensorsInfo[xyid].color4 = color;
								//Add this node id to the colored node list
								colored_node_list4[count] = xyid;
								count++;								
							}
							else
							{
								//Since this node could not be colored, it must be added back
								//into the uncolored node list
								uncolored_node_list[1][i] = current_level;
							}
						}		
					}
					//else
				//}//NEW
			}
			color++; //Increment to next color to use
			count = 0; //reset count for new color
			if(current_level == -1)
			{
				more_to_color_on_this_level = 0;
				colors_used4[j] = color -1;				
				colors_used_level4[j][color-2] = color -2;
			}
		}
		
		//Optimization 4
		uint8_t count2 = 0;
		//Scheudle all leaf nodes possible with the lowest level
		if(j==depth_of_tree)
		{
			for(uint8_t k = 0; k < colors_used4[j]; k++)
			{
				current_level = -1;
				count2 = 0;
				//Make a list of colored nodes
				for(uint8_t p = 0; p < DIM; p++)
				{
					if(sensorsInfo[p].color2 == k && sensorsInfo[p].level == j && !sensorsInfo[p].isDead)
					{
						colored_node_list[count2] = p;
						count2++;				
					}
				}
				
				
				
				for(uint8_t i = 0; i < DIM; i++)
				{
					if (sensorsInfo[i].level != -1 && !sensorsInfo[i].isDead)
					{
						//if (uncolored_node_list[1][i] == (j-(m+1)))
						if((sensorsInfo[i].totalChildren == 0) && (uncolored_node_list[1][i] != -1))
						{
							xyid = uncolored_node_list[0][i];
							
							current_level = uncolored_node_list[1][i];
							uncolored_node_list[1][i] = -1;						
						
							//Determine if this node will fit in with this color
							//Loop through each node on level i that is colored to see if
							//this node can be one of those colors
						
							uint8_t okay_to_add_node = 1;
							uint8_t tempNode;
							//Check each node
							for (uint8_t n = 0; n < count2; n++)
							{
								//See if the node is the proper color
								//and if it is on the proper level								
								//Robert Cleric modified 8/6/06
								//Check to see if the current node can be scheduled with the node that is already colored
								if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list4[n]].parent || Neighbors[xyid][sensorsInfo[colored_node_list4[n]].parent] || Neighbors[sensorsInfo[xyid].parent][colored_node_list4[n]] )										
								{
									//It is not okay to add this node
									okay_to_add_node = 0;																				
								}	
								tempNode = colored_node_list[n];				
							}
							if (okay_to_add_node)
							{
								//It is okay to color this node the current color
								sensorsInfo[xyid].color4 = sensorsInfo[tempNode].color4;
								sensorsInfo[xyid].schedWithDiffLevel4 = j;
								colored_node_list4[count2] = xyid;
								count2++;
							}
							if(!okay_to_add_node)
							{							
								//Since this node could not be colored, it must be added back
								//into the uncolored node list
								uncolored_node_list[1][i] = current_level;
							}	
						}
//						else
//							GCMCounter10++;
					}//NEW
				}
			}
		
		
		
printf("\nChecking to see how many total leaf nodes were scheduled\n");
uint8_t leafCount = 0;
uint8_t coloredLeafCount = 0;
for (uint8_t i = 0; i < DIM; i++)
{
	if (sensorsInfo[i].level != -1 && !sensorsInfo[i].isDead)
	{
		if(sensorsInfo[i].totalChildren == 0)
		{
			leafCount++;
			if(sensorsInfo[i].color4 != -1)
				coloredLeafCount++;
		}
	}
	
}
printf("Total Leafs:, %d\n", leafCount);
printf("Colored Leafs:, %d\n", coloredLeafCount);
printf("\n");
		}
		
		
		
/*		//Optimization 2 - after this level has been colored go up
		//some levels and see if there are any nodes with no children
		//that can also be scheduled with these nodes.	
		
		//Loop through each node on the levels i - 1, i - 2, i -3 
		uint8_t levelsToGoUp = 0;
		uint8_t count2 = 0;
		
		if (j == 0)
			levelsToGoUp = 0;
		else if (j==1)
			levelsToGoUp = 1;
		else if (j==2)
			levelsToGoUp = 2;
		else
			levelsToGoUp = 3;

		//for(uint8_t m = 0; m < levelsToGoUp; m++)
		for(uint8_t k = 0; k < (colors_used2[j]); k++)
		{
			current_level = -1;
			count2 = 0;
			//Make a list of colored nodes
			for(uint8_t p = 0; p < DIM; p++)
			{
				if(sensorsInfo[p].color2 == k && sensorsInfo[p].level == j)
				{
					colored_node_list[count2] = p;
					count2++;				
				}
			}
			for(uint8_t m = 0; m < levelsToGoUp; m++)
			{				
				for(uint8_t i = 0; i < DIM; i++)
				{
					if (uncolored_node_list[1][i] == (j-(m+1)))
					{
						xyid = uncolored_node_list[0][i];
						//Check to see if this node has no children
						if(sensorsInfo[xyid].totalChildren == 0)
						{						
							current_level = uncolored_node_list[1][i];
							uncolored_node_list[1][i] = -1;						
							//Determine if other nodes can go 
							//into this color
							//Loop through each node on level i that is colored to see if
							//this node can be one of those colors
						
							uint8_t okay_to_add_node = 1;
							uint8_t tempNode;
							//Check each node
							for (uint8_t n = 0; n < count2; n++)
							{
								//See if the node is the proper color
								//and if it is on the proper level								
								//Robert Cleric modified 8/6/06
								//Check to see if the current node can be scheduled with the node that is already colored
								if(sensorsInfo[xyid].parent == sensorsInfo[colored_node_list[n]].parent || Neighbors[xyid][sensorsInfo[colored_node_list[n]].parent] || Neighbors[sensorsInfo[xyid].parent][colored_node_list[n]] )										
								{
									//It is not okay to add this node
									okay_to_add_node = 0;																				
								}	
								tempNode = colored_node_list[n];				
								GCMCounter3++;
							}
							if (okay_to_add_node)
							{
								//It is okay to color this node the current color
								sensorsInfo[xyid].color2 = sensorsInfo[tempNode].color2;
								sensorsInfo[xyid].schedWithDiffLevel = j;
								colored_node_list[count2] = xyid;
								count2++;
							}
							if(!okay_to_add_node)
							{							
								//Since this node could not be colored, it must be added back
								//into the uncolored node list
								uncolored_node_list[1][i] = current_level;
							}
						}
						else
							GCMCounter3++;
					}
					else
						GCMCounter3++;
				}
			}
		}*/
	}
		

//	printf("Done coloring graph OPT2\n");		

//**	//Pruint8_t out the number of colors used on each level
//**	printf("\nNumber of colors used on each level for OPT4\n");
//**	for( uint8_t i = 0; i <= depth_of_tree[1]; i++)
//**	{
//**		printf("Level:, %d, colors:, %d\n", i, colors_used4[i]);
//**		total_colors_used4 = total_colors_used4 + colors_used4[i];
//**	}
//**	printf("\nTotal colors/slots used:, %d\n", total_colors_used4);
		
//printf("Verifying coloring of OPT2\n");
//uint8_t found = 0;
//for(uint8_t i = 0; i < DIM; i++)
//{
//	found = 0;
//	for(uint8_t j = 0; j < colors_used[sensorsInfo[i].level]; j++)
//	{
//		if (sensorsInfo[i].color2 == j)
//			found = 1;
//	}
//	if(!found)
//		printf("\n\n\nWE HAVE A PROBLEM\n\n\n");
//}

//uint8_t test = 0;
//for (uint8_t i = 0; i < DIM; i++)
//{
//	if(sensorsInfo[i].color2 == -1)
//		test++;
//}
//
//printf("Uncolored Nodes: %d\n", test);
	
}

#endif
