// HuffmanCoding.cpp (fixed for portability and robustness)
#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <chrono>
#include <cstdio>      // For std::FILE, std::fopen, std::fseek, std::ftell, std::fclose
#include <cstring>     // For std::memset
using namespace std;

// Portable file size function (since <filesystem> may not be available)
size_t get_file_size(const string& filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) return 0;
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fclose(f);
    return sz;
}

class Node {
public:
    char ch;
    int freq;
    Node* left;
    Node* right;

    Node(char c, int f) : ch(c), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        return a->freq > b->freq;
    }
};

class HuffmanCoding {
private:
    Node* root;
    unordered_map<char, string> huffmanCode;
    int treeNodes = 0, maxDepth = 0;

    void buildCode(Node* node, string str, int depth = 0) {
        if (!node) return;
        treeNodes++;
        if (depth > maxDepth) maxDepth = depth;
        if (!node->left && !node->right) {
            huffmanCode[node->ch] = str;
        }
        buildCode(node->left, str + "0", depth + 1);
        buildCode(node->right, str + "1", depth + 1);
    }

    void writeTree(Node* node, ofstream& out) {
        if (!node) return;
        if (!node->left && !node->right) {
            out << '1' << node->ch;
        } else {
            out << '0';
            writeTree(node->left, out);
            writeTree(node->right, out);
        }
    }

    Node* readTree(ifstream& in) {
        char bit;
        in.get(bit);
        if (!in) return nullptr;
        if (bit == '1') {
            char ch;
            in.get(ch);
            if (!in) return nullptr;
            return new Node(ch, 0);
        }
        Node* node = new Node('\0', 0);
        node->left = readTree(in);
        node->right = readTree(in);
        return node;
    }

    void freeTree(Node* node) {
        if (!node) return;
        freeTree(node->left);
        freeTree(node->right);
        delete node;
    }

public:
    HuffmanCoding() : root(nullptr) {}
    ~HuffmanCoding() { freeTree(root); }

    void compress(const string& inputFile, const string& outputFile, bool verbose = false) {
        auto start = chrono::high_resolution_clock::now();

        ifstream in(inputFile, ios::binary);
        if (!in) {
            cerr << "Error: Cannot open input file: " << inputFile << endl;
            return;
        }
        unordered_map<char, int> freqMap;
        char ch;
        while (in.get(ch)) freqMap[ch]++;
        in.clear(); in.seekg(0);

        if (freqMap.empty()) {
            cerr << "Error: Input file is empty or unreadable." << endl;
            in.close();
            return;
        }

        priority_queue<Node*, vector<Node*>, Compare> pq;
        for (auto& pair : freqMap) pq.push(new Node(pair.first, pair.second));

        while (pq.size() > 1) {
            Node* left = pq.top(); pq.pop();
            Node* right = pq.top(); pq.pop();
            Node* merged = new Node('\0', left->freq + right->freq);
            merged->left = left;
            merged->right = right;
            pq.push(merged);
        }

        root = pq.top();
        buildCode(root, "");

        ofstream out(outputFile, ios::binary);
        if (!out) {
            cerr << "Error: Cannot open output file: " << outputFile << endl;
            in.close();
            return;
        }
        writeTree(root, out);
        out << '\n';

        string encoded;
        while (in.get(ch)) encoded += huffmanCode[ch];

        int extraBits = (8 - (encoded.size() % 8)) % 8;
        if (extraBits > 0) encoded += string(extraBits, '0');
        out.put(static_cast<unsigned char>(extraBits));
        for (size_t i = 0; i < encoded.size(); i += 8) {
            string byteStr = encoded.substr(i, 8);
            bitset<8> byte(byteStr);
            out.put(static_cast<unsigned char>(byte.to_ulong()));
        }

        in.close();
        out.close();

        if (verbose) {
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            size_t inputSize = get_file_size(inputFile);
            size_t outputSize = get_file_size(outputFile);
            double ratio = (inputSize == 0) ? 0.0 : 100.0 * (1.0 - (double)outputSize / inputSize);
            cout << "\n🔹 Compression Stats:\n";
            cout << "   ➤ Input Size        : " << inputSize / 1024.0 << " KB\n";
            cout << "   ➤ Compressed Size   : " << outputSize / 1024.0 << " KB\n";
            cout << "   ➤ Compression Ratio : " << ratio << " %\n";
            cout << "   ➤ Huffman Tree Nodes: " << treeNodes << ", Max Depth: " << maxDepth << "\n";
            cout << "   ⏱️  Time Taken       : " << duration.count() << " ms\n\n";
        }
    }

