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
 * usage: ./DTMF <dtmf_keys> [-t <duration of tone in ms>] [-b <duration of break in ms>] [-d <duration of dial tone in ms>]
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

#define SAMPLE_RATE 44100
#define MAX_TONE_DURATION 44100
#define MAX_BREAK_DURATION 44100

#define DTONE_TYPE_EUROPE_STANDARD 1

void usage(char* prog_name)
{
	fprintf(stderr, "usage: %s <dtmf key(s)> [-t <duration of tone in ms>] [-b <duration of break in ms>] [-d <duration of dial tone in ms>]\n", prog_name);
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

int dial_tone(void* sample_buffer, int dtype, int duration_samples)
{
	int i;
	int16_t* buf = (int16_t*)sample_buffer;

	for (i = 0; i < duration_samples; i++)
	{
		if (dtype == DTONE_TYPE_EUROPE_STANDARD)
			*buf = sine_t(i, 30000, 425, SAMPLE_RATE);
		buf++;
	}

	buf += 4096;

	return 0;
}

int dtmf(char* keys, void* sample_buffer, int tone_duration, int break_duration)
{
	int ret = 0;
	int keys_length = strlen(keys);

	if (!areValidKeys(keys, keys_length))
	{
		fprintf(stderr, "Key string is invalid!\n");
		return -1;
	}

	int i;
	int freqLow;
	int freqHigh;
	int16_t* buf = (int16_t*)sample_buffer;

	for (i = 0; i < keys_length; i++)
	{
		getFrequencies(keys, i, &freqLow, &freqHigh);
		int s;
		for (s = 0; s <= ceil(SAMPLE_RATE / 1000 * tone_duration); s++)
		{
			int16_t lowSample = sine_t(s, 30000, freqLow, SAMPLE_RATE); // Create the low frequency sample
			int16_t highSample = sine_t(s, 30000, freqHigh, SAMPLE_RATE); // Create the high frequency sample
			int16_t sample = mix(lowSample, highSample); // Mix both

			*buf = sample; // Save
			buf++; // Move to next sample
		}

		// Not need to set it to zero here (buffer has already been
		// cleared by memset(...) in main)
		buf += (int16_t)ceil(SAMPLE_RATE / 1000 * break_duration);
	}

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		usage(argv[0]);
		return -1;
	}

	int tone_duration = 750;
	int break_duration = 250;
	int dial_tone_duration = 1000;
	int dtone_type = DTONE_TYPE_EUROPE_STANDARD;

	int i;

	char* keystring = NULL;

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-t"))
		{
			i++;
			if (i >= argc)
			{
				fprintf(stderr, "No tone duration value given!\n");
				usage(argv[0]);
				return -1;
			}
			tone_duration = atoi(argv[i]);
			if (tone_duration < 1 || tone_duration > MAX_TONE_DURATION)
			{
				fprintf(stderr, "Tone duration is out of range (valid range: %i to %i)\n", 1, MAX_TONE_DURATION);
				usage(argv[0]);
				return -1;
			}
		}
		else if (!strcmp(argv[i], "-b"))
		{
			i++;
			if (i >= argc)
			{
				fprintf(stderr, "No break duration value given!\n");
				usage(argv[0]);
				return -1;
			}
			break_duration = atoi(argv[i]);
			if (break_duration < 1 || break_duration > MAX_BREAK_DURATION)
			{
				fprintf(stderr, "Break duration is out of range (valid range: %i to %i)\n", 1, MAX_BREAK_DURATION);
				usage(argv[0]);
				return -1;
			}
		}
		else if (!strcmp(argv[i], "-d"))
		{
			i++;
			if (i >= argc)
			{
				fprintf(stderr, "No dial tone duration value given!\n");
				usage(argv[0]);
				return -1;
			}
			dial_tone_duration = atoi(argv[i]);
			if (dial_tone_duration < 1)
			{
				fprintf(stderr, "Dial tone duration is out of range (valid range: %i to whatever)\n", 1);
				usage(argv[0]);
				return -1;
			}
		}
		else
		{
			keystring = argv[i];
		}
	}

	if (!keystring)
	{
		fprintf(stderr, "No keystring given!\n");
		usage(argv[0]);
		return -1;
	}

	long num_dtone_samples = floor(dial_tone_duration / 1000) * 44100 + ceil(SAMPLE_RATE / 1000 * (dial_tone_duration % 1000));
	long sample_buffer_size = (strlen(keystring) * ceil((SAMPLE_RATE / 1000 * 
					(tone_duration + break_duration))) + num_dtone_samples) * 2 + 8192; 	// Sample buffer size =
														// Number of keys * Length of Tone with break *
								                        			// + Additional Padding


	void* sample_buffer = malloc(sample_buffer_size);
	if (!sample_buffer)
	{
		fprintf(stderr, "Couldn't allocate sample buffer! (sample_buffer_size = %lu)\n", sample_buffer_size);
		return -2;
	}

	memset(sample_buffer, 0, sample_buffer_size); // Clear the buffer

	int ret = dial_tone(sample_buffer, dtone_type, num_dtone_samples);
	if (ret)
	{
		fprintf(stderr, "Couldn't generate dial tone...\n");
		free(sample_buffer);
		return ret - 2;
	}

	ret = dtmf(keystring, sample_buffer + num_dtone_samples * 2, tone_duration, break_duration);
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
