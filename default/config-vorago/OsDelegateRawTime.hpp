// ======================================================================
// \title config/OsDelegateRawTime.hpp
// \brief configured selection of Os OSAL implementations
//
// This header selects, at compile time, which implementation each Os OSAL
// symbol (e.g. Os::RawTime) refers to. By default the symbol is an alias for
// the link-time delegate (e.g. Os::DelegateRawTime), preserving the historical
// behavior of selecting the implementation at link time.
//
// A platform that prefers compile-time implementation selection (for the
// performance benefit of avoiding indirect/virtual dispatch) may override this
// header in its own config/ folder and alias the symbol directly to a concrete
// implementation, for example:
//
//     namespace Os::Posix::RawTime { class PosixRawTime; }
//     namespace Os { using RawTime = Os::Posix::RawTime::PosixRawTime; }
//
// Only a forward declaration of the concrete type is required here: Os interface
// classes reference the alias solely through references to incomplete types.
// ======================================================================
#ifndef CONFIG_OS_DELEGATERAWTIME_HPP
#define CONFIG_OS_DELEGATERAWTIME_HPP

//!< Forward declaration of the link-time delegate
namespace Va416x0Os {
class TimerRawTime;
}
namespace Os {

using RawTime = Va416x0Os::TimerRawTime;  //!< Default: select the vorago implementation at link time

}  // namespace Os

#define OS_RAW_TIME_HEADER "Va416x0/Os/TimerRawTime/TimerRawTime.hpp"

#endif  // CONFIG_OS_DELEGATERAWTIME_HPP
