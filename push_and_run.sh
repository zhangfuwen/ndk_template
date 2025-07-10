adb logcat -b events -c; adb logcat -b events 
adb push ./build/android-arm64/ndk_binder_example /data/local/tmp/
adb shell /data/local/tmp/ndk_binder_example
sleep 1
fg
