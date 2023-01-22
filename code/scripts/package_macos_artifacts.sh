#!/bin/sh

ThisPath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH="$ThisPath/../build"

if [[ $1 == "Debug" ]]; then
  BUILD_TYPE=Debug
else
  BUILD_TYPE=Release
fi

echo '\033[0;34m' "Prepare project for $BUILD_TYPE archive..."
echo '\033[0m'

mkdir -p $BUILD_PATH/artifacts

for d in $BUILD_PATH/applications/*; do
  if [ -d "$d" ]; then
    if [ -d $d/*_artefacts ]; then
      cd $d/*_artefacts/$BUILD_TYPE

      # This is for targets which are built as applications
      if [ -d *.app ]; then
        # remove whitespace, add underscores, remove extension, make lower case
        FILENAME=$(basename *.app .app | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
        mv *.app $FILENAME.app
        zip -r $FILENAME.zip $FILENAME.app

        cp $FILENAME.app $BUILD_PATH/artifacts
        mv $FILENAME.zip $BUILD_PATH/artifacts
      fi

      # This is for targets which are built as plugins
      if [ -d $d/*_artefacts/$BUILD_TYPE/VST3 ]; then
        cd $d/*_artefacts/$BUILD_TYPE/VST3
        
        if [ -d *.vst3 ]; then
          # remove whitespace, add underscores, remove extension, make lower case
          FILENAME=$(basename *.vst3 .vst3 | xargs | sed -e 's/ /_/g' | tr '[:upper:]' '[:lower:]')
          mv *.vst3 $FILENAME.vst3
          cp $FILENAME.vst3 $BUILD_PATH/artifacts
        fi
      fi

    fi
  fi
done

# Put all VST3's into a single zipped folder
cd $BUILD_PATH/VST3/$BUILD_TYPE
mv *.vst3 $BUILD_PATH/artifacts

echo '\033[0;34m' "Output written to $BUILD_PATH/artifacts"
echo '\033[0m'

echo '\033[0;34m' "Done"
echo '\033[0m'

if [[ $BUILD_TYPE == "Release" ]]; then
  # Package all of the applications / plugins into a dmg

  APP_NAME="KMSWHDSP"

  DMG_PATH="$BUILD_PATH/$APP_NAME-v0.0.4.dmg"

  test -f "$DMG_PATH" && rm "$DMG_PATH"

  echo '\033[0;34m' "Creating apple disk image..."
  echo '\033[0m'
  cd $BUILD_PATH/../distribution
  appdmg "macos-dmg-config.json" "$DMG_PATH"
fi

