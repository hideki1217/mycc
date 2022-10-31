"aho"
    "ahoaho";

"u"
    "n"
    "chi";

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
