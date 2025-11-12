/**
 * \file
 * \brief A MemAllocator implementation class that uses malloc and tracks memory
 * allocations (depends on the fprime-baremetal OverrideNewDelete functionality)
 *
 *
 */

#ifndef TYPES_STRICTMALLOCALLOCATOR_HPP_
#define TYPES_STRICTMALLOCALLOCATOR_HPP_

#include <Fw/FPrimeBasicTypes.hpp>
#include <Fw/Types/MemAllocator.hpp>
#include <atomic>

namespace Va416x0Svc {

/*!
 *
 * This class is an implementation of the MemAllocator base class.
 * It uses the heap as the memory source.
 *
 * The identifier is used to identify the client requesting data
 * all data allocations are tracked
 *
 */

class StrictMallocAllocator : public Fw::MemAllocator {
  public:
    StrictMallocAllocator();
    virtual ~StrictMallocAllocator();

    //! Register this allocator with Os::Baremetal::OverrideNewDelete to
    //! use for future new calls, store the number of bytes allocated
    //! before that was done and the number of bytes allocated for any
    //! dynamic memory used by this class
    void setup(FwEnumStoreType numIds, FwEnumStoreType defaultId);
    //! Sets the ID that will be used whenever allocate() is called with Os::Baremetal::OverrideNewDelete::DEFAULT_ID
    void setDefaultId(FwEnumStoreType defaultId);
    //! Get the internal dynamic memory allocated
    FwSizeType getAllocationInternal();
    //! Get the dynamic memory allocated for a given ID
    FwSizeType getAllocationById(FwEnumStoreType identifier);
    //! Get dynamic memory allocated according to picolibrary
    FwSizeType getSystemAllocation();
    //! Gets the number of IDs allocation is tracked
    U32 getNumIds();
    //! Disable further dynamic memory allocation
    void disableAllocation();
    //! Report memory allocation
    void reportAllocation();
    /*! Allocate memory
     * \param identifier the memory segment identifier (not used)
     * \param size the requested size (not changed)
     * \param recoverable - flag to indicate the memory could be recoverable (always set to false)
    //! \param alignment - alignment requirement for the allocation. Default: maximum alignment defined by C++.
     * \return the pointer to memory. Zero if unable to allocate.
     */
    void* allocate(const FwEnumStoreType identifier,
                   FwSizeType& size,
                   bool& recoverable,
                   FwSizeType alignment = alignof(std::max_align_t));
    /*! Deallocate memory
     * \param identifier the memory segment identifier (not used)
     * \param ptr the pointer to memory returned by allocate()
     */
    void deallocate(const FwEnumStoreType identifier, void* ptr);

  private:
    //! If False, assert when allocate is called
    std::atomic<bool> m_allowAllocation;
    //! Length of the allocations array to allocate
    FwEnumStoreType m_numIds;
    //! ID to use if Os::Baremetal::OverrideNewDelete::DEFAULT_ID is passed to allocate
    FwEnumStoreType m_defaultId;
    //! Number of bytes dynamically allocated for this class
    FwSizeType m_internalAllocation;
    //! Number of bytes allocated before
    //! Os::Baremetal::OverrideNewDelete::registerMemAllocator() called
    FwSizeType m_preRegisterAllocation;
    //! Dynamically allocated array with the number of bytes
    //! allocated for each memory ID
    std::atomic<FwSizeType>* m_allocations;
};

} /* namespace Va416x0Svc */

#endif /* TYPES_STRICTMALLOCALLOCATOR_HPP_ */
