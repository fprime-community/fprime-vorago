// Copyright 2025 California Institute of Technology
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

// ======================================================================
// \title Os/Generic/IsrSafeQueue.cpp
// \brief priority queue implementation for Os::Queue
// ======================================================================

#include "IsrSafeQueue.hpp"
#include <Fw/Types/Assert.hpp>
#include <cstring>
#include <new>
#include "Os/Delegate.hpp"
#include "Os/Queue.hpp"
#include "Va416x0/Mmio/Lock/Lock.hpp"

namespace Va416x0Os {
namespace IsrSafeQueue {

FwSizeType IsrSafeQueueHandle ::find_index() {
    FwSizeType index = this->m_indices[this->m_startIndex % this->m_depth];
    this->m_startIndex = (this->m_startIndex + 1) % this->m_depth;
    return index;
}

void IsrSafeQueueHandle ::return_index(FwSizeType index) {
    this->m_indices[this->m_stopIndex % this->m_depth] = index;
    this->m_stopIndex = (this->m_stopIndex + 1) % this->m_depth;
}

void IsrSafeQueueHandle ::store_data(FwSizeType index, const U8* data, FwSizeType size) {
    FW_ASSERT(size <= this->m_maxSize);
    FW_ASSERT(index < this->m_depth);

    FwSizeType offset = this->m_maxSize * index;
    (void)::memcpy(this->m_data + offset, data, static_cast<size_t>(size));
    this->m_sizes[index] = size;
}

void IsrSafeQueueHandle ::load_data(FwSizeType index, U8* destination, FwSizeType size) {
    FW_ASSERT(size <= this->m_maxSize);
    FW_ASSERT(index < this->m_depth);
    FwSizeType offset = this->m_maxSize * index;
    (void)::memcpy(destination, this->m_data + offset, static_cast<size_t>(size));
}

IsrSafeQueue::~IsrSafeQueue() {
    delete[] this->m_handle.m_data;
    delete[] this->m_handle.m_indices;
    delete[] this->m_handle.m_sizes;
}

Os::QueueInterface::Status IsrSafeQueue:: create(const Fw::ConstStringBase& name, FwSizeType depth, FwSizeType messageSize) {
    // Ensure we are created exactly once
    FW_ASSERT(this->m_handle.m_indices == nullptr);
    FW_ASSERT(this->m_handle.m_sizes == nullptr);
    FW_ASSERT(this->m_handle.m_data == nullptr);

    // Allocate indices list
    FwSizeType* indices = new (std::nothrow) FwSizeType[depth];
    if (indices == nullptr) {
        return QueueInterface::Status::ALLOCATION_FAILED;
    }
    // Allocate sizes list or clean-up
    FwSizeType* sizes = new (std::nothrow) FwSizeType[depth];
    if (sizes == nullptr) {
        delete[] indices;
        return QueueInterface::Status::ALLOCATION_FAILED;
    }
    // Allocate sizes list or clean-up
    U8* data = new (std::nothrow) U8[depth * messageSize];
    if (data == nullptr) {
        delete[] indices;
        delete[] sizes;
        return QueueInterface::Status::ALLOCATION_FAILED;
    }
    // Allocate max heap or clean-up
    bool created = this->m_handle.m_heap.create(depth);
    if (not created) {
        delete[] indices;
        delete[] sizes;
        delete[] data;
        return QueueInterface::Status::ALLOCATION_FAILED;
    }
    // Assign initial indices and sizes
    for (FwSizeType i = 0; i < depth; i++) {
        indices[i] = i;
        sizes[i] = 0;
    }
    // Set local tracking variables
    this->m_handle.m_maxSize = messageSize;
    this->m_handle.m_indices = indices;
    this->m_handle.m_data = data;
    this->m_handle.m_sizes = sizes;
    this->m_handle.m_startIndex = 0;
    this->m_handle.m_stopIndex = 0;
    this->m_handle.m_depth = depth;
    this->m_handle.m_highMark = 0;

    return QueueInterface::Status::OP_OK;
}

Os::QueueInterface::Status IsrSafeQueue::send(const U8* buffer,
                                              FwSizeType size,
                                              FwQueuePriorityType priority,
                                              QueueInterface::BlockingType blockType) {
    // Check for sizing problem before locking
    if (size > this->m_handle.m_maxSize) {
        return Os::QueueInterface::Status::SIZE_MISMATCH;
    }
    // Artificial block scope for scope lock ensuring an unlock in all cases and ensuring an unlock before notify
    {
        Va416x0Mmio::Lock::CriticalSectionLock lock;
        if (this->m_handle.m_heap.isFull()) {
            return blockType == BlockingType::BLOCKING ? Os::QueueInterface::Status::NOT_SUPPORTED
                                                       : Os::QueueInterface::Status::FULL;
        }
        FwSizeType index = this->m_handle.find_index();

        // Space must exist, push must work
        FW_ASSERT(this->m_handle.m_heap.push(priority, index));
        this->m_handle.store_data(index, buffer, size);
        this->m_handle.m_sizes[index] = size;
        this->m_handle.m_highMark = FW_MAX(this->m_handle.m_highMark, this->getMessagesAvailable());
    }
    return QueueInterface::Status::OP_OK;
}

Os::QueueInterface::Status IsrSafeQueue::receive(U8* destination,
                                                 FwSizeType capacity,
                                                 QueueInterface::BlockingType blockType,
                                                 FwSizeType& actualSize,
                                                 FwQueuePriorityType& priority) {
    {
        Va416x0Mmio::Lock::CriticalSectionLock lock;
        if (this->m_handle.m_heap.isEmpty()) {
            return blockType == BlockingType::BLOCKING ? Os::QueueInterface::Status::NOT_SUPPORTED
                                                       : Os::QueueInterface::Status::EMPTY;
        }

        FwSizeType index;
        // Message must exist, so pop must pass and size must be valid
        FW_ASSERT(this->m_handle.m_heap.pop(priority, index));
        actualSize = this->m_handle.m_sizes[index];
        FW_ASSERT(actualSize <= capacity);
        this->m_handle.load_data(index, destination, actualSize);
        this->m_handle.return_index(index);
    }
    return QueueInterface::Status::OP_OK;
}

FwSizeType IsrSafeQueue::getMessagesAvailable() const {
    return this->m_handle.m_heap.getSize();
}

FwSizeType IsrSafeQueue::getMessageHighWaterMark() const {
    Va416x0Mmio::Lock::CriticalSectionLock lock;
    return this->m_handle.m_highMark;
}

Os::QueueHandle* IsrSafeQueue::getHandle() {
    return &this->m_handle;
}

}  // namespace IsrSafeQueue
}  // namespace Va416x0Os

namespace Os {

QueueInterface* QueueInterface::getDelegate(QueueHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<QueueInterface, Va416x0Os::IsrSafeQueue::IsrSafeQueue, QueueHandleStorage>(
        aligned_new_memory);
}

}  // namespace Os