    void decompress(const string& inputFile, const string& outputFile, bool verbose = false) {
        auto start = chrono::high_resolution_clock::now();
        ifstream in(inputFile, ios::binary);
        if (!in) {
            cerr << "Error: Cannot open input file: " << inputFile << endl;
            return;
        }
        root = readTree(in);
        if (!root) {
            cerr << "Error: Failed to read Huffman tree from file." << endl;
            in.close();
            return;
        }
        char next = in.peek();
        if (next == '\n') in.get();

        int extraBits = in.get();
        if (in.eof() || in.fail()) {
            cerr << "Error: Unexpected end of file or read error after tree." << endl;
            in.close();
            return;
        }
        string bitString = "";
        char byte;
        while (in.get(byte)) {
            bitset<8> bits(static_cast<unsigned char>(byte));
            bitString += bits.to_string();
        }
        if (extraBits > 0 && extraBits <= 8 && bitString.size() >= (size_t)extraBits)
            bitString = bitString.substr(0, bitString.size() - extraBits);

        ofstream out(outputFile, ios::binary);
        if (!out) {
            cerr << "Error: Cannot open output file: " << outputFile << endl;
            in.close();
            return;
        }
        Node* current = root;
        for (char bit : bitString) {
            if (bit == '0') current = current->left;
            else if (bit == '1') current = current->right;
            else continue;
            if (!current->left && !current->right) {
                out.put(current->ch);
                current = root;
            }
        }

        in.close();
        out.close();

        if (verbose) {
            auto end = chrono::high_resolution_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
            size_t inputSize = get_file_size(inputFile);
            size_t outputSize = get_file_size(outputFile);
            cout << "\n🔹 Decompression Stats:\n";
            cout << "   ➤ Compressed Size : " << inputSize / 1024.0 << " KB\n";
            cout << "   ➤ Output Size     : " << outputSize / 1024.0 << " KB\n";
            cout << "   ⏱️  Time Taken     : " << duration.count() << " ms\n\n";
        }
    }
};

int main() {
    HuffmanCoding h;
    string inputFile, outputFile;
    char choice;
    bool verbose = true;

    cout << "=== HUFFMAN COMPRESSION TOOL ===" << endl;
    cout << "1. Compress a file" << endl;
    cout << "2. Decompress a file" << endl;
    cout << "3. Exit" << endl;
    cout << "Enter your choice (1-3): ";
    cin >> choice;

    switch(choice) {
        case '1':
            cout << "\n=== COMPRESSION MODE ===" << endl;
            cout << "Enter input file name: ";
            cin >> inputFile;
            cout << "Enter output compressed file name: ";
            cin >> outputFile;
            cout << "\nCompressing...\n";
            h.compress(inputFile, outputFile, verbose);
            cout << "Compression completed!\n";
            break;
            
        case '2':
            cout << "\n=== DECOMPRESSION MODE ===" << endl;
            cout << "Enter compressed file name: ";
            cin >> inputFile;
            cout << "Enter output decompressed file name: ";
            cin >> outputFile;
            cout << "\nDecompressing...\n";
            h.decompress(inputFile, outputFile, verbose);
            cout << "Decompression completed!\n";
            break;
            
        case '3':
            cout << "Goodbye!\n";
            break;
            
        default:
            cout << "Invalid choice!\n";
            break;
    }
    
    return 0;
}