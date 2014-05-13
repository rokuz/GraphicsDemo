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

#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <sstream>
#include <functional>
#include <utility>
#include <fstream>

#include "matrix.h"
#include "vector.h"
#include "quaternion.h"
#include "ncamera2.h"
#include "bbox.h"

#include <windows.h>
#include "structs.h"

#include "window.h"

#include "logger.h"
#include "utils.h"
#include "timer.h"
#include "profiler.h"
#include "inputkeys.h"
#include "fpscounter.h"
#include "profiler.h"

#include "outputd3d11.h"
#include "destroyable.h"
#include "resourceview.h"

#include "pipelinestage.h"
#include "rasterizerstage.h"
#include "depthstencilstage.h"
#include "blendstage.h"

#include "renderTarget.h"
#include "geometry3D.h"
#include "line3D.h"
#include "texture.h"
#include "sampler.h"
#include "uniformbuffer.h"
#include "unorderedaccessbuffer.h"
#include "unorderedaccessiblebatch.h"

#include "gpuprogram.h"
#include "standardgpuprograms.h"

#include "freeCamera.h"
#include "lightManager.h"

#include "uimanager.h"
#include "uifactoryd3d11.h"

#include "application.h"

#undef min
#undef max