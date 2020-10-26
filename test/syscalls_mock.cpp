#include "syscalls_mock.hpp"

#include "syscalls_fake.hpp"

extern "C"
{
#include <cg_local.h>
}

void SyscallsMock::delegateTo(SyscallsFake& fake)
{
  fake.setDefaultActions(*this);
}

SyscallsMock::SyscallsMock() = default;

SyscallsMock::~SyscallsMock() = default;

TEST(SyscallsCvarMock, Register)
{
  SyscallsMock mock;
  vmCvar_t     vmCvar = {};
  EXPECT_CALL(mock, Cvar_Register(&vmCvar, testing::StrEq("varName"), testing::StrEq("3.14"), CVAR_ARCHIVE_ND))
    .Times(1);

  trap_Cvar_Register(&vmCvar, "varName", "3.14", CVAR_ARCHIVE_ND);
}

TEST(SyscallsCvarMock, Update)
{
  SyscallsMock mock;
  vmCvar_t     vmCvar = {};
  EXPECT_CALL(mock, Cvar_Update(&vmCvar)).Times(1);

  trap_Cvar_Update(&vmCvar);
}

TEST(SyscallsCvarMock, SetSafe)
{
  SyscallsMock mock;
  EXPECT_CALL(mock, Cvar_SetSafe(testing::StrEq("varName"), testing::StrEq("3.14"))).Times(1);

  trap_Cvar_Set("varName", "3.14");
}

TEST(SyscallsCvarMock, VariableStringBufferSafe)
{
  SyscallsMock mock;
  char         buffer[5] = {};
  EXPECT_CALL(mock, Cvar_VariableStringBufferSafe(testing::StrEq("varName"), buffer, sizeof(buffer), CVAR_PRIVATE))
    .Times(1);

  trap_Cvar_VariableStringBuffer("varName", buffer, sizeof(buffer));
}

TEST(SyscallsClientMock, GetCurrentSnapshotNumber)
{
  SyscallsMock mock;
  std::int32_t snapshotNumber = 0;
  std::int32_t serverTime     = 0;
  EXPECT_CALL(mock, CL_GetCurrentSnapshotNumber(&snapshotNumber, &serverTime)).Times(1);

  trap_GetCurrentSnapshotNumber(&snapshotNumber, &serverTime);
}

TEST(SyscallsClientMock, GetSnapshot)
{
  SyscallsMock mock;
  snapshot_t   snap = {};
  EXPECT_CALL(mock, CL_GetSnapshot(0, &snap)).Times(1);

  trap_GetSnapshot(0, &snap);
}
