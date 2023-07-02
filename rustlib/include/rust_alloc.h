#pragma once

/* Warning, this file is autogenerated by cbindgen. Don't modify this manually. */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Allocate memory in Rust.
 * The program will panic if memory cannot be allocated.
 */
uint8_t *RustAlloc(uintptr_t size);

/**
 * Deallocate memory allocated earlier in Rust.
 *
 * # Safety
 *
 * Pass only the pointer returned earlier by RustAlloc.
 * Don't deallocate memory more that once.
 */
void RustDealloc(uint8_t *pointer);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
