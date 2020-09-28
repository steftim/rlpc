#!/bin/bash

sudo cp libyandexmusic.so /usr/lib
mkdir build && cd build
cmake ../ && make
./clean.sh
