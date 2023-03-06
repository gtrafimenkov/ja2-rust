#include "Money.h"

#include "Laptop/LaptopSave.h"

// Get money balance (what is shown on the laptop screen).
i32 MoneyGetBalance() { return LaptopSaveInfo.iCurrentBalance; }

// Get money balance (what is shown on the laptop screen).
void MoneySetBalance(i32 value) { LaptopSaveInfo.iCurrentBalance = value; }
