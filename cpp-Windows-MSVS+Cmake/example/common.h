/**
 * Copyright (C) 2014 Daniel Turecek
 *
 * @file      common.h
 * @author    Daniel Turecek <daniel@turecek.de>
 * @date      2014-07-16
 *
 * Common file defining basic types, constatns and
 * usefull macros. Used almost in every source file
 * in the software.
 *
 */

#ifndef COMMON_H
#define COMMON_H

#define NO_DBGTRACE

#include <cmath>
#include <cstdint>
#include <cstdlib>

// basic data types
typedef int BOOL;
typedef uint8_t byte;
typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;
typedef size_t size;

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifdef _MSC_VER
#define NOMINMAX
#endif

#define THREADID std::thread::id

#ifdef _WIN64
#define WIN32_LEAN_AND_MEAN
#define PATH_SEPAR '\\'
#define PATH_SEPAR_STR "\\"
#define PATH_SEPAR_STRW L"\\"
#else
#include <stdint.h>
typedef void *HMODULE;
typedef void *HINSTANCE;
#define MAXDWORD 0xffffffff
#define PATH_SEPAR '/'
#define PATH_SEPAR_STR "/"
#endif

#define MAX_U32 0xffffffff

typedef enum _DataType
{
    DT_CHAR = 0,    // signed char
    DT_BYTE = 1,    // unsigned char
    DT_I16 = 2,     // signed short
    DT_U16 = 3,     // unsigned short
    DT_I32 = 4,     // int
    DT_U32 = 5,     // unsigned int
    DT_I64 = 6,     // long long
    DT_U64 = 7,     // unsigned long long
    DT_FLOAT = 8,   // float
    DT_DOUBLE = 9,  // double
    DT_BOOL = 10,   // BOOL (int)
    DT_STRING = 11, // const char *
    DT_PATH = 12,
    DT_LAST = 13,   // end of table
} DataType;

typedef unsigned DEVID;
typedef unsigned DATAID;
typedef unsigned EVENTID;
typedef unsigned FRAMEID;
typedef unsigned MENUITEMID;
typedef u64 THREADPARAM;

#define PXUNUSED(x) (void)x;
#define PXMIN(a, b) (((a) < (b)) ? (a) : (b))
#define PXMAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef _WIN64
#define EXPORTFUNC extern "C" __attribute__((visibility("default")))
#else
#define EXPORTFUNC extern "C" __declspec(dllexport)
#endif

#ifdef _WIN64
#define _STR(x) #x
#define STR(x) _STR(x)
#define TODO(x) __pragma(message("TODO: " _STR(x) " :: " __FILE__ "@" STR(__LINE__)))
#else
#define DOPRAGMA(x) _Pragma(#x)
#define TODO(x) DOPRAGMA(GCC warning #x)
#endif

inline bool cmpf(double a, double b, double epsilon = 0.0001)
{
    return std::abs(a - b) < epsilon;
}

#endif /* end of include guard: COMMON_H */
