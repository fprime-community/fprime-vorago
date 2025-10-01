/**
 * \file
 * \author B. Duckett
 * \brief Implementation of StrictMallocAllocator class
 *
 *
 */

#include "StrictMallocAllocator.hpp"
#include <malloc.h>  // for mallinfo()
#include <stdio.h>
#include <Fw/Types/Assert.hpp>
#include <algorithm>  // included for max
#include <fprime-baremetal/Os/OverrideNewDelete/OverrideNewDelete.hpp>

#if defined(__clang__) || defined(__GNUG__)
const size_t OVERHEAD = sizeof(size_t);
#else
static_assert(false, "you need to determine the size of your implementation's array OVERHEAD");
const size_t OVERHEAD =
    0;  // Declaration prevents additional diagnostics about OVERHEAD being undefined; the value used does not matter.
#endif
// operator overload from example in
// https://wiki.sei.cmu.edu/confluence/display/cplusplus/mem54-cpp.+provide+placement+new+with+properly+aligned+pointers+to+sufficient+storage+capacity
void* operator new[](size_t n, void* p, size_t bufsize) {
    FW_ASSERT(n <= bufsize, n, bufsize);
    return p;
}

namespace Va416x0Svc {

// Do a malloc and calculate how much memory is actually consumed
static void* wrapMalloc(size_t size, size_t& actualSize) {
    struct mallinfo mi = mallinfo();
    size_t origU = mi.uordblks;
    void* mem = ::malloc(static_cast<size_t>(size));
    mi = mallinfo();
    actualSize = mi.uordblks - origU;
    return mem;
}

StrictMallocAllocator::StrictMallocAllocator() : m_numIds(0), m_internalAllocation(0), m_preRegisterAllocation(0) {
    this->m_allowAllocation.store(true);
}

void StrictMallocAllocator::setDefaultId(FwEnumStoreType defaultId) {
    this->m_defaultId = defaultId;
}

void StrictMallocAllocator::setup(FwEnumStoreType numIds, FwEnumStoreType defaultId) {
    FW_ASSERT(numIds > 0 && defaultId >= 0, numIds, defaultId);
    this->m_numIds = numIds;
    this->m_defaultId = defaultId;
    // Calculate size of m_allocations array & then allocate that memory
    FwSizeType reqSize =
        sizeof(std::atomic<FwSizeType>) * this->m_numIds + std::max(OVERHEAD, alignof(std::atomic<FwSizeType>));
    size_t actSize;
    void* memory = wrapMalloc(reqSize, actSize);
    FW_ASSERT(memory != nullptr);
    // Create the array
    memset(memory, 0, reqSize);  // sets initial state
    this->m_allocations = ::new (memory, reqSize) std::atomic<FwSizeType>[this->m_numIds];
    // Track how much memory this class allocated and how has been allocated up to now
    this->m_internalAllocation = actSize;
    this->m_preRegisterAllocation = Os::Baremetal::OverrideNewDelete::registerMemAllocator(this);
    FW_ASSERT(this->m_internalAllocation <= this->m_preRegisterAllocation, this->m_preRegisterAllocation,
              this->m_internalAllocation);
}

StrictMallocAllocator::~StrictMallocAllocator() {}

void* StrictMallocAllocator::allocate(const FwEnumStoreType identifier, FwSizeType& size, bool& recoverable) {
    FW_ASSERT(this->m_allowAllocation.load() == true);
    FwEnumStoreType id = identifier;
    if (identifier == Os::Baremetal::OverrideNewDelete::DEFAULT_ID) {
        id = this->m_defaultId;
    }
    // heap memory is never recoverable
    recoverable = false;
    size_t actualSize;
    void* mem = wrapMalloc(size, actualSize);
    if (nullptr == mem) {
        size = 0;  // set to zero if can't get memory
    } else {
        // Check id is a valid index and m_allocations has been allocated,
        FW_ASSERT(id >= 0 && id <= this->m_numIds, id, size, this->m_numIds);
        FW_ASSERT(this->m_allocations != nullptr);
        // Then add actualSize this ID's memory total
        this->m_allocations[id].fetch_add(actualSize);
    }
    return mem;
}
U32 StrictMallocAllocator::getNumIds() {
    return this->m_numIds;
}
void StrictMallocAllocator::deallocate(const FwEnumStoreType identifier, void* ptr) {
    FW_ASSERT(false, identifier, FwAssertArgType(reinterpret_cast<PlatformPointerCastType>((ptr))));
    ::free(ptr);
}

FwSizeType StrictMallocAllocator::getAllocationInternal() {
    return this->m_internalAllocation;
}
FwSizeType StrictMallocAllocator::getAllocationById(FwEnumStoreType identifier) {
    FW_ASSERT(this->m_allocations != nullptr);
    FW_ASSERT(this->m_numIds > identifier, identifier, this->m_numIds);
    return this->m_allocations[identifier];
}
FwSizeType StrictMallocAllocator::getSystemAllocation() {
    return 0;
}
void StrictMallocAllocator::disableAllocation() {
    this->m_allowAllocation.store(false);
}
void StrictMallocAllocator::reportAllocation() {
    // FIXME: This should be reported in EVRs (IMO. Although on clipper this was a DP or serial output)
    struct mallinfo mi = mallinfo();
    FwSizeType total = 0;
    FwSizeType preReg = this->m_preRegisterAllocation;
    total += preReg;
    printf("Start of MEM report\n");
    for (FwEnumStoreType i = 0; i < this->m_numIds; i++) {
        FwSizeType val = this->m_allocations[i].load();
        if (val != 0) {
            printf("MEM: ID %3d allocated %10u bytes\n", i, val);
            total += val;
        }
    }
    printf("MEM: pre-tracking allocated  %10d bytes\n", preReg - this->m_internalAllocation);
    printf("MEM: internally allocated    %10d bytes\n", this->m_internalAllocation);
    printf("MEM: total allocated         %10d bytes\n", total);
    printf("MEM: stdlib allocated        %10d bytes\n", mi.uordblks);
    printf("MEM: unaccounted for         %10d bytes\n", mi.uordblks - total);
    printf("End of MEM report\n");
}
}  // namespace Va416x0Svc
