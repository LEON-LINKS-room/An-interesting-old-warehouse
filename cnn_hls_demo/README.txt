One of my research topics during my graduate studies focused on hardware acceleration of convolutional neural networks (CNNs for edge computing applications). FPGA-based acceleration of CNNs has become a prominent research direction due to its advantages in performance, power efficiency, and flexibility.

In this demo, a basic CNN convolution module is implemented using Vivado HLS 2018.3, aiming to demonstrate how typical deep learning operators can be efficiently mapped onto hardware using high-level synthesis (HLS). The design targets a fundamental convolution operation and serves as a representative example of CNN acceleration on FPGA platforms.

The overall architecture employs the AXI-Stream interface for data input and output, enabling fully streaming-based data processing. This design eliminates the need to buffer the entire input feature map, thereby improving system throughput and reducing on-chip memory and storage resource consumption.

To support efficient convolution computation in a streaming manner, a line buffer mechanism is introduced. By maintaining a static buffer for the most recent rows of input data, the convolution window can be dynamically constructed as pixels flow through the pipeline. This approach is well suited to FPGA-based streaming computation and conforms to hardware-friendly data access patterns.

Several optimization techniques are applied to improve performance:
1.The main processing loop is constrained with HLS PIPELINE, enabling pixel-level pipelined execution;
2.The channel dimension and convolution kernel dimensions are fully unrolled, exploiting fine-grained parallelism in the convolution operation;
3.ARRAY_PARTITION is applied to the line buffers and pixel caches to reduce memory access conflicts and enhance parallel data access efficiency.

Furthermore, the convolution weights and bias are configured via an AXI-Lite interface, providing flexibility and facilitating parameter reconfiguration during system-level integration.

A specific example implementation of this design is provided in the project cnn_demo, which demonstrates the functionality on the Xilinx Zynq xc7z020clg400-2 platform.
