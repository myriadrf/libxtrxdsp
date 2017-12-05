/*
 * Public xtrxdsp filters header file
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

#include <stdint.h>
#include <xtrxdsp.h>

#define FILTER_TAPS_120 120
#define FILTER_TAPS_40  40

#define FILTER_TAPS_64  64

/*
 * 0    - 0.2     0.06 db
 * 0.25 - 1     -61.32 dB
 */
extern const int16_t g_filter_taps_120_4x[FILTER_TAPS_120];


/*
 *  0   - 0.25     0.00 db
 *  0.5 - 1      -84.04 db
 */
extern const int16_t g_filter_taps_40_long_2x_4x[FILTER_TAPS_40];


/*
 * 0    - 0.05    0.01 db
 * 0.25 - 1      -75.5 db
 */
extern const int16_t g_filter_taps_40_long_4x_16x[FILTER_TAPS_40];


/*
 * 0   - 0.4    0.37 db
 * 0.5 - 1       -46 db
 */
extern const int16_t g_filter_taps_40_2x[FILTER_TAPS_40];
extern const int16_t g_filter_taps_40_2x_q[FILTER_TAPS_40];


extern const int16_t g_filter_int16_taps_64_2x[FILTER_TAPS_64];
extern const float g_filter_float_taps_64_2x[FILTER_TAPS_64];


typedef struct xtrxdsp_filter_state {
	union {
		void* history_data;
		float* history_data_float;
		int16_t* history_data_int;
	};
	union {
		const void* filter_taps;
		const float* filter_taps_float;
		const int16_t* filter_taps_int;
	};
	unsigned history_size; // In floats
	unsigned decim;
	unsigned inter;
	union {
		func_xtrxdsp_sc32_conv64_t func;
		func_xtrxdsp_iq16_conv64_t func_int;
	};
	func_xtrxdsp_bx_expand_t expand_func;
} xtrxdsp_filter_state_t;

/**
 * @brief xtrxdsp_filter_init Initializes FIR filter and pushes zeros as history
 *                            data
 * @param taps Filter taps (doesn't have to be aligned to SMID vector size)
 * @param count Number of filter taps
 * @param decim Decimation rate at output (2^decim)
 * @param inter Interpolation rate before filtering (2^inter)
 * @param max_sps_block Max samples per block
 * @param out Structure to initialize
 * @return 0 - success, -errno on error
 */
int xtrxdsp_filter_init(const float* taps,
						unsigned count,
						unsigned decim,
						unsigned inter,
						unsigned max_sps_block,
						xtrxdsp_filter_state_t *out);

int xtrxdsp_filter_initi(const int16_t* taps,
						 unsigned count,
						 unsigned decim,
						 unsigned inter,
						 unsigned max_sps_block,
						 xtrxdsp_filter_state_t *out);

void xtrxdsp_filter_free(xtrxdsp_filter_state_t *out);

unsigned xtrxdsp_filter_work(xtrxdsp_filter_state_t* state,
							 const float *__restrict indata,
							 float *__restrict outdata,
							 unsigned num_insamples);

unsigned xtrxdsp_filter_worki(xtrxdsp_filter_state_t* state,
							  const int16_t *__restrict indata,
							  int16_t *__restrict outdata,
							  unsigned num_insamples);
