/*
 * xtrxdsp sse2 optimization functions file
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

#ifdef __SSE2__

#define UNALIGN_STORE

#define XTRXDSP_TEMPLATE_IQ16_SC32_SSE2
#define XTRXDSP_TEMPLATE_IQ12_SC32
#define XTRXDSP_TEMPLATE_IQ8_SC32

#define XTRXDSP_TEMPLATE_IQ16_SC32I_SSE2
#define XTRXDSP_TEMPLATE_IQ12_SC32I
#define XTRXDSP_TEMPLATE_IQ8_SC32I

#define XTRXDSP_TEMPLATE_SC32_IQ16
#define XTRXDSP_TEMPLATE_SC32I_IQ16

#define XTRXDSP_TEMPLATE_SC32_CONV64_NAME _sse2
#define XTRXDSP_TEMPLATE_SC32_CONV64

//#define XTRXDSP_TEMPLATE_B8_EXPAND_X2_NAME _sse2
//#define XTRXDSP_TEMPLATE_B8_EXPAND_X2

//#define XTRXDSP_TEMPLATE_B8_EXPAND_X4_NAME _sse2
//#define XTRXDSP_TEMPLATE_B8_EXPAND_X4

#define XTRXDSP_TEMPLATE_IQ16_CONV64_NAME _sse2
#define XTRXDSP_TEMPLATE_IQ16_CONV64

#include "xtrxdsp_templates.c"

DECLARE_TEMPLATES(sse2)

#endif

