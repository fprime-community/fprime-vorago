// ======================================================================
// \title config-vorago/RawTimeSource.hpp
// \brief Vorago-specific RawTime source selection enumeration
// ======================================================================
#ifndef CONFIG_VORAGO_RAWTIMESOURCE_HPP_
#define CONFIG_VORAGO_RAWTIMESOURCE_HPP_

namespace Os {

//! \brief Timer source selection for RawTime on Vorago VA416x0 platform
//!
//! The Vorago VA416x0 platform provides multiple timer sources:
//! - RAWTIME_DEFAULT: Uses the platform's default cascade register timer (maintains backward compatibility)
//! - RAWTIME_TIMER_SINGLE: Single-register timer (high resolution, no cascade)
enum RawTimeSource {
    RAWTIME_DEFAULT = 0,      //!< Platform's default timer (current behavior)
    RAWTIME_TIMER_SINGLE = 2  //!< Single-register timer (high resolution)
};

constexpr RawTimeSource defaultRawTimeSource = RawTimeSource::RAWTIME_DEFAULT;

}  // namespace Os

#endif  // CONFIG_VORAGO_RAWTIMESOURCE_HPP_
