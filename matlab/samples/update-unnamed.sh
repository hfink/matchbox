#!/bin/bash
for f in named/*.wav
do
  unnamed="`echo $f | sed -e 's:named/\(..\)-.*-\(.\):\1-\2:g'`"
  ln -sf $f $unnamed
done
