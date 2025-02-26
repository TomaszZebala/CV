#include "Multiplier_tb.h"
#include "hls_stream.h"
#include "ap_int.h"
#include <iostream>

using namespace std;

hls::stream<axi_8bit_blk> inputStream("inputStream");
hls::stream<axi_8bit_blk> outputStream("outputStream");

void simulate_buttons(ap_uint<BTNS>& buttons) {
    buttons = 1;
}

void generate_input_data() {
    axi_8bit_blk data_in;
    for (int i = 0; i < 10; i++) {
        data_in.data = i;
        data_in.last = (i == 9);
        inputStream.write(data_in);
    }
}

void check_output_data() {
    axi_8bit_blk data_out;
    while (!outputStream.empty()) {
        outputStream.read(data_out);
        cout << "Output data: " << (int)data_out.data << " Last: " << (int)data_out.last << endl;
    }
}

int main() {
    ap_uint<BTNS> buttons;
    simulate_buttons(buttons);

    generate_input_data();

    multiplier_top(inputStream, outputStream, buttons);

    check_output_data();

    return 0;
}
