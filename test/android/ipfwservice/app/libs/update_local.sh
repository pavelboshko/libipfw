#!/bin/bash
RETURN_PATH=$PWD
echo $RETURN_PATH

cd ../../../../../libipfwjni/ || exit 1
sh gradlew assembleDebug || exit 1
cd $RETURN_PATH

cd ../../../../../../../tls-mobile/TLSLib/src/tlsjni/ || exit 1
sh gradlew assembleDebug || exit 1
cd $RETURN_PATH

cp ../../../../../libipfwjni/libipfwjni/build/outputs/aar/libipfwjni-debug.aar . || exit 1
cp ../../../../../../../tls-mobile/TLSLib/src/tlsjni/tlslib/build/outputs/aar/tlslib-debug.aar . || exit 1
echo "copy libs success"

