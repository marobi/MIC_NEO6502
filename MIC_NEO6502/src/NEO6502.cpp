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

#include "pico/stdio.h"

#include "pico/stdlib.h"
#include "pico/time.h"

#include "mos65C02.h"
#include "memory.h"
#include "roms.h"
#include "vdu.h"
#include "sound.h"
#include "palette.h"

#include "NEO6502.h"

#include "bios.h"
#include "mon.h"
#include "basic.h"
#include "demo.h"

#define MAX_SYSCONFIG_ROM_CARTRIDGES  8

/// <summary>
/// 
/// </summary>
typedef struct {
  char   ConfigName[32];
  bool   Ucase;
  bool   RomProtect;
  uint8_t TextColor;
  int8_t Slot[MAX_SYSCONFIG_ROM_CARTRIDGES];
} defSysConfig;

//---------------------------------------------------------
uint32_t       clockCount = 0UL;
unsigned long  lastClockTS;
unsigned long  frameClockTS;
boolean        logState = false;
boolean        romProtect;

//---------------------------------------------------------
defSysConfig gSysConfig[] = {
//   Config          Ucase  ROMp     ROMs
  { "EHBasic setup", false, true,  DEFAULT_TEXT_COLOR, { 0, 1, 2, -1 } },
  { "C-demo setup",  true,  false, 208               , { 0, 1, 3, -1 } },
  { "",              false, false, DEFAULT_TEXT_COLOR, {-1 } }
};

/// <summary>
/// 
/// </summary>
void NEO6502::init()
{
  initMemory();

  initROMSlots();
  // fill the cassette
  installROMCartridge(0, "BIOS",         bios_bin);
  installROMCartridge(1, "Supermon64",   supermon64_bin);
  installROMCartridge(2, "EhBasic",      basic_bin);
  installROMCartridge(3, "C-demo @1000", demo_bin);

  initSound();

  init6502();
  reset6502();

  initDisplay();

  // 4 stats
  clockCount = 0UL;
  lastClockTS = millis();

  helloDisplay();
}

/// <summary>
/// 
/// </summary>
/// <param name="romName"></param>
/// <param name="ROM"></param>
/// <param name="startAddress"></param>
/// <param name="romSize"></param>
/// <returns></returns>
bool NEO6502::addROM(const uint8_t vId)
{
  return loadROMCartridge(vId);
}

/// <summary>
/// 
/// </summary>
/// <param name="vId"></param>
void NEO6502::setSysConfig(const uint8_t vId)
{
  Serial.printf("System config: %s\n", gSysConfig[vId].ConfigName);

  setUCASE(gSysConfig[vId].Ucase);
  setROMProtect(gSysConfig[vId].RomProtect);
  setTextColor(gSysConfig[vId].TextColor);

  for (uint8_t c = 0; (c < MAX_SYSCONFIG_ROM_CARTRIDGES) && (gSysConfig[vId].Slot[c] >= 0); c++) {
    addROM(gSysConfig[vId].Slot[c]);
  }
}

/// <summary>
/// set the UCASE
/// </summary>
/// <param name="vUcase"></param>
void NEO6502::setUCASE(const boolean vUcase) {
  isUcase = vUcase;
}

/// <summary>
/// set ROM protect
/// </summary>
/// <param name="vProtect"></param>
void NEO6502::setROMProtect(const boolean vProtect)
{
  romProtect = vProtect;
}

/// <summary>
/// 
/// </summary>
/// <param name="vColor"></param>
void NEO6502::setTextColor(uint8_t vColor)
{
  currentTextColor = vColor;
  setColor(vColor);
}

////////////////////////////////////////////////////////////////////
// Serial Event
////////////////////////////////////////////////////////////////////

/*
  SerialEvent occurs whenever a new data comes in the
  hardware serial RX. Multiple bytes of data may be available.
 */
inline __attribute__((always_inline))
void NEO6502::serialEvent1()
{
  byte ch;

  if (Serial.available()) {
    switch (Serial.peek()) {

    case 0x12: // ^R
      Serial.read();
      Serial.println(F("RESET"));
      resetDisplay();
      reset6502();
      break;

    case 0x0C: // ^L
      Serial.read();
      Serial.println(F("LOGGING"));
      logState = !logState;
      clockCount = 0UL;
      break;

    case 0x04: // ^D
      Serial.read();
      Serial.print("VDU: ");
      for (uint8_t i = 0; i < 18; i++) {
        Serial.printf("%02X ", mem[0XD020 + i]);
      }
      Serial.println(F("\nSPRITE:"));
      for (uint8_t i = 0; i < 16; i++) {
        for (uint8_t j = 0; j < 16; j++) {
          Serial.printf("%02X ", mem[0XD100 + i * 16 + j]);
        }
        Serial.println();
      }
      Serial.println();

      Serial.println(F("\nTILE:"));
      for (uint8_t i = 0; i < 16; i++) {
        for (uint8_t j = 0; j < 16; j++) {
          Serial.printf("%02X ", mem[0XD200 + i * 16 + j]);
        }
        Serial.println();
      }
      Serial.println();
      break;

    case 0x13: // ^S
      Serial.read();
      Serial.println(F("SOUND"));
      soundOn = !soundOn;
      break;

    case 0x14: // ^T
      Serial.read();
      Serial.println(F("TRACE"));
      traceOn = !traceOn;
      break;

    default:
      if (mem[KBD] == 0x00) {             // read serial byte only if we can
        if (isUcase) {
          ch = toupper(Serial.read()); // apple1 expects upper case
          mem[KBD] = ch | 0x80;             // apple1 expects bit 7 set for incoming characters.
        }
        else {
          ch = Serial.read();
          mem[KBD] = ch;
        }
        if (traceOn) {
          Serial.printf("IN: [%02X]\n", ch);
        }
      }
      break;
    }
  }
  return;
}

/// <summary>
/// 
/// </summary>
void NEO6502::run() {
  static uint32_t i, j, f = 1;

  //forever
  for (;;) {
    tick6502();
    clockCount++;

    if (j-- == 0) {
      serialEvent1();
      scanSound();
      scanChar();
      scanVDU();

      j = 500UL;
    }

    if (autoUpdate) {
      if (f-- == 0) {
        if ((millis() - frameClockTS) >= FRAMETIME) {
          swapDisplay();
          frameClockTS = millis();
        }

        f = 5000UL;
      }
    }

    // only do stats when in logging mode
    if (logState) {
      if (i-- == 0) {
        if ((millis() - lastClockTS) >= 5000UL) {
          Serial.printf("kHz = %0.1f\n", clockCount / 5000.0);

          clockCount = 0UL;
          lastClockTS = millis();
        }

        i = 20000UL;
      }
    }
  }
}
