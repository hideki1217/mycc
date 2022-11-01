"aho"
    "ahoaho";

"u"
    "n"
    "chi";

#define A(args...) func(args)
A(ahoa;);
#undef A

#define A(...) __VA_ARGS__
A(a, b, c, d);
#undef A

#define A(exp) #exp
A;
A(a + 3);
#undef A

#define A(a, b, c) a #b c
A(1, x + 7, 4);
#undef A

#define AA "oo"
#define A(a, b) a##b
A(A, A);
#undef A
#undef AA

#define A(fmt, args...) printf(fmt, args)
A("%d, %d, %s", 2, aho, "ahoaho");
#undef A

#define A \
  oo;     \
  A;

A

#undef A

#define AAA "oo"
#define AAA_ A##AA

    AAA_;

#undef AAA
#undef AAA_

#define A

A;

#undef A

#include "test_pp_sample2.c"

#ifdef HOGE  // true

#ifdef HUGA  // false
"xxxx";
#else
"oooo";
#endif

#else

"xx";

#endif

#ifndef HOGE  // false

"xx";

#else

#ifndef HUGA  // true
"ooooo";
#else
"xx";
#endif

#endif

#define HHH ahahah
#undef HHH

#ifdef HHH  // false

#ifdef HUGA  // false
"xxxx";
#else
"oooo";
#endif
#
#
#else

"xx";
#
#endif

#ifndef HHH  // true

"xx";
#
#else

#ifndef HUGA  // true
"ooooo";
#else
"xx";
#endif

#endif

HOGE;

#ifdef COMP  // false

#include <stddef.h>

"comp"

#else

#define AHOHO 56

#endif

#pragma once

#pragma hoaho

#ifndef AHOHO  // false

#define CHOHO 3

#else

#define CHOHO AHOHO + 1

#endif

    CHOHO;
