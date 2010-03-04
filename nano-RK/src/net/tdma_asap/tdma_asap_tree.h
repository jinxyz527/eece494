// tdma_asap_tree.h
// Contains graph coloring and automatic schedule establishment
// using tree building.

/*
 * Authors
 * John Yackovich
 *
 */
#ifndef TDMA_TREE_H
#define TDMA_TREE_H

#include <tdma_asap_scheduler.h>


// TEST VARIABLES
#define COLOR_SLOT_STRIDE 3

// whether to add stealable slots or not to schedule
#define SLOT_STEALING_SCHED

// define whether to use discrete scheduling or regular coloring
// let's not test with this ;; it's not interesting in the CIC
#define DISCRETE_COLORING

// use a static schedule as computed by a slot spreading algorithm
#define TDMA_SLOT_SPREADING

// # of nodes max using TDMA
#define DIM 17

uint16_t tree_addr16;
typedef int8_t tdma_color_t;

// Master: to color graph must know depth of tree
uint8_t depth_of_tree;

typedef enum{
    PKT_TYPE=0,
    DEST_ADDR=1,
    // dest addr is 2 bytes
    MY_LEVEL=3,
    SIZE_OF_CHILDREN=4,
    MY_ADDR=5,
    // ...
    MY_NO_CHILDREN=7,
    CHILDREN_INFO=8,
} tdma_notify_pkt_t;

typedef struct
{
  uint8_t level;
  uint8_t isDead;
  tdma_color_t color;
#if 0
  tdma_color_t color2;
  tdma_color_t color4;
  uint8_t schedWithDiffLevel;
  uint8_t schedWithDiffLevel4;
#endif
  //uint8_t ableToSteal[MAX_SIBLINGS];
  node_addr_t address;
  node_addr_t parent;
  uint8_t totalChildren;
  node_addr_t children[TREE_MAX_CHILDREN];
} sensorInfo;

#ifdef TDMA_MASTER_MODE

//uint16_t uncolored_node_list[2][DIM];  //List of nodes that have not been colored yet.
//uint16_t colored_node_list[DIM];  //List of nodes that have  been colored
uint8_t Neighbors[DIM][DIM]; //To keep track of neighbors for graph coloring
//uint8_t colors_used[DIM];	//To record the num of colors used per level
//uint8_t colors_used_level[DIM][DIM];

    #if 0
// for OPT2
uint16_t uncolored_node_list2[2][DIM];  //List of nodes that have not been colored yet.
uint16_t colored_node_list2[DIM];  //List of nodes that have  been colored
uint8_t colors_used2[DIM];	//To record the num of colors used per level
uint8_t colors_used_level2[DIM][DIM];

// for OPT4
uint16_t uncolored_node_list4[2][DIM];  //List of nodes that have not been colored yet.
uint16_t colored_node_list4[DIM];  //List of nodes that have  been colored
uint8_t colors_used4[DIM];	//To record the num of colors used per level
uint8_t colors_used_level4[DIM][DIM];
    #endif
#endif

sensorInfo sensorsInfo[DIM];

//uint8_t graphColors[20][5];
//char graphColors[20][30];

// tree creation buffers
#define TREE_BUF_SIZE 115

RF_TX_INFO tree_rfTxInfo;
RF_RX_INFO tree_rfRxInfo;

// Tree creation buffers
uint8_t tree_tx_buf[TREE_BUF_SIZE];
uint8_t tree_rx_buf[TREE_BUF_SIZE];


// size of the subtree info array, in bytes
#define SUBTREE_INFO_SIZE 115
#define CHILD_INFO_SIZE 115

// how many times to push packets repeatedly to hopefully
// ensure delivery
#define CSMA_SEND_REPEAT            10
// Maximum backoff
#define TREE_BACKOFF_TIME_MS        100

// minimum backoff in ticks
#define TREE_BACKOFF_TIME_TICKS       650

// Time to wait for children to respond to me total
// while making the tree
// levels further away from the root will wait a shorter amount
#define TREE_WAIT_TIME_MS      10000

// Amount of time spent discovering neighbors.
#define TREE_NBR_DISCOVERY_MS   5000

// Predefined TDMA control pkts
#define TREE_MAKE   0xFF
#define TREE_NOTIFY 0xFE
#define TREE_SCHEDULE 0xFD
#define TREE_NBR    0xFC

// my parent
node_addr_t my_parent;
uint8_t my_level;

int8_t tdma_tree_create(uint8_t chan);
uint8_t tdma_tree_made();
uint8_t tdma_tree_level_get();
node_addr_t tdma_tree_parent_get();

node_addr_t child_addrs[TREE_MAX_CHILDREN];
uint8_t num_children;

#endif
