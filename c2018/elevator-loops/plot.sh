#!/usr/bin/env sh
export NUM_COLS="$(awk '{print NF}' "$1" | sort -nu | tail -n 1)"
# assumes time is the first column
gnuplot -e "set key autotitle columnhead; plot for [col=2:$NUM_COLS] '$1' using 1:col" -
