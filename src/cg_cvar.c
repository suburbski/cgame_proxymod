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
#include "cg_cvar.h"

#include "cg_local.h"
#include "q_assert.h"

#include <ctype.h>
#include <stdlib.h>

char const* ParseVec(char const* data, vec_t* vec, uint8_t size)
{
  assert(data);
  assert(vec);
  ASSERT_GT(size, 0);

  char* end;
  for (uint8_t i = 0; i < size; ++i)
  {
    vec[i] = strtof(data, &end);
    assert(data != end);
    data = end;
  }
  return data;
}

char const* ParseVec4(char const* data, vec4_t* vec, uint8_t size)
{
  assert(data);
  assert(vec);
  ASSERT_GT(size, 0);

  data = ParseVec(data, vec[0], ARRAY_LEN(*vec));
  for (uint8_t i = 1; i < size; ++i)
  {
    while (*data++ != '/') assert(data[-1] != '\0');
    data = ParseVec(data, vec[i], ARRAY_LEN(*vec));
  }
  return data;
}

int32_t cvar_getInteger(char const* var_name)
{
  char buffer[MAX_CVAR_VALUE_STRING];
  trap_Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
  char const* s    = buffer;
  int8_t      sign = 1;
  while (isspace(*s)) ++s;
  if (*s == '-')
  {
    sign = -1;
    ++s;
  }
  else if (*s == '+')
  {
    ++s;
  }
  if (*s == '0' && (s[1] == 'b' || s[1] == 'B'))
  {
    return sign * strtol(s + 2, NULL, 2);
  }
  return sign * strtol(s, NULL, 0);
}

float cvar_getValue(char const* var_name)
{
  char buffer[MAX_CVAR_VALUE_STRING];
  trap_Cvar_VariableStringBuffer(var_name, buffer, sizeof(buffer));
  return strtof(buffer, NULL);
}

void init_cvars(cvarTable_t const* cvars, size_t size)
{
  for (uint32_t i = 0; i < size; ++i)
  {
    trap_Cvar_Register(cvars[i].vmCvar, cvars[i].cvarName, cvars[i].defaultString, cvars[i].cvarFlags);
  }
}

void update_cvars(cvarTable_t const* cvars, size_t size)
{
  for (uint32_t i = 0; i < size; ++i)
  {
    trap_Cvar_Update(cvars[i].vmCvar);
  }
}
