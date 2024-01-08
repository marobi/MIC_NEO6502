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
#include "vdu.h"

/// <summary>
/// 
/// </summary>
typedef struct {
  char  CartridgeName[32];
  const uint8_t* ROMCartridge;
  boolean Loaded;
} defROMCartridge;

/// <summary>
/// ROM cartridge header definition
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
/// cassette with 8 cartridges
/// </summary>
defROMCartridge  gROMSlots[MAX_ROM_SLOTS];


/// <summary>
/// load cartridge into memory
/// </summary>
/// <param name="vId"></param>
/// <returns></returns>
boolean loadROMCartridge(const uint8_t vSlotId) {
  uint16_t startAddress;
  uint16_t romSize;
  uint8_t  romType;

  if (vSlotId < MAX_ROM_SLOTS) {
    defROM* hdr = (defROM*)gROMSlots[vSlotId].ROMCartridge;

    if ((hdr->SOH != 0x5A) || (hdr->EOH != 0xA5)) {
      log("*ERROR: Invalid ROM header");
      return false;
    }

    if (hdr->VERSION_MAJOR != 0x01) {
      log("*ERROR: Invalid ROM version");
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
      log("*ERROR: Invalid checksum");
      return false;
    }

//    Serial.printf("ROM: 0x%02x\n", hdr->TYPE);
    if ((hdr->TYPE & 0x01) != 0) {
      // set NMI
      mem[0xFFFA] = hdr->NMI_L;
      mem[0xFFFB] = hdr->NMI_H;
      log("NMI:\t0x%02x%02x\n", mem[0xFFFB], mem[0xFFFA]);
    }
    if ((hdr->TYPE & 0x02) != 0) {
      // set RESET
      mem[0xFFFC] = hdr->RESET_L;
      mem[0xFFFD] = hdr->RESET_H;
      log("RESET:\t0x%02x%02x\n", mem[0xFFFD], mem[0xFFFC]);
    }
    if ((hdr->TYPE & 0x04) != 0) {
      // set IRQ
      mem[0xFFFE] = hdr->IRQ_L;
      mem[0xFFFF] = hdr->IRQ_H;
      log("IRQ:\t0x%02x%02x\n", mem[0xFFFF], mem[0xFFFE]);
    }

    startAddress = (uint16_t)hdr->STARTADDRESS_H * 256 + hdr->STARTADDRESS_L;
    romSize = (uint16_t)hdr->SIZE_H * 256 + hdr->SIZE_L;
    log("%24s\t%04X: [%04X]\n", gROMSlots[vSlotId].CartridgeName, startAddress, romSize);
    
    // copy ROM in memory space
    memcpy(&mem[startAddress], gROMSlots[vSlotId].ROMCartridge + sizeof(defROM), romSize);

    return true;
  }
  else {
    log("*ERROR: Invalid ROM");
    return false;
  }
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
boolean initROMSlots()
{
  for (uint8_t i = 0; i < MAX_ROM_SLOTS; i++)
    gROMSlots[i].Loaded = false;

  return true;
}

/// <summary>
/// install a ROMCartridge at given position in cassette
/// </summary>
/// <param name="id"></param>
/// <param name="ROMCartridge"></param>
/// <returns></returns>
boolean installROMCartridge(const uint8_t vSlotId, const char* vROMName, const uint8_t* vROMCartridge)
{
  if (vSlotId < MAX_ROM_SLOTS) {
    strcpy(gROMSlots[vSlotId].CartridgeName, vROMName);
    gROMSlots[vSlotId].ROMCartridge = vROMCartridge;

    log("Slot %2d: %s\n", vSlotId, vROMName);
    return true;
  }
  else
    return false;
}
