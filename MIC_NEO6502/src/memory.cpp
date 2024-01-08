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
// 
// Author: Rien Matthijsse
// 
#include "config.h"
#include "memory.h"
#include "roms.h"

/// <summary>
/// 64k RAM
/// </summary>
#ifdef USE_PIO_CONFIG
uint8_t mem[MEMORY_SIZE]; // __attribute__((aligned(MEMORY_SIZE)));
#else
uint8_t mem[MEMORY_SIZE] __attribute__((aligned(sizeof(uint32_t))));
#endif

// address and data registers
uint16_t address;
uint8_t  data;

/// <summary>
/// initialise memory
/// </summary>
void initMemory() {
  address = 0UL;
  data = 0;
}

/// <summary>
/// read a byte from memory
/// </summary>
void readmemory() {
  data = mem[address];
}

/// <summary>
/// store a byte into memory
/// </summary>
void writememory() {
  if ((0xA000 <= address && address <= 0xCFFF) || (0xE000 <= address && address <= 0xFFF9)) { // exclude writing ROM
    Serial.printf("access violation [%04X]\n", address);
  }
  else
    mem[address] = data;
}
