/*
This software is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef _NEO6502_h
#define _NEO6502_h

#include "Arduino.h"

//#define UCASE_ONLY

extern boolean romProtect;

/// <summary>
/// 
/// </summary>
class NEO6502
{
 private:
   boolean isUcase = true;

	 void serialEvent1();

 public:
	 void init();
	 bool addROM(const char* romName, const uint8_t* ROM);
	 void run();
	 void setUCASE(const boolean vUcase);
	 void setROMProtect(const boolean vProtect);
	 void setTextColor(uint8_t vColor);
};

#endif
