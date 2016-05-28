#!/bin/bash
cd ../../build/game_server && cmake -DCMAKE_BUILD_TYPE=Debug ../../src/game_server && make
