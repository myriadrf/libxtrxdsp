/*
 * xtrxdsp source file
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

#include "xtrxdsp.h"

typedef void (*func_xtrxdsp_iq16_sc32_t)(const int16_t *__restrict,float *__restrict, float, size_t);
typedef uint64_t (*func_xtrxdsp_iq12_sc32_t)(const void *__restrict,float *__restrict, size_t, uint64_t prevstate);
typedef void (*func_xtrxdsp_iq8_sc32_t)(const int8_t *__restrict,float *__restrict, size_t);
typedef void (*func_xtrxdsp_iq8_ic16_t)(const int8_t *__restrict,int16_t *__restrict, size_t);

typedef void (*func_xtrxdsp_iq16_sc32i_t)(const int16_t *__restrict, float *__restrict, float *__restrict, float, size_t);
typedef void (*func_xtrxdsp_iq16_ic16i_t)(const int16_t *__restrict, int16_t *__restrict, int16_t *__restrict, size_t);

typedef void (*func_xtrxdsp_iq8_sc32i_t)(const int8_t *__restrict,float *__restrict,float *__restrict, size_t);
typedef void (*func_xtrxdsp_iq8_ic16i_t)(const int8_t *__restrict,int16_t *__restrict,int16_t *__restrict, size_t);
typedef void (*func_xtrxdsp_iq8_ic8i_t)(const int8_t *__restrict,int8_t *__restrict,int8_t *__restrict, size_t);

typedef void (*func_xtrxdsp_sc32_iq16_t)(const float *__restrict, int16_t *__restrict, float, size_t);
typedef void (*func_xtrxdsp_sc32i_iq16_t)(const float *__restrict i, const float *__restrict, int16_t *__restrict, float, size_t);

typedef void (*func_xtrxdsp_ic16i_iq16_t)(const int16_t *__restrict i, const int16_t *__restrict, int16_t *__restrict, size_t);

#ifndef NOPRINT
#include <stdio.h>
#define INFORM(x, ...) fprintf(stderr, x, ##__VA_ARGS__)
#else
#define INFORM(x, ...)
#endif

#include <stdbool.h>

#if defined(__x86_64__) || defined(__i386__)
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)

#if GCC_VERSION > 50000
#define RUNTIME_CHECK_FMA()  __builtin_cpu_supports("fma");
#else
#define RUNTIME_CHECK_FMA()  0
#endif

typedef struct cpu_features {
	bool sse2;
	bool sse41;
	bool avx;
	bool fma;
} cpu_features_t;

static void cpu_features_init(cpu_features_t* features)
{
	__builtin_cpu_init();

	features->sse2 = __builtin_cpu_supports("sse2");
	features->sse41 = __builtin_cpu_supports("sse4.1");
	features->avx = __builtin_cpu_supports("avx");
	features->fma = RUNTIME_CHECK_FMA();

	INFORM("CPU Features: SSE2%c SSE4.1%c AVX%c FMA%c\n",
		   features->sse2 ? '+' : '-',
		   features->sse41 ? '+' : '-',
		   features->avx ? '+' : '-',
		   features->fma ? '+' : '-');
}

#elif defined(__arm__) || defined(__aarch64__)
typedef struct cpu_features {
	bool neon;
} cpu_features_t;

static void cpu_features_init(cpu_features_t* features)
{
	features->neon = false; //TODO
}
#else

#warning Unknown platform!

typedef struct cpu_features {
	bool _nothing
} cpu_features_t

static void cpu_features_init(cpu_features_t* features)
{
}

#endif

static bool s_cpu_features_init = false;
static cpu_features_t s_cpu_features;

void xtrxdsp_init(void)
{
	if (!s_cpu_features_init) {
		cpu_features_init(&s_cpu_features);
		s_cpu_features_init = true;
	}
}


#define STRINGIFY2(x) #x
#define STRINGIFY(x)  STRINGIFY2(x)

#define SELECT_FUNC(method, func, suffix) \
	do { \
		INFORM("Using " method " for " STRINGIFY(func) "\n"); \
		return func##_##suffix;	\
	} while (0)


#define CHECK_FUNC_BODY_EX2(func, suffix, gccstring, gccstring2) \
	do { \
		if (s_cpu_features.gccstring && s_cpu_features.gccstring2) { \
			SELECT_FUNC(STRINGIFY(suffix), func, suffix); \
		} \
	} while (0)

#define CHECK_FUNC_BODY_EX(func, suffix, gccstring) \
	do { \
		if (s_cpu_features.gccstring) { \
			SELECT_FUNC(STRINGIFY(suffix), func, suffix); \
		} \
	} while (0)

#define CHECK_FUNC_BODY(func, suffix) \
	CHECK_FUNC_BODY_EX(func, suffix, suffix)


#ifdef XTRXDSP_HAS__AVX2__
#define CHECK_FUNC_AVX2(func) CHECK_FUNC_BODY(func, avx2)
#else
#define CHECK_FUNC_AVX2(func)
#endif

#if defined(XTRXDSP_HAS__AVX__) && defined(XTRXDSP_HAS__FMA__)
#define CHECK_FUNC_AVX_FMA(func) CHECK_FUNC_BODY_EX2(func, avx_fma, avx, fma)
#else
#define CHECK_FUNC_AVX_FMA(func)
#endif

#ifdef XTRXDSP_HAS__AVX__
#define CHECK_FUNC_AVX(func) CHECK_FUNC_BODY(func, avx)
#else
#define CHECK_FUNC_AVX(func)
#endif

#ifdef XTRXDSP_HAS__SSE4_1__
#define CHECK_FUNC_SSE4_1(func) CHECK_FUNC_BODY(func, sse41)
#else
#define CHECK_FUNC_SSE4_1(func)
#endif

#ifdef XTRXDSP_HAS__SSSE3__
#define CHECK_FUNC_SSSE3(func) CHECK_FUNC_BODY(func, ssse3)
#else
#define CHECK_FUNC_SSSE3(func)
#endif

#ifdef XTRXDSP_HAS__SSE2__
#define CHECK_FUNC_SSE2(func) CHECK_FUNC_BODY(func, sse2)
#else
#define CHECK_FUNC_SSE2(func)
#endif

#if defined(__x86_64__) || defined(__i386__)

static func_xtrxdsp_iq16_sc32_t resolve_xtrxdsp_iq16_sc32(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq16_sc32);
	CHECK_FUNC_AVX(xtrxdsp_iq16_sc32);
	CHECK_FUNC_SSE2(xtrxdsp_iq16_sc32);
	SELECT_FUNC("generic", xtrxdsp_iq16_sc32, no);
}


static func_xtrxdsp_iq12_sc32_t resolve_xtrxdsp_iq12_sc32(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq12_sc32);
	CHECK_FUNC_AVX(xtrxdsp_iq12_sc32);
	CHECK_FUNC_SSSE3(xtrxdsp_iq12_sc32);
	CHECK_FUNC_SSE2(xtrxdsp_iq12_sc32);
	SELECT_FUNC("generic", xtrxdsp_iq12_sc32, no);
}


static func_xtrxdsp_iq8_sc32_t resolve_xtrxdsp_iq8_sc32(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq8_sc32);
	CHECK_FUNC_AVX(xtrxdsp_iq8_sc32);
	CHECK_FUNC_SSE4_1(xtrxdsp_iq8_sc32);
	CHECK_FUNC_SSE2(xtrxdsp_iq8_sc32);
	SELECT_FUNC("generic", xtrxdsp_iq8_sc32, no);
}

static func_xtrxdsp_iq8_ic16_t resolve_xtrxdsp_iq8_ic16(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX(xtrxdsp_iq8_ic16);
	CHECK_FUNC_SSE2(xtrxdsp_iq8_ic16);
	SELECT_FUNC("generic", xtrxdsp_iq8_ic16, no);
}

static func_xtrxdsp_iq16_sc32i_t resolve_xtrxdsp_iq16_sc32i(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq16_sc32i);
	CHECK_FUNC_AVX(xtrxdsp_iq16_sc32i);
	CHECK_FUNC_SSE2(xtrxdsp_iq16_sc32i);
	SELECT_FUNC("generic", xtrxdsp_iq16_sc32i, no);
}

static func_xtrxdsp_iq16_ic16i_t resolve_xtrxdsp_iq16_ic16i(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq16_ic16i);
	CHECK_FUNC_AVX(xtrxdsp_iq16_ic16i);
	CHECK_FUNC_SSE2(xtrxdsp_iq16_ic16i);
	SELECT_FUNC("generic", xtrxdsp_iq16_ic16i, no);
}

static func_xtrxdsp_iq8_sc32i_t resolve_xtrxdsp_iq8_sc32i(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq8_sc32i);
	CHECK_FUNC_AVX(xtrxdsp_iq8_sc32i);
	CHECK_FUNC_SSE4_1(xtrxdsp_iq8_sc32i);
	CHECK_FUNC_SSE2(xtrxdsp_iq8_sc32i);
	SELECT_FUNC("generic", xtrxdsp_iq8_sc32i, no);
}

static func_xtrxdsp_iq8_ic16i_t resolve_xtrxdsp_iq8_ic16i(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq8_ic16i);
	CHECK_FUNC_AVX(xtrxdsp_iq8_ic16i);
	CHECK_FUNC_SSE2(xtrxdsp_iq8_ic16i);
	SELECT_FUNC("generic", xtrxdsp_iq8_ic16i, no);
}

static func_xtrxdsp_iq8_ic8i_t resolve_xtrxdsp_iq8_ic8i(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_iq8_ic8i);
	CHECK_FUNC_AVX(xtrxdsp_iq8_ic8i);
	CHECK_FUNC_SSE2(xtrxdsp_iq8_ic8i);
	SELECT_FUNC("generic", xtrxdsp_iq8_ic8i, no);
}

static func_xtrxdsp_sc32_iq16_t resolve_xtrxdsp_sc32_iq16(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_sc32_iq16);
	CHECK_FUNC_AVX(xtrxdsp_sc32_iq16);
	CHECK_FUNC_SSE2(xtrxdsp_sc32_iq16);
	SELECT_FUNC("generic", xtrxdsp_sc32_iq16, no);
}

static func_xtrxdsp_sc32i_iq16_t resolve_xtrxdsp_sc32i_iq16(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_sc32i_iq16);
	CHECK_FUNC_AVX(xtrxdsp_sc32i_iq16);
	CHECK_FUNC_SSE2(xtrxdsp_sc32i_iq16);
	SELECT_FUNC("generic", xtrxdsp_sc32i_iq16, no);
}

static func_xtrxdsp_ic16i_iq16_t resolve_xtrxdsp_ic16i_iq16(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX2(xtrxdsp_ic16i_iq16);
	CHECK_FUNC_AVX(xtrxdsp_ic16i_iq16);
	CHECK_FUNC_SSE2(xtrxdsp_ic16i_iq16);
	SELECT_FUNC("generic", xtrxdsp_ic16i_iq16, no);
}

func_xtrxdsp_sc32_conv64_t resolve_xtrxdsp_sc32_conv64(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX_FMA(xtrxdsp_sc32_conv64);
	CHECK_FUNC_AVX(xtrxdsp_sc32_conv64);
	CHECK_FUNC_SSE2(xtrxdsp_sc32_conv64);
	SELECT_FUNC("generic", xtrxdsp_sc32_conv64, no);
}

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x2(void)
{
	//CHECK_FUNC_AVX(xtrxdsp_b8_expand_x2);
	//CHECK_FUNC_SSE2(xtrxdsp_b8_expand_x2);
	SELECT_FUNC("generic", xtrxdsp_b8_expand_x2, no);
}

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x4(void)
{
	//CHECK_FUNC_AVX(xtrxdsp_b8_expand_x4);
	//CHECK_FUNC_SSE2(xtrxdsp_b8_expand_x4);
	SELECT_FUNC("generic", xtrxdsp_b8_expand_x4, no);
}

func_xtrxdsp_iq16_conv64_t resolve_xtrxdsp_iq16_conv64(void)
{
	xtrxdsp_init();
	CHECK_FUNC_AVX(xtrxdsp_iq16_conv64);
	CHECK_FUNC_SSE2(xtrxdsp_iq16_conv64);
	SELECT_FUNC("generic", xtrxdsp_iq16_conv64, no);
}

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x2(void)
{
//	CHECK_FUNC_AVX(xtrxdsp_b8_expand_x2);
//	CHECK_FUNC_SSE2(xtrxdsp_b8_expand_x2);
	SELECT_FUNC("generic", xtrxdsp_b4_expand_x2, no);
}

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x4(void)
{
//	CHECK_FUNC_AVX(xtrxdsp_b4_expand_x4);
//	CHECK_FUNC_SSE2(xtrxdsp_b4_expand_x4);
	SELECT_FUNC("generic", xtrxdsp_b4_expand_x4, no);
}

#else

static func_xtrxdsp_iq16_sc32_t resolve_xtrxdsp_iq16_sc32(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq16_sc32, no); }

static func_xtrxdsp_iq12_sc32_t resolve_xtrxdsp_iq12_sc32(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq12_sc32, no); }

static func_xtrxdsp_iq8_sc32_t resolve_xtrxdsp_iq8_sc32(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq8_sc32, no); }

static func_xtrxdsp_iq8_ic16_t resolve_xtrxdsp_iq8_ic16(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq8_ic16, no); }

static func_xtrxdsp_iq16_sc32i_t resolve_xtrxdsp_iq16_sc32i(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq16_sc32i, no); }

static func_xtrxdsp_iq16_ic16i_t resolve_xtrxdsp_iq16_ic16i(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq16_ic16i, no); }

static func_xtrxdsp_iq8_sc32i_t resolve_xtrxdsp_iq8_sc32i(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq8_sc32i, no); }

static func_xtrxdsp_sc32_iq16_t resolve_xtrxdsp_sc32_iq16(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_sc32_iq16, no); }

static func_xtrxdsp_sc32i_iq16_t resolve_xtrxdsp_sc32i_iq16(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_sc32i_iq16, no); }

static func_xtrxdsp_ic16i_iq16_t resolve_xtrxdsp_ic16i_iq16(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_ic16i_iq16, no); }

static func_xtrxdsp_iq8_ic16i_t resolve_xtrxdsp_iq8_ic16i(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq8_ic16i, no); }

static func_xtrxdsp_iq8_ic8i_t resolve_xtrxdsp_iq8_ic8i(void)
{ xtrxdsp_init(); SELECT_FUNC("generic", xtrxdsp_iq8_ic8i, no); }


func_xtrxdsp_sc32_conv64_t resolve_xtrxdsp_sc32_conv64(void)
{ return xtrxdsp_sc32_conv64_no; }

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x2(void)
{ return xtrxdsp_b8_expand_x2_no; }

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x4(void)
{ return xtrxdsp_b8_expand_x4_no; }

func_xtrxdsp_iq16_conv64_t resolve_xtrxdsp_iq16_conv64(void)
{ return xtrxdsp_iq16_conv64_no; }

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x2(void)
{ return xtrxdsp_b4_expand_x2_no; }

func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x4(void)
{ return xtrxdsp_b4_expand_x4_no; }

#endif

#if defined(__linux) && (defined(__x86_64__) || defined(__i386__))
// ifunc is available not on all platforms!

void xtrxdsp_iq16_sc32(const int16_t *__restrict iq,
					   float *__restrict out,
					   float scale,
					   size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq16_sc32")));


uint64_t xtrxdsp_iq12_sc32(const void *__restrict iq,
						   float *__restrict out,
						   size_t inbytes,
						   uint64_t prevstate)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq12_sc32")));


void xtrxdsp_iq8_sc32(const int8_t *__restrict iq,
					  float *__restrict out,
					  size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq8_sc32")));

void xtrxdsp_iq8_ic16(const int8_t *__restrict iq,
					  int16_t *__restrict out,
					  size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq8_ic16")));

void xtrxdsp_iq16_sc32i(const int16_t *__restrict iq,
						float *__restrict outa,
						float *__restrict outb,
						float scale,
						size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq16_sc32i")));

void xtrxdsp_iq16_ic16i(const int16_t *__restrict iq,
						int16_t *__restrict outa,
						int16_t *__restrict outb,
						size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq16_ic16i")));

void xtrxdsp_iq8_sc32i(const int8_t *__restrict iq,
					   float *__restrict outa,
					   float *__restrict outb,
					   size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq8_sc32i")));

void xtrxdsp_iq8_ic16i(const int8_t *__restrict iq,
					   int16_t *__restrict outa,
					   int16_t *__restrict outb,
					   size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq8_ic16i")));

void xtrxdsp_iq8_ic8i(const int8_t *__restrict iq,
					   int8_t *__restrict outa,
					   int8_t *__restrict outb,
					   size_t bytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_iq8_ic8i")));


void xtrxdsp_sc32_iq16(const float *__restrict iq,
					   int16_t *__restrict out,
					   float scale,
					   size_t outbytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_sc32_iq16")));

void xtrxdsp_sc32i_iq16(const float *__restrict i,
						const float *__restrict q,
						int16_t *__restrict out,
						float scale,
						size_t outbytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_sc32i_iq16")));

void xtrxdsp_ic16i_iq16(const int16_t *__restrict i,
						const int16_t *__restrict q,
						int16_t *__restrict out,
						size_t outbytes)
__attribute__ ((ifunc ("resolve_xtrxdsp_ic16i_iq16")));

DECLARE_SC32_CONV64_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_sc32_conv64")));

DECLARE_B8_EXPAND_X2_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_b8_expand_x2")));

DECLARE_B8_EXPAND_X4_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_b8_expand_x4")));

DECLARE_IQ16_CONV64_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_iq16_conv64")));

DECLARE_B4_EXPAND_X2_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_b4_expand_x2")));

DECLARE_B4_EXPAND_X4_FUNC() __attribute__ ((ifunc ("resolve_xtrxdsp_b4_expand_x4")));

#else
#define STATIC_RESOLVE(x, ...) \
	static func_##x##_t r_func; \
	if (r_func == NULL) r_func = resolve_##x(); \
	r_func(__VA_ARGS__);

#define STATIC_RESOLVE_RET(x, ...) \
	static func_##x##_t r_func; \
	if (r_func == NULL) r_func = resolve_##x(); \
	return r_func(__VA_ARGS__);

void xtrxdsp_iq16_sc32(const int16_t *__restrict iq,
					   float *__restrict out,
					   float scale,
					   size_t bytes)
{ STATIC_RESOLVE(xtrxdsp_iq16_sc32, iq, out, scale, bytes); }

uint64_t xtrxdsp_iq12_sc32(const void *__restrict iq,
						   float *__restrict out,
						   size_t inbytes,
						   uint64_t prevstate)
{ STATIC_RESOLVE_RET(xtrxdsp_iq12_sc32, iq, out, inbytes, prevstate); }

void xtrxdsp_iq8_sc32(const int8_t *__restrict iq,
					  float *__restrict out,
					  size_t bytes)
{ STATIC_RESOLVE(xtrxdsp_iq8_sc32, iq, out, bytes); }


void xtrxdsp_iq16_sc32i(const int16_t *__restrict iq,
						float *__restrict outa,
						float *__restrict outb,
						float scale,
						size_t bytes)
{ STATIC_RESOLVE(xtrxdsp_iq16_sc32i, iq, outa, outb, scale, bytes); }

void xtrxdsp_iq8_sc32i(const int8_t *__restrict iq,
					   float *__restrict outa,
					   float *__restrict outb,
					   size_t bytes)
{ STATIC_RESOLVE(xtrxdsp_iq8_sc32i, iq, outa, outb, bytes); }


void xtrxdsp_sc32_iq16(const float *__restrict iq,
					   int16_t *__restrict out,
					   float scale,
					   size_t outbytes)
{ STATIC_RESOLVE(xtrxdsp_sc32_iq16, iq, out, scale, outbytes); }

void xtrxdsp_sc32i_iq16(const float *__restrict i,
						const float *__restrict q,
						int16_t *__restrict out,
						float scale,
						size_t outbytes)
{ STATIC_RESOLVE(xtrxdsp_sc32i_iq16, i, q, out, scale, outbytes); }

void xtrxdsp_iq8_ic16(const int8_t *__restrict a, int16_t *__restrict b, size_t c)
{ STATIC_RESOLVE(xtrxdsp_iq8_ic16, a, b, c); }

void xtrxdsp_iq16_ic16i(const int16_t *__restrict a, int16_t *__restrict b, int16_t *__restrict c, size_t d)
{ STATIC_RESOLVE(xtrxdsp_iq16_ic16i, a, b, c, d); }

void xtrxdsp_iq8_ic16i(const int8_t *__restrict a, int16_t *__restrict b, int16_t *__restrict c, size_t d)
{ STATIC_RESOLVE(xtrxdsp_iq8_ic16i, a, b, c, d); }

void xtrxdsp_iq8_ic8i(const int8_t *__restrict a, int8_t *__restrict b, int8_t *__restrict c, size_t d)
{ STATIC_RESOLVE(xtrxdsp_iq8_ic8i, a, b, c, d); }

void xtrxdsp_ic16i_iq16(const int16_t *__restrict a, const int16_t *__restrict b, int16_t *__restrict c, size_t d)
{ STATIC_RESOLVE(xtrxdsp_ic16i_iq16, a, b, c, d); }

DECLARE_SC32_CONV64_FUNC()
{ STATIC_RESOLVE(xtrxdsp_sc32_conv64, data, conv, out, count, decim_bits); }

DECLARE_IQ16_CONV64_FUNC()
{ STATIC_RESOLVE(xtrxdsp_iq16_conv64, data, conv, out, count, decim_bits); }

// TODO get rid of it
typedef func_xtrxdsp_bx_expand_t func_xtrxdsp_b8_expand_x2_t;
typedef func_xtrxdsp_bx_expand_t func_xtrxdsp_b4_expand_x2_t;
typedef func_xtrxdsp_bx_expand_t func_xtrxdsp_b8_expand_x4_t;
typedef func_xtrxdsp_bx_expand_t func_xtrxdsp_b4_expand_x4_t;

DECLARE_B8_EXPAND_X2_FUNC()
{ STATIC_RESOLVE(xtrxdsp_b8_expand_x2, data, out, count_blocks); }

DECLARE_B8_EXPAND_X4_FUNC()
{ STATIC_RESOLVE(xtrxdsp_b8_expand_x4, data, out, count_blocks); }

DECLARE_B4_EXPAND_X2_FUNC()
{ STATIC_RESOLVE(xtrxdsp_b4_expand_x2, data, out, count_blocks); }

DECLARE_B4_EXPAND_X4_FUNC()
{ STATIC_RESOLVE(xtrxdsp_b4_expand_x4, data, out, count_blocks); }

#endif


