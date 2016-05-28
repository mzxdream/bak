#!/bin/bash
cd ../../build/login_server && cmake -DCMAKE_BUILD_TYPE=Debug ../../src/login_server && make
