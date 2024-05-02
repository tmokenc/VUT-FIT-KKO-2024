/**
 * @file huffman.c
 * @author Le Duy Nguyen (xnguye27)
 * @date 25/04/2024
 * @brief Defines Canoncial Huffman coding
 */

#include <stdlib.h>
#include "error.h"
#include "bit_array.h"
#include "huffman.h"
#include <string.h>

/// 256 + EOF
#define ALPHABET_LEN 257
#define EOF_BYTE (ALPHABET_LEN - 1)
#define HUFFMAN_NULL_VAL (ALPHABET_LEN)

typedef uint64_t Frequency;

typedef struct {
    uint64_t code;
    uint8_t len;
} Code;

typedef Code CodeBook[ALPHABET_LEN];

typedef struct {
    uint16_t character;
    Frequency frequency;
    Code code;
} Symbol;

typedef struct {
    Symbol data[ALPHABET_LEN];
    size_t size;
} Symbols;

typedef struct {
    Frequency frequency;
    size_t m;
} Node;

typedef struct {
    Node data[ALPHABET_LEN]; ///< No need for dynamic allocation here since maximum number of node will always be the alphabet length.
    size_t size;
} AlphabetMinHeap;

typedef struct node {
    uint16_t value; ///< ALPHABET_LEN == NULL
    struct node *left;
    struct node *right;
} HuffmanNode;

void symbols_from_bytes(Symbols *symbols, uint8_t *bytes, size_t len);
void symbols_push(Symbols *symbols, Symbol symbol);
void symbols_to_codebook(Symbols *symbols, CodeBook codebook);
void symbols_calc_code(Symbols *symbols);
void symbols_calc_code_len(Symbols *symbols);
void symbols_sort(Symbols *symbols);
void symbols_encode(Symbols *symbols, BitArray *output);
void symbols_decode(Symbols *symbols, BitArray *input);

void alphabet_min_heap_push(AlphabetMinHeap *heap, Frequency freq, size_t m);
Node alphabet_min_heap_pop(AlphabetMinHeap *heap);

HuffmanNode *huffman_node_new();
void huffman_node_free(HuffmanNode *root);
void huffman_node_insert(HuffmanNode *root, Symbol *symbol);
uint16_t huffman_node_read_next(HuffmanNode *root, BitArray *input);

HuffmanNode *build_huffman_tree(Symbols *symbols);

void bit_array_push_code(BitArray *arr, Code code);

BitArray huffman_compress(uint8_t *bytes, size_t len) {
    #define COMPRESS_ERROR_GUARD(func) func; \
        if ( got_error()) { \
            bit_array_free(&result); \
            return result; \
        }

    BitArray result = bit_array_new(NULL, 0);

    log("Creating list of symbols");
    Symbols symbols;
    symbols_from_bytes(&symbols, bytes, len);
    log("Calculate code len of symbols");
    symbols_calc_code_len(&symbols);

    log("Make codebook out of symbols");
    /// Code book
    CodeBook codebook;
    symbols_to_codebook(&symbols, codebook);

    log("Encoding codebook into the output");
    COMPRESS_ERROR_GUARD(symbols_encode(&symbols, &result));

    int count = 0;
    log("Encoding the huffman coding into the output");
    /// Encode
    for (size_t i = 0; i < len; i++) {
        Code code = codebook[bytes[i]];
        logfmt("Pushing char %d as %ld with length %d", bytes[i], code.code, code.len);
        COMPRESS_ERROR_GUARD(bit_array_push_code(&result, code));
        count += 1;
    }

    Code eof = codebook[ALPHABET_LEN-1];
    logfmt("Pushing EOF as %ld with length %d", eof.code, eof.len);
    COMPRESS_ERROR_GUARD(bit_array_push_code(&result, eof));

    logfmt("Compressed to %ld bytes", bit_array_byte_len(&result));

    return result;
}

BitArray huffman_decompress(uint8_t *bytes, size_t len) {
    #define DECOMPRESS_ERROR_GUARD(func, on_error) func; \
        if ( got_error()) { \
            on_error; \
            bit_array_free(&input); \
            bit_array_free(&result); \
            return result; \
        }

    BitArray input = bit_array_new(bytes, len);
    BitArray result = bit_array_new(NULL, 0);

    log("Decoding symbol list");
    Symbols symbols;
    DECOMPRESS_ERROR_GUARD(symbols_decode(&symbols, &input), );

    log("Building huffman tree");
    HuffmanNode *root = build_huffman_tree(&symbols);
    DECOMPRESS_ERROR_GUARD(, );

    uint16_t byte;

    int count = 0;
    log("Decompressing");
    while ((byte = huffman_node_read_next(root, &input)) != EOF_BYTE) {
        if (got_error()) {
            break;
        }

        logfmt("Decompressed 0x%02X", byte);

        DECOMPRESS_ERROR_GUARD(bit_array_push_n(&result, byte, 8), huffman_node_free(root));
        count += 1;
    }

    bit_array_free(&input);
    huffman_node_free(root);

    return result;
}

