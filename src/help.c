#include "help.h"

#include "assert.h"

typedef struct
{
  help_t const* help;
  size_t        size;
} helpTable_t;

static size_t      helpTableIdx = 0;
static helpTable_t helpTable[8];

static void preHelp(cvarKind_t kind, char const* defaultString);
static void postHelp(cvarKind_t kind);

void init_help(help_t const* help, size_t size)
{
  ASSERT_LT(helpTableIdx, ARRAY_LEN(helpTable));
#ifndef NDEBUG
  for (size_t i = 0; i < size; ++i)
  {
    assert(!Q_strncmp(help[i].cvarTable->cvarName, help[i].message[0], (int)strlen(help[i].cvarTable->cvarName)));
  }
#endif
  helpTable[helpTableIdx].help = help;
  helpTable[helpTableIdx].size = size;
  ++helpTableIdx;
}

void cvar_help(char const* cvarName)
{
  for (size_t i = 0; i < ARRAY_LEN(helpTable); ++i)
  {
    for (size_t j = 0; j < helpTable[i].size; ++j)
    {
      help_t const* help = &helpTable[i].help[j];
      if (Q_stricmp(cvarName, help->cvarTable->cvarName)) continue;

      assert(help->message);
      preHelp(help->kind, help->cvarTable->defaultString);
      for (size_t k = 0; k < ARRAY_LEN(help->message); ++k)
      {
        char const* message = help->message[k];
        if (!message) break;
        trap_Print(vaf("^3%s^7\n", message));
      }
      postHelp(help->kind);
      return;
    }
  }

  cvars_with_help();
}

void cvars_with_help(void)
{
  trap_Print("^3There is help for the cvars:^7\n");
  for (size_t i = 0; i < ARRAY_LEN(helpTable); ++i)
  {
    for (size_t j = 0; j < helpTable[i].size; ++j)
    {
      help_t const* help = &helpTable[i].help[j];
      trap_Print(vaf("  ^2%s^7\n", help->cvarTable->cvarName));
    }
  }
}

static void preHelp(cvarKind_t kind, char const* defaultString)
{
  ASSERT_GT(kind, 0);
  if (kind < 16)
  {
    cvarKind_t n     = kind;
    uint8_t    count = 0;
    while (n)
    {
      count += n & 1;
      n >>= 1;
    }
    ASSERT_GE(count, 1);
    ASSERT_LE(count, 4);
    trap_Print(vaf("^2This cvar accepts %u %s:^7\n", count, count == 1 ? "number" : "numbers"));

    float const x = cgs.glconfig.vidWidth / cgs.screenXScale;
    float const y = cgs.glconfig.vidHeight / cgs.screenXScale;
    if (kind & X) trap_Print(vaf("  ^3x^2-coordinate [0,%.0f]^7\n", x));
    if (kind & Y) trap_Print(vaf("  ^3y^2-coordinate [0,%.0f]^7\n", y));
    if (kind & W) trap_Print(vaf("  ^3w^2idth        [0,%.0f]^7\n", x));
    if (kind & H) trap_Print(vaf("  ^3h^2eight       [0,%.0f]^7\n", y));
    return;
  }

  ASSERT_GE(kind, 16);
  switch (kind)
  {
  case BINARY_LITERAL:
    trap_Print("^2This cvar can accept binary-literals - a sequence starting^7\n");
    trap_Print(vaf("^2with ^30b^2 followed by ^31^2's and ^30^2's (e.g. ^3%s^2).^7\n", defaultString));
    trap_Print("^2Enable/disable items by replacing ^3X^2's with ^31^2/^30^2 resp.\n");
    return;
  case RGBA:
    trap_Print("^2This cvar accepts a set of 4 numbers:^7\n");
    trap_Print("  ^3r^2ed   [0,1]^7\n");
    trap_Print("  ^3g^2reen [0,1]^7\n");
    trap_Print("  ^3b^2lue  [0,1]^7\n");
    trap_Print("  ^3a^2lpha [0,1]^7\n");
    return;
  case RGBAS:
    trap_Print("^2This cvar accepts sets of 4 numbers:^7\n");
    trap_Print("  ^3r^2ed   [0,1]^7\n");
    trap_Print("  ^3g^2reen [0,1]^7\n");
    trap_Print("  ^3b^2lue  [0,1]^7\n");
    trap_Print("  ^3a^2lpha [0,1]^7\n");
    return;
  default:
    assert(0);
    return;
  }
}

static void postHelp(cvarKind_t kind)
{
  ASSERT_GT(kind, 0);
  if (kind < 16)
  {
    return;
  }

  ASSERT_GE(kind, 16);
  switch (kind)
  {
  case BINARY_LITERAL:
    trap_Print("^2Examples: github.com/Jelvan1/cgame_proxymod#examples^7\n");
    return;
  case RGBA:
  case RGBAS:
    return;
  default:
    assert(0);
    return;
  }
}
