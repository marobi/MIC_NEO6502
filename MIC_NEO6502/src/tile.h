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

// tile.h

#ifndef _TILE_h
#define _TILE_h
#include "Arduino.h"

#define MAX_TILES 64

#define TILE_BUFFER 0xD200

typedef struct _deftile {
  uint16_t  w;
  uint16_t  h;
  uint8_t   palette_id;
  uint8_t   state; // 0 :: undef, 1 :: defined
  union {
    uint8_t  b[8];
    uint64_t s;
  } cache;
} defTile;

void initTiles();

void drawTile();

#endif

