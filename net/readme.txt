
                     d1[28x28]       dd1[28x28][5x5]

conv1                w1[20][5x5]
out 20               b1[20]
kernel 5
stride 1

                     d2[24x24][20]

pool1
MAX
kernel 2
stride 2

                     d3[12x12][20]   dd3[12x12][20][5x5]

conv2                w3[50][20x5x5]
out 50               b3[50]
kernel 5
stride 1

                     d4[8x8][50]

pool2
MAX
kernel 2
stride 2

                     d5[4x4][50]

InnerProduct         w5[500][4x4x50]
out 500              b5[500]

                     d6[500]

InnerProduct         w6[10][500]
out 10               b6[10]

                     d7[10]
