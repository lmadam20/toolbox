DTMF
======
Copyright (c) 2016 Leon Adam
(see file 'LICENSE' for licensing information)

Creates DTMF tones with approx. 1 key/s and
outputs the audio data to stdout


Usage
---------------------------------------
**Usage:** `./DTMF <DTMF keys> [-t <duration of tone in ms>] [-b <duration of break in ms>] [-d <duration of dial tone in ms>]`

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
