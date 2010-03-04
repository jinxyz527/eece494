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
#include <nrk_status.h>
#include <nrk_error.h>

uint8_t _nrk_startup_error()
{
uint8_t error;
error=0;
// Use the timer settings that are normally 0 on reset to detect
// if the OS has reboot by accident


// Check Watchdog timer
if( (MCUSR & (1<<WDRF)) != 0 )
	{
	// don't clear wdt
	error|=0x10;
	}


// Check Brown Out 
if( (MCUSR & (1<<BORF)) != 0 )
	{
	MCUSR &= ~(1<<BORF);	
	// Only add brownout if it isn't the first bootup
	if( (MCUSR & (1<<PORF)) == 0 )
		error|=0x04;
	}

// Check External Reset 
if( (MCUSR & (1<<EXTRF)) != 0 )
	{
	MCUSR &= ~(1<<EXTRF);	
	error|=0x02;
	}

// If any of the above errors went off, then the next errors will
// incorrectly be set!  So make sure to bail early!
if(error!=0) return error;

// Check if normal power up state is set and then clear it
if( (MCUSR & (1<<PORF)) != 0 )
	{
	MCUSR &= ~(1<<PORF);
	}
	else {
	error|=0x01;
	}

// check uart state 
if((volatile uint8_t)TCCR2A!=0) error|=0x01;

return error;
}


