#!/bin/bash

for i in `seq 0 4`
do
  printf "\nsl taille 8\n"
	time ./kenken -sl output/8grid$i
done