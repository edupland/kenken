#!/bin/bash

src_path=$PWD

printf "%b\n" "\nCommands used for coverage tests:\n"
tests_len=$(wc -l < ../test/check/coverage_tests.sh)
tail -n $((tests_len - 2)) ../test/check/coverage_tests.sh
printf "\n"

for DIR in $(find . -type d); do
	cd $src_path/$DIR && make coverage >> /dev/null 2>&1
	cd $src_path/.. && make cover_tests >> /dev/null 2>&1
	cd $src_path
	gcov $(find $DIR -maxdepth 1 -mindepth 1 -type f -name "*.c")
	printf "\n"
	for FILE in $(find . -name "*.gcov"); do
		cp $FILE ../test/coverage
	done
	find . -name "*.gc*" | xargs rm
done

printf "%b\n" "Coverage .gcov files in ./test/coverage\n"
