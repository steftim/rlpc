#!/bin/bash

sudo cp libyandexmusic.so /usr/lib
cmake . && make
./clean.sh