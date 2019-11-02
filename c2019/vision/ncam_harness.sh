#!/usr/bin/env bash

set -euo pipefail

./nannycam.py |& tee "nannycam-log-$(date +'%FT%T')"
