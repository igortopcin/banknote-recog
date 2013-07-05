Bank-note Recog
===============

This is an IME-USP master's programme project for MAC5768 - Computer Vision and Image Processing. Our goal is to build an Android application to recognize Brazilian bank notes. The project must be built on top of OpenCV and Android SDK.

Ubuntu/Debian Installation instructions
=======================================

Open CV
-------

Please read http://docs.opencv.org/doc/tutorials/introduction/linux_install/linux_install.html for detailed instructions on how to install OpenCV on Linux. The following is a cut-down version of that guide.

Install the following packages:

    sudo apt-get install build-essential
    sudo apt-get install cmake
    sudo apt-get install pkg-config
    sudo apt-get install libgtk2.0-dev
    sudo apt-get install libavformat-dev
    sudo apt-get install libavcodec-dev
    sudo apt-get install libswscale-dev
    sudo apt-get install ffmpeg

Then download OpenCV's source code from http://sourceforge.net/projects/opencvlibrary/. Unpack it and enter the directory where the source code is.

For example, if you unpacked it in your home directory, do

    cd ~/opencv-2.4.5

Now you should create a directory for cmake build files, and then call cmake:

    mkdir release
    cd release
    cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..

And finally call make to build OpenCV:

    make
    sudo make install


Eclipse
-------

Get and install Eclipse ADT bundle from http://developer.android.com/sdk/index.html. Simply download and extract it somewhere in your computer.
The location you unpacked it will be called <adt-bundle> in this guide.
To start Eclipse, simply run <adt-bundle>/eclipse/eclipse

OpenCV for Android
------------------

Follow the instructions in this page: http://docs.opencv.org/doc/tutorials/introduction/android_binary_package/O4A_SDK.html
Read the section "Manual OpenCV4Android SDK setup".

There is also another very good guide: http://docs.opencv.org/doc/tutorials/introduction/android_binary_package/android_dev_intro.html

Basically, you will have to download OpenCV for Android and extract it to <opencv-sdk>.
Add <opencv-sdk>/sdk/java to your Eclipse's workspace. This is a very important step in the process.


NDK
---

You should download http://developer.android.com/tools/sdk/ndk/index.html
Unpack it to a directory of your choice <ndk>. Now go to Window->Preferences->Android->NDK and enter the directory <ndk> you chose.

Add the following environment variables to your system: ANDROID_SDK_HOME, ANDROID_NDK_HOME, OPENCV_ANDROID_SDK_HOME. In Linux, it would look something like this:

    export ANDROID_SDK_HOME=<adt-bundle>/sdk
    export ANDROID_NDK_HOME=<ndk>
    export OPENCV_ANDROID_SDK_HOME=<opencv-sdk>
    
Then you should add $ANDROID_SDK_HOME/tools, $ANDROID_SDK_HOME/platform-tools and $ANDROID_NDK_HOME to your path.

    export PATH=$PATH:$ANDROID_SDK_HOME/tools:$ANDROID_SDK_HOME/platform-tools:$ANDROID_NDK_HOME


Compiling and Building the Project
==================================

First get the source code of this project at https://github.com/igortopcin/banknote-recog.

Desktop part
-------------

After checking out the code, do the following in a command line prompt:

    cd jni
    mkdir bin
    mkdir bin/results
    cd bin
    
Use cmake to build the desktop part of the project.

    cmake ../
    make
    
You should now have two executables: match and train

Use "train" to train the Matcher that will perform the banknote recognition:

    ./train ../../assets
    
This will train all the images listed in <project-root>/assets/classtraining.txt. This txt file has the followint format:

    <path-to-image-1.jpg> <tag associated to that image-1>
    <path-to-image-2.jpg> <tag associated to that image-2>
    ...
    <path-to-image-N.jpg> <tag associated to that image-N>
    
All paths are relative to <project-root>/assets.

Executing ./train will produce yml files in <project-root>/assets, which will then be copied to the android .apk by Eclipse ADT.

Now use "match" to try out the algorithm:

    ./match ../../assets ../../img/test/img1.jpg
    
This will try to match img1.jpg to all the other images listed in <project-root>/assets/classtraining.txt. The program 
will output some images to the <project-root>/jni/bin/results directory.

Android part
------------

Open Eclipse and choose "Import...", then "Import existing project into workspace". Follow the steps in the import wizard and you should have a working copy of the project in your Eclipse.
The project itself is a pretty standard Eclipse ADT project.

Note that you may have to perform a few adjustments due to difference in the chosen paths of <ndk>, <adt-bundle> and <opencv-sdk>.

Perform a full build by selecting Project >> "Build All" or "Build Project".

Redirecting C++ stdout to Android's logger
------------------------------------------

The native part of the Android has some important logs that are written to the stdout and stderr.
However, Android ignores those and writes them to /dev/null. In order to view them in the 
Android's logger, you should connect your device in debugger mode and do:

    adb shell stop
    adb shell setprop log.redirect-stdio true
    adb shell start

Future Work
===========

The time to develop this project was quite short, and there has been some difficulties arround making OpenCV Extractors/Detectors/Matchers work in Android.
The Android app we built is quite simple, and the intent was to show an example of how Android devices can be used to carry out Computer Vision tasks.

Known Issues
============

Sometimes the matching algorithms produce different results in different computers, even using the same set of training and test/query images.
This issue remains unresolved, and in some Android devices the results can be different from others.
