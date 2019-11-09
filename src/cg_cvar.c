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

#include "cg_utils.h"

#include <stdlib.h>

float cvar_getValue(char const* var_name)
{
  char buffer[MAX_CVAR_VALUE_STRING];
  g_syscall(CG_CVAR_VARIABLESTRINGBUFFER, var_name, buffer, sizeof(buffer));
  return strtof(buffer, NULL);
}

void init_cvars(cvarTable_t const* cvars, size_t size)
{
  for (uint32_t i = 0; i < size; ++i)
  {
    g_syscall(CG_CVAR_REGISTER, cvars[i].vmCvar, cvars[i].cvarName, cvars[i].defaultString, cvars[i].cvarFlags);
  }
}

void update_cvars(cvarTable_t const* cvars, size_t size)
{
  for (uint32_t i = 0; i < size; ++i)
  {
    g_syscall(CG_CVAR_UPDATE, cvars[i].vmCvar);
  }
}
