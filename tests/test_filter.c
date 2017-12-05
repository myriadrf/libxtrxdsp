/*
 * xtrxdsp filter test file
 * Copyright (c) 2017 Sergey Kostanbaev <sergey.kostanbaev@fairwaves.co>
 * For more information, please visit: http://xtrx.io
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <xtrxdsp_filters.h>
#include <stdio.h>
#include <unistd.h>


int main(int argc, char** argv)
{
	int res;
	unsigned decim = 0;
	unsigned inter = 0;
	unsigned blocksize = 8192;

	xtrxdsp_filter_state_t state;

	FILE* fin = stdin;
	FILE* fout = stdout;
	int opt;
	int integer_mode = 0;
	int filter_scale = 32767;
	float conv_scale = 32767;
	float r_conv_scale;

	while ((opt = getopt(argc, argv, "m:i:o:d:I:b:F:f:")) != -1) {
		switch (opt) {
		case 'f':
			conv_scale = atof(optarg);
			break;
		case 'F':
			filter_scale = atoi(optarg);
			break;
		case 'm':
			integer_mode = atoi(optarg);
			break;
		case 'i':
			fin = fopen(optarg, "rb+");
			break;
		case 'o':
			fout = fopen(optarg, "wb+");
			break;
		case 'd':
			decim = atoi(optarg);
			break;
		case 'I':
			inter = atoi(optarg);
			break;
		case 'b':
			blocksize = atoi(optarg);
			break;
		default: /* '?' */
			fprintf(stderr, "Usage: %s [-i input] [-o output] [-d decimation] [-b blocksize] [-m integer_mode]\n",
					argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	r_conv_scale = 1 / conv_scale;

	if (integer_mode) {
		/* make sure we're using the same filter but in diferrent mode */
		int16_t int_filter_taps[FILTER_TAPS_64];
		for (unsigned i = 0; i < FILTER_TAPS_64; i++) {
			int_filter_taps[i] = filter_scale * g_filter_float_taps_64_2x[i];
		}

		res = xtrxdsp_filter_initi(int_filter_taps,
								  FILTER_TAPS_64,
								  decim,
								  inter,
								  blocksize,
								  &state);
	} else {
		res = xtrxdsp_filter_init(g_filter_float_taps_64_2x,
								  FILTER_TAPS_64,
								  decim,
								  inter,
								  blocksize,
								  &state);
	}
	if (res)
		return 1;

	fin = freopen(NULL, "rb", fin);
	if (fin == NULL)
		exit(EXIT_FAILURE);

	if (integer_mode == 0) {
		const unsigned bs = blocksize;
		float inb[bs];
		float outb[bs * (1 << inter)];
		size_t rsz, wsz;
		unsigned out_samples;

		for (;;) {
			rsz = fread(inb, 1, bs * sizeof(float), fin);
			if (rsz != bs * sizeof(float)) {
				fprintf(stderr, "read %u\n", (unsigned)rsz);
				return 2;
			}

			out_samples = xtrxdsp_filter_work(&state, inb, outb, bs);

			wsz = fwrite(outb, 1, out_samples * sizeof(float), fout);
			if (wsz != out_samples * sizeof(float))
				return 3;
		}
	} else if (integer_mode == 2) {
		const unsigned bs = blocksize;
		float inb[bs];
		float outb[bs * (1 << inter)];
		int16_t t_inb[bs];
		int16_t t_outb[bs * (1 << inter)];
		size_t rsz, wsz;
		unsigned out_samples;
		unsigned i;

		for (;;) {
			rsz = fread(inb, 1, bs * sizeof(float), fin);
			if (rsz != bs * sizeof(float)) {
				fprintf(stderr, "read %u\n", (unsigned)rsz);
				return 2;
			}

			for (i = 0; i < bs; i++) {
				t_inb[i] = inb[i] * conv_scale;
			}
			out_samples = xtrxdsp_filter_worki(&state, t_inb, t_outb, bs);
			for (i = 0; i < out_samples; i++) {
				outb[i] = t_outb[i] * r_conv_scale;
			}

			wsz = fwrite(outb, 1, out_samples * sizeof(float), fout);
			if (wsz != out_samples * sizeof(float))
				return 3;
		}
	} else {
		const unsigned bs = blocksize;
		int16_t inb[bs];
		int16_t outb[bs * (1 << inter)];
		size_t rsz, wsz;
		unsigned out_samples;

		for (;;) {
			rsz = fread(inb, 1, bs * sizeof(int16_t), fin);
			if (rsz != bs * sizeof(int16_t)) {
				fprintf(stderr, "read %u\n", (unsigned)rsz);
				return 2;
			}

			out_samples = xtrxdsp_filter_worki(&state, inb, outb, bs);

			wsz = fwrite(outb, 1, out_samples * sizeof(int16_t), fout);
			if (wsz != out_samples * sizeof(int16_t)) {
				fprintf(stderr, "write %u\n", (unsigned)wsz);
				return 3;
			}
		}
	}

	return 0;
}
