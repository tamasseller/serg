## Self Extracting Resource Generator

This C++ software module can be used to embed losslessly compressed resources into executables, without any external dependencies. This allows for use in small embedded systems (i.e. MCUs) where there are no compression libraries available by default, in contrast to actual computer application environments where these are readily available.

### Goals

 1. Smallest possible decompression routine footprint.
 2. No hard dependency on the external environment.
 3. Easy integration into existing build processes.
 4. Useful compression ratio for common high-redundancy resources.
 5. Compression ratio scales well for small files.

### How to use

TBD

### How it works

Similarly to any common general purpose data compression algorithm serg is composed of:

  1. An LZ* step that is supposed to reduce global redundance by deduplication of reoccuring substrings,
  2. An entropy coder, that is range ANS in this case.
  
Both of these are implemented in such a way that the decompression routine is as simple and small as possible. 
They read, write and use data as a stream of bytes (as opposed to bit streams typically used in older entropy coders).

The LZ* step  (implemented in MLZ.h) uses an intermediate representation that is a list of blocks, each of which can be either literal or back-reference. 
Literal blocks contain a portion of the uncompressed data as-is. 
Back-references contain a length and an offset relative to the position of the block. These are used to capture re-occuring parts of the uncompressed data.

The range ANS entropy coder (implemented in Rans.h) is used minimize symbol level redundancy by mapping each byte value to variably sized intervals based on the frequency of the values. 
In this implementation the frequency of byte values are not encoded explicitly, instead this information is deduced from the stream during coding (adaptivity).
The mapping is updated only after an MLZ block is processed.
