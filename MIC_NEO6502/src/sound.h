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

// sound.h

#ifndef _SOUND_h
#define _SOUND_h

#include "Arduino.h"

// SOUND register definitions
#define SND_CMD   0xD040  // CMD register
#define SND_NOTE  0xD041  // NOTE
#define SND_DUR   0xD042  // DUR
#define SND_STAT  0xD04F  // STATUS

#define SND_PLAY  0x01  // PLAY
#define SND_STOP  0x02  // NOPLAY
#define SND_TEMP  0x03  // TEMPO BPM

extern bool soundOn;

void setTempo(const uint8_t);
void initSound();
void scanSound();

#endif

