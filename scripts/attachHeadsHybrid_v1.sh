#!/bin/bash

# To run this script on Windows, simply call "./pullAllSubmodulesDev_v1.sh"

echo 
echo [This script will checkout the dev branch of the 4 typical submodules]
echo 

echo flexsea-comm...
cd flexsea-comm
git checkout dev
cd ..
echo 

echo flexsea-system...
cd flexsea-system
git checkout dev
cd ..
echo 

echo flexsea-shared...
cd flexsea-shared
git checkout dev
cd ..
echo 

echo flexsea-projects...
cd flexsea-projects
git checkout master
cd ..
echo 

echo [Done]
echo 