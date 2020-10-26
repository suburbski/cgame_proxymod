#ifndef SYSCALLS_MOCK_HPP
#define SYSCALLS_MOCK_HPP

#include "syscalls.hpp"

#include <gmock/gmock.h>

class SyscallsFake;

class SyscallsMock : public Syscalls
{
public:
  MOCK_METHOD(
    void,
    Cvar_Register,
    (vmCvar_t * vmCvar, char const* varName, char const* defaultValue, std::int32_t flags),
    (final));

  MOCK_METHOD(void, Cvar_Update, (vmCvar_t * vmCvar), (final));

  MOCK_METHOD(void, Cvar_SetSafe, (char const* var_name, char const* value), (final));

  MOCK_METHOD(
    void,
    Cvar_VariableStringBufferSafe,
    (char const* var_name, char* buffer, std::int32_t bufsize, std::int32_t flag),
    (final));

  MOCK_METHOD(void, CL_GetCurrentSnapshotNumber, (std::int32_t * snapshotNumber, std::int32_t* serverTime), (final));

  MOCK_METHOD(qboolean, CL_GetSnapshot, (std::int32_t snapshotNumber, snapshot_t* snapshot), (final));

  void delegateTo(SyscallsFake& fake);

  SyscallsMock();

  ~SyscallsMock();
};

#endif // SYSCALLS_MOCK_HPP
