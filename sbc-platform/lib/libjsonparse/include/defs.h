/* Copyright StrongLoop, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef DEFS_H_
#define DEFS_H_

#include <assert.h>
#include <stddef.h>

/* util.c */
#if defined(__GNUC__)
#define ATTRIBUTE_FORMAT_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#define ATTRIBUTE_FORMAT_PRINTF(a, b)
#endif
void pr_info(const char* fmt, ...) ATTRIBUTE_FORMAT_PRINTF(1, 2);
void pr_warn(const char* fmt, ...) ATTRIBUTE_FORMAT_PRINTF(1, 2);
void pr_err(const char* fmt, ...) ATTRIBUTE_FORMAT_PRINTF(1, 2);
void pr_dbg(const char* fmt, ...) ATTRIBUTE_FORMAT_PRINTF(1, 2);
void* xmalloc(size_t size);

/* main.c */
const char* _getprogname(void);

/* ASSERT() is for debug checks, CHECK() for run-time sanity checks.
 * DEBUG_CHECKS is for expensive debug checks that we only want to
 * enable in debug builds but still want type-checked by the compiler
 * in release builds.
 */
#if defined(NDEBUG)
#define ASSERT(exp)
#define CHECK(exp)   do { if (!(exp)) abort(); } while (0)
#define DEBUG_CHECKS (0)
#define pr_debug(fmt, ...) pr_dbg(fmt, ##__VA_ARGS__)
#else
#define ASSERT(exp)  assert(exp)
#define CHECK(exp)   assert(exp)
#define DEBUG_CHECKS (1)
#define pr_debug(fmt, ...)
#endif

#define UNREACHABLE() CHECK(!"Unreachable code reached.")

/* This macro looks complicated but it's not: it calculates the address
 * of the embedding struct through the address of the embedded struct.
 * In other words, if struct A embeds struct B, then we can obtain
 * the address of A by taking the address of B and subtracting the
 * field offset of B in A.
 */
#define CONTAINER_OF(ptr, type, field)                                        \
    ((type *) ((char *) (ptr) - ((char *) &((type *) 0)->field)))

#endif  /* DEFS_H_ */
