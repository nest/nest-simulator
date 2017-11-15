#!/usr/bin/env bash

set -e

# NEST environment
source /home/nest/nest-install/bin/nest_vars.sh


if [ "$1" = 'notebook' ]; then
    exec jupyter notebook --ip="*" --port=8080 --no-browser
fi

if [ "$1" = 'interactive' ]; then
    read -p "Your python script: " name
	echo Starting: $name

	# Start
	exec /usr/bin/python /home/nest/data/$name
fi

exec "$@"
