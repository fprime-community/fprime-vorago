/*
** This header file should be included by deployments in order to link to the auto-generated
** telemetry configuration table source files.
*/

#ifndef TLM_CFG_HPP_
#define TLM_CFG_HPP_

#include "Fw/FPrimeBasicTypes.hpp"
#include "Fw/Types/MemAllocator.hpp"

namespace Va416x0 {
namespace TlmCfg {

//! The total number of channels in the deployment
extern const U32 CHANNEL_COUNT;

//! The total number of components in the deployment; note that this is calculated using the
//! maximum component ID in the deployment so it encompasses any gaps between the IDs
extern const U32 COMPONENT_COUNT;

//! Channel sizes, in bytes. This table can be indexed using the component lookup table
extern const U8 CHANNEL_TABLE[];

//! Element in the top-level component channel lookup table
struct ComponentLookup {
    U16 offset;  //!< Offset of the component block of channels
    U16 length;  //!< Number of channels in the component
};

//! Create the component lookup table using heap-allocated memory from the given allocator
ComponentLookup* createComponentLookupTable(FwEnumStoreType memId, Fw::MemAllocator& allocator);

}  // namespace TlmCfg
}  // namespace Va416x0

#endif  // TLM_CFG_HPP_
