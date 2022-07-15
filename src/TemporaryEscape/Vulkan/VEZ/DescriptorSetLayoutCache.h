//
// Copyright (c) 2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once

#include "SpinLock.h"
#include "VEZ.h"
#include "VkHelpers.h"
#include <map>
#include <unordered_map>
#include <vector>

namespace vez {
class Device;
class DescriptorSetLayout;

class DescriptorSetLayoutCache {
public:
    DescriptorSetLayoutCache(Device* device);

    ~DescriptorSetLayoutCache();

    VkResult CreateLayout(uint32_t setIndex, const std::vector<VezPipelineResource>& setResources,
                          DescriptorSetLayout** pLayout);

    void DestroyLayout(DescriptorSetLayout* layout);

private:
    Device* m_device = nullptr;
    std::map<DescriptorSetLayoutHash, DescriptorSetLayout*> m_layouts;
    std::unordered_map<DescriptorSetLayout*, uint32_t> m_layoutReferences;
    SpinLock m_spinLock;
};
} // namespace vez