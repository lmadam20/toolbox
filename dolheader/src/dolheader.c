/*
Copyright (c) 2016 Leon Adam
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h> //memset

#define u32 uint32_t
#define i8 int8_t

#if defined ENDIANNESS_LITTLE
	#define swap_bytes swap_bytes_u32
#elif defined ENDIANNESS_BIG
	#define swap_bytes dummy
#else
	#include <endian.h>
	#if __BYTE_ORDER == __LITTLE_ENDIAN
		#define swap_bytes swap_bytes_u32
	#elif __BYTE_ORDER == __BIG_ENDIAN
		#define swap_bytes dummy
	#else
		#error Endianness of your Machine is not supported!
	#endif
#endif

u32 dummy(u32 x)
{
	return x;
}

u32 swap_bytes_u32(u32 x)
{
	int y = (x & 0xff) << 24 |
			(x & 0xff00) << 8 |
			(x & 0xff0000) >> 8 |
			(x & 0xff000000) >> 24;
	return y;
}

typedef struct
{
	u32 text_file_offset[7]; // 7 text sections
	u32 data_file_offset[11]; // 11 data sections
	u32 loading_addresses[18];
	u32 section_sizes[18];
	u32 bss_address;
	u32 bss_size;
	u32 entrypoint;
	i8 padding[27]; // padding is from 0xE4 to 0xFF, so 0xFF - 0xEA = 0x1B = 27 bytes
} DOL_Header; //see http://wiibrew.org/wiki/DOL for more detailed description

void printHeader(DOL_Header* header, char* filename, int show_all)
{
	printf("Header of file '%s':\n", filename);
	printf("=================================================\n\n");

	int i;
	for (i = 0; i < 7; i++)
	{
		u32 offset = header->text_file_offset[i];
		u32 size = header->section_sizes[i];
		u32 address = header->loading_addresses[i];

		if (size > 0 || show_all)
		{
			printf("Section text%i:\n", i);
			printf("\t Offset in file: %u (0x%08x)\n", offset, offset);
			printf("\t Size: %u (0x%08x)\n", size, size);
			printf("\t Loading address: 0x%08x\n", address);
		}
	}

	for (i = 0; i < 11; i++)
	{
		u32 offset = header->data_file_offset[i];
		u32 size = header->section_sizes[i + 7];
		u32 address = header->loading_addresses[i + 7];

		if (size > 0 || show_all)
		{
			printf("Section data%i:\n", i);
			printf("\t Offset in file: %u (0x%08x)\n", offset, offset);
			printf("\t Size: %u (0x%08x)\n", size, size);
			printf("\t Loading address: 0x%08x\n", address);
		}
	}

	if (header->bss_size > 0 || show_all)
	{
		printf("Section bss:\n");
		printf("\t Size: %u (0x%08x)\n", header->bss_size, header->bss_size);
		printf("\t Loading address: 0x%08x\n", header->bss_address);
	}

	printf("\nEntrypoint: 0x%08x\n\n", header->entrypoint);
}

void swapEndianness(DOL_Header* header)
{
	int i;
	for (i = 0; i < 7; i++)
	{
		header->text_file_offset[i] = swap_bytes(header->text_file_offset[i]);
	}

	for (i = 0; i < 11; i++)
	{
		header->data_file_offset[i] = swap_bytes(header->data_file_offset[i]);
	}

	for (i = 0; i < 18; i++)
	{
		header->loading_addresses[i] = swap_bytes(header->loading_addresses[i]);
		header->section_sizes[i] = swap_bytes(header->section_sizes[i]);
	}

	header->bss_address = swap_bytes(header->bss_address);
	header->bss_size = swap_bytes(header->bss_size);
	header->entrypoint = swap_bytes(header->entrypoint);
}

int readDolHeader(DOL_Header* header, FILE* file)
{
	int result = fread(header, sizeof(DOL_Header), 1, file);
	if (result != 1) return -1;
	return 0;
}

void usage(char* argv[])
{
	printf("usage: %s [-a] <path to DOL file>\n\t "
			"Where '-a' indicates that all sections should be printed (event empty sections)\n", argv[0]);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		usage(argv);
		return -1;
	}

	int show_all = 0;
	char* file_names[argc - 1];
	int fname_i = 0;
	int file_given = 0;

	int i;
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-a"))
		{
			show_all = 1;
		}
		else
		{
			file_names[fname_i++] = argv[i];
			file_given = 1;
		}
	}

	if (file_given == 0)
	{
		usage(argv);
		return -1;
	}

	for (i = 0; i < fname_i; i++)
	{
		if (file_names[i] != NULL)
		{
			char* file_name = file_names[i];

			FILE* dol_file = NULL;

#if defined(__APPLE__) || defined(__linux__) || defined(__unix__)
			if (strcmp(file_name, "-")) dol_file = fopen(file_name, "r");
			else dol_file = stdin;
#else
			//Not sure whether Windows supports piping...
			dol_file = fopen(file_name, "r");
#endif

			if (!dol_file)
			{
				perror("Opening input file failed");
				return -2;
			}

			DOL_Header* hdr = (DOL_Header*)malloc(sizeof(DOL_Header));
			if (!hdr)
			{
				perror("Allocating header structure in memory failed");
				fclose(dol_file);
				return -3;
			}

			memset(hdr, 0, sizeof(DOL_Header));

			int result = readDolHeader(hdr, dol_file);
			if (result)
			{
				fprintf(stderr, "Failed to read header! (Are you sure your file is a DOL file?)\n");
				fclose(dol_file);
				free(hdr);
			}

			swapEndianness(hdr);
			printHeader(hdr, file_name, show_all);

			free(hdr);

			fclose(dol_file);

		}

	}

	return 0;
}
