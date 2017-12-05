/*
 * xtrxdsp avx_fma optimization functions file
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

#ifdef __AVX__

#define UNALIGN_STORE

#define XTRXDSP_TEMPLATE_SC32_CONV64_NAME _avx_fma
#define XTRXDSP_TEMPLATE_SC32_CONV64_AVX
//#define XTRXDSP_TEMPLATE_FMA

#include "xtrxdsp_templates.c"


#endif
