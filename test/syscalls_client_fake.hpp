#ifndef SYSCALLS_CLIENT_FAKE_HPP
#define SYSCALLS_CLIENT_FAKE_HPP

#include "syscalls_fake.hpp"

extern "C"
{
#include <cg_public.h>
}

#include <memory>

class SyscallsClientFake : public SyscallsFake
{
public:
  snapshot_t& getSnapshot();

  playerState_t& getPlayerState();

  void setDefaultActions(SyscallsMock& mock) final;

  SyscallsClientFake();

  ~SyscallsClientFake();

  class Impl;

private:
  std::unique_ptr<Impl> impl_;
};

#endif // SYSCALLS_CLIENT_FAKE_HPP
