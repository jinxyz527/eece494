#ifndef NRK_SW_WDT_H
#define NRK_SW_WDT_H
#include <nrk_cfg.h>
#include <nrk_time.h>

#ifdef NRK_SW_WDT

// default to 1 Software Watch dog timer
#ifndef NRK_MAX_SW_WDT
#define NRK_MAX_SW_WDT 1
#endif

typedef struct sw_wdt {
	void (*error_func)(void);
	uint8_t active;
	nrk_time_t period;
	nrk_time_t next_period;
} nrk_sw_wdt_t;

nrk_sw_wdt_t sw_wdts[NRK_MAX_SW_WDT];

int8_t nrk_sw_wdt_init(uint8_t id, nrk_time_t *period, void *func);
int8_t nrk_sw_wdt_update(uint8_t id);
int8_t nrk_sw_wdt_start(uint8_t id);
int8_t nrk_sw_wdt_stop(uint8_t id);
void _nrk_sw_wdt_check();
void _nrk_sw_wdt_init();
#endif

#endif
