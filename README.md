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
upgrade my very limited embedded systems chops and start making things bleep and boop. From some other projects
I just had a Beaglebone Black lying around and I wanted to use it to learn:

- How to write ALSA audio programs (a Linux audio API)
- How to cross-compile them for an embedded platform
- How to connect an external DAC to get some audio out without a full-blown USB audio interface or HDMI monitor

I had trouble finding articles that spelled this stuff out for a beginner embedded systems programmer,
so now that I got it up an running, here's what worked for me!

## Materials

- Beaglebone Black
- I2S DAC and audio amplifier. [This one from Sparkfun is cheap and works great!](https://www.sparkfun.com/products/14809)
- Speaker (I got a 4" 8ohm speaker)
- Jumper wires and breadboard
- Host computer running Debian Stretch (I used VMWare on macOS, but a native Linux box would have been preferable :) )

I chose I2S (Inter-IC Sound) protocol over USB or SPI mainly because it is yet another technology I have been meaning to learn and it leaves open 
a USB port to be used by another input device (planning to use it for USB MIDI in a future project). That 
said, it turns out I2S output on the Beaglebone Black is actually a little wonky. Unless you use an
external clock, you are limited to the 48kHz family of sample rates ([see here](https://hifiduino.wordpress.com/2014/03/10/beaglebone-black-for-audio/)). 
This was fine for my purposes, but it is an unfortunate drawback. The I2S pins are mainly used for the HDMI output, which 
[this article](https://www.raspberry-pi-geek.com/Archive/2013/02/HDMI-and-the-BeagleBone-Black-Multimedia-Environment) sheds some
helpful light upon.

# Initial Host Setup

## What is cross-compiling and why am I suggesting we do it?

In order to develop an ALSA program on a desktop computer (the "host") and have it run on an embedded device like the Beaglebone (the "target"),
we need to set up a "cross compilation" environment, allowing you to compile your programs on your powerful host that can run on your target. 
Note that cross compilation isn't strictly necessary to develop  applications for embedded Linux - another option is to "compile on target", 
which means you connect to your embedded device over SSH/UART/etc and use `gcc` on the board to compile it there directly. This is, in general,
leagues simpler than cross-compilation, but for larger codebases the process may be time-prohibitive (one time my poor Beaglebone spent 20+ 
hours compiling OpenSSL and curl) or there might simply not be enough memory on the device, causing `gcc` to error out. That's definitely not the case with the 
toy program in this article, but I wanted to describe how to cross compile anyhow so you can apply this process to
work with larger audio applications. It may help to learn more about cross compiling [here](https://landley.net/writing/docs/cross-compiling.html).

## Pick a host Linux distro

Before anything, you need to decide which type of host setup is best for your cross-compilation scenario.
As it turns out, this decision is not particularly straightforward. One thing that is standard at least is that
one will have an easier time using a Linux host -- macOS and Windows are possible as well, but when cross-compiling
for embedded Linux (as opposed to a "bare metal" embedded system), using a non-Linux host further muddies an 
already complicated process. 

From there, if you have chosen to use Linux, you need to choose a Linux distro suitable for cross-compilation. 
When it comes to cross compilation, not all distros are built equal-- historically, Debian was considered
Not Great for cross compilation. However, since Debian Stretch (9.x), [multi-arch](https://wiki.debian.org/Multiarch/HOWTO) 
support has made it a much more viable option, allowing you to install ARM system libraries/headers/compiler/linker 
(a "[cross toolchain](https://elinux.org/Toolchains)") for cross compiling without messing up your native system 
libraries in `/usr/local/lib` etc.

One other consideration to take into account which distro you have on the target system, because obtaining a pre-built 
cross toolchain for an older compiler can be a pain. Tools like Yocto and Docker can make this aspect more manageable, but for me
it worked to specifically use Debian Stretch, since that was installed on my Beaglebone as well -- originally I was using 
Debian Buster (10) and couldn't find a clean way to get multi-arch support for armhf with the specific version of
gcc I needed (6.3.0), and so my program failed to run on the board. I have a gut feeling that I should have been able to
make this work with Debian Buster (or any Linux distro) so feel free to reach out if you have recommendations!
But for now, my approach is just to use the same Linux distro on the host as the board has if possible.

So: assuming you have Debian Stretch on your Beaglebone, go get an [amd64 image of Debian Stretch](https://www.debian.org/distrib/netinst#smallcd)
and install it either in a [virtual machine](https://www.virtualbox.org/) or natively if you are so inclined.

Each distro has it's quirks, and with Debian Stretch specifically I find in most cases the initial setup I need to 
do after the "Wizard" is to [set up my user to be able to do sudo](https://linuxize.com/post/how-to-add-user-to-sudoers-in-debian/) 
and `sudo apt-get install git` to get git, which are also two things you'll need to do to follow this tutorial.


# Make an ALSA Toy

Now that you are set up with a host environment, let's talk about writing a simple ALSA program that we can compile on it.
At first, we will just compile this *on* the host, *for* the host, pretending the target doesn't exist. This gives us confidence
that the program is actually working and producing some bloops. Check out the toy ALSA program [here](https://github.com/jcampbellcodes/alsa-sweep).

You can clone and build the project like so from the command line:

```
cd ~ # optional, navigate wherever you like

git clone https://github.com/jcampbellcodes/alsa-sweep.git

cd alsa-sweep

make
```

This project uses `make`, but the following guidelines will apply to `CMake` and other build systems as well.
Since `CMake`/`make` are used by many IDEs, you should also be able to extrapolate these guidelines to getting set up
with an IDE to do this stuff. The main point is that none of your build scripts should assume a compiler. Use variables 
like ${CC} for C compiler commands and ${CXX} for C++ so that you can use the same build scripts to work with your host
compile toolchain as well as your cross-compiler, which instead of `g++` will be something like `arm-linux-gnueabihf-g++`.

## What does the program do?

Ironically, I don't think the mechanics of this program are directly the main idea. Mainly, I chose ALSA because it
is a prevalent and low-level API that will be easy to use as an example for cross-compiling with Debian multi-arch,
since you will need to install the armhf version when we get to cross compilation.

But in short, this is a small ALSA program that: 
- Opens the default sound device (check out which sound devices you have available by running `aplay -L` from the
command line) and get a handle to it:
```
snd_pcm_open(&handle, gDevice, SND_PCM_STREAM_PLAYBACK, 0)
```

- Sets the hardware and software parameters for the device:
```
snd_pcm_set_params( handle,
                    SND_PCM_FORMAT_FLOAT, // this determines the data type of the audio calculation below
                    SND_PCM_ACCESS_RW_INTERLEAVED,
                    gNumChans,
                    gSampleRate,
                    1, // allow resampling
                    500000)
```
- Generates some audio into `float buffer [gBufferLen];` (a triangle wave that slides up and down, because why not)
- Writes that buffer to the device using the handle:
```
snd_pcm_writei(handle, buffer, gBufferLen);
```

- And closes the device at the end:
```
snd_pcm_close(handle);
```

ALSA is a good API to know if you are going to be working on audio in Linux, however there are higher level APIs
such as [JACK](https://jackaudio.org/) you should consider that can make your life easier for making actual products.

There is a succint but [useful writeup](http://equalarea.com/paul/alsa-audio.html) by the creator of JACK on how to 
get started with simple ALSA applications, as well as [this article](https://soundprogramming.net/programming/alsa-tutorial-1-initialization/).

If you got through this and your computer did some sounds, then you are ready to move on to compiling the 
program for your embedded board!

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
