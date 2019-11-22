#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#if defined(BUILDING_SHARED)
/* Building shared library. */
#  define EXPORTIMPORT EXPORT
#elif defined(USING_SHARED)
/* Using shared library. */
#  define EXPORTIMPORT IMPORT
#else
/* Building static library. */
#  define EXPORTIMPORT
#endif

#if defined(_MSC_VER)
#  define EXPORT __declspec(dllexport)
#  define IMPORT __declspec(dllimport)
#  define QDECL __cdecl
#else
#  define EXPORT __attribute__((visibility("default")))
#  define IMPORT
#  if defined(__GNUC__)
#    define QDECL __attribute__((__cdecl__))
#  elif defined(__clang__)
#    define QDECL __cdecl
#  else
#    error Unsupported compiler.
#  endif
#endif

#endif // IMPORTEXPORT_H
