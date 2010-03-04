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

#include <nrk.h>
#include <include.h>
#include <nrk_error.h>
#include <nrk_time.h>
#include <nrk_sw_wdt.h>

#ifdef NRK_SW_WDT

void _nrk_sw_wdt_check()
{
uint8_t i;
nrk_time_t now;
nrk_time_t sub;
nrk_time_get(&now);
for(i=0; i<NRK_MAX_SW_WDT; i++ )
  {
	if(sw_wdts[i].active==1 && (nrk_time_sub(&sub,sw_wdts[i].next_period,now)==NRK_ERROR)) 
		{
		nrk_kernel_error_add(NRK_SW_WATCHDOG_ERROR,i );
		if(sw_wdts[i].error_func==NULL)
			// if hw wtd set, this will reboot node
			nrk_halt();
			else
			sw_wdts[i].error_func();
			// call func
		}
  }

}

void _nrk_sw_wdt_init()
{
uint8_t i;
for(i=0; i<NRK_MAX_SW_WDT; i++ )
	sw_wdts[i].active=0;
}

int8_t nrk_sw_wdt_init(uint8_t id, nrk_time_t *period, void *func)
{
if(id>=NRK_MAX_SW_WDT) return NRK_ERROR;
sw_wdts[id].error_func=func;
sw_wdts[id].period.secs=period->secs;
sw_wdts[id].period.nano_secs=period->nano_secs;
sw_wdts[id].active=0;
  return NRK_OK;
}

int8_t nrk_sw_wdt_update(uint8_t id)
{
nrk_time_t now;
if(id>=NRK_MAX_SW_WDT) return NRK_ERROR;
nrk_time_get(&now);
sw_wdts[id].next_period.secs=now.secs+sw_wdts[id].period.secs;
sw_wdts[id].next_period.nano_secs=now.nano_secs+sw_wdts[id].period.nano_secs;
nrk_time_compact_nanos(&(sw_wdts[id].next_period));
sw_wdts[id].active=1;
return NRK_OK;
}

int8_t nrk_sw_wdt_start(uint8_t id)
{
nrk_time_t now;
if(id>=NRK_MAX_SW_WDT) return NRK_ERROR;
nrk_time_get(&now);
sw_wdts[id].next_period.secs=now.secs+sw_wdts[id].period.secs;
sw_wdts[id].next_period.nano_secs=now.nano_secs+sw_wdts[id].period.nano_secs;
sw_wdts[id].active=1;

  return NRK_OK;
}

int8_t nrk_sw_wdt_stop(uint8_t id)
{
if(id>=NRK_MAX_SW_WDT) return NRK_ERROR;
sw_wdts[id].active=0;
  return NRK_OK;
}

#endif
