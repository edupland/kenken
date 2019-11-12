#!/bin/bash

./kenken
./kenken -h
./kenken -g3 -u -V
./kenken -g3 -M10
./kenken -g -O~
./kenken -g30 -O*
./kenken -g3 -m5 -M2
./kenken -g3 -o mygrid -o mygrid2
./kenken -g5 -m2 -M3 -V -O+s*/ -o mygrid
./kenken test/simple4.ken -o mygrid2
./kenken -s linear_inequalities_system test/simple4.ken
./kenken -s linear_inequalities_system -v -a test/simple4.ken
./kenken -s equations_system test/simple3.ken
./kenken -s equations_system -a -v test/simple3.ken
./kenken -s inequalities_system test/simple4.ken
./kenken -s back_tracking -a test/hard6.ken -V
./kenken -s back_tracking test/simple4.ken
./kenken -s logic test/simple4.ken
./kenken -s unknown_solver test/simple3.ken
./kenken test/simple3.ken -o mygrid -o mygrid2
./kenken test/simple3.ken test/simple4.ken
./kenken test/wrong_grid.ken
./kenken test/wrong_grid2.ken
./kenken test/wrong_grid3.ken
./kenken test/wrong_grid4.ken
./kenken -v test/simple9.ken
./kenken -a test/multi_sol3.ken
