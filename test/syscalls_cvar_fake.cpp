#include "syscalls_cvar_fake.hpp"

#include "syscalls_mock.hpp"

#include <cassert>
#include <cstring>
#include <map>

namespace
{
class Cvar
{
public:
  Cvar(char const* string, std::int32_t flags) : flags_(flags)
  {
    assert(string);
    set(string);
  }

  void set(char const* string)
  {
    assert(string);
    string_  = string;
    value_   = static_cast<float>(std::atof(string));
    integer_ = std::atoi(string);
  }

  std::string  string_;
  std::int32_t flags_   = 0;
  float        value_   = 0; // std::atof(string)
  std::int32_t integer_ = 0; // std::atoi(string)
};
} // namespace

class SyscallsCvarFake::Impl
{
public:
  void Cvar_Register(vmCvar_t* vmCvar, char const* varName, char const* defaultValue, std::int32_t flags)
  {
    assert(vmCvar);
    assert(varName);
    assert(defaultValue);

    assert(!nameToHandles_.count(varName));
    nameToHandles_.emplace(varName, nextCvarHandle_);
    cvars_.emplace_back(defaultValue, flags);

    vmCvar->handle = nextCvarHandle_++;
    Cvar_Update(vmCvar);
  }

  void Cvar_Update(vmCvar_t* vmCvar) const
  {
    assert(vmCvar);

    assert(vmCvar->handle < cvars_.size());
    auto const& cvar = cvars_[vmCvar->handle];

    assert(cvar.string_.size() + 1 <= sizeof(vmCvar->string));
    std::strncpy(vmCvar->string, cvar.string_.c_str(), sizeof(vmCvar->string) - 1);
    vmCvar->string[sizeof(vmCvar->string) - 1] = '\0';

    vmCvar->value   = cvar.value_;
    vmCvar->integer = cvar.integer_;
  }

  void Cvar_SetSafe(const char* var_name, const char* value)
  {
    assert(var_name);
    assert(value);

    auto* const cvar = find(var_name);
    assert(cvar);
    assert(!(cvar->flags_ & (CVAR_PROTECTED | CVAR_PRIVATE)));
    cvar->set(value);
  }

  void Cvar_VariableStringBufferSafe(char const* var_name, char* buffer, std::int32_t bufsize, std::int32_t flag) const
  {
    assert(var_name);
    assert(buffer);
    assert(bufsize > 0);

    auto const* const cvar = find(var_name);
    if (!cvar || cvar->flags_ & flag)
    {
      *buffer = '\0';
    }
    else
    {
      std::strncpy(buffer, cvar->string_.c_str(), static_cast<std::size_t>(bufsize - 1));
      buffer[bufsize - 1] = '\0';
    }
  }

  Cvar const* find(char const* var_name) const
  {
    assert(var_name);

    auto const nameToHandleIt = nameToHandles_.find(var_name);
    if (nameToHandleIt != nameToHandles_.cend())
    {
      auto const handle = nameToHandleIt->second;
      assert(handle < cvars_.size());
      return &cvars_[handle];
    }
    return nullptr;
  }

  Cvar* find(char const* var_name)
  {
    return const_cast<Cvar*>(static_cast<Impl const&>(*this).find(var_name));
  }

public:
  cvarHandle_t                        nextCvarHandle_ = 0;
  std::vector<Cvar>                   cvars_;
  std::map<std::string, cvarHandle_t> nameToHandles_;
};

void SyscallsCvarFake::setDefaultActions(SyscallsMock& mock)
{
  ON_CALL(mock, Cvar_Register).WillByDefault(testing::Invoke(impl_.get(), &Impl::Cvar_Register));
  ON_CALL(mock, Cvar_Update).WillByDefault(testing::Invoke(impl_.get(), &Impl::Cvar_Update));
  ON_CALL(mock, Cvar_SetSafe).WillByDefault(testing::Invoke(impl_.get(), &Impl::Cvar_SetSafe));
  ON_CALL(mock, Cvar_VariableStringBufferSafe)
    .WillByDefault(testing::Invoke(impl_.get(), &Impl::Cvar_VariableStringBufferSafe));
}

