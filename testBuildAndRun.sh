#!/bin/bash

PREFIX=/tmp/amsynth_test

./configure --prefix=$PREFIX
if [ $? -ne 0 ]; then echo "Error: ./configure failed"; exit 1; fi

make clean all
if [ $? -ne 0 ]; then echo "Error: build failed"; exit 1; fi

sudo make install
if [ $? -ne 0 ]; then echo "Error: installation failed"; exit 1; fi

$PREFIX/bin/amSynth &
amsynth_pid=$!
# give it time to start & maybe crash
sleep 2
ps -p $amsynth_pid
if [ $? -ne 0 ]; then echo "Error: run amSynth failed"; exit 1; fi

echo "****"
echo "**** Looks good!"
echo "****"

