#!/bin/sh

mkdir -p ./install
for plugin in forest person photo_shoot camera_saver
do
    cd ./$plugin
    mkdir -p build
    cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=../../install
    make
    make install
    cd ../..
done