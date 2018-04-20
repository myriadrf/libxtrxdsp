/*
 * xtrxdsp template optimization functions file
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

/* This file should not be complied manually */
#define SCALE16      (1.0f/32768)
#define SCALE8       (1.0f/128)
#define SCALE2(x)    ((x)/65536)

#define UNALIGN_IQ_BUFFER

#ifdef UNALIGN_STORE
#define _MM_STOREX_PS    _mm_storeu_ps
#define _MM256_STOREX_PS _mm256_storeu_ps
#else
#define _MM_STOREX_PS    _mm_store_ps
#define _MM256_STOREX_PS _mm256_store_ps
#endif

/* hints for compiler */
#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#ifdef XTRXDSP_TEMPLATE_IQ16_SC32
static inline
void xtrxdsp_iq16_sc32_template(const int16_t *__restrict iq,
                                float *__restrict out,
                                float scale,
                                size_t bytes)
{
    size_t i = 0;
    const uint64_t *ld = (const uint64_t *)iq;

    for (; i < bytes - (bytes % 8); i += 8) {
        uint64_t v = *(ld++);
        float a = (int16_t)(v);
        float b = (int16_t)(v>>16);
        float c = (int16_t)(v>>32);
        float d = (int16_t)(v>>48);

        *(out++) = a * scale;
        *(out++) = b * scale;
        *(out++) = c * scale;
        *(out++) = d * scale;
    }

    const int16_t *ldw = (const int16_t *)ld;
    for (;i < bytes; i += 2) {
        *(out++) = *(ldw++) * scale;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ16_SC32I
static inline
void xtrxdsp_iq16_sc32i_template(const int16_t *__restrict iq,
                                 float *__restrict outa,
                                 float *__restrict outb,
                                 float scale,
                                 size_t bytes)
{
    size_t i = 0;
    const uint64_t *ld = (const uint64_t *)iq;

    for (; i < bytes - (bytes % 8); i += 8) {
        uint64_t v = *(ld++);
        float a = (int16_t)(v);
        float b = (int16_t)(v>>16);
        float c = (int16_t)(v>>32);
        float d = (int16_t)(v>>48);

        *(outa++) = a * scale;
        *(outb++) = b * scale;
        *(outa++) = c * scale;
        *(outb++) = d * scale;
    }

    const int16_t *ldw = (const int16_t *)ld;
    for (;i < bytes; i += 4) {
        *(outa++) = *(ldw++) * scale;
        if (i == bytes)
            break;
        *(outb++) = *(ldw++) * scale;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ16_IC16I
static inline
void xtrxdsp_iq16_ic16i_template(const int16_t *__restrict iq,
								 int16_t *__restrict outa,
								 int16_t *__restrict outb,
								 size_t bytes)
{
	for (; bytes > 3; bytes -= 4) {
		*(outa++) = *(iq++);
		*(outb++) = *(iq++);
	}
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ12_SC32
static inline
uint64_t xtrxdsp_iq12_sc32_template(const void *__restrict iq,
                                    float *__restrict out,
                                    size_t inbytes,
                                    uint64_t prevstate)
{
    const uint8_t *ld = (const uint8_t *)iq;
    size_t i  = 0;
    uint8_t v0, v1, v2;
    float a, b;
    unsigned q;

  /*  memory stream:  MSB...LSB
   * 0x00     f0[7:0]
   * 0x01     {f1[3:0],f0[11:8]}
   * 0x02     f1[11:4]
   * .....
   */

 /*     v2    v1    v0
  *  +-----+-----+-----+
  *  |  8  |  8  |  8  |
  *  +-----+-----+-----+
  *  |  12    |   12   |
  *  +-----+-----+-----+
  *
  *        +-----+-----+
  *  as =  |v1|  v0 |00|
  *        +-----+-----+
  *  bs =  | v2  |v1|00|
  *        +-----+-----+
  */
    switch (prevstate & 0xf) {
    case 0:
        v0 = *(ld++);
        v1 = *(ld++);
        v2 = *(ld++);
        i += 3;
    case 1:
        v0 = (prevstate >> 8) & 0xff;
        v1 = (prevstate >> 8) & 0xff;
        v2 = *(ld++);
        i++;
        break;
    case 2:
        v0 = (prevstate >> 8) & 0xff;
        v1 = *(ld++);
        v2 = *(ld++);
        i += 2;
        break;
    default:
        return -1;
    }

    a = (int16_t) (((uint16_t)v0 << 4) | ((uint16_t)v1 << 12));
    b = (int16_t) (((uint16_t)v2 << 8) | (v1 & 0xf0));

    *(out++) = a * SCALE16;
    *(out++) = b * SCALE16;

    for (; i < inbytes - (inbytes % 3); i += 3) {
        v0 = *(ld++);
        v1 = *(ld++);
        v2 = *(ld++);

        a = (int16_t) (((uint16_t)v0 << 4) | ((uint16_t)v1 << 12));
        b = (int16_t) (((uint16_t)v2 << 8) | (v1 & 0xf0));

        *(out++) = a * SCALE16;
        *(out++) = b * SCALE16;
    }

    switch (inbytes % 3) {
    default:
        return 0;
    case 1:
        return 1 | ((unsigned)(*ld) << 8);
    case 2:
        q = (*ld++);
        return (2 | (q << 8)) | ((unsigned)(*ld) << 16);
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ12_SC32I
static inline
uint64_t xtrxdsp_iq12_sc32i_template(const void *__restrict iq,
                                     float *__restrict outa,
                                     float *__restrict outb,
                                     size_t inbytes,
                                     uint64_t prevstate)
{
    const uint8_t *ld = (const uint8_t *)iq;
    size_t i  = 0;
    uint8_t v0, v1, v2;
    float a, b;

    for (; i < inbytes - (inbytes % 3); i += 3) {
        v0 = *(ld++);
        v1 = *(ld++);
        v2 = *(ld++);

        a = (int16_t) (((uint16_t)v0 << 4) | ((uint16_t)v1 << 12));
        b = (int16_t) (((uint16_t)v2 << 8) | (v1 & 0xf0));

        *(outa++) = a * SCALE16;
        *(outb++) = b * SCALE16;
    }


    uint64_t res = inbytes % 3;
    for (; i < inbytes; ++i) {
        res <<= 8;
        res |= *(ld++);
    }

    return res;
}
#endif


#ifdef XTRXDSP_TEMPLATE_IQ8_SC32
static inline
void xtrxdsp_iq8_sc32_template(const int8_t *__restrict iq,
                               float *__restrict out,
                               size_t bytes)
{
    size_t i = 0;
    const uint32_t *ld = (const uint32_t *)iq;

    for (; i < bytes - (bytes % 4); i += 4) {
        uint32_t v = *(ld++);
        float a = (int8_t)(v);
        float b = (int8_t)(v>>8);
        float c = (int8_t)(v>>16);
        float d = (int8_t)(v>>24);

        *(out++) = a * SCALE8;
        *(out++) = b * SCALE8;
        *(out++) = c * SCALE8;
        *(out++) = d * SCALE8;
    }

    const int8_t *ldb = (const int8_t *)ld;
    for (;i < bytes; i +=2 ) {
        *(out++) = *(ldb++) * SCALE8;
        *(out++) = *(ldb++) * SCALE8;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ8_SC32I
static inline
void xtrxdsp_iq8_sc32i_template(const int8_t *__restrict iq,
                                float *__restrict outa,
                                float *__restrict outb,
                                size_t bytes)
{
    size_t i = 0;
    const uint32_t *ld = (const uint32_t *)iq;

    for (; i < bytes - (bytes % 4); i += 4) {
        uint32_t v = *(ld++);
        float a = (int8_t)(v);
        float b = (int8_t)(v>>8);
        float c = (int8_t)(v>>16);
        float d = (int8_t)(v>>24);

        *(outa++) = a * SCALE8;
        *(outb++) = b * SCALE8;
        *(outa++) = c * SCALE8;
        *(outb++) = d * SCALE8;
    }

    const int8_t *ldb = (const int8_t *)ld;
    for (;i < bytes; i +=2 ) {
        *(outa++) = *(ldb++) * SCALE8;
        *(outb++) = *(ldb++) * SCALE8;
    }
}
#endif


/*********************************************************************************************/
/* SSE2 */


#ifdef XTRXDSP_TEMPLATE_IQ16_SC32_SSE2
static inline
void xtrxdsp_iq16_sc32_template(const int16_t *__restrict iq,
                                float *__restrict out,
                                float inscale,
                                size_t bytes)
{
  size_t i = bytes;
  const __m128i* vp = (const __m128i* )iq;
  __m128i d0, d1, d2, d3;
  __m128 f0, f1, f2, f3;
  __m128 scale = _mm_set_ps(SCALE2(inscale), SCALE2(inscale), SCALE2(inscale), SCALE2(inscale));
  __m128i t0;
  __m128i t1;

#ifdef UNALIGN_IQ_BUFFER
  size_t unalign;
  /* Check for unalign start of IQ */
  if (unlikely((unalign = ((uintptr_t)iq & 0x1c))) > 0) {
      const int16_t *ldw = (const int16_t *)vp;
      for (;unalign < 32; unalign += 4, i -= 4) {
          if (i == 0)
              return;
          *(out++) = *(ldw++) * inscale;
          *(out++) = *(ldw++) * inscale;
      }
      vp = (const __m128i* )ldw;
  }
#endif

  if (i >= 32) {
      t0 = _mm_load_si128(vp++);
      t1 = _mm_load_si128(vp++);

      for (; i >= 64; i -= 32) {
          d0 = _mm_unpacklo_epi16(_mm_set1_epi16(0), t0);
          d1 = _mm_unpackhi_epi16(_mm_set1_epi16(0), t0);
          d2 = _mm_unpacklo_epi16(_mm_set1_epi16(0), t1);
          d3 = _mm_unpackhi_epi16(_mm_set1_epi16(0), t1);

          t0 = _mm_load_si128(vp++);
          t1 = _mm_load_si128(vp++);

          f0 = _mm_cvtepi32_ps(d0);    // Latency 3
          f1 = _mm_cvtepi32_ps(d1);    // Latency 3
          f2 = _mm_cvtepi32_ps(d2);    // Latency 3
          f3 = _mm_cvtepi32_ps(d3);    // Latency 3

          f0 = _mm_mul_ps(f0, scale);  // Latency 5
          _MM_STOREX_PS(out, f0); out+=4;
          f1 = _mm_mul_ps(f1, scale);
          _MM_STOREX_PS(out, f1); out+=4;
          f2 = _mm_mul_ps(f2, scale);  // Latency 5
          _MM_STOREX_PS(out, f2); out+=4;
          f3 = _mm_mul_ps(f3, scale);
          _MM_STOREX_PS(out, f3); out+=4;
      }

      i -= 32;

      // Last portion of 32 + bytes % 32
      d0 = _mm_unpacklo_epi16(_mm_set1_epi16(0), t0);
      d1 = _mm_unpackhi_epi16(_mm_set1_epi16(0), t0);
      d2 = _mm_unpacklo_epi16(_mm_set1_epi16(0), t1);
      d3 = _mm_unpackhi_epi16(_mm_set1_epi16(0), t1);

      if (i >= 16) {
          t0 = _mm_load_si128(vp++);
      }

      f0 = _mm_cvtepi32_ps(d0);    // Latency 3
      f1 = _mm_cvtepi32_ps(d1);    // Latency 3
      f2 = _mm_cvtepi32_ps(d2);    // Latency 3
      f3 = _mm_cvtepi32_ps(d3);    // Latency 3

      f0 = _mm_mul_ps(f0, scale);  // Latency 5
      _MM_STOREX_PS(out, f0); out+=4;
      f1 = _mm_mul_ps(f1, scale);
      _MM_STOREX_PS(out, f1); out+=4;
      f2 = _mm_mul_ps(f2, scale);  // Latency 5
      _MM_STOREX_PS(out, f2); out+=4;
      f3 = _mm_mul_ps(f3, scale);
      _MM_STOREX_PS(out, f3); out+=4;

      if (i == 0)
          return;
  } else if (i >= 16) {
      t0 = _mm_load_si128(vp++);
  }

  if (i >= 16) {
      i -= 16;

      // Last portion of 32 + bytes % 32
      d0 = _mm_unpacklo_epi16(_mm_set1_epi16(0), t0);
      d1 = _mm_unpackhi_epi16(_mm_set1_epi16(0), t0);

      f0 = _mm_cvtepi32_ps(d0);    // Latency 3
      f1 = _mm_cvtepi32_ps(d1);    // Latency 3

      f0 = _mm_mul_ps(f0, scale);  // Latency 5
      _MM_STOREX_PS(out, f0); out+=4;
      f1 = _mm_mul_ps(f1, scale);
      _MM_STOREX_PS(out, f1); out+=4;
  }

  // remaining part
  if (unlikely(i > 0)) {
      const int16_t *ldw = (const int16_t *)vp;
      //for (;i != i % 4; i -= 4) {
      switch (i) {
      case 8:
          *(out++) = *(ldw++) * inscale;
          *(out++) = *(ldw++) * inscale;
      case 4:
          *(out++) = *(ldw++) * inscale;
          *(out++) = *(ldw++) * inscale;
      }
      //}
  }
}
#endif


#ifdef XTRXDSP_TEMPLATE_IQ16_SC32I_SSE2
static inline
void xtrxdsp_iq16_sc32i_template(const int16_t *__restrict iq,
                                 float *__restrict outa,
                                 float *__restrict outb,
                                 float inscale,
                                 size_t bytes)
{
  size_t i = bytes;
  const __m128i* vp = (const __m128i* )iq;

  __m128i d0, d1, d2, d3;
  __m128 f0, f1, f2, f3;
  __m128 z0, z1, z2, z3;
  __m128 scale = _mm_set_ps(SCALE2(inscale), SCALE2(inscale), SCALE2(inscale), SCALE2(inscale));
  __m128i ands  = _mm_set_epi32(0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000);
  __m128i t0;
  __m128i t1;

#ifdef UNALIGN_IQ_BUFFER
  size_t unalign;
  /* Check for unalign start of IQ */
  if (unlikely((unalign = ((uintptr_t)iq & 0x18))) > 0) {
      const int16_t *ldw = (const int16_t *)vp;
      for (;unalign < 32; unalign += 8, i-= 8) {
          if (i == 0)
              return;
#if 0
          *(outa++) = *(ldw++) * inscale;
          *(outb++) = *(ldw++) * inscale;
          *(outa++) = *(ldw++) * inscale;
          *(outb++) = *(ldw++) * inscale;
#else
          t0 = _mm_loadl_epi64((const __m128i* )ldw); ldw += 4;
          d0 = _mm_and_si128(t0, ands); // B3..B0
          d1 = _mm_slli_si128(t0, 2);
          f1 = _mm_cvtepi32_ps(d0);     // Latency 3
          d1 = _mm_and_si128(d1, ands); // A3..A0
          f0 = _mm_cvtepi32_ps(d1);     // Latency 3
          z0 = _mm_mul_ps(f0, scale);   // Latency 5
          _mm_storel_epi64(( __m128i* )outa, _mm_castps_si128(z0)); outa+=2;
          z1 = _mm_mul_ps(f1, scale);
          _mm_storel_epi64(( __m128i* )outb, _mm_castps_si128(z1)); outb+=2;
#endif
      }
      vp = (const __m128i* )ldw;
  }
#endif

  if (i >= 32) {
      t0 = _mm_load_si128(vp++);
      t1 = _mm_load_si128(vp++);

      for (; i >= 64; i -= 32) {
          d0 = _mm_and_si128(t0, ands); // B3..B0
          d1 = _mm_slli_si128(t0, 2);
          d2 = _mm_and_si128(t1, ands); // B7..B4
          d3 = _mm_slli_si128(t1, 2);

          t0 = _mm_load_si128(vp++);
          t1 = _mm_load_si128(vp++);

          d1 = _mm_and_si128(d1, ands); // A3..A0
          f1 = _mm_cvtepi32_ps(d0);    // Latency 3
          d3 = _mm_and_si128(d3, ands); // A4..A4

          f0 = _mm_cvtepi32_ps(d1);    // Latency 3

          f2 = _mm_cvtepi32_ps(d3);    // Latency 3
          f3 = _mm_cvtepi32_ps(d2);    // Latency 3

          z0 = _mm_mul_ps(f0, scale);  // Latency 5
          _MM_STOREX_PS(outa, z0); outa+=4;
          z1 = _mm_mul_ps(f1, scale);
          _MM_STOREX_PS(outb, z1); outb+=4;
          z2 = _mm_mul_ps(f2, scale);  // Latency 5
          _MM_STOREX_PS(outa, z2); outa+=4;
          z3 = _mm_mul_ps(f3, scale);
          _MM_STOREX_PS(outb, z3); outb+=4;
      }

      i -= 32;

      d0 = _mm_and_si128(t0, ands); // B3..B0
      d1 = _mm_slli_si128(t0, 2);
      d2 = _mm_and_si128(t1, ands); // B3..B0
      d3 = _mm_slli_si128(t1, 2);

      if (i >= 16) {
         t0 = _mm_load_si128(vp++);
      }

      d1 = _mm_and_si128(d1, ands); // A3..A0
      f1 = _mm_cvtepi32_ps(d0);    // Latency 3
      d3 = _mm_and_si128(d3, ands); // A4..A4

      f0 = _mm_cvtepi32_ps(d1);    // Latency 3

      f2 = _mm_cvtepi32_ps(d3);    // Latency 3
      f3 = _mm_cvtepi32_ps(d2);    // Latency 3

      z0 = _mm_mul_ps(f0, scale);  // Latency 5
      _MM_STOREX_PS(outa, z0); outa+=4;
      z1 = _mm_mul_ps(f1, scale);
      _MM_STOREX_PS(outb, z1); outb+=4;
      z2 = _mm_mul_ps(f2, scale);  // Latency 5
      _MM_STOREX_PS(outa, z2); outa+=4;
      z3 = _mm_mul_ps(f3, scale);
      _MM_STOREX_PS(outb, z3); outb+=4;

      if (i == 0)
          return;
  } else if (i >= 16) {
      t0 = _mm_load_si128(vp++);
  }

  if (unlikely(i >= 16)) {
      i -= 16;

      // Last portion of 32 + bytes % 32
      d0 = _mm_and_si128(t0, ands); // B3..B0
      d1 = _mm_slli_si128(t0, 2);
      d1 = _mm_and_si128(d1, ands);

      f0 = _mm_cvtepi32_ps(d1);    // Latency 3
      f1 = _mm_cvtepi32_ps(d0);    // Latency 3

      f0 = _mm_mul_ps(f0, scale);  // Latency 5
      f1 = _mm_mul_ps(f1, scale);

      _MM_STOREX_PS(outa, f0); outa += 4;
      _MM_STOREX_PS(outb, f1); outb += 4;
  }

  /* remaining part unaligned part */
  if (i > 0) {
      const int16_t *ldw = (const int16_t *)vp;
      /* only 8 remaining bytes are possible here */
      //for (;i != i % 8; i -= 8) {
#if 0
      *(outa++) = *(ldw++) * inscale;
      *(outb++) = *(ldw++) * inscale;
      *(outa++) = *(ldw++) * inscale;
      *(outb++) = *(ldw++) * inscale;
#else
      t0 = _mm_loadl_epi64((const __m128i* )ldw); //ldw += 4;
      d0 = _mm_and_si128(t0, ands); // B3..B0
      d1 = _mm_slli_si128(t0, 2);
      f1 = _mm_cvtepi32_ps(d0);     // Latency 3
      d1 = _mm_and_si128(d1, ands); // A3..A0
      f0 = _mm_cvtepi32_ps(d1);     // Latency 3
      z0 = _mm_mul_ps(f0, scale);   // Latency 5
      _mm_storel_epi64(( __m128i* )outa, _mm_castps_si128(z0)); //outa+=2;
      z1 = _mm_mul_ps(f1, scale);
      _mm_storel_epi64(( __m128i* )outb, _mm_castps_si128(z1)); //outb+=2;
#endif
      //}
  }
}
#endif

#ifdef XTRXDSP_TEMPLATE_B8_EXPAND_X2
DECLARE_B8_EXPAND_X2_FUNC(XTRXDSP_TEMPLATE_B8_EXPAND_X2_NAME)
{
    unsigned n, q;
    const uint64_t *p_in = (const uint64_t *)data;
    uint64_t *p_out = (uint64_t *)out;

    for (n = 0, q = 0; n < count_blocks; n++, q += 2) {
        p_out[q] = p_out[q + 1] = p_in[n];
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_B8_EXPAND_X4
DECLARE_B8_EXPAND_X4_FUNC(XTRXDSP_TEMPLATE_B8_EXPAND_X4_NAME)
{
    unsigned n, q;
    const uint64_t *p_in = (const uint64_t *)data;
    uint64_t *p_out = (uint64_t *)out;

    for (n = 0, q = 0; n < count_blocks; n++, q += 4) {
        p_out[q] = p_out[q + 1] = p_out[q + 2] = p_out[q + 3] = p_in[n];
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_B4_EXPAND_X2
DECLARE_B4_EXPAND_X2_FUNC(XTRXDSP_TEMPLATE_B4_EXPAND_X2_NAME)
{
    unsigned n, q;
    const uint32_t *p_in = (const uint32_t *)data;
    uint32_t *p_out = (uint32_t *)out;

    for (n = 0, q = 0; n < count_blocks; n++, q += 2) {
        p_out[q] = p_out[q + 1] = p_in[n];
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_B4_EXPAND_X4
DECLARE_B4_EXPAND_X4_FUNC(XTRXDSP_TEMPLATE_B4_EXPAND_X4_NAME)
{
    unsigned n, q;
    const uint32_t *p_in = (const uint32_t *)data;
    uint32_t *p_out = (uint32_t *)out;

    for (n = 0, q = 0; n < count_blocks; n++, q += 4) {
        p_out[q] = p_out[q + 1] = p_out[q + 2] = p_out[q + 3] = p_in[n];
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_SC32_CONV64
DECLARE_SC32_CONV64_FUNC(XTRXDSP_TEMPLATE_SC32_CONV64_NAME)
{
    unsigned i, n;

    for (n = 0; n < count - 127; n += (2 << decim_bits)) {
        float acc_i = 0;
        float acc_q = 0;

        for (i = 0; i < 64; i++) {
            acc_i += data[n + 2*i] * conv[i];
            acc_q += data[n + 2*i + 1] * conv[i];
        }

        out[(n >> decim_bits) + 0] = acc_i;
        out[(n >> decim_bits) + 1] = acc_q;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IQ16_CONV64
DECLARE_IQ16_CONV64_FUNC(XTRXDSP_TEMPLATE_IQ16_CONV64_NAME)
{
    unsigned i, n;

    for (n = 0; n < count - 127; n += (2 << decim_bits)) {
        int64_t acc_i = 0;
        int64_t acc_q = 0;

        for (i = 0; i < 64; i++) {
            acc_i += (int64_t)data[n + 2*i] * conv[i];
            acc_q += (int64_t)data[n + 2*i + 1] * conv[i];
        }

        out[(n >> decim_bits) + 0] = acc_i >> 16;
        out[(n >> decim_bits) + 1] = acc_q >> 16;
    }
}
#endif



#ifdef XTRXDSP_TEMPLATE_SC32_CONV64_AVX
static __m256 conv_shuffle_filter_taps_avx(__m256 fi)
{
    // [f0 f1 f2 f3]
    __m128 lo = _mm256_castps256_ps128(fi);
    // [f4 f5 f6 f7]
    __m128 hi = _mm256_extractf128_ps(fi, 1);
    // [f0 f1 f4 f5]
    __m128 lo_s = _mm_shuffle_ps(lo, hi, _MM_SHUFFLE(1,0,1,0));
    // [f2 f3 f6 f4]
    __m128 hi_s = _mm_shuffle_ps(lo, hi, _MM_SHUFFLE(3,2,3,2));

    return _mm256_insertf128_ps(_mm256_castps128_ps256(lo_s), hi_s, 1);
}

__attribute__((optimize("unroll-loops")))
DECLARE_SC32_CONV64_FUNC(XTRXDSP_TEMPLATE_SC32_CONV64_NAME)
{
    unsigned i, n;

#define FA(x) conv_shuffle_filter_taps_avx(x)

    __m256 f[8];
     f[0] = FA(_mm256_load_ps(conv)); //8 taps
     f[1] = FA(_mm256_load_ps(conv + 8)); //8 taps
     f[2] = FA(_mm256_load_ps(conv + 16)); //8 taps
     f[3] = FA(_mm256_load_ps(conv + 24)); //8 taps

     f[4] = FA(_mm256_load_ps(conv + 32)); //8 taps
     f[5] = FA(_mm256_load_ps(conv + 40)); //8 taps
     f[6] = FA(_mm256_load_ps(conv + 48)); //8 taps
     f[7] = FA(_mm256_load_ps(conv + 56)); //8 taps

    __m256 l0, l1, l2, l3;
    __m256 ma0, ma1;

    __m256 mt0, mt1, mta, mts, mth;
    float ci, cq;

    for (n = 0; n < count - 127; n += (2 << decim_bits)) {
        ma0 = _mm256_setzero_ps(); //Acc_I
        ma1 = _mm256_setzero_ps(); //Acc_Q

        for (i = 0; i < 8; i++) {
            l0 = _mm256_loadu_ps(data + n + 2*8*i);    //[I0 Q0 I1 Q1 I2 Q2 I3 Q3]
            l1 = _mm256_loadu_ps(data + n + 2*8*i + 8);//[I4 Q4 I5 Q5 I6 Q6 I7 Q7]

            //  [I0 I1 I4 I5 I2 I3 I6 I7 ]
            l2 = _mm256_shuffle_ps(l0, l1, _MM_SHUFFLE(2,0,2,0)); // I

            //  [Q0 Q1 Q4 Q5 Q2 Q3 I6 I7 ]
            l3 = _mm256_shuffle_ps(l0, l1, _MM_SHUFFLE(3,1,3,1)); // Q

#ifdef XTRXDSP_TEMPLATE_FMA
            ma0 = _mm256_fmadd_ps(l2, f[i], ma0);
            ma1 = _mm256_fmadd_ps(l3, f[i], ma1);
#else
            /* gcc 5.x handles _mm256_add_ps(..., _mm256_mul_ps(..)) to form FMA
             * and it produces better code than pure _mm256_fmadd_ps call
             */
            ma0 = _mm256_add_ps(ma0, _mm256_mul_ps(l2, f[i]));
            ma1 = _mm256_add_ps(ma1, _mm256_mul_ps(l3, f[i]));
#endif
        }

        mt0 = _mm256_permute2f128_ps(ma0, ma1, _MM_SHUFFLE(0, 2, 0, 0)); //Sum_I[127:0]   . Sum_Q[127:0]
        mt1 = _mm256_permute2f128_ps(ma0, ma1, _MM_SHUFFLE(0, 3, 0, 1)); //Sum_I[255:128] . Sum_Q[255:128]

        // [ si3      si2      si1      si0      sq3      sq2      sq1     sq0 ]
        mta = _mm256_add_ps(mt0, mt1);
        mts = _mm256_shuffle_ps(mta, mta, _MM_SHUFFLE(2, 3, 0, 1));

        // [ si3+si2  si3+si2  si1+si0  si1+si0  sq3+sq2  sq3+sq2  sq1+sq0 sq1+sq0 ]
        mts = _mm256_add_ps(mts, mta);

        // [ si1+si0  si1+si0  si3+si2  si3+si2  sq1+sq0 sq1+sq0  sq3+sq2  sq3+sq2 ]
        mth = _mm256_shuffle_ps(mts, mts, _MM_SHUFFLE(1, 0, 3, 2));
        mth = _mm256_add_ps(mts, mth);

        ci = _mm_cvtss_f32(_mm256_castps256_ps128(mth));
        cq = _mm_cvtss_f32(_mm256_extractf128_ps(mth, 1));

        out[(n >> decim_bits) + 0] = ci;
        out[(n >> decim_bits) + 1] = cq;

    }
}
#endif







#ifdef XTRXDSP_TEMPLATE_SC32_IQ16
static inline
void xtrxdsp_sc32_iq16_template(const float *__restrict iq,
                                int16_t *__restrict out,
                                float scale,
                                size_t outbytes)
{
    for (; outbytes > 1; outbytes -= 2) {
        *out++ = *iq++ * scale;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_SC32I_IQ16
static inline
void xtrxdsp_sc32i_iq16_template(const float *__restrict i,
                                 const float *__restrict q,
                                 int16_t *__restrict out,
                                 float scale,
                                 size_t outbytes)
{
    for (; outbytes > 3; outbytes -= 4) {
        *out++ = *i++ * scale;
        *out++ = *q++ * scale;
    }
}
#endif

#ifdef XTRXDSP_TEMPLATE_IC16I_IQ16
static inline
void xtrxdsp_ic16i_iq16_template(const int16_t *__restrict i,
								 const int16_t *__restrict q,
								 int16_t *__restrict out,
								 size_t outbytes)
{
	for (; outbytes > 3; outbytes -= 4) {
		*out++ = *i++;
		*out++ = *q++;
	}
}
#endif



#if defined(XTRXDSP_TEMPLATE_SC32I_IQ16_AVX) || defined(XTRXDSP_TEMPLATE_SC32I_IQ16_AVX2)
static inline
void xtrxdsp_sc32i_iq16_template(const float *__restrict i,
                                 const float *__restrict q,
                                 int16_t *__restrict out,
                                 float scale,
                                 size_t outbytes)
{
    float si = scale * 0x10000;
    const float *pi = i;
    const float *pq = q;

    __m256 li0, li1, lq0, lq1;
    __m256 si0, si1, sq0, sq1;
    __m256i ni0, ni1, nq0, nq1;
#ifdef XTRXDSP_TEMPLATE_SC32I_IQ16_AVX2
    __m256i sni0, sni1, snq0, snq1;
    __m256i o0, o1;
#else
    __m256 sni0, sni1, snq0, snq1;
    __m256 o0, o1;
#endif

    __m256 scaleq = _mm256_set_ps(si, si, si, si, si, si, si, si);
    __m256 scalei = _mm256_set_ps(scale, scale, scale, scale, scale, scale, scale, scale);

    __m256i andmask = _mm256_set_epi16(0xFFFF, 0, 0xFFFF, 0, 0xFFFF, 0, 0xFFFF, 0,
                                       0xFFFF, 0, 0xFFFF, 0, 0xFFFF, 0, 0xFFFF, 0);

    for (; outbytes > 64; outbytes -= 64, pi += 16, pq += 16, out += 32) {
        li0 = _mm256_loadu_ps(pi);
        li1 = _mm256_loadu_ps(pi + 8);
        lq0 = _mm256_loadu_ps(pq);
        lq1 = _mm256_loadu_ps(pq + 8);

        si0 = _mm256_mul_ps(li0, scalei);
        si1 = _mm256_mul_ps(li1, scalei);
        sq0 = _mm256_mul_ps(lq0, scaleq);
        sq1 = _mm256_mul_ps(lq1, scaleq);

        ni0 = _mm256_cvttps_epi32(si0);
        ni1 = _mm256_cvttps_epi32(si1);
        nq0 = _mm256_cvttps_epi32(sq0);
        nq1 = _mm256_cvttps_epi32(sq1);

#ifdef XTRXDSP_TEMPLATE_SC32I_IQ16_AVX2
        sni0 = _mm256_andnot_si256(andmask, ni0);
        sni1 = _mm256_andnot_si256(andmask, ni1);
        snq0 = _mm256_and_si256(andmask, nq0);
        snq1 = _mm256_and_si256(andmask, nq1);

        o0 = _mm256_or_si256(sni0, snq0);
        o1 = _mm256_or_si256(sni1, snq1);

        _mm256_storeu_si256(out, o0);
        _mm256_storeu_si256(out + 16, o1);
#else
        sni0 = _mm256_andnot_ps(_mm256_castsi256_ps(andmask), _mm256_castsi256_ps(ni0));
        sni1 = _mm256_andnot_ps(_mm256_castsi256_ps(andmask), _mm256_castsi256_ps(ni1));
        snq0 = _mm256_and_ps(_mm256_castsi256_ps(andmask), _mm256_castsi256_ps(nq0));
        snq1 = _mm256_and_ps(_mm256_castsi256_ps(andmask), _mm256_castsi256_ps(nq1));

        o0 = _mm256_or_ps(sni0, snq0);
        o1 = _mm256_or_ps(sni1, snq1);

        _mm256_storeu_ps((float*)(out), o0);
        _mm256_storeu_ps((float*)(out + 16), o1);
#endif
    }

    for (; outbytes > 3; outbytes -= 4) {
        *out++ = *pi++ * scale;
        *out++ = *pq++ * scale;
    }

}
#endif

