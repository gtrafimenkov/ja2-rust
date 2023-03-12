use super::state::STATE;

#[no_mangle]
pub extern "C" fn GetMapPanelDirty() -> bool {
    unsafe { STATE.ui.map_panel_dirty }
}

#[no_mangle]
pub extern "C" fn SetMapPanelDirty(value: bool) {
    unsafe { STATE.ui.map_panel_dirty = value }
}
