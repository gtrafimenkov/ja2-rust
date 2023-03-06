# JA2 rust library

## Dev environment setup

### Windows

- install rust
- `rustup target add i686-pc-windows-msvc`
- `cargo install --force cbindgen`

### Linux

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
cargo install --force cbindgen
```

## Docs on integration C and Rust

- https://docs.rust-embedded.org/book/interoperability/rust-with-c.html
- https://github.com/eqrion/cbindgen
