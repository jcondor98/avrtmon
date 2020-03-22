#!/bin/sh

# Extract the name of a unit from its source file name
function unit_name {
  basename "$1" | sed -e 's/\.[ch]//g'
}

# Given a source file, print its dependencies
function unit_deps {
  local unit="$(unit_name $1)"
  for depfile in $(cat $1 | grep '#include "' | cut -d '"' -f 2); do
    local dep="$(unit_name $depfile)"
    [ "$dep" != "$unit" ] && echo $dep
  done
}


( echo 'digraph {'
  for f in $(ls sources/avr/*.c); do
    unit="$(unit_name $f)"
    if [ "$unit" != "main" ]; then
      for dep in $(unit_deps $f); do
        echo "$unit -> $dep;"
      done
    fi
  done
  echo '}'
) | dot -T pdf -o $1
