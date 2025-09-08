# Copyright 2025 California Institute of Technology
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

module Va416x0 {

    @ Status values returned from reading
    enum AdcSamplerStatus{
        SUCCESS
        BUSY
    }

    # FIXME:  Long term, I'd like for each deployment to be able to configure it since 
    # I expect different systems will have different ADC needs
    # To reduce noise, the Vorago programmer's guide recommends reading samples
    # multiple times and then averaging the result. So it's reasonable to assume that 
    # reading a measurement 16x before reading the next will be a common use case. 
    # The vorago only completes 21.5  16x reads (340) within 1 ms RTI, setting this to 32. 
    @ Maximum size of the request arrays - value TBR
    constant ADC_MAX_REQUEST_SIZE = 32

    @ Maximum size of the data arrays - value TBR
    constant ADC_MAX_DATA_SIZE = 32

    @ Values read from an ADC channel are 12-bits (there is an option to store the 
    @ channel ID in the top 4 bits, but it's not enabled) 
    @ If a channel is read multiple times (limit 16), the sum of all reads is 
    @ stored as the value in this array
    @ Larger than  
    array AdcData = [ADC_MAX_DATA_SIZE] U16

    @ AdcSampler & AdcCollector will convert each entry in this array to a bit-packed 
    @ struct 4-bytes in size but FPP doesn't support bit packed arrays, so use U32s instead
    array AdcRequests = [ADC_MAX_REQUEST_SIZE] U32 

}