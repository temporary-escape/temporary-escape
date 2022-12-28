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
#include <queue>
#include <set>

namespace vez {
class Device;

class SyncPrimitivesPool {
public:
    SyncPrimitivesPool(Device* device);

    ~SyncPrimitivesPool();

    VkResult AcquireFence(VkFence* pFence);
    void ReleaseFence(VkFence fence);
    bool Exists(VkFence fence);

    VkResult AcquireSemaphore(uint32_t semaphoreCount, VkSemaphore* pSemaphores);
    void ReleaseSemaphores(uint32_t semaphoreCount, const VkSemaphore* pSemaphores);
    bool Exists(VkSemaphore semaphore);

private:
    Device* m_device = nullptr;
    std::set<VkFence> m_allFences;
    std::set<VkSemaphore> m_allSemaphores;
    std::queue<VkFence> m_availableFences;
    std::queue<VkSemaphore> m_availableSemaphores;
    SpinLock m_fenceLock, m_semaphoreLock;
};
} // namespace vez