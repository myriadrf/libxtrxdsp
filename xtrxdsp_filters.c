/*
 * xtrxdsp filters source file
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

#include <errno.h>
#include "xtrxdsp_filters.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>


#define MAX_TAPS 64

typedef enum internal_xtrxdsp_tap_type {
	TT_FLOAT = 0,
	TT_INT16 = 1,
} internal_xtrxdsp_tap_type_t;

static int internal_xtrxdsp_filter_init(const void* taps,
										internal_xtrxdsp_tap_type_t tt,
										unsigned count,
										unsigned decim,
										unsigned inter,
										unsigned max_sps_block,
										xtrxdsp_filter_state_t *out)
{
	unsigned ntaps = MAX_TAPS;
	if (count > MAX_TAPS)
		return -EINVAL;

	if (inter > 2)
		return -EINVAL;

	if (decim > 6)
		return -EINVAL;

	/* this filtering algorithm isn't optimized for this case */
	if (max_sps_block < count)
		return -EINVAL;

	size_t tsz = (tt == TT_FLOAT) ? sizeof(float) : sizeof(int16_t);
	size_t size = ntaps * tsz;
	if (inter == 0) {
		size += ntaps * 4 * tsz;
	} else {
		size += (max_sps_block * (1 << inter) + ntaps * 2) * tsz;
	}
	void* mem;
	if (posix_memalign(&mem, 64, size) < 0)
		return -ENOMEM;

	memset(mem, 0, size);
	memcpy(mem, taps, count * tsz);

	out->history_data = mem + ntaps * tsz;
	out->filter_taps = mem;
	out->decim = decim;
	out->inter = inter;
	out->history_size = 2 * ntaps;

	if (tt == TT_FLOAT) {
		out->func = resolve_xtrxdsp_sc32_conv64();
		if (inter == 1) {
			out->expand_func = resolve_xtrxdsp_b8_expand_x2();
		} else if (inter == 2) {
			out->expand_func = resolve_xtrxdsp_b8_expand_x4();
		} else {
			out->expand_func = NULL;
		}
	} else {
		out->func_int = resolve_xtrxdsp_iq16_conv64();
		if (inter == 1) {
			out->expand_func = resolve_xtrxdsp_b4_expand_x2();
		} else if (inter == 2) {
			out->expand_func = resolve_xtrxdsp_b4_expand_x4();
		} else {
			out->expand_func = NULL;
		}
	}
	return 0;
}

void xtrxdsp_filter_free(xtrxdsp_filter_state_t *out)
{
	if (out->filter_taps) {
		free((void*)out->filter_taps);
	}
	out->filter_taps = NULL;
	out->history_data = NULL;
}

unsigned xtrxdsp_filter_work(xtrxdsp_filter_state_t* state,
							 const float *__restrict indata,
							 float *__restrict outdata,
							 unsigned num_insamples)
{
	/* We need at least number of taps to perform the coonvolution,
	 * it's purely for optimization
	 */
	assert(num_insamples >= state->history_size);
	if (state->inter == 0) {
		memcpy(state->history_data_float + state->history_size,
			   indata,
			   state->history_size * sizeof(float));

		state->func(state->history_data_float,
					state->filter_taps_float,
					outdata,
					2 * state->history_size,
					state->decim);

		state->func(indata,
					state->filter_taps_float,
					outdata + (state->history_size >> state->decim),
					num_insamples,
					state->decim);

		/* store data for the next run */
		memcpy(state->history_data_float,
			   indata + num_insamples - state->history_size,
			   state->history_size * sizeof(float));

		return num_insamples >> state->decim;
	} else {
		state->expand_func((const void*)indata,
							(void*)(state->history_data_float + state->history_size),
							num_insamples >> 1);

		state->func(state->history_data_float,
					state->filter_taps_float,
					outdata,
					(num_insamples << state->inter) + state->history_size,
					state->decim);

		memcpy(state->history_data_float,
				state->history_data_float + (num_insamples << state->inter),
				state->history_size * sizeof(float));

		return (num_insamples << state->inter) >> state->decim;
	}
}

/* UNFORTUNATLY THERE'S TEMPLATE IN C, WE CAN USE MACROS BUT IT'S NASTY */
/* THIS IS COPY PASTED VERSION OF FUNCTION ABOVE */
/* USE THE UPPER FUNCTION AS A MASTER AND REPEAT EVERYTHING HERE */

unsigned xtrxdsp_filter_worki(xtrxdsp_filter_state_t* state,
							  const int16_t *__restrict indata,
							  int16_t *__restrict outdata,
							  unsigned num_insamples)
{
	/* We need at least number of taps to perform the coonvolution,
	 * it's purely for optimization
	 */
	assert(num_insamples >= state->history_size);
	if (state->inter == 0) {
		memcpy(state->history_data_int + state->history_size,
			   indata,
			   state->history_size * sizeof(int16_t));

		state->func_int(state->history_data_int,
						state->filter_taps_int,
						outdata,
						2 * state->history_size,
						state->decim);

		state->func_int(indata,
						state->filter_taps_int,
						outdata + (state->history_size >> state->decim),
						num_insamples,
						state->decim);

		/* store data for the next run */
		memcpy(state->history_data_int,
			   indata + num_insamples - state->history_size,
			   state->history_size * sizeof(int16_t));

		return num_insamples >> state->decim;
	} else {
		state->expand_func((const void*)indata,
						   (void*)(state->history_data_int + state->history_size),
						   num_insamples >> 1);

		state->func_int(state->history_data_int,
						state->filter_taps_int,
						outdata,
						(num_insamples << state->inter) + state->history_size,
						state->decim);

		memcpy(state->history_data_int,
			   state->history_data_int + (num_insamples << state->inter),
			   state->history_size * sizeof(int16_t));

		return (num_insamples << state->inter) >> state->decim;
	}
}

int xtrxdsp_filter_init(const float* taps,
						unsigned count,
						unsigned decim,
						unsigned inter,
						unsigned max_sps_block,
						xtrxdsp_filter_state_t *out)
{
	return internal_xtrxdsp_filter_init((const void*)taps,
										TT_FLOAT,
										count,
										decim,
										inter,
										max_sps_block,
										out);
}

int xtrxdsp_filter_initi(const int16_t* taps,
						 unsigned count,
						 unsigned decim,
						 unsigned inter,
						 unsigned max_sps_block,
						 xtrxdsp_filter_state_t *out)
{
	return internal_xtrxdsp_filter_init((const void*)taps,
										TT_INT16,
										count,
										decim,
										inter,
										max_sps_block,
										out);
}

