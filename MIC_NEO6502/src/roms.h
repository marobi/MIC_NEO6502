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

// roms.h

#ifndef _ROMS_h
#define _ROMS_h

#include "Arduino.h"

#define MAX_ROM_SLOTS  8

bool initROMCassette();
bool installROMCartridge(const uint8_t id, const char *ROMName, const uint8_t* ROMCartridge);
bool loadROMCartridge(const uint8_t vId);

#endif

