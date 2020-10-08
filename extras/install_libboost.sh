#!/bin/bash

# Install the boost library
wget --no-verbose https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz
tar -xzf boost_1_72_0.tar.gz
rm -fr boost_1_72_0.tar.gz
cp -fr boost_1_72_0 $HOME/.cache/boost_1_72_0.install
rm -fr boost_1_72_0
