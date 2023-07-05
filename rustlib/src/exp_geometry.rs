#[repr(C)]
#[allow(non_snake_case)]
// #[derive(Default, Copy, Clone)]
/// Copy of SGPRect from the C codebase.
pub struct GRect {
    iLeft: i32,
    iTop: i32,
    iRight: i32,
    iBottom: i32,
}

// struct Box {
//     left: u16,
//     top: u16,
//     width: u16,
//     height: u16,
// }

#[no_mangle]
/// Create new GRect structure
pub extern "C" fn NewRect(left: i32, top: i32, width: i32, height: i32) -> GRect {
    GRect {
        iLeft: left,
        iTop: top,
        iRight: left + width,
        iBottom: top + height,
    }
}
