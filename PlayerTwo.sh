#!/bin/bash

RANDOM=$$$(date +%s)

tileSelected=$[ ( $RANDOM % $1 ) ]
echo $tileSelected
