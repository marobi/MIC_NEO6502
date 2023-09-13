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
#include "sound.h"

#define _PWM_LOGLEVEL_          0
#define TIMER_INTERRUPT_DEBUG   0
#define TIMER0_INTERVAL_MS      1000

#include <RP2040_PWM.h>
#include <RPi_Pico_TimerInterrupt.h>
#include <cppQueue.h>

#include "memory.h"
#include "pins.h"

//creates pwm instance
RP2040_PWM* PWM_Instance;
RPI_PICO_Timer ITimer0(0);

bool soundOn = true;

float gFrequency = 440;
float gDutyCycle = 50;
uint8_t gBPM     = 120;

uint16_t gWholenote; // whole note

//
typedef struct _sTone {
  char     Tone[4];
  uint16_t Freq;
} sTone;

//
typedef struct _sNote{
  char     Note[4];
  int8_t   Duration;
} sNote;

//
cppQueue	sound_queue(sizeof(sNote), 64, FIFO);	// Instantiate queue

static sTone tones[] = {
  { "P" , 0 },
  { "B0" , 31 },
  { "C1" , 33 },
  { "CS1" , 35 },
  { "D1" , 37 },
  { "DS1" , 39 },
  { "E1" , 41 },
  { "F1" , 44 },
  { "FS1" , 46 },
  { "G1" , 49 },
  { "GS1" , 52 },
  { "A1" , 55 },
  { "AS1" , 58 },
  { "B1" , 62 },
  { "C2" , 65 },
  { "CS2" , 69 },
  { "D2" , 73 },
  { "DS2" , 78 },
  { "E2" , 82 },
  { "F2" , 87 },
  { "FS2" , 93 },
  { "G2" , 98 },
  { "GS2" , 104 },
  { "A2" , 110 },
  { "AS2" , 117 },
  { "B2" , 123 },
  { "C3" , 131 },
  { "CS3" , 139 },
  { "D3" , 147 },
  { "DS3" , 156 },
  { "E3" , 165 },
  { "F3" , 175 },
  { "FS3" , 185 },
  { "G3" , 196 },
  { "GS3" , 208 },
  { "A3" , 220 },
  { "AS3" , 233 },
  { "B3" , 247 },
  { "C4" , 262 },
  { "CS4" , 277 },
  { "D4" , 294 },
  { "DS4" , 311 },
  { "E4" , 330 },
  { "F4" , 349 },
  { "FS4" , 370 },
  { "G4" , 392 },
  { "GS4" , 415 },
  { "A4" , 440 },
  { "AS4" , 466 },
  { "B4" , 494 },
  { "C5" , 523 },
  { "CS5" , 554 },
  { "D5" , 587 },
  { "DS5" , 622 },
  { "E5" , 659 },
  { "F5" , 698 },
  { "FS5" , 740 },
  { "G5" , 784 },
  { "GS5" , 831 },
  { "A5" , 880 },
  { "AS5" , 932 },
  { "B5" , 988 },
  { "C6" , 1047 },
  { "CS6" , 1109 },
  { "D6" , 1175 },
  { "DS6" , 1245 },
  { "E6" , 1319 },
  { "F6" , 1397 },
  { "FS6" , 1480 },
  { "G6" , 1568 },
  { "GS6" , 1661 },
  { "A6" , 1760 },
  { "AS6" , 1865 },
  { "B6" , 1976 },
  { "C7" , 2093 },
  { "CS7" , 2217 },
  { "D7" , 2349 },
  { "DS7" , 2489 },
  { "E7" , 2637 },
  { "F7" , 2794 },
  { "FS7" , 2960 },
  { "G7" , 3136 },
  { "GS7" , 3322 },
  { "A7" , 3520 },
  { "AS7" , 3729 },
  { "B7" , 3951 },
  { "C8" , 4186 },
  { "CS8" , 4435 },
  { "D8" , 4699 },
  { "DS8" , 4978 },
  { "" , 10000}
};

/// <summary>
/// set tempo
/// </summary>
/// <param name="bpm"></param>
void setTempo(const uint8_t bpm) {
  gWholenote = (60000UL * 4) / bpm;
}

/// <summary>
/// 
/// </summary>
/// <param name="vTone"></param>
/// <returns></returns>
inline __attribute__((always_inline))
uint16_t getFreq(String vTone) {
  uint8_t t = 0;

  while (tones[t].Freq < 10000) {
    if (strcmp(tones[t].Tone, vTone.c_str()) == 0)
      return tones[t].Freq;
    t++;
  }

  return 0;
}

