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

#include "roms.h"
#include "memory.h"

/// <summary>
/// ROM package header definition
/// </summary>
typedef struct {            // 16 bytes
  uint8_t	SOH;              // fixed 0x5A
  uint8_t	VERSION_MINOR;    // 0x01
  uint8_t	VERSION_MAJOR;    // 0x01
  uint8_t	STARTADDRESS_L;   // load address
  uint8_t	STARTADDRESS_H;
  uint8_t	SIZE_L;           // ROM size
  uint8_t	SIZE_H;
  uint8_t	TYPE;             // ROM type
  uint8_t	NMI_L;            // NMI vector
  uint8_t	NMI_H;
  uint8_t	RESET_L;          // RESET vector
  uint8_t	RESET_H;
  uint8_t IRQ_L;            // IRQ vector
  uint8_t IRQ_H;
  uint8_t CSUM;             // header checksum
  uint8_t EOH;              // fixed 0xA5
} defROM;


/// <summary>
/// load a ROM image into memory
/// </summary>
/// <param name="romName"></param>
/// <param name="romImage"></param>
/// <returns></returns>
boolean loadROM(const char* romName, const uint8_t* romImage) {
  uint16_t startAddress;
  uint16_t romSize;
  uint8_t romType;

  if (romImage) {
    defROM* hdr = (defROM *)romImage;

    if ((hdr->SOH != 0x5A) || (hdr->EOH != 0xA5)) {
      Serial.println("*ERROR: Invalid ROM header");
      return false;
    }

    if (hdr->VERSION_MAJOR != 0x01) {
      Serial.println("*ERROR: Invalid ROM version");
      return false;
    }

    // calc csum
    uint8_t csum = hdr->STARTADDRESS_L;
    csum += hdr->STARTADDRESS_H;
    csum += hdr->SIZE_L;
    csum += hdr->SIZE_H;
    csum += hdr->TYPE;
    csum += hdr->NMI_L;
    csum += hdr->NMI_H;
    csum += hdr->RESET_L;
    csum += hdr->RESET_H;
    csum += hdr->IRQ_L;
    csum += hdr->IRQ_H;

    if (csum != hdr->CSUM) {
      Serial.println("*ERROR: Invalid checksum");
      return false;
    }

//    Serial.printf("ROM: 0x%02x\n", hdr->TYPE);
    if ((hdr->TYPE & 0x01) != 0) {
      // set NMI
      mem[0xFFFA] = hdr->NMI_L;
      mem[0xFFFB] = hdr->NMI_H;
      Serial.printf("NMI:\t0x%02x%02x\n", mem[0xFFFB], mem[0xFFFA]);
    }
    if ((hdr->TYPE & 0x02) != 0) {
      // set RESET
      mem[0xFFFC] = hdr->RESET_L;
      mem[0xFFFD] = hdr->RESET_H;
      Serial.printf("RESET:\t0x%02x%02x\n", mem[0xFFFD], mem[0xFFFC]);
    }
    if ((hdr->TYPE & 0x04) != 0) {
      // set IRQ
      mem[0xFFFE] = hdr->IRQ_L;
      mem[0xFFFF] = hdr->IRQ_H;
      Serial.printf("IRQ:\t0x%02x%02x\n", mem[0xFFFF], mem[0xFFFE]);
    }

    startAddress = (uint16_t)hdr->STARTADDRESS_H * 256 + hdr->STARTADDRESS_L;
    romSize = (uint16_t)hdr->SIZE_H * 256 + hdr->SIZE_L;
    Serial.printf("%16s\t%04X: [%04X]\n", romName, startAddress, romSize);
    
    // copy ROM in memory space
    memcpy(&mem[startAddress], romImage + sizeof(defROM), romSize);

    return true;
  }
  else {
    Serial.println("*ERROR: Invalid ROM");
    return false;
  }
}