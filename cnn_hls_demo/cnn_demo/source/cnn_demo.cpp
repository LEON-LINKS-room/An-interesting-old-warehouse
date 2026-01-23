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

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

#define IN_H   14
#define IN_W   14
#define IN_C   3

#define K      3

#define OUT_H  (IN_H - K + 1)
#define OUT_W  (IN_W - K + 1)

//类型定义
typedef ap_int<8>  data_t;
typedef ap_int<32> acc_t;
typedef ap_axis<8, 0, 0, 0> axis_t;

void cnn_conv_layer(
    hls::stream<axis_t> &in_stream,
    hls::stream<axis_t> &out_stream,
    data_t weight[K][K][IN_C],
    data_t bias
) {
#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE s_axilite port=weight
#pragma HLS INTERFACE s_axilite port=bias
#pragma HLS INTERFACE s_axilite port=return

    //行缓冲
    static data_t linebuf[K][IN_W][IN_C];
#pragma HLS ARRAY_PARTITION variable=linebuf complete dim=1
#pragma HLS ARRAY_PARTITION variable=linebuf complete dim=3

    for (int y = 0; y < IN_H; y++) {
        for (int x = 0; x < IN_W; x++) {
#pragma HLS PIPELINE II=1

            data_t pixel[IN_C];
#pragma HLS ARRAY_PARTITION variable=pixel complete

            //读取3个通道
            for (int c = 0; c < IN_C; c++) {
#pragma HLS UNROLL
                axis_t tmp = in_stream.read();
                pixel[c] = (data_t)tmp.data;
            }

            //行缓冲上移
            for (int ky = K - 1; ky > 0; ky--) {
                for (int c = 0; c < IN_C; c++) {
#pragma HLS UNROLL
                    linebuf[ky][x][c] = linebuf[ky - 1][x][c];
                }
            }

            //写入新行
            for (int c = 0; c < IN_C; c++) {
#pragma HLS UNROLL
                linebuf[0][x][c] = pixel[c];
            }

            //卷积计算
            if (y >= K - 1 && x >= K - 1) {
                acc_t sum = bias;

                for (int ky = 0; ky < K; ky++) {
                    for (int kx = 0; kx < K; kx++) {
                        for (int c = 0; c < IN_C; c++) {
#pragma HLS UNROLL
                            sum += linebuf[ky][x - kx][c] *
                                   weight[ky][kx][c];
                        }
                    }
                }

                axis_t out;
                out.data = (ap_uint<8>)sum;
                out.keep = -1;
                out.last = (y == IN_H - 1 && x == IN_W - 1);
                out_stream.write(out);
            }
        }
    }
}

