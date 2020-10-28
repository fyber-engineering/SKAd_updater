#!/usr/bin/env bash
set -x

cmake -S . -B build

sleep 2
cmake --build build --target format
cmake --build build --target fix-format
cmake --build build --clean-first --target skad_updater -- -j 6

sleep 2
ls -la build/src/skad_updater

cmake --build build --target tests_run -- -j 6

sleep 2

cd build/tests/ || exit

./tests_run

echo "Done"