#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

void *myalloc(q, n, m)
    void *q;
    unsigned n, m;
{
    q = Z_NULL;
    return calloc(n, m);
}

void myfree(void *q, void *p)
{
    q = Z_NULL;
    free(p);
}

static alloc_func zalloc = myalloc;
static free_func zfree = myfree;

// Level 1 for zlib prioritizing speed
// Level 0 (or whatever) for Huffman coding only
long get_num_bytes_in_compressed_data(Byte *uncompr, uLong uncomprLen, int level) {
    int err;
    uLong comprLen;

    if (level == 1) {
        comprLen = compressBound(uncomprLen);
        Byte *compr = (Byte *)calloc(comprLen * sizeof(Byte), 1); 
    	err = compress2(compr, &comprLen, uncompr, uncomprLen, Z_BEST_SPEED);
        CHECK_ERR(err, "compress2");
        free(compr);
        return comprLen;
    } else {
    	z_stream c_stream; /* compression stream */
        c_stream.zalloc = zalloc;
        c_stream.zfree = zfree;
        c_stream.opaque = (voidpf)0;

        err = deflateInit2(&c_stream, Z_NO_COMPRESSION, Z_DEFLATED, 15, 8, Z_HUFFMAN_ONLY);
    	CHECK_ERR(err, "deflateInit2");

        comprLen  = deflateBound(&c_stream, uncomprLen);
        Byte *compr = (Byte *)calloc(comprLen * sizeof(Byte), 1); 

        c_stream.next_out = compr;
        c_stream.avail_out = (uInt)comprLen;
        c_stream.next_in = uncompr;
        c_stream.avail_in = (uInt)uncomprLen;

    	err = deflate(&c_stream, Z_FINISH);
        if (err != Z_STREAM_END) {
            fprintf(stderr, "deflate should report Z_STREAM_END\n");
            exit(1);
        }
        comprLen = c_stream.total_out;
        err = deflateEnd(&c_stream);
        CHECK_ERR(err, "deflateEnd");
        free(compr);
        return comprLen;
    }
}


int main (int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage is: [IN_FILE] [START_BYTE] [END_BYTE]\n");
        return 0;
    }

    long start_byte = strtol(argv[2], NULL, 10);
    long end_byte = strtol(argv[3], NULL, 10);

    if (end_byte < start_byte) {
        printf("END_BYTE must be after START_BYTE\n");
        return 0;
    }

    FILE *fileptr;
    Byte *uncompr;
    long filelen;

    fileptr = fopen(argv[1], "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    if (end_byte > filelen) {
        printf("END_BYTE must be less than the number of bytes in the file\n");
        return 0;
    }
    rewind(fileptr);                      // Jump back to the beginning of the file

    


    end_byte = filelen - 1;



    long num_bytes = end_byte - start_byte + 1;
    uncompr = (Byte *)malloc(num_bytes * sizeof(Byte)); // Enough memory for the chunk
    fseek(fileptr, start_byte, SEEK_SET);
    size_t num_read = fread(uncompr, 1, num_bytes, fileptr); // Read in the chunk
    fclose(fileptr); // Close the file

    printf("%lu is compressed size\n", get_num_bytes_in_compressed_data(uncompr, num_bytes, 0));

    free(uncompr);
    return 0;

}