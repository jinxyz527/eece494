/***********************************************
**  tdma_asap_scheduler.c
************************************************
** Part of TDMA-ASAP, includes graph coloring and
** schedule decision functions.
**
** Author: John Yackovich
************************************************/

#include <include.h>
#include <ulib.h>
#include <stdlib.h>
#include <tdma_asap_scheduler.h>
#include <stdio.h>
#include <nrk.h>
#include <nrk_error.h>
#include <nrk_timer.h>
#include <avr/eeprom.h>
#include <nrk_eeprom.h>

//#define TDMA_SCHED_DEBUG

void tdma_schedule_clr()
{
    tdma_schedule_size = 0;
}

int8_t tdma_schedule_add(tdma_slot_t slot, tdma_slot_type_t slot_type, int8_t priority)
{
    if (tdma_schedule_size >= TDMA_SCHEDULE_MAX_SIZE)
        return NRK_ERROR;

    tdma_schedule[tdma_schedule_size].slot = slot;
    tdma_schedule[tdma_schedule_size].type = slot_type;
    tdma_schedule[tdma_schedule_size].priority = priority;

#ifdef TDMA_SCHED_DEBUG
    printf("Added slot: %d , %d\r\n", slot, tdma_schedule[tdma_schedule_size].slot);
#endif
    tdma_schedule_size++;

    return NRK_OK; 
}

tdma_schedule_entry_t tdma_schedule_get_next(tdma_slot_t slot)
{

    int8_t smallest;
    int8_t smallest_larger;

    //get the smallest slot bigger than this one
    tdma_schedule_entry_t result;

    result = tdma_schedule[0];
    smallest = -1;
    smallest_larger = -1;

    for (int8_t i = 0; i < tdma_schedule_size; i++)
    {
        if ((smallest == -1) || tdma_schedule[i].slot < tdma_schedule[smallest].slot)
        {
            smallest = i;

        }
        if (slot < tdma_schedule[i].slot) /* ADDED NOT TO STAY  && tdma_schedule[i].priority <= 0)*/
        {
            if ((smallest_larger == -1 ) || tdma_schedule[i].slot < tdma_schedule[smallest_larger].slot)
                smallest_larger = i;
            //result =  tdma_schedule[i];
            //break;
        }
    }

    if (smallest_larger == -1)
        smallest_larger = smallest;

    //printf("getting entry %d\r\n", i);
    //tdma_schedule_entry_t entry = tdma_schedule[cur_schedule_entry];
    //cur_schedule_entry = (cur_schedule_entry + 1) % tdma_schedule_size;
    //printf("next slot: %d cur %d\r\n", tdma_schedule[smallest_larger].slot, slot);

    //return result;
    return tdma_schedule[smallest_larger];
}

void tdma_schedule_print()
{
    // Go through tdma schedule, pruint8_t slots
    uint8_t cur_entry;
    nrk_kprintf(PSTR("TDMA SCHEDULE\r\n"));

    for (cur_entry = 0; cur_entry < tdma_schedule_size; cur_entry++)
    {
        printf("-s %d t %d p %d\r\n", 
                tdma_schedule[cur_entry].slot,
                tdma_schedule[cur_entry].type,
                tdma_schedule[cur_entry].priority);
    }

}


/*******
 * Compares 2 nrk_time_t values, returns 1, 0, -1 for gt, eq, lt
 */
int8_t nrk_time_cmp(nrk_time_t time1, nrk_time_t time2)
{
    if (time1.secs > time2.secs)
        return 1;
    else if (time1.secs == time2.secs)
    {
        //secs are equal, check nanos
        if (time1.nano_secs > time2.nano_secs)
            return 1;
        else
            return -1;
    }
    else
    {
        //time is less
        return -1;
    }
}
