/*
 * Public xtrxdsp header file
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

#ifndef XTRXDSP_H
#define XTRXDSP_H

#include <stdint.h>
#include <stdlib.h>

#include <xtrxdsp_config.h>

void xtrxdsp_init(void);

/* dynamically choise at runtime */

#define DECLARE_IQ16_SC32_FUNC(funcname) \
	void xtrxdsp_iq16_sc32_##funcname(const int16_t *__restrict iq, \
	float *__restrict out, \
	float scale, \
	size_t bytes)

#define DECLARE_IQ12_SC32_FUNC(funcname) \
	uint64_t xtrxdsp_iq12_sc32_##funcname(const void *__restrict iq, \
	float *__restrict out, \
	size_t inbytes, \
	uint64_t prevstate)

#define DECLARE_IQ8_SC32_FUNC(funcname) \
	void xtrxdsp_iq8_sc32_##funcname(const int8_t *__restrict iq, \
	float *__restrict out, \
	size_t bytes)

#define DECLARE_SC32_IQ16_FUNC(funcname) \
	void xtrxdsp_sc32_iq16_##funcname(const float *__restrict iq, \
	int16_t *__restrict out, \
	float scale, \
	size_t bytes)

#define DECLARE_IQ16_SC32I_FUNC(funcname) \
	void xtrxdsp_iq16_sc32i_##funcname(const int16_t *__restrict iq, \
	float *__restrict outa, \
	float *__restrict outb, \
	float scale, \
	size_t bytes)

#define DECLARE_IQ12_SC32I_FUNC(funcname) \
	uint64_t xtrxdsp_iq12_sc32i_##funcname(const void *__restrict iq, \
	float *__restrict outa, \
	float *__restrict outb, \
	size_t inbytes, \
	uint64_t prevstate)

#define DECLARE_IQ8_SC32I_FUNC(funcname) \
	void xtrxdsp_iq8_sc32i_##funcname(const int8_t *__restrict iq, \
	float *__restrict outa, \
	float *__restrict outb, \
	size_t bytes)

#define DECLARE_SC32I_IQ16_FUNC(funcname) \
	void xtrxdsp_sc32i_iq16_##funcname(const float *__restrict i, \
	const float *__restrict q, \
	int16_t *__restrict out, \
	float scale, \
	size_t bytes)


#define DECLARE_IQ16_SC32_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ16_SC32_FUNC(funcname) { xtrxdsp_iq16_sc32_template(iq, out, scale, bytes); }

#define DECLARE_IQ12_SC32_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ12_SC32_FUNC(funcname) { return xtrxdsp_iq12_sc32_template(iq, out, inbytes, prevstate); }

#define DECLARE_IQ8_SC32_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ8_SC32_FUNC(funcname) { xtrxdsp_iq8_sc32_template(iq, out, bytes); }

#define DECLARE_SC32_IQ16_FUNC_TEMPLATE(funcname) \
	DECLARE_SC32_IQ16_FUNC(funcname) { xtrxdsp_sc32_iq16_template(iq, out, scale, bytes); }


#define DECLARE_IQ16_SC32I_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ16_SC32I_FUNC(funcname) { xtrxdsp_iq16_sc32i_template(iq, outa, outb, scale, bytes); }

#define DECLARE_IQ12_SC32I_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ12_SC32I_FUNC(funcname) { return xtrxdsp_iq12_sc32i_template(iq, outa, outb, inbytes, prevstate); }

#define DECLARE_IQ8_SC32I_FUNC_TEMPLATE(funcname) \
	DECLARE_IQ8_SC32I_FUNC(funcname) { xtrxdsp_iq8_sc32i_template(iq, outa, outb, bytes); }

#define DECLARE_SC32I_IQ16_FUNC_TEMPLATE(funcname) \
	DECLARE_SC32I_IQ16_FUNC(funcname) { xtrxdsp_sc32i_iq16_template(i, q, out, scale, bytes); }


#define DECLARE_TEMPLATES(funcname)             \
	DECLARE_IQ16_SC32_FUNC_TEMPLATE(funcname)   \
	DECLARE_IQ12_SC32_FUNC_TEMPLATE(funcname)   \
	DECLARE_IQ8_SC32_FUNC_TEMPLATE(funcname)    \
	DECLARE_SC32_IQ16_FUNC_TEMPLATE(funcname)   \
	DECLARE_IQ16_SC32I_FUNC_TEMPLATE(funcname)  \
	DECLARE_IQ12_SC32I_FUNC_TEMPLATE(funcname)  \
	DECLARE_IQ8_SC32I_FUNC_TEMPLATE(funcname)   \
	DECLARE_SC32I_IQ16_FUNC_TEMPLATE(funcname)



