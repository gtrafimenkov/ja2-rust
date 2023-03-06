build:
	cargo build
	cbindgen --config cbindgen.toml --crate rustlib --output rustlib.h.new
	diff -q rustlib.h.new rustlib.h || mv rustlib.h.new rustlib.h

test:
	cargo test
