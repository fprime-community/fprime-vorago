module Va416x0Drv {
    
    constant MAX_SPI_SUBORDINATES = 8 

    @ General purpose main-mode SPI driver for VA416X0
    passive component SpiController {

        sync input port SpiReadWrite: [MAX_SPI_SUBORDINATES] Drv.SpiReadWrite

    }
}
