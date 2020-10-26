#ifndef SYSCALLS_CVAR_FAKE_HPP
#define SYSCALLS_CVAR_FAKE_HPP

#include "syscalls_fake.hpp"

#include <memory>

class SyscallsCvarFake : public SyscallsFake
{
public:
  void setDefaultActions(SyscallsMock& mock) final;

  SyscallsCvarFake();

  ~SyscallsCvarFake();

  class Impl;

private:
  std::unique_ptr<Impl> impl_;
};

#endif // SYSCALLS_CVAR_FAKE_HPP
