module Va416x0 {

    module TlmGdsChanCfg {

        @ Size of each ping-pong buffer used by TlmGdsChan. This must be sized appropriately in order
        @ to fit all channels for any single deployment (currently 3 KiB)
        dictionary constant PingPongBufferSize = 3072
    }
}
