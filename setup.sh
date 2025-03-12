#!/bin/bash

# Source only necessary components
source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh || { echo "Failed to source LCG setup"; exit 1; }

# Set installation directory
export INSTALL_PATH="$PWD/install"

# Update paths efficiently
export PATH="$INSTALL_PATH/lib:$PATH"
export LD_LIBRARY_PATH="$INSTALL_PATH/lib:$LD_LIBRARY_PATH"
export YAMLPATH="/cvmfs/sft.cern.ch/lcg/releases/yamlcpp/0.6.3-d05b2/x86_64-centos7-gcc12-opt/lib"

# Optimize correctionlib path retrieval
CORRECTION_DIR="/cvmfs/sft.cern.ch/lcg/releases/correctionlib/2.2.2-18bbf/x86_64-centos7-gcc12-opt"

export CORRECTION_PATH="${CORRECTION_DIR}/include"
export CORRECTION_LIBPATH="${CORRECTION_DIR}/lib"

echo $CORRECTION_PATH
echo $CORRECTION_LIBPATH

# Use cached JSONPOG path if possible
export JSONPOG_INTEGRATION_PATH="/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration"
