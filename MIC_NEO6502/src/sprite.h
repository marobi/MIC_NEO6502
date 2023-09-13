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

// sprite.h

#ifndef _SPRITE_h
#define _SPRITE_h

#include "Arduino.h"

#define MAX_SPRITES 32

#define SPRITE_BUFFER 0xD100

typedef struct _defsprite {
  uint16_t  x;
  uint16_t  y;
  uint16_t  w;
  uint16_t  h;
  int8_t    dx;
  int8_t    dy;
  uint8_t   mode;
  uint8_t   palette_id;
  uint8_t   color;
  uint8_t   buffer;
  uint8_t   state;
  union {
    uint8_t  b[8];
    uint64_t s;
  } cache;
} defSprite;

void initSprites();

void initSprite();
void drawSprite();
void moveSprite();
void collSprite();

#endif

