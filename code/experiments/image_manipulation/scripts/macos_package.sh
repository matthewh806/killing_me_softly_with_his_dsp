#!/bin/sh

APP_NAME="SoxMosh"
APP_VERSION="0.0.3"
CURRENT_PATH="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH="$CURRENT_PATH/../dist"
OUTPUT_PATH="$CURRENT_PATH/../$APP_NAME"

echo $CURRENT_PATH

echo '\033[0;34m' "Preparing package in $OUTPUT_PATH"
echo '\033[0m'

pyinstaller soxmosh_cli.py --onefile --clean --windowed --noconfirm

rm -r $OUTPUT_PATH
mkdir $OUTPUT_PATH
cp $BUILD_PATH/soxmosh_cli $OUTPUT_PATH
cp "$CURRENT_PATH/../README.md" $OUTPUT_PATH
cp "$CURRENT_PATH/../input_images/perfect_blue_face.bmp" $OUTPUT_PATH
cp "$CURRENT_PATH/../input_json/example_effects.json" $OUTPUT_PATH
cp "$CURRENT_PATH/../instructions.txt" $OUTPUT_PATH
cp "$CURRENT_PATH/../changelog.txt" $OUTPUT_PATH

echo '\033[0;34m' "Zipping package ready for distribution"
echo '\033[0m'

cd "$OUTPUT_PATH"

ZIP_NAME="$APP_NAME-v$APP_VERSION-macOS.zip"
test -f $ZIP_NAME && rm $ZIP_NAME
zip -r "../$ZIP_NAME" *

echo '\033[0;34m' "Done"
echo '\033[0m'