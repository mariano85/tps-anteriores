#!/bin/bash

#export ANSISOP_PATH=/home/utnso/workspace/Programa/programa.config"
export ANSISOP_PATH=$(pwd)/programa.config
echo "Seteo variable de entonrno ANSISOP_PATH OK!"
echo $ANSISOP_PATH
exec $SHELL -i
