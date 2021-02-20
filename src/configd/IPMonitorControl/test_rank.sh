#!/bin/sh

while true
do
    ./if_rank_assert -i en0 -r 1 &
    ./if_rank_assert -i en0 -r 2 &
    ./if_rank_assert -i en0 -r 3 &
    ./if_rank_assert -i en0 -r 4 &
    wait
done


