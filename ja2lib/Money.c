#include "Money.h"

#include "Laptop/LaptopSave.h"

// Get money balance (what is shown on the laptop screen).
i32 MoneyGetBalance() { return LaptopSaveInfo._iCurrentBalance; }

// Set money balance (what is shown on the laptop screen).
void MoneySetBalance(i32 value) { LaptopSaveInfo._iCurrentBalance = value; }

// Add given amount to the money balance
void MoneyAddToBalance(i32 amount) { LaptopSaveInfo._iCurrentBalance += amount; }
