#!/bin/bash

set -o braceexpand

for file in `ls ./assignment_references/`; 
do 
	echo "$file: "
	eval "idiff ./assignment_{images,references}/$file | egrep 'pixels|RMS'"
done
