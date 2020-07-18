# alsa sweep example

This repo is for a [blog post](https://www.jackcampbellsounds.com/2020/07/01/developingembeddedlinuxaudioapplications.html) about an approach for writing ALSA apps for 
embedded linux (beaglebone black used as an example)

It will involve cross-compiling this code using Debian multiarch
by running
```
make CXX=arm-linux-gnueabihf
```

Copying the binary to the board

Hooking up an I2S DAC to the beaglebone and a speaker

hearing the sweep!
