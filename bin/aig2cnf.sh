#!/bin/sh

for file in *.aig;
do
name="${file%.*}"
tee abc_script <<EOF
	read_aiger $name.aig
	write_cnf $name.cnf
EOF
./abc -f abc_script
done
