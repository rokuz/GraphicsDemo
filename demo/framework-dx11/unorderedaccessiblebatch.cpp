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

#include "stdafx.h"
#include "unorderedaccessiblebatch.h"

namespace framework
{

UnorderedAccessibleBatch::UnorderedAccessibleBatch()
{
	for (int i = 0; i < D3D11_PS_CS_UAV_REGISTER_COUNT; i++)
	{
		elements[i] = nullptr;
		initialValues[i] = -1;
	}
	elementsNumber = 0;
}

void UnorderedAccessibleBatch::add(std::shared_ptr<UnorderedAccessBuffer> buffer, unsigned int initialValue)
{
	if (elementsNumber >= D3D11_PS_CS_UAV_REGISTER_COUNT)
	{
		utils::Logger::toLog("Error: unordered accessible batch is overflown");
		return;
	}
	ID3D11UnorderedAccessView* view = buffer->getView().asUAView();
	elements[elementsNumber] = view;
	initialValues[elementsNumber] = initialValue;
	elementsNumber++;
}

void UnorderedAccessibleBatch::add(std::shared_ptr<RenderTarget> renderTarget, unsigned int initialValue)
{
	if (elementsNumber >= D3D11_PS_CS_UAV_REGISTER_COUNT)
	{
		utils::Logger::toLog("Error: unordered accessible batch is overflown.\n");
		return;
	}

	int cnt = renderTarget->getViewCount();
	int rt = 0;
	for (; elementsNumber < D3D11_PS_CS_UAV_REGISTER_COUNT && rt < cnt; elementsNumber++, rt++)
	{
		ID3D11UnorderedAccessView* view = renderTarget->getView(rt).asUAView();
		elements[elementsNumber] = view;
		initialValues[elementsNumber] = initialValue;
	}
	if (rt != cnt)
	{
		utils::Logger::toLog("Warning: some UAVs have not been added to a batch.\n");
	}

}

}