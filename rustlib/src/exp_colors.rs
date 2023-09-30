use super::colors;

/// Convert RGB565 value to RGB32.
#[no_mangle]
pub extern "C" fn rgb565_to_rgb32(rgb565: u16) -> u32 {
    colors::rgb565_to_rgb32(rgb565)
}

/// Convert RGB32 value to RGB32.
#[no_mangle]
pub extern "C" fn rgb32_to_rgb565(rgb565: u32) -> u16 {
    colors::rgb32_to_rgb565(rgb565)
}
