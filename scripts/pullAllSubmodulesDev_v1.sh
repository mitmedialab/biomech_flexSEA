#!/bin/bash

# To run this script on Windows, simply call "./pullAllSubmodulesDev_v1.sh"

echo 
echo [This script will pull the latest version of the 4 typical submodules]
echo 

echo flexsea-comm...
cd flexsea-comm
git pull
cd ..
echo 

echo flexsea-system...
cd flexsea-system
git pull
cd ..
echo 

echo flexsea-shared...
cd flexsea-shared
git pull
cd ..
echo 

echo flexsea-projects...
cd flexsea-projects
git pull
cd ..
echo 

echo [Done]
echo 