#! /usr/bin/env bash

DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

find $DIR/../a4{atlas,hist,io,process,root} -iname "*.cpp" -o -iname "*.h" > $DIR/source-list

uncrustify --no-backup --replace -c $DIR/uncrustify.cnf -F $DIR/source-list
