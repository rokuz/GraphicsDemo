/*
 * Copyright (c) 2014 Roman Kuznetsov 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma warning(disable:4996)

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <locale>
#include <mutex>
#include <functional>
#include <time.h>
#include <chrono>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#elif defined __APPLE__
#include <mach/mach_time.h>
#endif

#include "vector.h"
#include "matrix.h"
#include "quaternion.h"

#include "inputkeys.h"
#include "logger.h"
#include "timer.h"
#include "profiler.h"
#include "fpscounter.h"

#include "utils.h"

#undef min
#undef max