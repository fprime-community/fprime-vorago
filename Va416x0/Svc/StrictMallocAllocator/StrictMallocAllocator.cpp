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
#include <unistd.h> // For sbrk()
#include <Fw/Types/Assert.hpp>
#include <algorithm>  // included for max
#include <fprime-baremetal/Os/MemoryIdScope/MemoryIdScope.hpp>
#include <fprime-baremetal/Os/OverrideNewDelete/OverrideNewDelete.hpp>

#if defined(__clang__) || defined(__GNUG__)
constexpr size_t ARRAY_OVERHEAD = sizeof(size_t);
#else
static_assert(false, "you need to determine the size of your implementation's array overhead");
const size_t ARRAY_OVERHEAD = 0;  // Declaration prevents additional diagnostics about ARRAY_OVERHEAD being undefined;
                                  // the value used does not matter.
#endif
// operator overload from example in
// https://wiki.sei.cmu.edu/confluence/display/cplusplus/mem54-cpp.+provide+placement+new+with+properly+aligned+pointers+to+sufficient+storage+capacity
void* operator new[](size_t n, void* p, size_t bufsize) {
    FW_ASSERT(n <= bufsize, n, bufsize);
    return p;
}

// End of heap (used to calculate free memory)
extern char __heap_end[];
// Start of heap (used to calculate total heap memory)
extern char __heap_start[];

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
        sizeof(std::atomic<FwSizeType>) * this->m_numIds + std::max(ARRAY_OVERHEAD, alignof(std::atomic<FwSizeType>));
    size_t actSize;
    void* memory = wrapMalloc(reqSize, actSize);
    FW_ASSERT(memory != nullptr);
    // Create the array
    this->m_allocations = ::new (memory, reqSize) std::atomic<FwSizeType>[this->m_numIds];
    // Track how much memory this class allocated and how has been allocated up to now
    this->m_internalAllocation = actSize;
    this->m_preRegisterAllocation = Os::Baremetal::OverrideNewDelete::registerMemAllocator(this);
    FW_ASSERT(this->m_internalAllocation <= this->m_preRegisterAllocation, this->m_preRegisterAllocation,
              this->m_internalAllocation);
}

StrictMallocAllocator::~StrictMallocAllocator() {}

void* StrictMallocAllocator::allocate(const FwEnumStoreType identifier,
                                      FwSizeType& size,
                                      bool& recoverable,
                                      FwSizeType alignment) {
    // FIXME: alignment is ignored right now b/c MallocAllocator::allocate() ignores it too
    FW_ASSERT(this->m_allowAllocation.load() == true);
    FwEnumStoreType id = identifier;
    if (identifier == Os::Baremetal::MemoryIdScope::DEFAULT_ID) {
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
        FW_ASSERT(id >= 0 && id < this->m_numIds, id, size, this->m_numIds);
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
    // Currently asserting on deallocate because the memory tracking approach being
    // used doesn't work for deallocation AND because the only project
    // using this feature doesn't call deallocate().
    // If this assert ever trips, this function should be updated to use mallinfo()
    // before & after the free to determine how much memory was released and that
    // needs to be tested to verify it works correctly.
    FW_ASSERT(false, identifier, FwAssertArgType(reinterpret_cast<PlatformPointerCastType>((ptr))));
    ::free(ptr);
}

FwSizeType StrictMallocAllocator::getAllocationInternal() {
    return this->m_internalAllocation;
}
FwSizeType StrictMallocAllocator::getAllocationById(FwEnumStoreType identifier) {
    FW_ASSERT(this->m_allocations != nullptr);
    FW_ASSERT(this->m_numIds > identifier && identifier >= 0, identifier, this->m_numIds);
    return this->m_allocations[identifier];
}
FwSizeType StrictMallocAllocator::getSystemAllocation() {
    struct mallinfo mi = mallinfo();
    return static_cast<FwSizeType>(mi.uordblks);
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
    // Calculate remainder
    // FIXME: This is a pretty naive method for calculating free space, but it works as a
    // decent approximation and mallinfo.fordblks is always reporting zero
    void* heap_current = sbrk(0);
    size_t remaining_heap = static_cast<size_t>(__heap_end - static_cast<char*>(heap_current));

    size_t total_heap = static_cast<size_t>(__heap_end - __heap_start);

    printf("MEM: pre-tracking allocated  %10d bytes\n", preReg - this->m_internalAllocation);
    printf("MEM: internally allocated    %10d bytes\n", this->m_internalAllocation);
    printf("MEM: total allocated         %10d bytes\n", total);
    printf("MEM: stdlib allocated        %10d bytes\n", mi.uordblks);
    printf("MEM: unaccounted for         %10d bytes\n", mi.uordblks - total);
    printf("MEM: unused heap             %10d bytes\n", remaining_heap);
    printf("MEM: total heap              %10d bytes\n", total_heap);
    
    printf("End of MEM report\n");
}
}  // namespace Va416x0Svc
