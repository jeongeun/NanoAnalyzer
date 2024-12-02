#!/bin/bash
echo "Check if TTY"
if [ "`tty`" != "not a tty" ]; then
  echo "DO NOT RUN THIS IN INTERACTIVE MODE, IT DELETES LOCAL FILES"
else
  echo "ENVIRONMENT"
  env
  echo "VOMS PROXY INFO"
  voms-proxy-info -all
  echo "STARTING CRAB SCRIPT"
  python3 crab_script.py $1
fi
