#!/bin/sh

APP_NAME="Cache"
APP_VERSION="0.0.2"
CURRENT_PATH="$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH="$CURRENT_PATH/dist"
OUTPUT_PATH="$CURRENT_PATH/$APP_NAME"

echo $CURRENT_PATH

echo '\033[0;34m' "Preparing package in $OUTPUT_PATH"
echo '\033[0m'

pyinstaller frequency_domain_CLI.py --onefile --clean --windowed --noconfirm --paths /Users/matthew/Projects/sound/killing_me_softly_with_his_dsp/code/experiments

rm -r $OUTPUT_PATH
mkdir $OUTPUT_PATH
cp $BUILD_PATH/frequency_domain_CLI $OUTPUT_PATH
cp "$CURRENT_PATH/../sounds/daniel_guitar_mono_trimmed.wav" $OUTPUT_PATH
cp "$CURRENT_PATH/../sounds/secret_message.wav" $OUTPUT_PATH
cp "$CURRENT_PATH/instructions.txt" $OUTPUT_PATH
cp "$CURRENT_PATH/changelog.txt" $OUTPUT_PATH

echo '\033[0;34m' "Zipping package ready for distribution"
echo '\033[0m'

cd "$OUTPUT_PATH"

ZIP_NAME="$APP_NAME-v$APP_VERSION-macOS.zip"
test -f $ZIP_NAME && rm $ZIP_NAME
zip -r "../$ZIP_NAME" *

echo '\033[0;34m' "Done"
echo '\033[0m'