#include <stddef.h>
#include <stdint.h>
#include "quickjs-libc.h"

#define EXPORT __attribute__((visibility("default")))

int qjs_run_bytecode(uint8_t* bytecode, size_t bytecode_size, int argc, char **argv);
