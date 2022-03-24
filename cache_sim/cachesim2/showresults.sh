#!/bin/sh

for group in basic writeback multibyte multitier
do
    right=0
    wrong=0
    for f in keys/$group*
    do
        if diff -q $f results/$(basename $f) >/dev/null
        then echo ' ' ' OK  ' $f
            right=$((right+1))
        else
            echo ' ' 'WRONG' $f
            wrong=$((wrong+1))
        fi
    done

    echo $group: passed $right / $((right+wrong)) tests >&2
done
