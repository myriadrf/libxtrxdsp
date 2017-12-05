/*
 * xtrxdsp unit test file
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <xtrxdsp.h>

#include <time.h>

#define F_VALS 1000

static int g_errors = 0;

#define CHECK_I(x, y) do { if (x != y) { fprintf(stderr, "Expected %d (" #x ") got %d (" #y ")!\n", x, y); g_errors++; } } while(0)

void test_xtrxdsp_sc32i_iq16()
{
	float ival[F_VALS];
	float qval[F_VALS];
	int16_t out_v[2*F_VALS];

	int i;
	for (i = 0; i < F_VALS; i++) {
		ival[i] = -100 + 5*i;
		qval[i] = 100 - 4*i;
	}
#ifdef __AVX__
	xtrxdsp_sc32i_iq16_avx(ival, qval, out_v, 1.0, sizeof(out_v));
#else
	xtrxdsp_sc32i_iq16_no(ival, qval, out_v, 1.0, sizeof(out_v));
#endif

	for (i = 0; i < F_VALS; i++) {
		CHECK_I(out_v[2*i],     -100 + 5*i);
		CHECK_I(out_v[2*i + 1],  100 - 4*i);
	}
}

int main(int argc, char** argv)
{
	test_xtrxdsp_sc32i_iq16();

	printf("Total errors: %d\n", g_errors);
	return 0;
}
