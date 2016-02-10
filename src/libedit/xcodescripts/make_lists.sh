#!/bin/sh

set -x
cd local
S=$SRCROOT/src
sh $S/makelist -h $S/vi.c >vi.h
sh $S/makelist -h $S/emacs.c >emacs.h
sh $S/makelist -h $S/common.c >common.h
sh $S/makelist -bh $S/vi.c $S/emacs.c $S/common.c >help.h
sh $S/makelist -fc vi.h emacs.h common.h >fcns.c
sh $S/makelist -fh vi.h emacs.h common.h >fcns.h
sh $S/makelist -bc $S/vi.c $S/emacs.c $S/common.c >help.c
sh $S/makelist -n $S/tokenizer.c >tokenizern.c
sh $S/makelist -n $S/history.c >historyn.c

