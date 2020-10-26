#include "syscalls.hpp"

extern "C"
{
#include <cg_syscall.h>
}

#include <cassert>
#include <cstdarg>
#include <cstring>

// Max number of arguments to pass from a vm to engine's syscall handler function for the vm.
// syscall number + 15 arguments
#define MAX_VMSYSCALL_ARGS 16

namespace
{
template <typename T>
T* ptr(std::intptr_t x)
{
  return reinterpret_cast<T*>(x);
}
} // namespace

Syscalls::Syscalls()
{
  assert(!syscalls_);
  syscalls_ = this;
  dllEntry(&VM_DllSyscall);
}

Syscalls::~Syscalls()
{
  assert(syscalls_ == this);
  syscalls_ = nullptr;
  dllEntry(nullptr);
}

/*
============
VM_DllSyscall

Dlls will call this directly

 rcg010206 The horror; the horror.

  The syscall mechanism relies on stack manipulation to get its args.
   This is likely due to C's inability to pass "..." parameters to
   a function in one clean chunk. On PowerPC Linux, these parameters
   are not necessarily passed on the stack, so while (&arg[0] == arg)
   is true, (&arg[1] == 2nd function parameter) is not necessarily
   accurate, as arg's value might have been stored to the stack or
   other piece of scratch memory to give it a valid address, but the
   next parameter might still be sitting in a register.

  Quake's syscall system also assumes that the stack grows downward,
   and that any needed types can be squeezed, safely, into a signed int.

  This hack below copies all needed values for an argument to a
   array in memory, so that Quake can get the correct values. This can
   also be used on systems where the stack grows upwards, as the
   presumably standard and safe stdargs.h macros are used.

  As for having enough space in a signed int for your datatypes, well,
   it might be better to wait for DOOM 3 before you start porting.  :)

  The original code, while probably still inherently dangerous, seems
   to work well enough for the platforms it already works on. Rather
   than add the performance hit for those platforms, the original code
   is still in use there.

  For speed, we just grab 15 arguments, and don't worry about exactly
   how many the syscall actually needs; the extra is thrown away.

============
*/
std::intptr_t QDECL Syscalls::VM_DllSyscall(std::intptr_t arg, ...)
{
  assert(syscalls_);

  // rcg010206 - see commentary above
  std::intptr_t args[MAX_VMSYSCALL_ARGS];
  args[0] = arg;

  std::va_list ap;
  va_start(ap, arg);
  for (std::uint8_t i = 1; i < ARRAY_LEN(args); i++) args[i] = va_arg(ap, std::intptr_t);
  va_end(ap);

  return syscalls_->CL_CgameSystemCalls(args[0], args + 1);
}

/*
====================
CL_CgameSystemCalls

The cgame module is making a system call
====================
*/
std::intptr_t Syscalls::CL_CgameSystemCalls(std::intptr_t cmd, std::intptr_t* args)
{
  switch (cmd)
  {
  case CG_CVAR_REGISTER:
    Cvar_Register(
      ptr<vmCvar_t>(args[0]), ptr<char const>(args[1]), ptr<char const>(args[2]), static_cast<std::int32_t>(args[3]));
    return 0;
  case CG_CVAR_UPDATE:
    Cvar_Update(ptr<vmCvar_t>(args[0]));
    return 0;
  case CG_CVAR_SET:
    Cvar_SetSafe(ptr<char const>(args[0]), ptr<char const>(args[1]));
    return 0;
  case CG_CVAR_VARIABLESTRINGBUFFER:
    Cvar_VariableStringBufferSafe(
      ptr<char const>(args[0]), ptr<char>(args[1]), static_cast<std::int32_t>(args[2]), CVAR_PRIVATE);
    return 0;
  case CG_GETCURRENTSNAPSHOTNUMBER:
    CL_GetCurrentSnapshotNumber(ptr<std::int32_t>(args[0]), ptr<std::int32_t>(args[1]));
    return 0;
  case CG_GETSNAPSHOT:
    return CL_GetSnapshot(static_cast<std::int32_t>(args[0]), ptr<snapshot_t>(args[1]));
  }
  assert(false);
  return 0;
}