void symbols_from_bytes(Symbols *symbols, uint8_t *bytes, size_t len) {
    memset(symbols, 0, sizeof(Symbols));

    Frequency freq[ALPHABET_LEN] = {0};
    freq[ALPHABET_LEN - 1] = 1; // EOF

    for (size_t i = 0; i < len; i++) {
        freq[bytes[i]] += 1;
    }

    for (int i = 0; i < ALPHABET_LEN; i++) {
        if (!freq[i]) {
            continue;
        }

        Symbol symbol = {0};
        symbol.character = i;
        symbol.frequency = freq[i];

        symbols_push(symbols, symbol);
        logfmt("Symbol %d has frequency %ld", i, freq[i]);
    }
}

void symbols_push(Symbols *symbols, Symbol symbol) {
    symbols->data[symbols->size++] = symbol;
}

void symbols_to_codebook(Symbols *symbols, CodeBook codebook) {
    memset(codebook, 0, sizeof(CodeBook));

    symbols_calc_code(symbols);

    for (size_t i = 0; i < symbols->size; i++) {
        codebook[symbols->data[i].character] = symbols->data[i].code;
    }
}

void symbols_calc_code(Symbols *symbols) {
    symbols_sort(symbols);

    symbols->data[0].code.code = 0;
    logfmt("calc_code: Character %d has code %d with size %d", symbols->data[0].character, 0, symbols->data[0].code.len);

    for (size_t i = 1; i < symbols->size; i++) {
        Code previous = symbols->data[i-1].code;
        Symbol *symbol = &symbols->data[i];
        uint16_t diff = symbol->code.len - previous.len;

        symbol->code.code = (previous.code + 1) << diff;
        logfmt("calc_code: Character %d has code %ld with size %d", symbol->character, symbol->code.code, symbol->code.len);
    }

}

void symbols_calc_code_len(Symbols *symbols) {
    log("Calculating code len");
    if (!symbols->size) {
        return;
    }

    size_t m = symbols->size;
    AlphabetMinHeap heap = {0};
    uint16_t hr[ALPHABET_LEN * 2] = {0};

    for (size_t i = 0; i < symbols->size; i++) {
        alphabet_min_heap_push(&heap, symbols->data[i].frequency, m + i);
    }

    /// Hirschberg-Sieminski
    while (m > 1) {
        Node a = alphabet_min_heap_pop(&heap);
        Node b = alphabet_min_heap_pop(&heap);

        m -= 1;

        hr[a.m] = m;
        hr[b.m] = m;

        alphabet_min_heap_push(&heap, a.frequency + b.frequency, m);
    }

    for (size_t i = 0; i < symbols->size; i++) {
        int j = hr[symbols->size + i];
        int l = 1;

        while (j > 1) {
            j = hr[j];
            l += 1;
        }

        symbols->data[i].code.len = l;
        logfmt("Symbol %d has code length of %d", symbols->data[i].character, l);
    }
}

/// Perform insertion sort
void symbols_sort(Symbols *symbols) {
    for (size_t i = 1; i < symbols->size; i++) {
        Symbol symbol = symbols->data[i];
        int j = i - 1;

        while (j >= 0 && symbols->data[j].code.len > symbol.code.len) {
            symbols->data[j + 1] = symbols->data[j];
            j -= 1;
        }

        symbols->data[j + 1] = symbol;
    }
}


// Function to swap two nodes in the heap
void swap_nodes(Node *a, Node *b) {
    Node temp = *a;
    *a = *b;
    *b = temp;
}

// Function to heapify the heap upwards after insertion
void heapify_up(AlphabetMinHeap *heap, size_t index) {
    while (index > 0) {
        size_t parent_index = (index - 1) / 2;
        if (heap->data[parent_index].frequency > heap->data[index].frequency) {
            swap_nodes(&heap->data[parent_index], &heap->data[index]);
            index = parent_index;
        } else {
            break;
        }
    }
}

// Function to heapify the heap downwards after removal
void heapify_down(AlphabetMinHeap *heap, size_t index) {
    size_t left_child_index = 2 * index + 1;
    size_t right_child_index = 2 * index + 2;
    size_t smallest = index;

    if (left_child_index < heap->size && heap->data[left_child_index].frequency < heap->data[smallest].frequency) {
        smallest = left_child_index;
    }

    if (right_child_index < heap->size && heap->data[right_child_index].frequency < heap->data[smallest].frequency) {
        smallest = right_child_index;
    }

    if (smallest != index) {
        swap_nodes(&heap->data[index], &heap->data[smallest]);
        heapify_down(heap, smallest);
    }
}

