#!/bin/sh

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

      if [ -d *.app ]; then
        # remove whitespace, add underscores, remove extension, make lower case
        FILENAME=$(basename *.app .app | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
        zip -r $FILENAME.zip *.app
        mv $FILENAME.zip $BUILD_PATH/artifacts
      fi
    fi
  fi
done
