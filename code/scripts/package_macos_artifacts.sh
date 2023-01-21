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

      # This is for targets which are built as applications
      if [ -d *.app ]; then
        # remove whitespace, add underscores, remove extension, make lower case
        FILENAME=$(basename *.app .app | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
        zip -r $FILENAME.zip *.app
        mv $FILENAME.zip $BUILD_PATH/artifacts
      fi

      # This is for targets which are built as plugins
      if [ -d $d/*_artefacts/$BUILD_TYPE/VST3 ]; then
        cd $d/*_artefacts/$BUILD_TYPE/VST3
        
        if [ -d *.vst3 ]; then
          # remove whitespace, add underscores, remove extension, make lower case
          FILENAME=$(basename *.vst3 .vst3 | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
          mv *.vst3 $FILENAME.vst3
          mv $FILENAME.vst3 $BUILD_PATH/VST3/$BUILD_TYPE
        fi
      fi

    fi
  fi
done

# Put all VST3's into a single zipped folder
cd $BUILD_PATH/VST3/$BUILD_TYPE
VSTFILENAME="KMSWHDSP_VSTs.zip"
zip -r $VSTFILENAME *.vst3
mv $VSTFILENAME $BUILD_PATH/artifacts
