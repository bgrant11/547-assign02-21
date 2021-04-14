# assignment-2-bgrant11

auto.sh
	runs a series of small tests
	produced by gen.sh

auto2.sh 
	runs a series of large tests
	produced by gen2.sh

in <- directory
	contains input and scripts to make input

in/i <- directory
	contains inputs that are used in makefile tests

in/exhaust <- directory
	contains scripts to generate an exhaustive amount of inputs

gen.sh
	creates a bunch of smaller inputs

gen2.sh
	creates a bunch of large inputs

move.sh
	a script to move a bunch of inputs that were not moved correctly

in/exhaust/tests <- directory 
	contains all generated input files

