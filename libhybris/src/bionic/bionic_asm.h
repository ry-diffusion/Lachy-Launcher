/*
 * Copyright (C) 2013 The Android Open Source Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _PRIVATE_BIONIC_ASM_H_
#define _PRIVATE_BIONIC_ASM_H_

#define MAX_ERRNO 4095 /* For recognizing system call error returns. */

#define __bionic_asm_custom_entry(f)
#define __bionic_asm_custom_end(f)
#define __bionic_asm_function_type @function

#if defined(__i386__)
#include "bionic_asm_x86.h"
#endif

#define CDECL(f) _##f

#ifdef __llvm__
#define ENTRY_NO_DWARF(f)     \
  .text;                      \
  .globl CDECL(f);            \
  .balign __bionic_asm_align; \
  CDECL(f) : __bionic_asm_custom_entry(CDECL(f));

#else
#define ENTRY_NO_DWARF(f)              \
  .text;                               \
  .globl f;                            \
  .balign __bionic_asm_align;          \
  .type f, __bionic_asm_function_type; \
  f:                                   \
  __bionic_asm_custom_entry(f);

#endif

#define ENTRY(f) ENTRY_NO_DWARF(f).cfi_startproc

#ifdef __llvm__
#define END_NO_DWARF(f) __bionic_asm_custom_end(CDECL(f))

#else
#define END_NO_DWARF(f) \
  .size f, .- f;        \
  __bionic_asm_custom_end(f)

#endif

#define END(f)  \
  .cfi_endproc; \
  END_NO_DWARF(f)

/* Like ENTRY, but with hidden visibility. */
#define ENTRY_PRIVATE(f) \
  ENTRY(f);              \
  .hidden f

/* Like ENTRY_NO_DWARF, but with hidden visibility. */
#define ENTRY_PRIVATE_NO_DWARF(f) \
  ENTRY_NO_DWARF(f);              \
  .hidden f

#ifdef __llvm__
#define __BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(f)

#define ALIAS_SYMBOL(alias, original) \
  .globl _##alias;                    \
  .equ _##alias, _##original

#else
#define __BIONIC_WEAK_ASM_FOR_NATIVE_BRIDGE(f) .weak f;

#define ALIAS_SYMBOL(alias, original) \
  .globl alias;                       \
  .equ alias, original

#endif

#endif
