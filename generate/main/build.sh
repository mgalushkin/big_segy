#!/bin/bash

mkdir ../build

cmake -S . -B ../build
make -C ../build