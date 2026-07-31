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
//! - RAWTIME_DEFAULT: Uses the platform's default timer (maintains backward compatibility)
//! - RAWTIME_SYSTICK: ARM Cortex-M SysTick timer
//! - RAWTIME_TIMER_CASCADE: High-resolution cascade timer
enum RawTimeSource {
    RAWTIME_DEFAULT = 0,       //!< Platform's default timer (current behavior)
    RAWTIME_SYSTICK = 1,       //!< ARM SysTick timer
    RAWTIME_TIMER_CASCADE = 2  //!< High-resolution cascade timer
};

}  // namespace Os

#endif  // CONFIG_VORAGO_RAWTIMESOURCE_HPP_
