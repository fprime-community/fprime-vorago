// ======================================================================
// \title  AdcSamplerTester.cpp
// \author root
// \brief  cpp file for AdcSampler component test harness implementation class
// ======================================================================

#include "AdcSamplerTester.hpp"
#include  "Va416x0/Mmio/Amba/Amba.hpp"

namespace Va416x0 {

// FIXME These are just copied from AdcSampler.cpp
static inline U32 REQ_GET_CHAN_EN(U32 request) {
    return request >> 16;
}

static inline U32 REQ_GET_CNT(U32 request) {
    return ((request >> 12) & 0xf);
}

static inline U32 REQ_GET_MUX_ENABLE(U32 request) {
    return ((request >> 8) & 0xf);
}

static inline U32 REQ_GET_MUX_CHAN(U32 request) {
    return ((request >> 2) & 0x1f);
}

static inline U32 REQ_GET_IS_MUX(U32 request) {
    return ((request >> 1) & 0x1);
}

static inline U32 REQ_GET_IS_SWEEP(U32 request) {
    return ((request) & 0x1);
}

constexpr U32 GPIO_ADDRESS = 0x40012000;

enum {
    GPIO_PORT_STRIDE = 0x400,
    GPIO_EXCEPTION_BASE = Va416x0Types::ExceptionNumber::INTERRUPT_PORTA_0,
    GPIO_EXCEPTION_STRIDE = 16,
    GPIO_CASCADE_BASE = 0,
    GPIO_CASCADE_STRIDE = 16,

    DATAIN = 0x000,
    DATAINRAW = 0x004,
    DATAOUT = 0x008,
    DATAOUTRAW = 0x00C,
    SETOUT = 0x010,
    CLROUT = 0x014,
    TOGOUT = 0x018,
    DATAMASK = 0x01C,
    DIR = 0x020,
    PULSE = 0x024,
    PULSEBASE = 0x028,
    DELAY1 = 0x02C,
    DELAY2 = 0x030,
    IRQ_SEN = 0x034,
    IRQ_EDGE = 0x038,
    IRQ_EVT = 0x03C,
    IRQ_ENB = 0x040,
    IRQ_RAW = 0x044,
    IRQ_END = 0x048,
    EDGE_STATUS = 0x04C,
};

static U32 read_gpio_dataout(U32 gpio_port)  {
    return Va416x0Mmio::Amba::read_u32(GPIO_ADDRESS | (gpio_port * GPIO_PORT_STRIDE) | DATAOUT);
}

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

AdcSamplerTester ::AdcSamplerTester()
    : AdcSamplerGTestBase("AdcSamplerTester", AdcSamplerTester::MAX_HISTORY_SIZE), component("AdcSampler") {
    this->initComponents();
    this->connectPorts();
}

AdcSamplerTester ::~AdcSamplerTester() {}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

// // Configuration for a muxless adc sample config
// Va416x0::AdcConfig no_mux_config = {
//     0,
//     9,  
//     Va416x0Mmio::Gpio::PORTA,
//     {8, 7, 6, 5, 0, 1, 2, 3, 4},
//     // Address Pins
//     {Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTA, 0), Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTA, 0),
//      Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTA, 0), Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTA, 0),
//      Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTA, 0)}};

// Va416x0::AdcRequests no_mux_requests = {
//     adc_sampler_request(1 << 0, 15, 0, 0, 0, 0),
//     adc_sampler_request(1 << 1, 15, 0, 0, 1, 1),
//     adc_sampler_request(1 << 2, 15, 0, 0, 2, 2),
//     adc_sampler_request(1 << 3, 15, 0, 0, 3, 3),
//     adc_sampler_request(1 << 4, 15, 0, 0, 4, 4),
//     adc_sampler_request(1 << 5, 15, 0, 0, 5, 5),
//     adc_sampler_request(1 << 6, 15, 0, 0, 6, 6),
//     adc_sampler_request(1 << 7, 15, 0, 0, 7, 7),
//     adc_sampler_request(1 << 8, 15, 0, 0, 8, 8),
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0,
//     0};

Va416x0::AdcConfig three_mux_pin_config = {
    5,
    3,  
    Va416x0Mmio::Gpio::PORTA,
    {1, 5, 3, 0, 0, 0, 0, 0, 0, 0},
    // Address Pins
    {Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTB, 0), Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTB, 1),
     Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTC, 2), Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTD, 3),
     Va416x0Mmio::Gpio::Pin(Va416x0Mmio::Gpio::PORTE, 4)}};

Va416x0::AdcRequests three_mux_pin_config_requests = {
    // AV 1
    adc_sampler_request(1 << 0, 15, 0, 1, 0, 31),
    adc_sampler_request(1 << 0, 15, 0, 1, 1, 1),
    adc_sampler_request(1 << 0, 15, 0, 1, 2, 2),
    adc_sampler_request(1 << 0, 15, 0, 1, 0, 3),
    adc_sampler_request(1 << 0, 15, 0, 1, 1, 4),
    adc_sampler_request(1 << 0, 15, 0, 1, 2, 16),
    adc_sampler_request(1 << 0, 15, 0, 1, 1, 24),
    adc_sampler_request(1 << 0, 15, 0, 1, 0, 0),
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0};

