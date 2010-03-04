//#include <nrk_timer.h>
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
#include "tdma_asap_stats.h"

/*
inline void _tdma_timer4_setup()
{
  TCCR4A=0;  
  TCCR4B=BM(CS10);  // clk I/O no prescale
  TCNT4=0;  // 16 bit
  GTCCR |= BM(PSRASY);              // reset prescaler
  GTCCR |= BM(PSRSYNC);              // reset prescaler
}

inline void _tdma_timer4_start()
{
    TCCR4B = BM(CS10); // no prescale
}

inline void _tdma_timer4_reset()
{
    GTCCR |= BM(PSRSYNC);
    TCNT4 = 0;
}

inline void _tdma_timer4_stop()
{
    TCCR4B = 0;
}

inline uint16_t _tdma_timer4_get()
{
    uint16_t tmp;
    tmp = TCNT4;
    return tmp;
}
*/

inline void _tdma_timer5_setup()
{
  TCCR5A=0;  
  TCCR5B=BM(CS10);  // clk I/O no prescale
  TCNT5=0;  // 16 bit
  GTCCR |= BM(PSRASY);              // reset prescaler
  GTCCR |= BM(PSRSYNC);              // reset prescaler
}

inline void _tdma_timer5_start()
{
    TCCR5A = 0;
    TCCR5B = BM(CS10); // no prescale
}

inline void _tdma_timer5_reset()
{
    GTCCR |= BM(PSRSYNC);
    TCNT5 = 0;
}

inline void _tdma_timer5_stop()
{
    TCCR5A = 0;
}

inline uint16_t _tdma_timer5_get()
{
    volatile uint16_t tmp;
    tmp = TCNT5;
    return tmp;
}

void tdma_stats_setup(uint8_t pid)
{

    // used to grab the stats structure
    tdma_pid = pid;

    //steal_rdo_running = 0;
    //reg_rdo_running = 0;
    rdo_running = 0;
    ticks_rdo = 0;
    //reg_micros_rdo = 0;
    //steal_micros_rdo = 0;
    steal_cnt=0;
    backoff_cnt = 0;
    dbl_pkt_cnt = 0;
    mpkt_log_curbuf=0;
    sync_rx_cnt = 0;
    sync_slot_cnt = 0;
    //recording_awk_reg = 0;
    //recording_awk_steal =0;
    //reg_micros_awk =0;
    //steal_micros_awk =0;

    tree_creation_time.secs = 0;
    tree_creation_time.nano_secs = 0;

    //_tdma_timer4_setup();
    _tdma_timer5_setup();
}

// radio stats : use timer 5

void stats_start_rdo()
{
    rdo_running = 1;
    _tdma_timer5_reset();
}

void stats_stop_rdo()
{
    if (rdo_running)
    {
        rdo_running = 0;
        ticks_rdo += _tdma_timer5_get();
        //printf("r%20"PRIu32" %d\r\n", ticks_rdo, _tdma_timer5_get() );
        _tdma_timer5_stop();
    }
}
/*
void steal_start_rdo()
{
    steal_rdo_running=1;
    _tdma_timer5_reset();
    _tdma_timer5_start();
}

void steal_stop_rdo()
{
    if (steal_rdo_running)
    {
        steal_rdo_running = 0;
        steal_micros_rdo += _tdma_timer5_get();
        _tdma_timer5_stop();
    }
}

void steal_cca_check()
{
    steal_micros_rdo+=250;
}

void reg_start_rdo()
{
    reg_rdo_running=1;
    _tdma_timer5_reset();
    _tdma_timer5_start();
}

void reg_stop_rdo()
{
    if (reg_rdo_running)
    {
        steal_rdo_running = 0;
        reg_micros_rdo += _tdma_timer5_get() / 4;
        _tdma_timer5_stop();
    }
}
*/

inline void steal_log()
{
    steal_cnt++;
}

inline void steal_backoff_log()
{
    backoff_cnt++;
}

void tdma_stats_mpkt_dump()
{
    uint8_t i = 0;
    //printf("MD %d\r\n", mpkt_log_curbuf);
    while (i < mpkt_log_curbuf)
    {
        printf("%s\r\n", mpkt_log[i]);
        i++;
    }
    mpkt_log_curbuf = 0;
}

void tdma_stats_dump()
{
  // PRINTS OUT...
  //
  // My MAC address
  // time tdma task has spent awake (secs)
  // time tdma task has spent awake (nanosecs)
  // time tdma task used radio
  // number of slot steals
  // number of backoffs
  // number of rx syncs
  // number of sync slots

#ifdef NRK_STATS_TRACKER
  nrk_stats_get(tdma_pid, &tdma_stat_struct);

  tdma_cpu_time = _nrk_ticks_to_time(tdma_stat_struct.total_ticks);
  nrk_time_sub(&tdma_cpu_time, tdma_cpu_time, tree_creation_time);
#endif

  printf("SD %d %lu %lu %lu %lu %lu %u %u\r\n",
    tdma_mac_get(),
    tdma_cpu_time.secs,
    tdma_cpu_time.nano_secs,
    ticks_rdo,
    steal_cnt,
    backoff_cnt,
    sync_rx_cnt,
    sync_slot_cnt);

}

void steal_dbl_pkt()
{
    dbl_pkt_cnt++;
}

/*
// awake stats = use timer 4
void stats_begin_awk_steal()
{
    recording_awk_steal=1;
    _tdma_timer4_reset();
    _tdma_timer4_start();
}

void stats_end_awk_steal()
{

    if (recording_awk_steal)
    {
        recording_awk_steal = 0;
        steal_micros_awk += _tdma_timer4_get() / 4;
        _tdma_timer5_stop();
    }
}

void stats_begin_awk_reg()
{
    recording_awk_reg=1;
    _tdma_timer4_reset();
    _tdma_timer4_start();

}

void stats_end_awk_reg()
{

    if (recording_awk_reg)
    {
        recording_awk_reg = 0;
        reg_micros_awk += _tdma_timer4_get() / 4;
        _tdma_timer5_stop();
    }
}

// decide not to record this stat
void stats_cancel_awk_reg()
{
    recording_awk_reg = 0;
    _tdma_timer5_stop();
    _tdma_timer5_reset();
}

// decide not to record this stat
void stats_cancel_awk_steal()
{
    recording_awk_steal = 0;
    _tdma_timer5_stop();
    _tdma_timer5_reset();
}

void stats_record_failed_rx_reg()
{
    reg_micros_awk += 2000; // 2MS for TDMA_RX_WAIT_TIME_MS
}
*/
