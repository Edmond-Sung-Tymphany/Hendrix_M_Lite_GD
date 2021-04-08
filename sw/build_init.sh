#! /bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

printf "%s\n" "-----------------------"
printf "%s\n" "Copyright Tymphany 2013"
printf "%s\n" "-----------------------"
printf "%s\n" "Wiki page: http://sw.tymphany.com/redmine/projects/freertos/wiki"
printf "%s\n" "-----------------------"

if [ -e "$DIR/scripts/build_steps.py" ]; then
    python $DIR/scripts/build_steps.py
    source exports
    command rm exports
else
    printf "%s\n" "Fatal: \"build_steps.py\" not found."
fi

printf "%s\n" "-----------------------"
