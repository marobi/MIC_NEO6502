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
#include "NEO6502.h"
#include "mos65C02.h"
#include "pins.h"
#include "memory.h"

#define RESET_LOW   false
#define RESET_HIGH  true

#define ENABLE_LOW  false
#define ENABLE_HIGH true

#define CLOCK_LOW   false
#define CLOCK_HIGH  true

#define RW_READ     true
#define RW_WRITE    false

#define DELAY_FACTOR_SHORT() asm volatile("nop\nnop\nnop\nnop\n");
#define DELAY_FACTOR_LONG()  asm volatile("nop\nnop\nnop\nnop\n");

// # of clock cycles to keep reset pin low
#define RESET_COUNT  8

// mask used for the mux address/data bus: GP0-7
constexpr uint32_t BUS_MASK = 0xFF;

//
uint8_t   resetCount;
boolean   inReset = false;
uint8_t   dataDir = 2;

/// <summary>
/// drive the clockpin
/// </summary>
/// <param name="enable"></param>
inline __attribute__((always_inline))
void  setClock(const boolean clock) {
  gpio_put(uP_CLOCK, clock);
}

/// <summary>
/// drive the reset pin
/// </summary>
/// <param name="enable"></param>
inline __attribute__((always_inline))
void setReset(const boolean reset) {
  gpio_put(uP_RESET, reset);
}

/// <summary>
/// drive the enable pins
/// </summary>
/// <param name="enable"></param>
inline __attribute__((always_inline))
void  setEnable(const uint32_t enable) {
  gpio_put_masked(en_MASK, enable);
}

/// <summary>
/// set direction of databus mux
/// </summary>
/// <param name="direction"></param>
inline __attribute__((always_inline))
void  setDir(const uint8_t direction) {
  if (direction != dataDir) {
    switch (direction) {
    case OUTPUT:  gpio_set_dir_masked(BUS_MASK, BUS_MASK);
      break;
    case INPUT:   gpio_set_dir_masked(BUS_MASK, (uint32_t)0UL);
      break;
    }

    dataDir = direction;
  }
}

#ifndef USE_PIO_CONFIG
/// <summary>
/// read the R/W pin
/// </summary>
/// <returns></returns>
inline __attribute__((always_inline))
bool  getRW() {
  return gpio_get(uP_RW);
}
#endif

#ifndef USE_PIO_CONFIG
/// <summary>
/// read the address bus
/// </summary>
/// <returns></returns>
inline __attribute__((always_inline))
uint16_t  getAddress() {
  // we are already in INPUT
  //setDir(INPUT);

  // read A0-7
  setEnable(en_A0_7);
  DELAY_FACTOR_SHORT();
  uint16_t addr = gpio_get_all() & BUS_MASK;

  // read A8-15
  setEnable(en_A8_15);
  DELAY_FACTOR_SHORT();
  addr |= (gpio_get_all() & BUS_MASK) << 8;

  return addr;
}
#endif

#ifndef USE_PIO_CONFIG
/// <summary>
/// read the databus
/// </summary>
/// <returns></returns>
inline __attribute__((always_inline))
uint8_t  getData() {
  // we are already in INPUT
  //setDir(INPUT);

  setEnable(en_D0_7);
  DELAY_FACTOR_SHORT();
  uint8_t data = gpio_get_all() & BUS_MASK;

  return data;
}
#endif

#ifndef USE_PIO_CONFIG
/// <summary>
/// write the databus
/// </summary>
/// <param name="data"></param>
inline __attribute__((always_inline))
void  putData(const uint8_t data) {

  setDir(OUTPUT);
  setEnable(en_D0_7);
  gpio_put_masked(BUS_MASK, (uint32_t)data);
}
#endif

/// <summary>
/// initialise the 65C02
/// </summary>
void init6502() {
  // CLOCK
  pinMode(uP_CLOCK, OUTPUT);
  setClock(CLOCK_HIGH);

  // RESET
  pinMode(uP_RESET, OUTPUT);
  setReset(RESET_HIGH);

  // RW
  pinMode(uP_RW, INPUT_PULLUP);

  // BUS ENABLE
  gpio_init_mask(en_MASK);
  gpio_set_dir_out_masked(en_MASK); // enable as output
  setEnable(en_NONE); // all high

  // ADDRESS
  // DATA
  gpio_init_mask(BUS_MASK);
  setDir(INPUT);
}

/// <summary>
/// 
/// </summary>
void handleReset() {
  // reset mgmt
  if (inReset) {
    if (resetCount-- == 0) {
      // end of reset
      setReset(RESET_HIGH);
      inReset = false;
      //        Serial.printf("RESET release\n");
    }
  }
}

/// <summary>
/// reset the 65C02
/// </summary>
void reset6502() {
  setReset(RESET_LOW);

  resetCount = RESET_COUNT;
  inReset = true;
}

#ifndef USE_PIO_CONFIG
/// <summary>
/// clock cycle 65C02
/// </summary>
//inline __attribute__((always_inline))
void  tick6502()
{
  setEnable(en_NONE); // all high

  //------------------------------------------------------------------------------------
  // set INPUT
  setDir(INPUT);

  setClock(CLOCK_LOW);

  DELAY_FACTOR_LONG();

  //------------------------------------------------------------------------------------
  setClock(CLOCK_HIGH);

  address = getAddress();

  // do RW action
  switch (getRW()) {
  case RW_READ:
#ifdef USE_DIRECT_READ
    data = mem[address];
#else
    readmemory(); // data = @address
#endif
    putData(data);
    //      Serial.printf("R %04X %02X\n", address, data);
    break;

  case RW_WRITE:
    data = getData();
    if (! romProtect)
      mem[address] = data;
    else
      writememory(); // @address = data

    //      Serial.printf("W %04X %02X\n", address, data);
    break;
  }

  handleReset();
}
#endif
