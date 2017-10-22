#!/bin/sh

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

cd "$parent_path"

mkdir -p bin
mkdir -p out

function run_test {
	f=$1
	TEST_SUCCESS="true"
	NAME=$(basename "$f" .bjou)
	../bin/bjou "test/$NAME.bjou" -o "bin/$NAME"
	if [ $? -ne 0 ]
  	then
    	TEST_SUCCESS="false"
  	fi

	if [ "$TEST_SUCCESS" == "true" ]
	then
		"bin/$NAME" > "out/$NAME.txt"
		if [ $? -ne 0 ]
  		then
    		TEST_SUCCESS="false"
  		fi

		DIFF=$(diff "out/$NAME.txt" "check/$NAME.txt")
		if [ "$DIFF" != "" ] 
		then
			TEST_SUCCESS="false"
		fi
	fi
	COLOR=`tput setaf 2`
	RESET=`tput sgr0`
	if [ "$TEST_SUCCESS" == "false" ]
	then
		COLOR=`tput setaf 1`
	fi
	echo ${COLOR}$NAME${RESET}
}

if [ "$#" -eq 0 ]
then
	for f in test/*.bjou;
	do
		run_test $f
	done
else
	run_test $1
fi




