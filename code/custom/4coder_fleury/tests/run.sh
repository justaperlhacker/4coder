#!/bin/bash
# Build and run the standalone lexer + ctags-parser test suites.
#
#   ./run.sh         # normal run, exit 1 on any failure
#   ./run.sh -v      # verbose: PASS lines and full token dumps on lexer failures

set -e
cd "$(dirname "$0")"

g++ -std=c++11 -w -o lexer_tests lexer_tests.cpp
g++ -std=c++11 -w -o tags_tests  tags_tests.cpp

rc=0
./lexer_tests "$@" || rc=1
echo
./tags_tests "$@" || rc=1
exit $rc