extern void xtrxdsp_iq16_sc32(const int16_t *__restrict iq,
							  float *__restrict out,
							  float scale,
							  size_t bytes);

extern uint64_t xtrxdsp_iq12_sc32(const void *__restrict iq,
								  float *__restrict out,
								  size_t inbytes,
								  uint64_t prevstate);

extern void xtrxdsp_iq8_sc32(const int8_t *__restrict iq,
							 float *__restrict out,
							 size_t bytes);


extern void xtrxdsp_iq16_sc32i(const int16_t *__restrict iq,
							   float *__restrict outa,
							   float *__restrict outb,
							   float scale,
							   size_t bytes);
#if 0
extern uint64_t xtrxdsp_iq12_sc32i(const void *__restrict iq,
								   float *__restrict outa,
								   float *__restrict outb,
								   size_t inbytes,
								   uint64_t prevstate);
#endif

extern void xtrxdsp_iq8_sc32i(const int8_t *__restrict iq,
							  float *__restrict outa,
							  float *__restrict outb,
							  size_t bytes);

extern void xtrxdsp_sc32_iq16(const float *__restrict iq,
							  int16_t *__restrict out,
							  float scale,
							  size_t outbytes);

extern void xtrxdsp_sc32i_iq16(const float *__restrict i,
							   const float *__restrict q,
							   int16_t *__restrict out,
							   float scale,
							   size_t outbytes);

/* non vector optimized version */
DECLARE_IQ16_SC32_FUNC(no);
DECLARE_IQ12_SC32_FUNC(no);
DECLARE_IQ8_SC32_FUNC(no);

DECLARE_IQ16_SC32I_FUNC(no);
DECLARE_IQ12_SC32I_FUNC(no);
DECLARE_IQ8_SC32I_FUNC(no);

DECLARE_SC32_IQ16_FUNC(no);
DECLARE_SC32I_IQ16_FUNC(no);

#define CONCAT(x, y) x##y

/* CONVOLUTIONS */
#define DECLARE_SC32_CONV64_BASE(func) \
	void func (const float *__restrict data, \
	const float *__restrict conv, \
	float *__restrict out, \
	unsigned count, \
	unsigned decim_bits)

#define DECLARE_SC32_CONV64_FUNC(funcname) \
	DECLARE_SC32_CONV64_BASE(CONCAT(xtrxdsp_sc32_conv64,funcname))

#define DECLARE_BX_EXPAND_X_BASE(func) \
	void func (const void *__restrict data, \
	void *__restrict out, \
	unsigned count_blocks)

#define DECLARE_B8_EXPAND_X2_FUNC(funcname) \
	DECLARE_BX_EXPAND_X_BASE(CONCAT(xtrxdsp_b8_expand_x2,funcname))

#define DECLARE_B8_EXPAND_X4_FUNC(funcname) \
	DECLARE_BX_EXPAND_X_BASE(CONCAT(xtrxdsp_b8_expand_x4,funcname))


#define DECLARE_IQ16_CONV64_BASE(func) \
	void func (const int16_t *__restrict data, \
	const int16_t *__restrict conv, \
	int16_t *__restrict out, \
	unsigned count, \
	unsigned decim_bits)

#define DECLARE_IQ16_CONV64_FUNC(funcname) \
	DECLARE_IQ16_CONV64_BASE(CONCAT(xtrxdsp_iq16_conv64,funcname))

#define DECLARE_B4_EXPAND_X2_FUNC(funcname) \
	DECLARE_BX_EXPAND_X_BASE(CONCAT(xtrxdsp_b4_expand_x2,funcname))

#define DECLARE_B4_EXPAND_X4_FUNC(funcname) \
	DECLARE_BX_EXPAND_X_BASE(CONCAT(xtrxdsp_b4_expand_x4,funcname))

DECLARE_SC32_CONV64_FUNC();
DECLARE_B8_EXPAND_X2_FUNC();
DECLARE_B8_EXPAND_X4_FUNC();
DECLARE_IQ16_CONV64_FUNC();
DECLARE_B4_EXPAND_X2_FUNC();
DECLARE_B4_EXPAND_X4_FUNC();

