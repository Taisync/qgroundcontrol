- Make the following compatibility modifications to adapt to the streaming of the Taisync remote control:
  1. GStreamer use v1.18.6 on Android
  2. For adapting v1.18.6, there are several steps to change:
     1. replace [gstelement.h](./gstelement.h) to **${GStreamer_ROOT_DIR}/include/gstreamer-1.0/gst/gstelement.h**
     2. add [gstreamer-gl-prototypes-1.0.pc](./gstreamer-gl-prototypes-1.0.pc) to **${GStreamer_ROOT_DIR}/lib/pkgconfig/gstreamer-gl-prototypes-1.0.pc**

- In QT 6.8.3 on Android 13, there is a compatibility issue with full-screen input methods. You can use the following file to replace the one in the QT library (**Optional**):
  - [Qt6Android.jar](Qt6Android.jar)
    - copy this file to QT/6.8.3/android_arm64_v8a/jar/Qt6Android.jar
    - build apk as usual
    - this will cancel the full-screen input