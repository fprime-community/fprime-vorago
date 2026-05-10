/*
**
*/

#ifndef ISR_PRIORITY_CFG_HPP_
#define ISR_PRIORITY_CFG_HPP_

#include "Fw/FPrimeBasicTypes.hpp" // FIXME is this needed?

namespace Scythe {
namespace IsrPriorityCfg {

// Interrupt priority values based on execution groups
// Formula: NVIC_Priority = Execution_Group * 0x20

constexpr U8 PRIORITY_GROUP_0 = 0x00;
constexpr U8 PRIORITY_GROUP_1 = 0x20;
constexpr U8 PRIORITY_GROUP_2 = 0x40;
constexpr U8 PRIORITY_GROUP_3 = 0x60;
constexpr U8 PRIORITY_GROUP_4 = 0x80;
constexpr U8 PRIORITY_GROUP_5 = 0xA0;
constexpr U8 PRIORITY_GROUP_6 = 0xC0;
constexpr U8 PRIORITY_GROUP_7 = 0xE0;

}  // namespace IsrPriorityCfg
}  // namespace Scythe

#endif  // ISR_PRIORITY_CFG_HPP_
