#include "zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
// Level 0 for Huffman coding only
uLong get_num_bytes_in_compressed_data(Byte *uncompr, uLong uncomprLen, int level) {
    int err;
    uLong comprLen;
    struct timeval tic, toc;

    if (level == 1) {
        comprLen = compressBound(uncomprLen);
        Byte *compr = (Byte *)calloc(comprLen * sizeof(Byte), 1); 
        gettimeofday(&tic, NULL);
    	err = compress2(compr, &comprLen, uncompr, uncomprLen, Z_BEST_SPEED);
        gettimeofday(&toc, NULL);
        CHECK_ERR(err, "compress2");
        free(compr);
        printf("%lu us\n", (toc.tv_sec - tic.tv_sec) * 1000000 + toc.tv_usec - tic.tv_usec); 
        return comprLen;
    } else if (level == 0) {
        gettimeofday(&tic, NULL);
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
        gettimeofday(&toc, NULL);
        CHECK_ERR(err, "deflateEnd");
        free(compr);
        printf("%lu us\n", (toc.tv_sec - tic.tv_sec) * 1000000 + toc.tv_usec - tic.tv_usec); 
        return comprLen;
    } else {
        return 0;
    }
}


int main (int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage is: [IN_FILE] [LEVEL] [START_BYTE] [END_BYTE]\n");
        return 0;
    }
    // 0-indexed bytes
    long start_byte = strtol(argv[3], NULL, 10);
    long end_byte = strtol(argv[4], NULL, 10);
    int level = strtol(argv[2], NULL, 10);

    if (end_byte < start_byte) {
        printf("END_BYTE must be after START_BYTE\n");
        return 0;
    } else if (end_byte < 0 || start_byte < 0) {
        printf("END_BYTE and START_BYTE must be nonnegative\n");
        return 0;
    }

    if (level != 0 && level != 1) {
        printf("Level must be 0 or 1\n");
        return 0;
    }

    FILE *fileptr;
    Byte *uncompr;
    long filelen;

    fileptr = fopen(argv[1], "rb");  // Open the file in binary mode
    fseek(fileptr, 0, SEEK_END);          // Jump to the end of the file
    filelen = ftell(fileptr);             // Get the current byte offset in the file
    if (end_byte >= filelen) {
        printf("END_BYTE must be less than the number of bytes in the file\n");
        return 0;
    }
    rewind(fileptr);                      // Jump back to the beginning of the file

    long num_bytes = end_byte - start_byte + 1;
    uncompr = (Byte *)malloc(num_bytes * sizeof(Byte)); // Enough memory for the chunk
    fseek(fileptr, start_byte, SEEK_SET);
    size_t num_read = fread(uncompr, 1, num_bytes, fileptr); // Read in the chunk
    fclose(fileptr); // Close the file

    uLong compressedLen = get_num_bytes_in_compressed_data(uncompr, num_bytes, level);
    double compressionRatio = (double)compressedLen / num_bytes;

    printf("%lf\n", compressionRatio);

    free(uncompr);
    return 0;

}