SyscallsCvarFake::SyscallsCvarFake() : impl_(std::make_unique<Impl>())
{
}

SyscallsCvarFake::~SyscallsCvarFake() = default;

TEST(SyscallsCvarFake, Register)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar1 = {};
  vmCvar_t               vmCvar2 = {};

  fake.Cvar_Register(&vmCvar1, "varName1", "3.14", CVAR_ARCHIVE_ND);
  fake.Cvar_Register(&vmCvar2, "varName2", "42", CVAR_ARCHIVE_ND);
  EXPECT_EQ(vmCvar1.value, 3.14f);
  EXPECT_EQ(vmCvar1.integer, 3);
  EXPECT_STREQ(vmCvar1.string, "3.14");
  EXPECT_EQ(vmCvar2.value, 42.f);
  EXPECT_EQ(vmCvar2.integer, 42);
  EXPECT_STREQ(vmCvar2.string, "42");
}

TEST(SyscallsCvarFake, Update)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar = {};

  fake.Cvar_Register(&vmCvar, "varName", "3.14", CVAR_ARCHIVE_ND);
  fake.Cvar_SetSafe("varName", "42");
  fake.Cvar_Update(&vmCvar);
  EXPECT_EQ(vmCvar.value, 42.f);
  EXPECT_EQ(vmCvar.integer, 42);
  EXPECT_STREQ(vmCvar.string, "42");
}

TEST(SyscallsCvarFake, SetSafe)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar = {};

  fake.Cvar_Register(&vmCvar, "varName", "3.14", CVAR_ARCHIVE_ND);
  fake.Cvar_SetSafe("varName", "42");
  auto const* const cvar = fake.find("varName");
  ASSERT_TRUE(cvar);
  EXPECT_EQ(cvar->value_, 42.f);
  EXPECT_EQ(cvar->integer_, 42);
  EXPECT_EQ(cvar->string_, "42");
  EXPECT_EQ(vmCvar.value, 3.14f);
  EXPECT_EQ(vmCvar.integer, 3);
  EXPECT_STREQ(vmCvar.string, "3.14");
}

TEST(SyscallsCvarFake, VariableStringBufferSafeNoCvar)
{
  SyscallsCvarFake::Impl fake;
  char                   buffer[5] = "3.14";

  fake.Cvar_VariableStringBufferSafe("varName", buffer, sizeof(buffer), CVAR_PRIVATE);
  EXPECT_EQ(std::strlen(buffer), 0u);
}

TEST(SyscallsCvarFake, VariableStringBufferSafeCvarPresent)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar    = {};
  char                   buffer[5] = {};

  fake.Cvar_Register(&vmCvar, "varName", "3.14", CVAR_ARCHIVE_ND);
  fake.Cvar_VariableStringBufferSafe("varName", buffer, sizeof(buffer), CVAR_PRIVATE);
  EXPECT_STREQ(buffer, "3.14");
}

TEST(SyscallsCvarFake, VariableStringBufferSafeWrongCvar)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar    = {};
  char                   buffer[5] = "3.14";

  fake.Cvar_Register(&vmCvar, "varName", "3.14", CVAR_ARCHIVE_ND);
  fake.Cvar_VariableStringBufferSafe("WrongVarName", buffer, sizeof(buffer), CVAR_PRIVATE);
  EXPECT_EQ(std::strlen(buffer), 0u);
}

TEST(SyscallsCvarFake, VariableStringBufferSafePrivateCvar)
{
  SyscallsCvarFake::Impl fake;
  vmCvar_t               vmCvar    = {};
  char                   buffer[5] = "3.14";

  fake.Cvar_Register(&vmCvar, "varName", "3.14", CVAR_PRIVATE);
  fake.Cvar_VariableStringBufferSafe("varName", buffer, sizeof(buffer), CVAR_PRIVATE);
  EXPECT_EQ(std::strlen(buffer), 0u);
}
