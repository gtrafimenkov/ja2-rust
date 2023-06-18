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
 * Get money balance (what is shown on the laptop screen).
 */
int32_t LaptopMoneyGetBalance(void);

/**
 * Set money balance (what is shown on the laptop screen).
 */
void LaptopMoneySetBalance(int32_t value);

/**
 * Add given amount to the money balance
 */
void LaptopMoneyAddToBalance(int32_t amount);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
