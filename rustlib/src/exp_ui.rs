use super::state::STATE;

#[no_mangle]
pub extern "C" fn GetMapPanelDirty() -> bool {
    unsafe { STATE.ui.map_panel_dirty }
}

#[no_mangle]
pub extern "C" fn SetMapPanelDirty(value: bool) {
    unsafe { STATE.ui.map_panel_dirty = value }
}

#[no_mangle]
pub extern "C" fn GetMapScreenBottomDirty() -> bool {
    unsafe { STATE.ui.map_screen_bottom_dirty }
}

#[no_mangle]
pub extern "C" fn SetMapScreenBottomDirty(value: bool) {
    unsafe { STATE.ui.map_screen_bottom_dirty = value }
}
