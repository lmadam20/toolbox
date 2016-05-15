DTMF
======
Copyright (c) 2016 Leon Adam
(see file 'LICENSE' for licensing information)

Creates DTMF tones with a approx. 1 key/s and
outputs the audio data to stdout


Usage
---------------------------------------
**Usage:** `./DTMF <DTMF keys>` 

Example usage:
---------------------------------------
`./DTMF 0123456789ABCD*# > test.raw`

Notes
---------------------------------------
To convert the samples to a usable audio format, use
tools like ffmpeg.

Compiling
---------------------------------------
With GCC: `gcc -o DTMF dtmf.c -lm`

TODO
---------------------------------------
* Adjustable tone speed
* Implement dial tone
