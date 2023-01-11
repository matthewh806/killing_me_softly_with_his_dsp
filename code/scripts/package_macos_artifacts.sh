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

cd $BUILD_PATH/VST3/$BUILD_TYPE
VSTFILENAME="KMSWHDSP_VSTs.zip"
zip -r $VSTFILENAME *.vst3
mv $VSTFILENAME $BUILD_PATH/artifacts

# Repeition of above becuase of the extra Standalone dir.
# TODO: Fix the building of this specific app! Its a standalone plugin rn.
PULSAR_PATH="$BUILD_PATH/applications/pulsar/Pulsar_artefacts//$BUILD_TYPE/Standalone"
cd $PULSAR_PATH
# zip -r pulsar.zip "Pulsar Project.app"
if [ -d *.app ]; then
  # remove whitespace, add underscores, remove extension, make lower case
  FILENAME=$(basename *.app .app | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
  zip -r $FILENAME.zip *.app
  mv $FILENAME.zip $BUILD_PATH/artifacts
fi
