#ifndef MULTIPLIER_H
#define MULTIPLIER_H

#include "ap_axi_sdata.h"
#include "hls_stream.h"
#include "ap_int.h"
#include "common.h"


#define FILTER_COEFF_0 3
#define FILTER_COEFF_1 5
#define BTNS		   2

// Deklaracje funkcji
void multiplier_top(
    hls::stream<axi_8bit_blk> &inputStream,
    hls::stream<axi_8bit_blk> &outputStream,
    ap_uint<BTNS> buttons
);

void multiplier_core(
    hls::stream<axi_8bit_blk> &inStream,
    hls::stream<axi_8bit_blk> &outStream,
    const uint8 coeff
);

#endif // FILTER_H

