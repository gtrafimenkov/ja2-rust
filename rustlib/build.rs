use std::env;
use std::path::PathBuf;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let src_dir = PathBuf::from(&crate_dir).join("src");
    let config = cbindgen::Config::from_file("cbindgen.toml").unwrap();

    // select src/exp_*.rs files for processing
    let mut files = Vec::new();
    for entry in std::fs::read_dir(src_dir).unwrap() {
        let entry = entry.unwrap();
        if entry.path().is_file() {
            let file_name = entry.file_name().to_string_lossy().to_string();
            if file_name.starts_with("exp_") && file_name.ends_with(".rs") {
                files.push(entry.path());
            }
        }
    }

    for file in files {
        let new_name = file.file_name().unwrap().to_string_lossy().to_string();
        let new_name = new_name.replace("exp_", "rust_").replace(".rs", ".h");
        let dest = PathBuf::from(&crate_dir).join("include").join(new_name);
        cbindgen::Builder::new()
            .with_config(config.clone())
            .with_src(file)
            .generate()
            .unwrap()
            .write_to_file(dest);
    }
}
