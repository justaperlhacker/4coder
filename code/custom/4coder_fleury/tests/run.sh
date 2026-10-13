#!/bin/bash
# Build and run the standalone lexer test suite.
#
#   ./run.sh         # normal run, exit 1 on any failure
#   ./run.sh -v      # verbose: dumps token stream for failing cases

set -e
cd "$(dirname "$0")"

g++ -std=c++11 -w -o lexer_tests lexer_tests.cpp
./lexer_tests "$@"