inline __attribute__((always_inline))
uint16_t getDuration(const int dur) {
  uint16_t noteDuration;

  if (dur > 0) {
    // regular note, just proceed
    noteDuration = (gWholenote) / dur;
  }
  else if (dur < 0) {
    // dotted notes are represented with negative durations!!
    noteDuration = (gWholenote) / abs(dur);
    noteDuration *= 1.5; // increases the duration in half for dotted notes
  }

  return noteDuration;
}

/// <summary>
/// 
/// </summary>
/// <param name="t"></param>
/// <returns></returns>
uint8_t thMode = 0x00;
uint16_t thDuration;

bool TimerHandler(struct repeating_timer* t)
{
  sNote n;

  switch (thMode) {
  case 0x00:
    if (!sound_queue.isEmpty()) {

      sound_queue.pull(&n);

      //    Serial.printf("Note [%s][%d][%d]\n", n.Note, n.Duration, getDuration(n.Duration));

      gFrequency = getFreq(n.Note);

      if (gFrequency > 0)
        PWM_Instance->setPWM(uP_BUZZ, gFrequency, gDutyCycle);
      else
        PWM_Instance->setPWM(uP_BUZZ, 440, 0);

      thDuration = getDuration(n.Duration);
      ITimer0.setInterval(900L * thDuration, TimerHandler);
      thMode++;

      mem[SND_STAT] = 0;
    }
    else { // idle
      PWM_Instance->setPWM(uP_BUZZ, 440, 0);
      ITimer0.setInterval(20 * 1000, TimerHandler);

      mem[SND_STAT] = 1;
    }
    break;

  case 0x01: // little break
    PWM_Instance->setPWM(uP_BUZZ, 440, 0);
    ITimer0.setInterval(100L * thDuration, TimerHandler);
    thMode = 0x00;
    break;

  default: // should never occur
    thMode = 0x00;
    break;
  }

  return true;
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
inline __attribute__((always_inline))
uint8_t SoundQueueIsEmpty() {
  return sound_queue.isEmpty();
}

/// <summary>
/// 
/// </summary>
/// <returns></returns>
inline __attribute__((always_inline))
uint8_t SoundQueueIsFull() {
  return sound_queue.isFull();
}

/// <summary>
/// 
/// </summary>
/// <param name="vTone"></param>
/// <param name="vDuration"></param>
/// <returns></returns>
inline __attribute__((always_inline))
boolean pushNote(String vNote, int8_t vDuration) {
  sNote n;

  strcpy(n.Note, vNote.c_str());
  n.Duration = vDuration;

  sound_queue.push(&n);
  return sound_queue.isFull();
}

/// <summary>
/// 
/// </summary>
inline __attribute__((always_inline))
void SoundReset() {
  //  Serial.println("SOUND: reset");
  sound_queue.clean();
}

/// <summary>
/// 
/// </summary>
inline __attribute__((always_inline))
void setSound(uint8_t vCmd) {
//  Serial.printf("SOUND: %02d\n", vCmd);

  if (soundOn){
    switch (vCmd) {
    case SND_PLAY:
      pushNote(tones[mem[SND_NOTE]].Tone, (int8_t)mem[SND_DUR]);
      break;

    case SND_STOP:
      sound_queue.clean();
      break;

    case SND_TEMP:
      setTempo(mem[SND_DUR]);
      break;
    }
  }
}

/// <summary>
/// 
/// </summary>
void scanSound() {
  uint8_t cmd = mem[SND_CMD];

  if (cmd != 0x00) {
    setSound(cmd);
    mem[SND_CMD] = 0x00;
  }
}

//
sNote sIntro[] = {
#if 0
  {"C3", 8},
  {"G3", 8},
  {"E3", 8},
  {"C4", 8},
  {"P",  4},
#else // STARTREK
  {"D4", -8}, {"G4", 16}, {"C5", -4},
  {"B4", 8}, {"G4", -16}, {"E4", -16}, {"A4", -16},
  {"D5", 2},
#endif
  {"", 0}
};

/// <summary>
/// 
/// </summary>
void initSound() {
  uint8_t i = 0;

  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler)) {
    Serial.print(F("SOUND driver installed\n"));
  }
  else
    Serial.println(F("Can't set Timer. Select another freq"));

  PWM_Instance = new RP2040_PWM(uP_BUZZ, 440, 0);

  setTempo(gBPM);

  // play intro
  while (strcmp(sIntro[i].Note, "") != 0) {
    pushNote(sIntro[i].Note, sIntro[i].Duration);
    i++;
  }

  mem[SND_CMD] = 0x00;
}
