#!/bin/bash -e

echo ".. generating rust_$1.h"

# Replacing generated headers only when there are actual changes to avoid
# unnecessary rebuild of the C code consuming this header.

cbindgen -q --config cbindgen.toml src/exp_$1.rs --output /tmp/rust_$1.h
diff -q /tmp/rust_$1.h rust_$1.h >/dev/null || mv /tmp/rust_$1.h rust_$1.h
