/*

Copyright 2016 Leon Adam

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

 */

/*
 * dtmf.c: Creates DTMF tones with a approx. 1 key/s and
 * outputs the audio data to stdout
 *
 * usage: ./<executable_name> <dtmf_keys>
 *
 * Example usage:
 * ./DTMF 0123456789ABCD*# > test.raw
 *
 *
 * To convert the samples to a usable audio format, use
 * tools like ffmpeg.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

void usage(char* prog_name)
{
	fprintf(stderr, "usage: %s [dtmf key(s)]\n", prog_name);
}

int isValidKey(char key)
{
	/*
	 * 0x30 to 0x39 = 0 ... 9
	 * 0x41 to 0x44 = A ... D
	 * 0x2a = *
	 * 0x23 = #
	 */
	return (key >= 0x30 && key <= 0x39) || (key >= 0x41 && key <= 0x44) || key == 0x2a || key == 0x23;
}

int areValidKeys(char* keys, int keys_length)
{
	int i;
	int valid = 1;
	for (i = 0; i < keys_length; i++)
	{
		if (valid) valid = isValidKey(*((char*)keys + i));
	}
	return valid;
}

void getFrequencies(char* keys, int i, int* freqLow, int* freqHigh)
{
	char key = *((char*)keys + i);
	*freqLow = 100;
	*freqHigh = 3000;

	if (key == '1' || key == '2' || key == '3' || key == 'A') *freqLow = 697;
	if (key == '4' || key == '5' || key == '6' || key == 'B') *freqLow = 770;
	if (key == '7' || key == '8' || key == '9' || key == 'C') *freqLow = 852;
	if (key == '*' || key == '0' || key == '#' || key == 'D') *freqLow = 941;

	if (key == '1' || key == '4' || key == '7' || key == '*') *freqHigh = 1209;
	if (key == '2' || key == '5' || key == '8' || key == '0') *freqHigh = 1336;
	if (key == '3' || key == '6' || key == '9' || key == '#') *freqHigh = 1477;
	if (key == 'A' || key == 'B' || key == 'C' || key == 'D') *freqHigh = 1633;
}

int sine_t(double t, int A, int f, int rate)
{
	return A * sin((2.0 * M_PI * f * t) / rate);
}

/*
 * Mixes sample a and b
 * by calculating the average of both.
 */
int mix(int a, int b)
{
	return (a + b) / 2;
}

int dtmf(char* keys, void* sample_buffer)
{
	int ret = 0;
	int keys_length = strlen(keys);

	if (!areValidKeys(keys, keys_length))
	{
		fprintf(stderr, "Key string is invalid!\n");
		ret = -1;
	}

	int i;
	int freqLow;
	int freqHigh;
	int16_t* buf = (int16_t*)sample_buffer;

	for (i = 0; i < keys_length; i++)
	{
		getFrequencies(keys, i, &freqLow, &freqHigh);
		int s;
		for (s = 0; s <= 33075; s++) // 33075 equals to 3/4 seconds
		{
			int16_t lowSample = sine_t(s, 30000, freqLow, 44100); // Create the low frequency sample
			int16_t highSample = sine_t(s, 30000, freqHigh, 44100); // Create the high frequency sample
			int16_t sample = mix(lowSample, highSample); // Mix both

			*buf = sample; // Save
			buf++; // Move to next sample
		}

		// Not need to set it to zero here (buffer has already been
		// cleared by memset(...) in main)
		buf += 11025;
	}

	return ret;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		usage(argv[0]);
		return -1;
	}

	long sample_buffer_size = strlen(argv[1]) * 2 * 44100 + 8192; // Sample buffer size = Number of keys * Length of Sample *
	                                                              // Samplerate + Additional Padding

	void* sample_buffer = malloc(sample_buffer_size);
	if (!sample_buffer)
	{
		fprintf(stderr, "Couldn't allocate sample buffer!\n");
		return -2;
	}

	memset(sample_buffer, 0, sample_buffer_size); // Clear the buffer

	int ret = dtmf(argv[1], sample_buffer);
	if (ret)
	{
		fprintf(stderr, "Couldn't generate tones...\n");
		free(sample_buffer);
		return ret - 2;
	}

	fwrite(sample_buffer, sample_buffer_size, 1, stdout); // Write the raw data to stdout

	free(sample_buffer);

	return 0;
}