DECLARE_SC32_CONV64_FUNC(_no);
DECLARE_B8_EXPAND_X2_FUNC(_no);
DECLARE_B8_EXPAND_X4_FUNC(_no);
DECLARE_IQ16_CONV64_FUNC(_no);
DECLARE_B4_EXPAND_X2_FUNC(_no);
DECLARE_B4_EXPAND_X4_FUNC(_no);

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>

#ifdef XTRXDSP_HAS__SSE2__
/* SSE2   */
DECLARE_IQ16_SC32_FUNC(sse2);
DECLARE_IQ12_SC32_FUNC(sse2);
DECLARE_IQ8_SC32_FUNC(sse2);

DECLARE_IQ16_SC32I_FUNC(sse2);
DECLARE_IQ12_SC32I_FUNC(sse2);
DECLARE_IQ8_SC32I_FUNC(sse2);

DECLARE_SC32_IQ16_FUNC(sse2);
DECLARE_SC32I_IQ16_FUNC(sse2);
#endif

#ifdef XTRXDSP_HAS__SSSE3__
/* SSSE3  */
DECLARE_IQ12_SC32_FUNC(ssse3);
DECLARE_IQ12_SC32_FUNC(ssse3_nh);
#endif

/* SSE4.1 */
#ifdef XTRXDSP_HAS__SSE4_1__
DECLARE_IQ12_SC32_FUNC(sse41);
DECLARE_IQ8_SC32_FUNC(sse41);
#endif

/* AVX   */
#ifdef XTRXDSP_HAS__AVX__
DECLARE_IQ16_SC32_FUNC(avx);
DECLARE_IQ12_SC32_FUNC(avx);
DECLARE_IQ8_SC32_FUNC(avx);

DECLARE_IQ16_SC32I_FUNC(avx);
DECLARE_IQ12_SC32I_FUNC(avx);
DECLARE_IQ8_SC32I_FUNC(avx);


DECLARE_SC32_IQ16_FUNC(avx);
DECLARE_SC32I_IQ16_FUNC(avx);
#endif

/* AVX2   */
#ifdef XTRXDSP_HAS__AVX2__
DECLARE_IQ16_SC32_FUNC(avx2);
DECLARE_IQ12_SC32_FUNC(avx2);
DECLARE_IQ8_SC32_FUNC(avx2);

DECLARE_IQ16_SC32I_FUNC(avx2);
DECLARE_IQ12_SC32I_FUNC(avx2);
DECLARE_IQ8_SC32I_FUNC(avx2);


DECLARE_SC32_IQ16_FUNC(avx2);
DECLARE_SC32I_IQ16_FUNC(avx2);
#endif

#ifdef XTRXDSP_HAS__SSE2__
DECLARE_SC32_CONV64_FUNC(_sse2);
//DECLARE_B8_EXPAND_X2_FUNC(_sse2);
//DECLARE_B8_EXPAND_X4_FUNC(_sse2);
DECLARE_IQ16_CONV64_FUNC(_sse2);
#endif

#ifdef XTRXDSP_HAS__AVX__
DECLARE_SC32_CONV64_FUNC(_avx);
//DECLARE_B8_EXPAND_X2_FUNC(_avx);
//DECLARE_B8_EXPAND_X4_FUNC(_avx);
DECLARE_IQ16_CONV64_FUNC(_avx);

#ifdef XTRXDSP_HAS__FMA__
DECLARE_SC32_CONV64_FUNC(_avx_fma);
#endif
#endif


#endif /* defined(__x86_64__) || defined(__i386__) */

typedef DECLARE_BX_EXPAND_X_BASE( (*func_xtrxdsp_bx_expand_t) );

typedef DECLARE_SC32_CONV64_BASE( (*func_xtrxdsp_sc32_conv64_t) );
func_xtrxdsp_sc32_conv64_t resolve_xtrxdsp_sc32_conv64(void);
func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x2(void);
func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b8_expand_x4(void);


typedef DECLARE_IQ16_CONV64_BASE( (*func_xtrxdsp_iq16_conv64_t) );
func_xtrxdsp_iq16_conv64_t resolve_xtrxdsp_iq16_conv64(void);
func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x2(void);
func_xtrxdsp_bx_expand_t resolve_xtrxdsp_b4_expand_x4(void);

#endif /* _XTRXDSP_H_ */
