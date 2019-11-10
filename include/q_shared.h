#ifndef Q_SHARED_H
#define Q_SHARED_H

#include <stdint.h>

//=================================================

//=============================================================

typedef uint8_t byte;

typedef enum
{
  qfalse,
  qtrue
} qboolean;

typedef uint32_t qhandle_t;
typedef int32_t  sfxHandle_t;
typedef int32_t  fileHandle_t;
typedef int32_t  clipHandle_t;

#define ARRAY_LEN(x) (sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x) (ARRAY_LEN(x) - 1)

// angle indexes
#define PITCH 0 // up / down
#define YAW 1   // left / right
#define ROLL 2  // fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define MAX_STRING_CHARS 1024  // max length of a string passed to Cmd_TokenizeString
#define MAX_STRING_TOKENS 1024 // max tokens resulting from Cmd_TokenizeString
#define MAX_TOKEN_CHARS 1024   // max length of an individual token

//=============================================

// portable case insensitive compare
int Q_stricmp(char const* s1, char const* s2);
int Q_strncmp(char const* s1, char const* s2, int n);
int Q_stricmpn(char const* s1, char const* s2, int n);

//=============================================

#endif // Q_SHARED_H
