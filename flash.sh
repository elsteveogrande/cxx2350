#!/usr/bin/env bash

set -euxo pipefail

EX=$1
ELF="build/examples/${EX}.elf"
make $ELF
echo "program ${ELF} verify reset" | nc localhost 4444

