
/*******************************************************
 *
 *  Routing Table Routines for EECE 494, Assignment 2.
 *
 *  Created by _________, University of British Columbia
 *
 *  This is where you will put your routines to implement
 *  the routing table.  You should use the headers as
 *  supplied.  All your code will go in this
 *  file (and in cam.h)
 *
 ******************************************************/

#include "defs.h"

#define HASH_TABLE_SIZE 40000

hashtable_t *pHashTbl;

unsigned int SDBMHash(const void *key, unsigned int len)
{
    unsigned int hash = 0;
    unsigned int i = 0;
    const char *k = key;

    for(i = 0; i < len; k++, i++)
        hash = (*k) + (hash << 6) + (hash << 16) - hash;

    return hash;
}

void cam_init()
{
    pHashTbl = malloc(sizeof(hashtable_t));
    if (!pHashTbl) {
        exit(1);
    }

    pHashTbl->nodes = calloc(HASH_TABLE_SIZE, sizeof(hashnode_t*));
    if (!pHashTbl->nodes) {
        free(pHashTbl);
        exit(1);
    }

    pHashTbl->size = HASH_TABLE_SIZE;
    pHashTbl->hashFunc = SDBMHash;
}

void cam_add_entry(ip_address_t *address, int port)
{
    unsigned long index;
    hashnode_t *pNode, *newNode;

    index = pHashTbl->hashFunc(address, sizeof(ip_address_t)) % pHashTbl->size;
    pNode = pHashTbl->nodes[index];
    while (pNode) {
//        ip_address_print(pNode->address);
        if (!memcmp(pNode->address, address, sizeof(ip_address_t))) {
            // update value
            pNode->port = port;
            return;
        }
        pNode = pNode->pNext;
    }

    newNode = malloc(sizeof(hashnode_t));
    if (!newNode) {
        cam_free();
        exit(1);
    }

    newNode->address = malloc(sizeof(ip_address_t));
    if (!newNode) {
        free(newNode);
        cam_free();
        exit(1);
    }

    // add new node to beginning of linked list
    ip_address_copy( address, newNode->address );
    newNode->port = port;
    newNode->pNext = pHashTbl->nodes[index];
    pHashTbl->nodes[index] = newNode;
}

int cam_lookup_address(ip_address_t *address)
{
    unsigned long index;
    hashnode_t *pNode;

    index = pHashTbl->hashFunc(address, sizeof(ip_address_t)) % pHashTbl->size;
    pNode = pHashTbl->nodes[index];
    while (pNode) {
        if (!memcmp(pNode->address, address, sizeof(ip_address_t))) {
            return pNode->port;
        }
        pNode = pNode->pNext;
    }
    return -1;
}

void cam_free()
{
    int i;
    hashnode_t *pNode, *pNext;

    for (i = 0; i < pHashTbl->size; i++) {
        pNode = pHashTbl->nodes[i];
        while (pNode) {
            pNext = pNode->pNext;
            free(pNode->address);
            free(pNode);
            pNode = pNext;
        }
    }
    free(pHashTbl->nodes);
    free(pHashTbl);
}
