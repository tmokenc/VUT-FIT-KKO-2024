#import "@preview/starter-journal-article:0.3.0": article, author-meta
#import "@preview/bytefield:0.0.5": *
#import table: cell
#show: article.with(
  title: "KKO Project 2024",
  authors: (
    "Nguyen Le Duy": author-meta(
      "FIT",
      email: "xnguye27@vutbr.cz",
    ),
  ),
  affiliations: (
    "FIT": "Brno University of Technology. Faculty of Information Technology",
  ),
  abstract: [],
  keywords: ("Huffman Coding", "RLE", "Image compression"),
  bib: bibliography("./refs.yml", style: "institute-of-electrical-and-electronics-engineers")
)

#set par(
  first-line-indent: 0pt,
  justify: true,
)

- *Programing Language*: C
- *Development OS*: Manjaro Linux
- *GIT*: https://github.com/tmokenc/VUT-FIT-KKO-2024 (private, will only be public after the submission deadline)
= Implementation

== Data Structures

=== Bit Array
Files: `bit_array.h` | `bit_array.c`

A dynamically growing bit array implemented on the heap with built-in cursor for reading from it easily.

=== Image
Files: `image.h` | `image.c`

Provides various methods for creating and manipulating an image buffer.

== Run-length encoding (RLE) 
Files: rle.h | rle.c

The RLE technique used is based on the third option for encoding greyscale images from the presentation by doc. Vasicek @presentation. This method utilizes one bit to distinguish the byte type. The bits are grouped in sets of eight, with each group written into the output stream before the 8 bytes it belongs to. This incurs a 12.5% overhead in the output but allows each RLE segment to have up to 257 repeat values, as 0 or 1 repeat values are predetermined.

== Model 
Files: `transform.h` | `transform.c`

The model used is delta encoding (@delta), which stores the difference between two pixels.

== Huffman coding
Files: `huffman.h` | `huffman.c`

The Huffman coding is implemented using canonical Huffman coding for effectively storing the codebook into the output by just storing the length and value (without its frequency).

The alphabet consists of numerical values ranging from 0 to 255, plus an End-Of-File (EOF) symbol, making the total length 257.

Several data structures are required for Huffman coding:

- *Symbols*: A list of symbols used to construct Huffman codes.
- *AlphabetMinHeap*: A min-heap implementation using an array.
- *Codebook*: A list of Huffman codes used for encoding.
- *HuffmanTree*: A binary tree that stores the Huffman codes for decoding. This structure is heap-allocated.

Since the maximum size of the data to be stored in these data structures is known beforehand (the size of the alphabet), most of them are stack-allocated, which eliminates the need for dynamic memory allocation.

For simplicity, these data structure implementations are located within `huffman.c`, as they are used directly for Huffman encoding and decoding.

= Data Representation
The first 2 bytes represent the width of the image, and the next 2 bytes represent the height, meaning the maximum size of the image is $2^16=65536$ pixels for both width and height. Following these bytes is the data section. All parts together are then compressed using Huffman coding.

#bytefield(
  bitheader(
    "bits",
    0,
    16,
    32,
    96,
  ),
  bits(16, fill: lime.lighten(30%))[Width],
  bits(16, fill: lime.lighten(30%))[Height],
  bits(64)[Pre-processed Data]
  
)

== Preprocessed data
Before being encoded with Huffman coding, the data is preprocessed in various ways, which can be toggled using command line parameters.

=== Static
Parameters: `None`

The entire image buffer is passed directly into Huffman encoding.

=== Model
Parameter: `-m`

Applies a model (delta encoding) before sending the data to RLE and then Huffman encoding.

=== Adaptive
Parameters: `-a`

The image is split into blocks with a maximum size of 32x32 pixels, configurable via the -b parameter. Each block is stored in one of four forms, whichever provides the smallest compression size:

+ *Raw*: No preprocessing is performed.
+ *Horizontal*: Image data is serialized horizontally.
+ *Vertical*: Image data is serialized vertically.
+ *Circular*: Image data is serialized in a circular/spiral pattern.

All forms except *Raw* are then encoded using the RLE method.

#grid(
  columns: 3,
  gutter: 10pt,

  figure(
    image("horizontal.png"),
    caption: [Horizontal @algorithm],
  ),
  figure(
    image("vertical.png"),
    caption: [Vertical @algorithm],
  ),
  figure(
    image("circular.png"),
    caption: [Circular @algorithm],
  )
)

Since there are four forms, 2 bits of metadata overhead are needed for each block. All metadata is grouped together and rounded up to the next multiple of 8 to fit in bytes. For example, if the image size is 300x200 pixels and the block size is 16x16, the total metadata overhead is $ ceil(300 / 16) * ceil(200 / 16) * 2 = 19 * 13 = 494$ bits, which equals $ceil(494/8) = 62$ bytes. This metadata overhead increases exponentially as block size decreases.

The metadata group is stored first, followed by the block data.

=== Adaptive Model
Parameter: `-m -a`

Combines both Adaptive and Model, meaning each block will have the model applied before being sent to RLE.

= Benchmark

According to benchmarks using data available on Moodle, the `Static Model` version outperforms other versions due to having no overhead. `Adaptive` and `Adaptive Model` versions perform similarly, sometimes even outperforming the `Static Model` in specific cases. Determining the optimal block size is challenging; however, based on benchmark observations, the optimal block size appears to be $ceil(sqrt("image_size"))$.

The following table shows the benchmark results with a block size of 128 (default).

#table(
  columns: 6,
  table.header(
    [Filename],
    [Entropy],
    [Mode],
    [Compressed],
    [Effectiveness],
    [Time],
  ),

