/*
  ==============================
  Written by id software, nightmare and hk of mdd
  This file is part of mdd client proxymod.

  mdd client proxymod is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  mdd client proxymod is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with mdd client proxymod.  If not, see <http://www.gnu.org/licenses/>.
  ==============================
  Note: mdd client proxymod contains large quantities from the quake III arena source code
*/
#ifndef CG_CVAR_H
#define CG_CVAR_H

#include "q_shared.h"

#include <stddef.h>

typedef struct
{
  vmCvar_t* vmCvar;
  char*     cvarName;
  char*     defaultString;
  int32_t   cvarFlags;
} cvarTable_t;

char const* ParseVec(char const* data, vec_t* vec, uint8_t size);
char const* ParseVec4(char const* data, vec4_t* vec, uint8_t size);

int32_t cvar_getInteger(char const* var_name);
float   cvar_getValue(char const* var_name);

void init_cvars(cvarTable_t const* cvars, size_t size);
void update_cvars(cvarTable_t const* cvars, size_t size);

#endif // CG_CVAR_H
