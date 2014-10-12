Cross Platform Video Capture
=============================

Cross Platform Video Capture library for Mac, Windows and Linux.

See [Read The Docs](http://video-capture.readthedocs.org/) for the documentation. 

Compiling
=========

 **Linux**

 ````sh
 cd video_capture/build
 ./release.sh
 ````        

 **Windows**
 
 Open a GIT BASH shell!

 ````sh
 cd video_capture/build
 ./release.sh
 ````    

Decklink
========
Currently we're adding support for Decklink capture devices, here are just 
some notes about the development.

````sh

  - You need to download the DeckLink SDK
  - On Linux you use compile with the DeckLinkAPI.cpp and link with libDeckLinkAPI.so which 
    is loaded automatically when you installed desktop video. 
  - Download the Desktop Video AUR package on Arch Linux: https://aur.archlinux.org/packages/decklink/
  - Extract the tarbal 
  - Run `makepkg -s`  (-s installs dependencies)
  - Install with: `sudo pacman -U decklink-10.2.1a1-1-x86_64.pkg.tar.xz`


````

TODO:
=====

 - On Windows we need to set the desired framerate
 - We probably want to pass a device index to getOutputFormats()
