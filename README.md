🗜️ Huffman Compression — Lossless Compression Tool in C++
Efficiently compress and decompress text files using Huffman Coding Algorithm implemented in C++.


📌 Overview
Huffman Compression is a lossless compression project built in C++ that allows you to reduce the size of large text files by encoding data using the frequency of characters. This implementation ensures original data is perfectly recovered after decompression, making it suitable for file storage and transmission.

🚀 Features
📦 Compress and decompress any plain text file.

🧠 Implements Huffman Encoding Tree with bit-level optimization.

🔐 Ensures Lossless Compression (100% recovery).

📁 Binary file packing with embedded frequency table and header.

💬 Verbose Mode support for debugging & CLI interaction.

💡 Clean, modular OOP-based implementation in C++.

🛠️ Technologies Used
Tech	Usage
C++	Core implementation
STL (Priority Queue, Map)	Tree and frequency logic
File I/O	Read/Write binary & text files
Bit Manipulation	Efficient encoding/decoding
Object-Oriented Design	Clean and maintainable code structure

🧪 How It Works
Build Frequency Map: Counts character frequencies from the input file.

Build Huffman Tree: Creates a min-heap based tree using frequencies.

Encode Input: Encodes input file into a compressed binary using generated codes.

Write Header + Binary: Saves metadata + compressed binary to a .bin file.

Decompress: Reads the header, rebuilds the tree, decodes back to original.

⚙️ Setup & Run Instructions
🖥️ Compilation
      g++ huffman.cpp -o huffman

📥 Compress a File
      ./huffman -c input.txt compressed.bin

📤 Decompress the File
      ./huffman -d compressed.bin output.txt
