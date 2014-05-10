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

#ifndef __GEOM_FORMAT_H__
#define __GEOM_FORMAT_H__

namespace geom
{

// Format description:
//
// 4 bytes		- magic number
// 3 x 4 bytes	- bounding box min (x, y, z)
// 3 x 4 bytes	- bounding box max (x, y, z)
// 4 bytes		- components count in vertex declaration
// 4 bytes		- number of additional UVs
// 4 bytes		- size of vertex (in bytes)
// for each vertex component:
//		4 bytes	- size of a vertex component (in bytes)
//		4 bytes	- offset of a vertex component (in bytes)
// 4 bytes		- number of meshes
// for each mesh:
//		4 bytes	- offset of a mesh in index buffer (in bytes)
//		4 bytes	- number of indices in a mesh (in bytes)
// X bytes		- vertex buffer
// Y bytes		- index buffer

const size_t MAGIC_GEOM = 0x12345002;

}

#endif