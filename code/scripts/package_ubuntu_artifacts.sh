#!/bin/bash

ThisPath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH="$ThisPath/../build"

if [ -z $1 ]; then
  BUILD_TYPE=Debug
else
  BUILD_TYPE=Release
fi

mkdir -p $BUILD_PATH/artifacts

for d in $BUILD_PATH/applications/*; do
  if [ -d "$d" ]; then
    if [ -d $d/*_artefacts ]; then
      cd $d/*_artefacts/$BUILD_TYPE
      ARCHIVE_PATH=$(pwd)

      for file in $ARCHIVE_PATH/*; do
	if [ -f "$file" ]; then
   	  # remove whitespace, add underscores, remove extension, make lower case
      	  FILENAME=$(basename "$file" | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
      	  cp "$file" $BUILD_PATH/artifacts/$FILENAME
	fi
      done
   fi
  fi
done
