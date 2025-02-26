#include "Multiplier.h"


void multiplier_top(
    hls::stream<axi_8bit_blk> &inputStream,
    hls::stream<axi_8bit_blk> &outputStream,
	ap_uint<BTNS>  buttons
) {
	#pragma HLS INTERFACE mode=axis port=inputStream
	#pragma HLS INTERFACE mode=axis port=outputStream
	#pragma HLS INTERFACE mode=ap_ctrl_none port=return
	#pragma HLS INTERFACE mode=ap_none port=buttons
    uint8 factor;
    if(buttons == 0 || buttons == 3)
    	factor = 1;
    else if(buttons == 1)
    	factor = FILTER_COEFF_0;
    else if(buttons == 2)
    	factor = FILTER_COEFF_1;

    multiplier_core(inputStream, outputStream, factor);
}



void multiplier_core(
    hls::stream<axi_8bit_blk> &inStream,
    hls::stream<axi_8bit_blk> &outStream,
    const uint8 coeff
) {
	#pragma HLS INTERFACE mode=axis port=inStream
	#pragma HLS INTERFACE mode=axis port=outStream
	#pragma HLS inline off

    axi_8bit_blk data_in, data_out;
    uint16 result = 0;

    while (!inStream.empty()) {
        inStream.read(data_in);
        result = data_in.data * coeff;

        data_out.data = result & 0xFF;
        data_out.last = data_in.last;
        data_out.keep = -1;
        data_out.strb = -1;
        outStream.write(data_out);
    }
}
