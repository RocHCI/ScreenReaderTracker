#!/bin/bash

for img in imgs/*.jpg
do
    echo "$img"
    ./rgmRect "$img"
done