void AdcSamplerTester ::testStartReadMuxEnableDisableDelay() {
   
    this->component.setup(three_mux_pin_config, 0xe0, 20, Va416x0Mmio::Timer(18));
    printf("Testing MUX index 0, pin 1, port A\n");
    {
        // Get the mux enable gpio port
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(three_mux_pin_config.gpio_port);
        // Check the dummy value is set
        EXPECT_TRUE(REQ_GET_MUX_ENABLE(this->component.m_lastMuxRequest) == ADC_MUX_PINS_EN_MAX);

        this->component.startRead_handlerBase(0, 8, three_mux_pin_config_requests, this->m_data);
        EXPECT_TRUE(this->component.m_requestIdx == 0);

        // Read in the port pin values
        U32 pin_values = read_gpio_dataout(three_mux_pin_config.gpio_port.get_gpio_port());
        printf("pin values 0x%08x\n", pin_values);
        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_ena = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_ena);
        // Confirm that only a single mux is enabled (0) and all others are 1
        EXPECT_TRUE((pin_values & (1 << three_mux_pin_config.mux_en_output[req_mux_ena])) == 0);
        EXPECT_TRUE((pin_values & (1 << 5)));
        EXPECT_TRUE((pin_values & (1 << 3)));
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIdx]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }

    // Go to next request
    printf("Testing MUX index 1, pin 5, port A\n");
    this->component.m_requestIdx.fetch_add(1);
    EXPECT_TRUE(this->component.m_requestIdx == 1);
    this->component.startReadInner();
    {
        // Get the mux enable gpio port
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(three_mux_pin_config.gpio_port);
        // Read in the port pin values
        U32 pin_values = read_gpio_dataout(three_mux_pin_config.gpio_port.get_gpio_port());
        printf("pin values 0x%08x\n", pin_values);
        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_ena = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_ena);
        // Confirm that only a single mux is enabled (0) and all others are 1
        EXPECT_TRUE((pin_values & (1 << three_mux_pin_config.mux_en_output[req_mux_ena])) == 0);
        EXPECT_TRUE((pin_values & (1 << 1)));
        EXPECT_TRUE((pin_values & (1 << 3)));
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIdx]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }

    // Go to next request
    printf("Testing MUX index 2, pin 3, port A\n");
    this->component.m_requestIdx.fetch_add(1);
    EXPECT_TRUE(this->component.m_requestIdx == 2);
    this->component.startReadInner();
    {
        // Get the mux enable gpio port
        Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(three_mux_pin_config.gpio_port);
        // Read in the port pin values
        U32 pin_values = read_gpio_dataout(three_mux_pin_config.gpio_port.get_gpio_port());
        printf("pin values 0x%08x\n", pin_values);
        // For the current request get the enable channel pin, in this case request index 0 uses mux enable 0
        U32 req_mux_ena = REQ_GET_MUX_ENABLE(this->component.m_curRequest);
        printf("Mux enable pin %d\n", req_mux_ena);
        // Confirm that only a single mux is enabled (0) and all others are 1
        EXPECT_TRUE((pin_values & (1 << three_mux_pin_config.mux_en_output[req_mux_ena])) == 0);
        EXPECT_TRUE((pin_values & (1 << 1)));
        EXPECT_TRUE((pin_values & (1 << 5)));
        // Confirm the requests match as expected
        EXPECT_TRUE(this->component.m_curRequest == three_mux_pin_config_requests[this->component.m_requestIdx]);
        // Confirm the last mux request changed
        EXPECT_TRUE(this->component.m_lastMuxRequest == this->component.m_curRequest);
    }
}

void AdcSamplerTester ::testStartReadGpioConfiguration() {
    this->component.setup(three_mux_pin_config, 0xe0, 20, Va416x0Mmio::Timer(18));
    printf("Testing address indexing\n");
    {
        for (U32 idx = 0; idx < three_mux_pin_config.num_en_pins; idx++) {
            Va416x0Mmio::Gpio::Port gpioPort = Va416x0Mmio::Gpio::Port(three_mux_pin_config.mux_addr_output[idx].getGpioPortNumber());
            gpioPort.write_dataout(0);
        }
        this->component.startRead_handlerBase(0, 8, three_mux_pin_config_requests, this->m_data);
        EXPECT_TRUE(this->component.m_requestIdx == 0);
        for (U32 idx = 0; idx < three_mux_pin_config.num_en_pins; idx++) {
            U32 pin_number = three_mux_pin_config.mux_addr_output[idx].getPinNumber();

            EXPECT_TRUE(read_gpio_dataout(three_mux_pin_config.mux_addr_output[idx].getGpioPortNumber()) & static_cast<U32>(1 << pin_number));
        }

    }
}

void AdcSamplerTester ::testSetup() {
}






}  // namespace Va416x0
