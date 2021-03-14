#include "syscalls_client_fake.hpp"

#include "syscalls_mock.hpp"

extern "C"
{
#include <cg_vm.h>
}

#include <cstring>
#include <vector>

#define PACKET_BACKUP                                                                                                  \
  32 // number of old messages that must be kept on client and
     // server for delta comrpession and ping estimation
#define PACKET_MASK (PACKET_BACKUP - 1)

class SyscallsClientFake::Impl
{
public:
  Impl() : snapshots_(PACKET_BACKUP)
  {
    // Fake predicted player state, VM_ArgPtr(defrag()->pps_offset)
    std::memset(&g_VM, 0, sizeof(g_VM));
    g_VM.dataSegment = reinterpret_cast<byte*>(&pps_);
  }

  void CL_GetCurrentSnapshotNumber(std::int32_t* snapshotNumber, std::int32_t* serverTime) const
  {
    assert(snapshotNumber);
    assert(serverTime);
    *snapshotNumber = snapshotNumber_;
    *serverTime     = getSnapshot(snapshotNumber_).serverTime;
  }

  qboolean CL_GetSnapshot(std::int32_t snapshotNumber, snapshot_t* snapshot) const
  {
    assert(snapshot);
    std::memcpy(snapshot, &getSnapshot(snapshotNumber), sizeof(snapshot_t));
    return qtrue;
  }

  const snapshot_t& getSnapshot(std::int32_t snapshotNumber) const
  {
    return snapshots_[snapshotNumber & PACKET_MASK];
  }

  snapshot_t& getSnapshot(std::int32_t snapshotNumber)
  {
    return snapshots_[snapshotNumber & PACKET_MASK];
  }

public:
  std::int32_t            snapshotNumber_ = 0;
  std::vector<snapshot_t> snapshots_;

  playerState_t pps_ = {};
};

snapshot_t& SyscallsClientFake::getSnapshot()
{
  return impl_->getSnapshot(impl_->snapshotNumber_);
}

playerState_t& SyscallsClientFake::getPlayerState()
{
  return impl_->pps_;
}

void SyscallsClientFake::setDefaultActions(SyscallsMock& mock)
{
  ON_CALL(mock, CL_GetCurrentSnapshotNumber)
    .WillByDefault(testing::Invoke(impl_.get(), &Impl::CL_GetCurrentSnapshotNumber));
  ON_CALL(mock, CL_GetSnapshot).WillByDefault(testing::Invoke(impl_.get(), &Impl::CL_GetSnapshot));
}

SyscallsClientFake::SyscallsClientFake() : impl_(std::make_unique<Impl>())
{
}

SyscallsClientFake::~SyscallsClientFake() = default;

TEST(SyscallsClientFake, GetCurrentSnapshotNumber)
{
  SyscallsClientFake::Impl fake;
  fake.snapshotNumber_           = 1;
  fake.getSnapshot(1).serverTime = 2;
  std::int32_t snapshotNumber    = 0;
  std::int32_t serverTime        = 0;

  fake.CL_GetCurrentSnapshotNumber(&snapshotNumber, &serverTime);
  EXPECT_EQ(snapshotNumber, 1);
  EXPECT_EQ(serverTime, 2);
}

TEST(SyscallsClientFake, GetSnapshot)
{
  SyscallsClientFake::Impl fake;
  fake.getSnapshot(1).serverTime = 123;
  fake.getSnapshot(2).serverTime = 321;
  snapshot_t snap1               = {};
  snapshot_t snap2               = {};

  fake.CL_GetSnapshot(1, &snap1);
  fake.CL_GetSnapshot(2, &snap2);
  EXPECT_EQ(snap1.serverTime, 123);
  EXPECT_EQ(snap2.serverTime, 321);
}
