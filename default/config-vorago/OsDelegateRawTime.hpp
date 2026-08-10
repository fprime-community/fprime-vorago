// ======================================================================
// \title config/OsDelegateRawTime.hpp
// \brief Vorago compile-time selection of Os::RawTime implementation
//
// This header uses compile-time selection by aliasing Os::RawTime directly
// to Va416x0Os::TimerRawTime, bypassing the link-time DelegateRawTime
// mechanism used in F Prime's default configuration.
//
// Benefits of compile-time selection:
// - Eliminates virtual function dispatch overhead
// - Enables compiler to inline methods
// - Better optimization opportunities

// ======================================================================
#ifndef CONFIG_OS_DELEGATERAWTIME_HPP
#define CONFIG_OS_DELEGATERAWTIME_HPP

//!< Forward declaration of the concrete TimerRawTime implementation
namespace Va416x0Os {
class TimerRawTime;
}
namespace Os {

using RawTime = Va416x0Os::TimerRawTime;  //!< Compile-time alias to TimerRawTime (not DelegateRawTime)

}  // namespace Os

#define OS_RAW_TIME_HEADER "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"

#endif  // CONFIG_OS_DELEGATERAWTIME_HPP
