/** ****************************************************************************
 *
 * 	\file	lwl_port.h
 *
 * 	\brief	Header file for LwL porting macros.
 *
 ******************************************************************************/
/*
 *  2026-04-29
 *
 *  Copyright (c) Ari Suomi
 *
 *------------------------------------------------------------------------------
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#ifndef LWL_PORT_H_INCLUDED
#define LWL_PORT_H_INCLUDED

/*******************************************************************************
;
;	I N C L U D E S
;
;-----------------------------------------------------------------------------*/

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>

/*******************************************************************************
;
;	D E F I N E S
;
;-----------------------------------------------------------------------------*/

/**
 * Macro for assertions in the list implementations.
 *
 * \param	expr_		The asserted boolean expression.
 */
#define lwl__portAssert(expr_) assert(expr_)

/**
 * Minimum alignment of list nodes in bytes. The lowest bits of node pointers
 * that are always zero because of this alignment may be used to store flags,
 * like the color of red black tree nodes.
 *
 * Defaults to the pointer size on architectures that are known to align
 * pointers to their size, and to 1 (no free bits) on other targets, like 8-bit
 * AVR. A too small value is always safe, a too large one is not. RISC-V uses 4
 * for both the 32-bit and the 64-bit variants.
 *
 * Define before including the library headers to override. Define as 1 when
 * nodes are placed in packed structures.
 */
#ifndef LWL_PORT_NODE_ALIGNMENT
#	if defined(__x86_64__) || defined(_M_X64) || \
		defined(__aarch64__) || defined(_M_ARM64)
#		define LWL_PORT_NODE_ALIGNMENT 8
#	elif defined(__i386__) || defined(_M_IX86) || \
		defined(__arm__) || defined(_M_ARM) ||     \
		defined(__riscv)
#		define LWL_PORT_NODE_ALIGNMENT 4
#	else
#		define LWL_PORT_NODE_ALIGNMENT 1
#	endif
#endif

/*******************************************************************************
;
;	T Y P E S
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	D A T A
;
;-----------------------------------------------------------------------------*/

/*******************************************************************************
;
;	F U N C T I O N S
;
;-----------------------------------------------------------------------------*/
/* clang targeting MSVC defines _MSC_VER but not __GNUC__ */
#if defined(__GNUC__) || defined(__clang__)
/** ****************************************************************************
 *
 * 	\brief		Count leading zeroes from 16-bit value.
 *
 * 	\param		num		The value to count leading zeroes in. Must not be 0.
 *
 * 	\return 	Number of leading zeroes.
 *
 * 	\details	__builtin_clz counts the zeroes in an unsigned int, which is
 * 				16 bits on targets like AVR and MSP430 and 32 bits on most
 * 				others, so the extra bits of an unsigned int are subtracted.
 *
 ******************************************************************************/
static inline uint8_t lwl__portClz16(
	uint16_t num
) {
	lwl__portAssert(num != 0U);

	unsigned int leadingZeroes = (unsigned int)__builtin_clz((unsigned int)num);

	return (uint8_t)(leadingZeroes - (sizeof(unsigned int) * CHAR_BIT - 16U));
}
#elif defined(_MSC_VER)
#	include <intrin.h>

static __forceinline uint8_t lwl__portClz16(unsigned short num) {
	lwl__portAssert(num != 0U);

	unsigned long idx;
	_BitScanReverse(&idx, num);
	return (uint8_t)(15U - idx);
}
#else
/** ****************************************************************************
 *
 * 	\brief		Count leading zeroes from 16-bit value.
 *
 * 	\param		num		The value to count leading zeroes in. Must not be 0.
 *
 * 	\return 	Number of leading zeroes.
 *
 ******************************************************************************/
static inline uint8_t lwl__portClz16(
	uint16_t num
) {
	lwl__portAssert(num != 0U);

	uint_fast8_t count = 0U;

	if ((num & 0xFF00U) == 0U) {
		count += 8U;
		num <<= 8U;
	}

	if ((num & 0xF000U) == 0U) {
		count += 4U;
		num <<= 4U;
	}

	if ((num & 0xC000U) == 0U) {
		count += 2U;
		num <<= 2U;
	}

	if ((num & 0x8000U) == 0U) {
		count += 1U;
	}

	return (uint8_t)count;
}
#endif

#endif // LWL_PORT_H_INCLUDED
