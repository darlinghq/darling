#!/bin/bash

for folder in "$@"
do
	find $folder -type d -exec chmod 775 {} \;
	find $folder -type f -exec chmod 664 {} \;
done
