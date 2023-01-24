#!/bin/sh 

ThisPath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_PATH="$ThisPath/../build"

if [[ $1 == "Debug" ]]; then
  BUILD_TYPE=Debug
else
  BUILD_TYPE=Release
fi

echo '\033[0;34m' "Performing VST3 validation"
echo '\033[0m'

PATH_TO_VALIDATOR=$BUILD_PATH/bin/$BUILD_TYPE/validator

# Capture all available VSTs
MULTILINE=$($PATH_TO_VALIDATOR \
 -list)

# echo "${MULTILINE}"

IFS='
'
count=0
for item in ${MULTILINE}
do
  count=$((count+1))
done

echo '\033[0;34m' "Found $count VST3s on the system"
echo '\033[0m'

failed_count=0
for item in ${MULTILINE}
do
    if [[ $item == *.vst3 ]]; then
        echo '\033[0;34m' "Testing $item"
        echo '\033[0m'

        $PATH_TO_VALIDATOR -q $item

        if [[ $? > 0 ]]; then
            failed_count=$((failed_count+1))
        fi
    fi
done

echo '\033[0;34m' "Found errors in $failed_count VST3s"
echo '\033[0m'
