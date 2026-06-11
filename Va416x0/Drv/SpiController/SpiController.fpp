module Va416x0Drv {
    
    constant MAX_SPI_SUBORDINATES = 8 

    @ Executes main-mode SPI transactions on an individual VA41630 SPI peripheral
    passive component SpiController {

        sync input port SpiReadWrite: [MAX_SPI_SUBORDINATES] Drv.SpiReadWrite

    }
}
