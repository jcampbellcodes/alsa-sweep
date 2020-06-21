# alsa sweep example

This repo is for a blog post about an approach for writing ALSA apps for 
embedded linux (beaglebone black used as an example)

It will involve cross-compiling this code using Debian multiarch
by running
```
make CXX=arm-linux-gnueabihf
```

Copying the binary to the board

Hooking up an I2S DAC to the beaglebone and a speaker

hearing the sweep!


# Writing and Cross Compiling Audio Programs For Embedded Linux

This post is more of a journal entry masquerading as a tutorial. For some time, I have wanted to
upgrade my very limited embedded systems chops and start making things beep. From some other projects
I had a Beaglebone Black lying around and I wanted to use it to learn:
- How to write ALSA programs (a Linux audio API)
- How to cross-compile for an embedded platform
- How to connect an external DAC to get some audio out without a full-blown USB audio interface or HDMI monitor

I had trouble finding articles that spelled this stuff out for a beginner embedded systems programmer,
so now that I got it up an running, here's what worked for me!

## Materials

- Beaglebone Black
- I2S DAC and audio amplifier. [This one from Sparkfun is cheap and works great!](https://www.sparkfun.com/products/14809)
- Speaker (I got a 4" 8ohm speaker)
- Jumper wires and breadboard
- Host computer running Debian Stretch (I used VMWare on macOS, but a native Linux box would have been preferable :) )

I chose I2S mainly because it is yet another technology I have been meaning to learn and it leaves open 
a USB port to be used by another input device (planning to use it for USB MIDI in a future project). That 
said, it turns out I2S output on the Beaglebone Black is actually a little wonky. Unless you use an
external clock, you are limited to the 48kHz family of sample rates ([see here](https://hifiduino.wordpress.com/2014/03/10/beaglebone-black-for-audio/)). This was fine for my purposes, but it is an
unfortunate drawback. The I2S pins are mainly used for the HDMI output, which [this article](https://www.raspberry-pi-geek.com/Archive/2013/02/HDMI-and-the-BeagleBone-Black-Multimedia-Environment) sheds some
helpful light upon.

# Initial Host Setup

Copped out: chose a Linux distro that has the same std libs as the host
This article won’t go over these details, but there are ways to sandbox/create a custom linux 
distro (Yocto is popular, Docker images may also work for cross compilation)
My Beaglebone happened to have Debian 9 on it, so I used a Debian 9 host. When I used Debian 10, 
I had trouble getting an older gcc cross compile toolchain for Beaglebone -- the one with
arm-linux-gnueabihf was too new and created binaries that expected the wrong C libs

debian stretch uses gcc 6.3.0

Versioning has been a huge headache, and due to the fact that Yocto exists, I think that’s 
just the way things are...

Setting up a new system:
Get debian buster amd64
Make your user a superuser
Install and set up git

All told, I used VMWare on a Mac to do this.


# Make an ALSA Toy

Hosted here:
https://github.com/jcampbellcodes/alsa-sweep

Quickly go over what the program does
The point of it in this article is just that its an API that can be useful for embedded linux audio apps, and it already exists on the beaglebone
Consider something higher level like JACK or audio middleware layer
Just wanted to have an example that uses an audio API and doesn’t just shell out
Alsa still needs multiarch to cross compile so it is ironically a simple example

Compile your program on the host first to make sure it is all set
Just uses a makefile, since that can be extrapolated to figure out what to do with CMake or other build systems. The main point is that your build script doesn’t assume a compiler, so that you can use CC or CXX to switch out the host compiler for your cross compile toolchain


# Cross Compilation

What is a cross compile toolchain?

Note that this program is small enough that you really could compile it “on target” by moving over the sources to the board. But I personally had a project I wanted to cross compile

## Set up the toolchain

Install cross toolchain stuff:
Determine your architecture
Research the board. Run a command. For beaglebone it’s armhf
Describe rpi?
```
sudo dpkg --add-architecture armhf
sudo apt update 
sudo apt install crossbuild-essential-armhf
sudo apt install <libname>:armhf
```
get alsa
`sudo apt-get install libasound2-dev:armhf`
Sometimes for libs you may need to compile yourself.
Do the correct configure, make, make install


## Compile the program

ADAPT this code to use the cmake stuff obvs, rather than pac.
My code just needs CXX, but including the CC line as well since you might need that if you are compiling 
C programs
`CC=arm-linux-gnueabihf-gcc CXX=arm-linux-gnueabihf-g++ make`

 NOTE: may have to chmod +x when it’s on the board

# Set up the Beaglebone Black

- show the regular docs
- we are assuming the board is "factory reset"

# Hooking up the Components

Mention Device Tree Overlays
Beaglebone as of this writing enables I2S pins by default since they are used by the HDMI functionality
Show the video on device trees and where in the hdmi device tree the I2S stuff is specified

DAC, speaker, beaglebone pins

BBB Description	Header pin      	Description
SPI1_CS0        	P9_28           	Bitstream
SPI1_D0         	P9_29           	Left/Right clock
SPI1_SCLK       	P9_31           	Bit clock
GND                 P9_00 (check)
VC3V3               P9_03

- a note on device tree overlays

- do an alsa speaker test

# Make a sound!

- install your program with scp
- might have to chmod it
- run an you should hear the siren!
