#ifndef SYSCALLS_HPP
#define SYSCALLS_HPP

extern "C"
{
#include <cg_public.h>
}

#include <cstdint>

class Syscalls
{
public:
  virtual void Cvar_Register(vmCvar_t* vmCvar, char const* varName, char const* defaultValue, std::int32_t flags) = 0;

  virtual void Cvar_Update(vmCvar_t* vmCvar) = 0;

  virtual void Cvar_SetSafe(char const* var_name, char const* value) = 0;

  virtual void Cvar_VariableStringBufferSafe(
    char const*  var_name,
    char*        buffer,
    std::int32_t bufsize,
    std::int32_t flag) = 0;

  virtual void CL_GetCurrentSnapshotNumber(std::int32_t* snapshotNumber, std::int32_t* serverTime) = 0;

  virtual qboolean CL_GetSnapshot(std::int32_t snapshotNumber, snapshot_t* snapshot) = 0;

  Syscalls();

  virtual ~Syscalls();

private:
  static std::intptr_t QDECL VM_DllSyscall(std::intptr_t arg, ...);

  std::intptr_t CL_CgameSystemCalls(std::intptr_t cmd, std::intptr_t* args);

private:
  inline static Syscalls* syscalls_ = nullptr;
};

#endif // SYSCALLS_HPP
