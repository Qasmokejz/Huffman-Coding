#include "bitwriter.h"
#include "io.h"
#include "node.h"
#include "pq.h"

#include <assert.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct Code {
    uint64_t code;
    uint8_t code_length;
} Code;

uint64_t fill_histogram(Buffer *inbuf, double *histogram) {
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }
    ++histogram[0x00];
    ++histogram[0xff];
    uint64_t filesize = 0;
    uint8_t *curr = (uint8_t *) calloc(1, sizeof(uint8_t));

    while (read_uint8(inbuf, curr)) {
        ++histogram[*curr];
        ++filesize;
    }
    free(curr);
    return filesize;
}

Node *create_tree(double *histogram, uint16_t *num_leaves) {
    PriorityQueue *pq = pq_create();
    for (int i = 0; i < 256; i++) {
        if (histogram[i]) {
            uint8_t symbol = (uint8_t) i;
            double weight = histogram[i];
            Node *n = node_create(symbol, weight);
            enqueue(pq, n);
            (*num_leaves)++;
        }
    }
    while (!pq_is_empty(pq) && !pq_size_is_1(pq)) {
        Node *left;
        dequeue(pq, &left);
        Node *right;
        dequeue(pq, &right);
        Node *n = node_create(-1, left->weight + right->weight);
        n->left = left;
        n->right = right;
        enqueue(pq, n);
    }
    Node *only_entry;
    dequeue(pq, &only_entry);
    pq_free(&pq);
    return only_entry;
}

void fill_code_table(Code *code_table, Node *node, uint64_t code, uint8_t code_length) {
    if (node->left != NULL || node->right != NULL) {
        /* Recursive calls left and right. */
        fill_code_table(code_table, node->left, code, code_length + 1);
        code |= 1 << code_length;
        fill_code_table(code_table, node->right, code, code_length + 1);
    } else {
        /* Leaf node: store the Huffman Code. */
        code_table[node->symbol].code = code;
        code_table[node->symbol].code_length = code_length;
    }
}

void huff_write_tree(BitWriter *outbuf, Node *node) {
    if (node->left != NULL || node->right != NULL) {
        huff_write_tree(outbuf, node->left);
        huff_write_tree(outbuf, node->right);
        bit_write_bit(outbuf, 0);
    } else {
        bit_write_bit(outbuf, 1);
        bit_write_uint8(outbuf, (uint8_t) node->symbol);
    }
}

void huff_compress_file(BitWriter *outbuf, Buffer *inbuf, uint32_t filesize, uint16_t num_leaves,
    Node *code_tree, Code *code_table) {
    bit_write_uint8(outbuf, (uint8_t) 'H');
    bit_write_uint8(outbuf, (uint8_t) 'C');
    bit_write_uint32(outbuf, filesize);
    bit_write_uint16(outbuf, num_leaves);
    huff_write_tree(outbuf, code_tree);

    uint8_t *curr_byte = (uint8_t *) calloc(1, sizeof(uint8_t));
    while (read_uint8(inbuf, curr_byte)) {
        uint64_t code = code_table[*curr_byte].code;
        uint8_t code_length = code_table[*curr_byte].code_length;
        for (uint8_t i = 0; i < code_length; i++) {
            bit_write_bit(outbuf, code & 1);
            code >>= 1;
        }
    }
    free(curr_byte);
}

int main(int argc, char *argv[]) {
    int opt_i = 0;
    int opt_o = 0;
    int opt_h = 0;
    int option;
    char *input_filename = NULL;
    char *output_filename = NULL;
    while ((option = getopt(argc, argv, "i:o:h")) != -1) {
        switch (option) {
        case 'i':
            input_filename = optarg;
            opt_i = 1;
            break;
        case 'o':
            output_filename = optarg;
            opt_o = 1;
            break;
        case '?':
            opt_h = 1;
            fprintf(stderr, "Invalid option %c, possible options: -i -o -h\n", optopt);
            break;
        case 'h':
        default:
            opt_h = 1;
            printf("Possible options: -i -o -h\n");
            break;
        }
    }
    if (opt_h) {
        return 0;
    }
    if (opt_i && opt_o) {
        Buffer *read_buff = read_open(input_filename);
        double hist[256] = { 0.0 };
        uint32_t filesize = fill_histogram(read_buff, hist);
        read_close(&read_buff);
        uint16_t num_leaves = 0;
        Node *node = create_tree(hist, &num_leaves);
        Code *code_table = calloc(256, sizeof(Code));
        fill_code_table(code_table, node, 0, 0);
        Buffer *read_buff2 = read_open(input_filename);
        BitWriter *write_buff = bit_write_open(output_filename);
        huff_compress_file(write_buff, read_buff2, filesize, num_leaves, node, code_table);
        read_close(&read_buff2);
        bit_write_close(&write_buff);
        free(code_table);
        node_free(&node);
    }
    return 0;
}
