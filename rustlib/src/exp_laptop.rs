use super::state::STATE;

/// Get money balance (what is shown on the laptop screen).
#[no_mangle]
pub extern "C" fn LaptopMoneyGetBalance() -> i32 {
    unsafe { STATE.laptop.current_balance }
}

/// Set money balance (what is shown on the laptop screen).
#[no_mangle]
pub extern "C" fn LaptopMoneySetBalance(value: i32) {
    unsafe {
        STATE.laptop.current_balance = value;
    }
}

/// Add given amount to the money balance
#[no_mangle]
pub extern "C" fn LaptopMoneyAddToBalance(amount: i32) {
    unsafe {
        STATE.laptop.current_balance += amount;
    }
}
