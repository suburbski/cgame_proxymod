#ifndef SYSCALLS_FAKE_HPP
#define SYSCALLS_FAKE_HPP

class SyscallsMock;

class SyscallsFake
{
public:
  virtual void setDefaultActions(SyscallsMock& mock) = 0;

  virtual ~SyscallsFake() = default;
};

#endif // SYSCALLS_FAKE_HPP
