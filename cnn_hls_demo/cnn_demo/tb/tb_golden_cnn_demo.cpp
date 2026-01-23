/*******************************************************************************
MIT License

Copyright (c) 2021 LEON-LINKS-room

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include <iostream>
#include <hls_stream.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>

//定义
#define IN_H   14
#define IN_W   14
#define IN_C   3
#define K      3
#define OUT_H  (IN_H - K + 1)
#define OUT_W  (IN_W - K + 1)

typedef ap_int<8>  data_t;
typedef ap_int<32> acc_t;
typedef ap_axis<8, 0, 0, 0> axis_t;

//DUT声明
void cnn_conv_layer(
    hls::stream<axis_t> &in_stream,
    hls::stream<axis_t> &out_stream,
    data_t weight[K][K][IN_C],
    data_t bias
);

//Golden Reference用于对比
void golden_conv(
    data_t in[IN_H][IN_W][IN_C],
    data_t out[OUT_H][OUT_W],
    data_t weight[K][K][IN_C],
    data_t bias
) {
    for (int y = 0; y < OUT_H; y++) {
        for (int x = 0; x < OUT_W; x++) {
            acc_t sum = bias;
            for (int ky = 0; ky < K; ky++) {
                for (int kx = 0; kx < K; kx++) {
                    for (int c = 0; c < IN_C; c++) {
                        sum += in[y + ky][x + kx][c] *
                               weight[ky][kx][c];
                    }
                }
            }
            out[y][x] = (data_t)sum;
        }
    }
}

int main() {

    hls::stream<axis_t> in_stream;
    hls::stream<axis_t> out_stream;

    data_t input[IN_H][IN_W][IN_C];
    data_t weight[K][K][IN_C];
    data_t bias = 1;

    data_t golden_out[OUT_H][OUT_W];

    //初始化输入
    for (int y = 0; y < IN_H; y++) {
        for (int x = 0; x < IN_W; x++) {
            for (int c = 0; c < IN_C; c++) {
                input[y][x][c] = y + x + c;
            }
        }
    }

    //初始化权重
    for (int ky = 0; ky < K; ky++) {
        for (int kx = 0; kx < K; kx++) {
            for (int c = 0; c < IN_C; c++) {
                weight[ky][kx][c] = 1;
            }
        }
    }

    //运行golden
    golden_conv(input, golden_out, weight, bias);

    //运行DUT
    for (int y = 0; y < IN_H; y++) {
        for (int x = 0; x < IN_W; x++) {
            for (int c = 0; c < IN_C; c++) {
                axis_t in;
                in.data = (ap_uint<8>)input[y][x][c];
                in.keep = -1;
                in.last = (y == IN_H - 1 &&
                           x == IN_W - 1 &&
                           c == IN_C - 1);
                in_stream.write(in);
            }
        }
    }

    cnn_conv_layer(in_stream, out_stream, weight, bias);

    //比较
    bool pass = true;
    for (int y = 0; y < OUT_H; y++) {
        for (int x = 0; x < OUT_W; x++) {
            axis_t out = out_stream.read();
            data_t dut_val = (data_t)out.data;

            if (dut_val != golden_out[y][x]) {
                std::cout << "Mismatch @("
                          << y << "," << x << ") "
                          << "DUT=" << dut_val
                          << " Golden=" << golden_out[y][x]
                          << std::endl;
                pass = false;
                goto END;
            }
        }
    }

END:
    if (pass)
        std::cout << "TEST PASSED" << std::endl;
    else
        std::cout << "TEST FAILED" << std::endl;

    return 0;
}

