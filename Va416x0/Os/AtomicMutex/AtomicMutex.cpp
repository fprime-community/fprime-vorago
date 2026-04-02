// ======================================================================
// \title Os/Stub/Mutex.cpp
// \brief stub implementation for Os::Mutex
// ======================================================================
#include "AtomicMutex.hpp"
#include "Os/Delegate.hpp"

namespace Va416x0Os {
namespace AtomicMutex {

Os::MutexInterface::Status AtomicMutex::take() {
    // Attempt to mark the mutex as taken.
    if (this->m_handle.m_mutex_taken.exchange(true)) {
        // The mutex was already taken, so fail the operation.
        // (This stub is for platforms without the ability to block.)
        return Os::MutexInterface::Status::ERROR_BUSY;
    }
    // The mutex was not already taken.
    // Now that it has been marked as taken, we have successfully entered the critical section.
    return Os::MutexInterface::Status::OP_OK;
}

Os::MutexInterface::Status AtomicMutex::release() {
    // Attempt to mark the mutex as not taken.
    if (!this->m_handle.m_mutex_taken.exchange(false)) {
        // The mutex was already not taken, which indicates a coding defect.
        return Os::MutexInterface::Status::ERROR_OTHER;
    }
    // The mutex was taken.
    // Now that it has been marked as not taken, we have successfully exited the critical section.
    return Os::MutexInterface::Status::OP_OK;
}

Os::MutexHandle* AtomicMutex::getHandle() {
    return &this->m_handle;
}
}  // namespace AtomicMutex
}  // namespace Va416x0Os


namespace Os {

//! \brief get a delegate for MutexInterface that intercepts calls for stub file usage
//! \param aligned_new_memory: aligned memory to fill
//! \param to_copy: pointer to copy-constructor input
//! \return: pointer to delegate
MutexInterface* MutexInterface::getDelegate(MutexHandleStorage& aligned_new_memory) {
    return Os::Delegate::makeDelegate<MutexInterface, Va416x0Os::AtomicMutex::AtomicMutex>(aligned_new_memory);
}

}