void alphabet_min_heap_push(AlphabetMinHeap *heap, Frequency freq, size_t m) {
    if (heap->size >= ALPHABET_LEN) {
        // Heap is full
        fprintf(stderr, "ERR alphabet_min_heap_push: Pushing more than expected\n");
        set_error(Error_InternalError);
        return;
    }

    size_t index = heap->size;
    heap->data[index].frequency = freq;
    heap->data[index].m = m;
    heap->size++;

    heapify_up(heap, index);
}

Node alphabet_min_heap_pop(AlphabetMinHeap *heap) {
    if (heap->size == 0) {
        // Heap is empty
        Node empty_node = {0}; // Assuming Frequency and m are initialized to 0
        return empty_node;
    }

    Node min_node = heap->data[0];
    heap->size--;

    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        heapify_down(heap, 0);
    }

    return min_node;
}


void symbols_encode(Symbols *symbols, BitArray *output) {
    bit_array_push_n(output, symbols->size - 2, 8); // Not including EOF

    uint8_t eof_code_len = 0;
    for (size_t i = 0; i < symbols->size; i++) {
        Symbol symbol = symbols->data[i];

        if (symbol.character == EOF_BYTE) {
            eof_code_len = symbol.code.len;
            continue;
        }

        bit_array_push_n(output, symbol.character, 8);
        bit_array_push_n(output, symbol.code.len - 1, 8); // -1 since 0 cannot be the code length
    }

    bit_array_push_n(output, eof_code_len - 1, 8); // EOF be the last
}

void symbols_decode(Symbols *symbols, BitArray *input) {
    memset(symbols, 0, sizeof(Symbols));

    uint16_t size = bit_array_read_n(input, 8) + 1;

    for (uint16_t i = 0; i < size; i++) {
        Symbol symbol = {0};

        symbol.character = bit_array_read_n(input, 8);
        if (got_error()) return;

        symbol.code.len = bit_array_read_n(input, 8) + 1;
        if (got_error()) return;

        symbols_push(symbols, symbol);
    }

    Symbol eof = {0};
    eof.character = EOF_BYTE;
    eof.code.len = bit_array_read_n(input, 8) + 1;
    if (got_error()) return;
    symbols_push(symbols, eof);

    symbols_calc_code(symbols);
}

HuffmanNode *huffman_node_new() {
    HuffmanNode *node = malloc(sizeof(HuffmanNode));

    if (!node) {
        fprintf(stderr, "ERR huffman_node_new: Cannot allocate space for new node\n");
        set_error(Error_OutOfMemory);
        return NULL;
    }

    node->value = HUFFMAN_NULL_VAL;
    node->left = NULL;
    node->right = NULL;

    return node;
}

void huffman_node_free(HuffmanNode *node) {
    if (node) {
        free(node->left);
        free(node->right);
        free(node);
    }
}

void huffman_node_insert(HuffmanNode *node, Symbol *symbol) {
    uint64_t path = symbol->code.code;
    uint16_t index = symbol->code.len - 1;
    logfmt("huffman insert code %ld with %d", path, symbol->character);

    for (int i = 0; i < symbol->code.len; i++) {
        HuffmanNode **next = NULL;
        if ((path >> index) & 1) {
            next = &node->left;
        } else {
            next = &node->right;
        }

        if (*next == NULL) {
            *next = huffman_node_new();
            if (got_error()) return;
        }

        node = *next;
        index -= 1;
    }

    node->value = symbol->character;
}

uint16_t huffman_node_read_next(HuffmanNode *node, BitArray *input) {
    int depth = 0;
    while (node->value == HUFFMAN_NULL_VAL) {
        HuffmanNode **next = NULL;

        if (bit_array_read(input)) {
            next = &node->left;
        } else {
            next = &node->right;
        }

        if (*next == NULL) {
            fprintf(stderr, "ERR huffman_node_read_next: Cannot find the matching prefix\n");
            set_error(Error_IndexOutOfBound);
            return 0;
        }

        node = *next;
        depth += 1;
    }

    logfmt("read depth %d with %d", depth, node->value);

    return node->value;
}

HuffmanNode *build_huffman_tree(Symbols *symbols) {
    HuffmanNode *root = huffman_node_new();
    if (got_error()) return NULL;

    for (size_t i = 0; i < symbols->size; i++) {
        huffman_node_insert(root, &symbols->data[i]);

        if (got_error()) {
            huffman_node_free(root);
            return NULL;
        }
    }

    return root;
}

void bit_array_push_code(BitArray *arr, Code code) {
    /// Pushing using order MSB 
    int index = code.len - 1;
    for (int i = 0; i < code.len; i++) {
        bool bit = (code.code >> index) & 1;
        bit_array_push(arr, bit);
        if (got_error()) return;
        index -= 1;
    }
}