table.cell(rowspan: 4)[df1h.raw],
table.cell(rowspan: 4)[8],
[Static],cell(align: right)[282374],cell(align: right)[8.61],cell(align: right)[0,035 s],
[Static Model],cell(align: right)[417],cell(align: right)[0.01],cell(align: right)[0,003 s],
[Adaptive],cell(align: right)[3276],cell(align: right)[0.09],cell(align: right)[0,013 s],
[Adaptive Model],cell(align: right)[1989],cell(align: right)[0.06],cell(align: right)[0,013 s],
table.cell(rowspan: 4)[df1hvx.raw],
table.cell(rowspan: 4)[4.51],
[Static],cell(align: right)[82622],cell(align: right)[2.52],cell(align: right)[0,026 s],
[Static Model],cell(align: right)[28090],cell(align: right)[0.85],cell(align: right)[0,014 s],
[Adaptive],cell(align: right)[134389],cell(align: right)[4.10],cell(align: right)[0,028 s],
[Adaptive Model],cell(align: right)[78031],cell(align: right)[2.38],cell(align: right)[0,015 s],
table.cell(rowspan: 4)[df1v.raw],
table.cell(rowspan: 4)[8],
[Static],cell(align: right)[1953],cell(align: right)[0.05],cell(align: right)[0,003 s],
[Static Model],cell(align: right)[848],cell(align: right)[0.02],cell(align: right)[0,004 s],
[Adaptive],cell(align: right)[3276],cell(align: right)[0.09],cell(align: right)[0,011 s],
[Adaptive Model],cell(align: right)[1992],cell(align: right)[0.06],cell(align: right)[0,016 s],
table.cell(rowspan: 4)[hd01.raw],
table.cell(rowspan: 4)[3.83],
[Static],cell(align: right)[103579],cell(align: right)[3.16],cell(align: right)[0,026 s],
[Static Model],cell(align: right)[92305],cell(align: right)[2.81],cell(align: right)[0,015 s],
[Adaptive],cell(align: right)[119814],cell(align: right)[3.65],cell(align: right)[0,026 s],
[Adaptive Model],cell(align: right)[120161],cell(align: right)[3.66],cell(align: right)[0,027 s],
table.cell(rowspan: 4)[hd02.raw],
table.cell(rowspan: 4)[3.64],
[Static],cell(align: right)[98204],cell(align: right)[2.99],cell(align: right)[0,017 s],
[Static Model],cell(align: right)[90388],cell(align: right)[2.75],cell(align: right)[0,015 s],
[Adaptive],cell(align: right)[114169],cell(align: right)[3.48],cell(align: right)[0,026 s],
[Adaptive Model],cell(align: right)[114604],cell(align: right)[3.49],cell(align: right)[0,023 s],
table.cell(rowspan: 4)[hd07.raw],
table.cell(rowspan: 4)[5.58],
[Static],cell(align: right)[160874],cell(align: right)[4.90],cell(align: right)[0,014 s],
[Static Model],cell(align: right)[112812],cell(align: right)[3.44],cell(align: right)[0,014 s],
[Adaptive],cell(align: right)[184173],cell(align: right)[5.62],cell(align: right)[0,033 s],
[Adaptive Model],cell(align: right)[184173],cell(align: right)[5.62],cell(align: right)[0,035 s],
table.cell(rowspan: 4)[hd08.raw],
table.cell(rowspan: 4)[4.21],
[Static],cell(align: right)[116221],cell(align: right)[3.54],cell(align: right)[0,019 s],
[Static Model],cell(align: right)[104646],cell(align: right)[3.19],cell(align: right)[0,017 s],
[Adaptive],cell(align: right)[135116],cell(align: right)[4.12],cell(align: right)[0,032 s],
[Adaptive Model],cell(align: right)[135351],cell(align: right)[4.13],cell(align: right)[0,023 s],
table.cell(rowspan: 4)[hd09.raw],
table.cell(rowspan: 4)[6.62],
[Static],cell(align: right)[221818],cell(align: right)[6.76],cell(align: right)[0,034 s],
[Static Model],cell(align: right)[155555],cell(align: right)[4.74],cell(align: right)[0,029 s],
[Adaptive],cell(align: right)[218487],cell(align: right)[6.66],cell(align: right)[0,030 s],
[Adaptive Model],cell(align: right)[218487],cell(align: right)[6.66],cell(align: right)[0,050 s],
table.cell(rowspan: 4)[hd12.raw],
table.cell(rowspan: 4)[6.17],
[Static],cell(align: right)[180299],cell(align: right)[5.50],cell(align: right)[0,025 s],
[Static Model],cell(align: right)[126682],cell(align: right)[3.86],cell(align: right)[0,017 s],
[Adaptive],cell(align: right)[203442],cell(align: right)[6.20],cell(align: right)[0,031 s],
[Adaptive Model],cell(align: right)[203442],cell(align: right)[6.20],cell(align: right)[0,031 s],
table.cell(rowspan: 4)[nk01.raw],
table.cell(rowspan: 4)[6.47],
[Static],cell(align: right)[231943],cell(align: right)[7.07],cell(align: right)[0,033 s],
[Static Model],cell(align: right)[213942],cell(align: right)[6.52],cell(align: right)[0,034 s],
[Adaptive],cell(align: right)[213520],cell(align: right)[6.51],cell(align: right)[0,034 s],
[Adaptive Model],cell(align: right)[213520],cell(align: right)[6.51],cell(align: right)[0,036 s],

)
