#! /usr/bin/env bash


ASSERT_PC=1 BRK=0xe029 ./run.sh romtest/cpu_dummy_reads.nes

if [[ $? -eq '2' ]]; then
  exit 0
else
  exit 1
fi


