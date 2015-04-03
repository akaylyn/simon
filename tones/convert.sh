#!/bin/bash

FILES=orig/*
SOX=$(which sox)
WIN_COUNT=0;
LOSE_COUNT=0;
ROCK_COUNT=0;
BAFF_COUNT=0;
PREFIX=""
CURR_DIR=$(pwd)

echo "Location of sox application: $SOX"
for f in $FILES
do
    echo "Processing $f file..."

    case "$f" in
        *win*)
            PREFIX="10$WIN_COUNT"
            let WIN_COUNT++
            ;;
        *lose*)
            PREFIX="30$LOSE_COUNT"
            let LOSE_COUNT++
            ;;
        *rock*)
            PREFIX="50$ROCK_COUNT"
            let ROCK_COUNT++
            ;;
        *baff*)
            PREFIX="70$BAFF_COUNT"
            let BAFF_COUNT++
            ;;
    esac

    CMD="$SOX \"$f\" -t wav -b 16 \"$CURR_DIR/$PREFIX\\ ${f##*/}\" rate 44100 channels 2 norm -1 silence 1 1 0.01%"
    echo "Processing command: $CMD"
    $CMD
done
