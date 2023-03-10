use std::path::PathBuf;

/// Remove all files in a directory, but not the directory itself or any subdirectories.
pub fn remove_files_in_directory(path: &PathBuf) -> std::io::Result<()> {
    let iter = std::fs::read_dir(path)?;
    for entry in iter {
        let entry = entry?;
        if entry.path().is_file() || entry.path().is_symlink() {
            std::fs::remove_file(entry.path())?
        }
    }
    Ok(())
}
