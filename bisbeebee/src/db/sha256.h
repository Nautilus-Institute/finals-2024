// //////////////////////////////////////////////////////////
// sha256.h
// Copyright (c) 2014,2015 Stephan Brumme. All rights reserved.
// see http://create.stephan-brumme.com/disclaimer.html
//
#include <string.h>

#pragma once

// define fixed size integer types
#ifdef _MSC_VER
// Windows
typedef unsigned __int8  uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
// GCC
#include <stdint.h>
#endif


/// add arbitrary number of bytes
void add(const void* data, size_t numBytes);

/// return latest hash as 64 hex characters
void getHashString(char* buffer);
/// return latest hash as bytes
void getHash(unsigned char *buffer);

/// restart
void reset();

void sha256sum(const void* data, size_t numBytes, unsigned char* output);

void sha256sumstr(const void* data, size_t numBytes, char* output);

