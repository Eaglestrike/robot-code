#!/usr/bin/env sh
rustup target add arm-unknown-linux-gnueabi
rustup component add rustfmt
rustup component add clippy
# rustup component add rls
