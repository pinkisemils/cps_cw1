#!/bin/bash
MT_BINS="openmp.out.static openmp.out.dynamic threaded.out"
BINS="serial.out futures.out"

TIME="/usr/bin/time"
MOCK="threaded.out"
export TIMEFORMAT=%R


# magick from stackoverflow
_onexit() {
    [[ $TMPD ]] && rm -rf "$TMPD"
}

TMPD="$(mktemp -d)"
trap _onexit EXIT

_time_2() {
    "$@" 2>&3
}

_time_1() {
    time _time_2 "$@"
}

_time() {
    declare time_label="$1"
    shift
    exec 3>&2
    _time_1 "$@" 2>"$TMPD/timing.$time_label"
    echo "time[$1-$time_label]"
    cat "$TMPD/timing.$time_label" >> "$1.csv"
}

for bin in $MT_BINS
do
    for it in {1..100}
    do
        _time $it "./$bin"
    done
done


for bin in $BINS
do
    for it in {1..100}
    do
        _time $it "./$bin"
    done
done
