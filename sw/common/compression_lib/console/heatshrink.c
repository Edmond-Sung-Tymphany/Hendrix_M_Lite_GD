#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>

#include "../decompress/deCompressorTypeA.h"
#include "../compress/CompressorTypeA.h"

#include "../common/heatshrink_encoder.h"
#include "../common/heatshrink_decoder.h"

#include "../common/configuration.h"

#define DEF_WINDOW_SZ2 6
#define DEF_LOOKAHEAD_SZ2 4
#define DEF_DECODER_INPUT_BUFFER_SIZE 256
#define DEF_BUFFER_SIZE (256 * 1024)

#define RANDOM_BIN "randomBinFile.bin"

#define COMP_RAW_BIN       "tempFiles/c1_raw_bin.bin"
#define COMP_COM_BIN       "tempFiles/c2_com_bin.bin"
#define RESULT_COMP_HEADER "tempFiles/c3_ResultCompHeader.h"
#define DECOMP_RAW_BIN     "tempFiles/c4_raw_bin.bin"
#define DECOMP_RAW_HEADER  "tempFiles/c5_raw_header.h"

#define DECOMP_COM_BIN "tempFiles/d1_com_bin.bin"

#define RAW_DSP_HEADER "RawDspHeader_normal.h"

#define ARRAY_NAME "mini"

#define COMP_HEADER_COLUMN_NUM   (20)
#define DECOMP_HEADER_COLUMN_NUM (2)

#if 0
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...) /* NO-OP */
#endif

#if _WIN32
#include <errno.h>
#define HEATSHRINK_ERR(retval, ...) do { \
fprintf(stderr, __VA_ARGS__); \
fprintf(stderr, "Undefined error: %d\n", errno); \
exit(retval); \
} while(0)
#else
#include <err.h>
#define HEATSHRINK_ERR(...) err(__VA_ARGS__)
#endif

/*
 * We have to open binary files with the O_BINARY flag on Windows. Most other
 * platforms don't differentiate between binary and non-binary files.
 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

#define HS_PRINTF(...)

//#define HS_PRINTF(x) printf x

static const int version_major = HEATSHRINK_VERSION_MAJOR;
static const int version_minor = HEATSHRINK_VERSION_MINOR;
static const int version_patch = HEATSHRINK_VERSION_PATCH;
static const char author[] = HEATSHRINK_AUTHOR;
static const char url[] = HEATSHRINK_URL;

static void usage(void) {
    fprintf(stderr, "heatshrink version %u.%u.%u by %s\n",
        version_major, version_minor, version_patch, author);
    fprintf(stderr, "Home page: %s\n\n", url);
    fprintf(stderr,
        "Usage:\n"
        "  heatshrink [-h] [-e|-d] [-v] [-w SIZE] [-l BITS] [IN_FILE] [OUT_FILE]\n"
        "\n"
        "heatshrink compresses or decompresses byte streams using LZSS, and is\n"
        "designed especially for embedded, low-memory, and/or hard real-time\n"
        "systems.\n"
        "\n"
        " -h        print help\n"
        " -e        encode (compress, default)\n"
        " -d        decode (decompress)\n"
        " -v        verbose (print input & output sizes, compression ratio, etc.)\n"
        "\n"
        " -w SIZE   Base-2 log of LZSS sliding window size\n"
        "\n"
        "    A larger value allows searches a larger history of the data for repeated\n"
        "    patterns, potentially compressing more effectively, but will use\n"
        "    more memory and processing time.\n"
        "    Recommended default: -w 8 (embedded systems), -w 10 (elsewhere)\n"
        "  \n"
        " -l BITS   Number of bits used for back-reference lengths\n"
        "\n"
        "    A larger value allows longer substitutions, but since all\n"
        "    back-references must use -w + -l bits, larger -w or -l can be\n"
        "    counterproductive if most patterns are small and/or local.\n"
        "    Recommended default: -l 4\n"
        "\n"
        " If IN_FILE or OUT_FILE are unspecified, they will default to\n"
        " \"-\" for standard input and standard output, respectively.\n");
    exit(1);
}

typedef enum { IO_READ, IO_WRITE, } IO_mode;
typedef enum { OP_ENC, OP_DEC, OP_UTEST, OP_OPTIMAL} Operation;
typedef enum { CTYPE_A, CTYPE_B, } CompressionType;


typedef struct {
    int fd;                     /* file descriptor */
    IO_mode mode;
    size_t fill;                /* fill index */
    size_t read;                /* read index */
    size_t size;
    size_t total;
    uint8_t buf[];
} io_handle;

typedef struct {
    uint8_t window_sz2;
    uint8_t lookahead_sz2;
    size_t decoder_input_buffer_size;
    size_t buffer_size;
    uint8_t verbose;
    CompressionType ct;
    Operation cmd;
    char *in_fname;
    char *out_fname;
    char *array_name;
    char *columnNum;
    uint16_t times;
    io_handle *in;
    io_handle *out;
} config;

static heatshrink_encoder hse;
static heatshrink_decoder hsd;
static config cfg;

static void die(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

/* Open an IO handle. Returns NULL on error. */
static io_handle *handle_open(char *fname, IO_mode m, size_t buf_sz) {
    io_handle *io = NULL;
    io = malloc(sizeof(*io) + buf_sz);
    if (io == NULL) { return NULL; }
    memset(io, 0, sizeof(*io) + buf_sz);
    io->fd = -1;
    io->size = buf_sz;
    io->mode = m;

    if (m == IO_READ) {
        if (0 == strcmp("-", fname)) {
            io->fd = STDIN_FILENO;
        } else {
            io->fd = open(fname, O_RDONLY | O_BINARY);
        }
    } else if (m == IO_WRITE) {
        if (0 == strcmp("-", fname)) {
            io->fd = STDOUT_FILENO;
        } else {
            io->fd = open(fname, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC /*| O_EXCL*/, 0644);
        }
    }

    if (io->fd == -1) {         /* failed to open */
        free(io);
        HEATSHRINK_ERR(1, "open");
        return NULL;
    }

    return io;
}

/* Read SIZE bytes from an IO handle and return a pointer to the content.
 * BUF contains at least size_t bytes. Returns 0 on EOF, -1 on error. */
static ssize_t handle_read(io_handle *io, size_t size, uint8_t **buf) {
    LOG("@ read %zd\n", size);
    if (buf == NULL) { return -1; }
    if (size > io->size) {
        fprintf(stderr, "size %zd, io->size %zd\n", size, io->size);
        return -1;
    }
    if (io->mode != IO_READ) { return -1; }

    size_t rem = io->fill - io->read;
    if (rem >= size) {
        *buf = &io->buf[io->read];
        return size;
    } else {                    /* read and replenish */
        if (io->fd == -1) {     /* already closed, return what we've got */
            *buf = &io->buf[io->read];
            return rem;
        }

        memmove(io->buf, &io->buf[io->read], rem);
        io->fill -= io->read;
        io->read = 0;
        ssize_t read_sz = read(io->fd, &io->buf[io->fill], io->size - io->fill);
        if (read_sz < 0) { HEATSHRINK_ERR(1, "read"); }
        io->total += read_sz;
        if (read_sz == 0) {     /* EOF */
            if (close(io->fd) < 0) { HEATSHRINK_ERR(1, "close"); }
            io->fd = -1;
        }
        io->fill += read_sz;
        *buf = io->buf;
        return io->fill > size ? size : io->fill;
    }
}

/* Drop the oldest SIZE bytes from the buffer. Returns <0 on error. */
static int handle_drop(io_handle *io, size_t size) {
    LOG("@ drop %zd\n", size);
    if (io->read + size <= io->fill) {
        io->read += size;
    } else {
        return -1;
    }
    if (io->read == io->fill) {
        io->read = 0;
        io->fill = 0;
    }
    return 0;
}

/* Sink SIZE bytes from INPUT into the io handle. Returns the number of
 * bytes written, or -1 on error. */
static ssize_t handle_sink(io_handle *io, size_t size, uint8_t *input) {
    LOG("@ sink %zd\n", size);
    if (size > io->size) { return -1; }
    if (io->mode != IO_WRITE) { return -1; }

    if (io->fill + size > io->size) {
        ssize_t written = write(io->fd, io->buf, io->fill);
        LOG("@ flushing %zd, wrote %zd\n", io->fill, written);
        io->total += written;
        if (written == -1) { HEATSHRINK_ERR(1, "write"); }
        memmove(io->buf, &io->buf[written], io->fill - written);
        io->fill -= written;
    }
    memcpy(&io->buf[io->fill], input, size);
    io->fill += size;
    return size;
}

static void handle_close(io_handle *io) {
    if (io->fd != -1) {
        if (io->mode == IO_WRITE) {
            ssize_t written = write(io->fd, io->buf, io->fill);
            io->total += written;
            LOG("@ close: flushing %zd, wrote %zd\n", io->fill, written);
            if (written == -1) { HEATSHRINK_ERR(1, "write"); }
        }
        close(io->fd);
        io->fd = -1;
    }
}

static void readInputFile(char** data, unsigned short *length) 
{
    unsigned short read_sz = 0;
    
    read_sz = handle_read(cfg.in, 18192, data);

    *length = read_sz;

    HS_PRINTF(("\nread_sz0[%d]\n",read_sz));

    return;
}

static void writeOutPutFile(unsigned char* data, unsigned short *length) 
{
    unsigned short write_sz = 0;

    unsigned short writeLen = *length;
    
    HS_PRINTF(("\nwrite_length[%d]\n",writeLen));

    write_sz = handle_sink(cfg.out, writeLen, data);

    *length = write_sz;

    HS_PRINTF(("\nwrite_sz[%d]\n",write_sz));

    return;
}

static void proc_args(int argc, char **argv) {
    cfg.window_sz2 = DEF_WINDOW_SZ2;
    cfg.lookahead_sz2 = DEF_LOOKAHEAD_SZ2;
    cfg.buffer_size = DEF_BUFFER_SIZE;
    cfg.decoder_input_buffer_size = DEF_DECODER_INPUT_BUFFER_SIZE;
    cfg.cmd = OP_ENC;
    cfg.verbose = 0;
    cfg.in_fname = "-";
    cfg.out_fname = "-";
    cfg.array_name = "-";
    cfg.columnNum = 0;
    cfg.times = 1;

    int a = 0;
    while ((a = getopt(argc, argv, "hedufc:i:w:l:t:v")) != -1) {
        switch (a) {
        case 'e':               /* encode */
            cfg.cmd = OP_ENC; break;
        case 'd':               /* decode */
            cfg.cmd = OP_DEC; break;
        case 'u':               /* decode */
            cfg.cmd = OP_UTEST; break;
        case 'f':               /* decode */
            cfg.cmd = OP_OPTIMAL; break;
        case 'i':               /* input buffer size */
            cfg.decoder_input_buffer_size = atoi(optarg);
            break;
        case 'w':               /* window bits */
            cfg.window_sz2 = atoi(optarg);
            break;
        case 'l':               /* lookahead bits */
            cfg.lookahead_sz2 = atoi(optarg);
            break;
        case 'c':               /* compression type */
            cfg.ct = atoi(optarg);
            break;
        case 't':               /* compression type */
            cfg.times = atoi(optarg);
            break;        
        case 'v':               /* verbosity++ */
            cfg.verbose++;
            break;
        case '?':               /* unknown argument */
        default:
            usage();
        }
    }
   
    argc -= optind;
    argv += optind;
    if (argc > 0) 
    {
        cfg.in_fname = argv[0];
        argc--;
        argv++;
    }
    if (argc > 0) 
    { 
        cfg.array_name = argv[0]; 
        argc--;
        argv++;
    }
    if (argc > 0) 
    { 
        cfg.out_fname = argv[0]; 
        argc--;
        argv++;
    }
    if (argc > 0) 
    { 
        cfg.columnNum = atoi(argv[0]); 
        argc--;
        argv++;
    }
}

unsigned char encode()
{
    //fprintf(stderr, "\nStarted");
    char temp[200];
    
    unsigned char compAll[18192]    = {0};
    unsigned char readBuffer[18192] = {0}; 
    
    unsigned char blockCount; 

    unsigned char *buffer;
    unsigned short blockSize;
    unsigned short compDataLength;
    unsigned short allLengthOut;
    unsigned short allLength;
    unsigned short regLength;
    unsigned short dataLength;
    unsigned short compressionRatio = 0;

    unsigned short offSet = 0;

    if (CTYPE_A == cfg.ct)
    {
        int ii, zz;
        
        sprintf(temp, "./FileConverter.exe htb %s %s %s", cfg.in_fname, cfg.array_name, COMP_RAW_BIN);
        system(temp);
        HS_PRINTF(("\ndo: [%s]\n",temp));
 
        cfg.in = handle_open(COMP_RAW_BIN, IO_READ, cfg.buffer_size);
        if (cfg.in == NULL) 
        {
            HS_PRINTF(("\nFILE IN fail"));
        }            
        cfg.out = handle_open(COMP_COM_BIN , IO_WRITE, cfg.buffer_size);
        if (cfg.out == NULL)
        {
            HS_PRINTF(("\nFILE OUT fail"));
        }          
        
        readInputFile(&buffer, &allLength);

        HS_PRINTF(("\nallLength[%d]",allLength));
        
        if (0 == CompressorTypeA_Compress(buffer, allLength, compAll, &compDataLength))
        {
            handle_close(cfg.in);
            handle_close(cfg.out);
            
            free(cfg.in);
            free(cfg.out);
            return 100;
        }
        
        writeOutPutFile(compAll, &compDataLength);

        compressionRatio = (100*compDataLength)/allLength;
        HS_PRINTF(("\ncompression ratio = %d%% [%d -> %d]",compressionRatio, allLength, compDataLength));

        handle_close(cfg.in);
        handle_close(cfg.out);            

        sprintf(temp, "./FileConverter.exe bth %s %s %s %d",COMP_COM_BIN,cfg.array_name, cfg.out_fname,cfg.columnNum);
        system(temp);

        HS_PRINTF(("\ndo: [%s]\n",temp));            

        cfg.in = handle_open(COMP_COM_BIN , IO_READ, cfg.buffer_size);
        if (cfg.in == NULL) 
        {
            HS_PRINTF(("\nFILE IN fail"));
        }            
        cfg.out = handle_open(DECOMP_RAW_BIN , IO_WRITE, cfg.buffer_size);
        if (cfg.out == NULL)
        {
            HS_PRINTF(("\nFILE OUT fail"));
        }   

        readInputFile(&buffer, &allLength);
        HS_PRINTF(("\nCompressed All length[%d]",allLength));

        blockCount = buffer[1];
        HS_PRINTF(("\nblockCOunt[%d]\n",blockCount));
        
        deCompressorTypeA_Create(buffer);
        ii = 0;
        while(1)
        {
           if (0 == deCompressorTypeA_Read(&readBuffer[2*ii]))
            {
                HS_PRINTF(("\ndone ^--^ [%d]",ii*2));
                HS_PRINTF(("\ncompression ratio = %d%%\n",compressionRatio));
                break;
            }
            else
            {
                //HS_PRINTF(("\nDecodedResult[%d][0x%02x]",readBuffer[2*ii],readBuffer[2*ii+1]));
            }
            ii++;
        }

        dataLength = ii * 2;            

        for (ii = 0; ii < dataLength; ii++)
        {
            //HS_PRINTF(("\nreadBuffer[%d][0x%02x]",ii,readBuffer));            
        }

        writeOutPutFile(readBuffer, &dataLength);        
        handle_close(cfg.out);    

        sprintf(temp, "./FileConverter.exe bth %s %s %s %d",DECOMP_RAW_BIN, cfg.array_name, DECOMP_RAW_HEADER, DECOMP_HEADER_COLUMN_NUM);
        system(temp);
        HS_PRINTF(("\ndo: [%s]\n",temp));
    }
    handle_close(cfg.in);
    handle_close(cfg.out);

    free(cfg.in);
    free(cfg.out);

    return compressionRatio;
}


unsigned char decode()
{
    //fprintf(stderr, "\nStarted");
    char temp[200];
    
    unsigned char compAll[18192]    = {0};
    unsigned char readBuffer[18192] = {0}; 
    
    unsigned char blockCount; 

    unsigned char *buffer;
    unsigned short blockSize;
    unsigned short compDataLength;
    unsigned short allLengthOut;
    unsigned short allLength;
    unsigned short regLength;
    unsigned short dataLength;
    unsigned short compressionRatio = 0;

    unsigned short offSet = 0;

    if (CTYPE_A == cfg.ct)
    {
        int ii, zz;
        
        sprintf(temp, "./FileConverter.exe htb %s %s %s", cfg.in_fname, cfg.array_name, DECOMP_COM_BIN);
        system(temp);
        HS_PRINTF(("\ndo: [%s]\n",temp));
        
        cfg.in = handle_open(DECOMP_COM_BIN, IO_READ, cfg.buffer_size);
        if (cfg.in == NULL) 
        {
            HS_PRINTF(("\nFILE IN fail"));
        }            
        cfg.out = handle_open(DECOMP_RAW_BIN , IO_WRITE, cfg.buffer_size);
        if (cfg.out == NULL)
        {
            HS_PRINTF(("\nFILE OUT fail"));
        }

        readInputFile(&buffer, &allLength);
        HS_PRINTF(("\nCompressed All length[%d]",allLength));

        blockCount = buffer[1];
        HS_PRINTF(("\nblockCOunt[%d]\n",blockCount));
        
        deCompressorTypeA_Create(buffer);
        ii = 0;
        while(1)
        {
           if (0 == deCompressorTypeA_Read(&readBuffer[2*ii]))
            {
                HS_PRINTF(("\ndone ^--^ [%d]",ii*2));
                HS_PRINTF(("\ncompression ratio = %d%%\n",compressionRatio));
                break;
            }
            else
            {
                HS_PRINTF(("\nDecodedResult[%d][0x%02x]",readBuffer[2*ii],readBuffer[2*ii+1]));
            }
            ii++;
        }

        dataLength = ii * 2;            

        for (ii = 0; ii < dataLength; ii++)
        {
            HS_PRINTF(("\nreadBuffer[%d][0x%02x]",ii,readBuffer));            
        }

        writeOutPutFile(readBuffer, &dataLength);        
        handle_close(cfg.out);    

        sprintf(temp, "./FileConverter.exe bth %s %s %s %d",DECOMP_RAW_BIN, cfg.array_name, cfg.out_fname, DECOMP_HEADER_COLUMN_NUM);
        system(temp);
        HS_PRINTF(("\ndo: [%s]\n",temp));
    }
    handle_close(cfg.in);
    handle_close(cfg.out);

    free(cfg.in);
    free(cfg.out);

    return compressionRatio;
}


void findOptimal(unsigned char *window, unsigned char *lookup)
{
    unsigned char compressionRatio = 100;
    unsigned char w = HEATSHRINK_STATIC_WINDOW_BITS;
    unsigned char l = HEATSHRINK_STATIC_LOOKAHEAD_BITS;
    unsigned char resRatio;

    unsigned char ww;
    unsigned char ll;

    cfg.ct = CTYPE_A;

    cfg.in_fname   = "rawDspHeader.h";
    cfg.array_name = "mini";
    cfg.out_fname  = "tempOut.h";    

    for (ww = 9; ww < 10; ww++)
    {
        config_set_w(ww);
        for (ll = 4; ll < 10; ll++)
        {
            config_set_l(ll);
            HS_PRINTF(("\ntry w[%d], l[%d]",ww, ll));
            resRatio = encode();
            if (resRatio < compressionRatio)
            {
                compressionRatio = resRatio;
                w = ww;
                l = ll;
            }
        }        
    }
    window = w;
    lookup = l;

    HS_PRINTF(("\n=>best w[%d] l[%d] cr[%d%%]", w, l, compressionRatio));
}

void unitTest()
{
    unsigned char *buffer1;
    unsigned char *buffer2;
    unsigned short length1;
    unsigned short length2;
    unsigned short ii, zz, passed, unableToCompress;

    unsigned char comareResult = 1;
    unsigned char compressionRatio = 100;

    passed = 0;
    unableToCompress = 0;

    for (zz = 0; zz < cfg.times; zz++)
    {        
        char temp[200];

        sprintf(temp, "./FileConverter.exe random");
        system(temp);
        
        cfg.ct = CTYPE_A;

        if (0 == strcmp(cfg.in_fname,"-"))
        {
            cfg.in_fname = RAW_DSP_HEADER;
        }
        if (0 == strcmp(cfg.array_name,"-"))
        {
            cfg.array_name = ARRAY_NAME;
        }
        if (0 == strcmp(cfg.out_fname,"-"))
        {
            cfg.out_fname = RESULT_COMP_HEADER;
        }
        if (0 == cfg.columnNum)
        {
            cfg.columnNum = COMP_HEADER_COLUMN_NUM;
        }
        
        compressionRatio = encode();
        if (100 == compressionRatio)
        {
            printf("\ncompressionRatio[%d] is negative",zz);
            HS_PRINTF(("\nCannot compress"));
            printf("\ntest[%d] not passed to due unable ot compress\n",zz);
            HS_PRINTF(("\n\ntest[%d] not passed",zz));
            unableToCompress++;
            continue;
        }
        else
        {
            printf("\ncompressionRatio[%d] is %d%%",zz, compressionRatio);
        }

        HS_PRINTF(("\nCompressionRatio = %d%%", compressionRatio));
        
        cfg.in = handle_open(COMP_RAW_BIN, IO_READ, cfg.buffer_size);
        if (cfg.in == NULL) 
        {
            HS_PRINTF(("\nFILE IN fail"));
        }            

        readInputFile(&buffer1, &length1);

        cfg.in = handle_open(DECOMP_RAW_BIN , IO_READ, cfg.buffer_size);
        if (cfg.in == NULL) 
        {
            HS_PRINTF(("\nFILE IN fail"));
        }

        readInputFile(&buffer2, &length2);

        HS_PRINTF(("\n\nlength1[%d], length2[%d]", length1, length2));

        if (length1 == length2)
        {
            for (ii = 0; ii < length1; ii++)
            {
                if (buffer1[ii] != buffer2[ii])
                {
                    comareResult = 0;
                    break;
                }
            }
        }
        else
        {
            comareResult = 0;
        }

        if (0 == comareResult)
        {
            printf("\ntest[%d] not passed\n",zz);
            HS_PRINTF(("\n\ntest[%d] not passed",zz));
        }
        else
        {
            passed++;
            printf("\ntest[%d] pass\n",zz)
            HS_PRINTF(("\n\ntest[%d] passed",zz));
        }
    }
    printf("\nResult: Passed[%d] | Failed[%d] | Unable to compress[%d]\n", passed, cfg.times - unableToCompress - passed, unableToCompress);
}

int main(int argc, char **argv) 
{    
    unsigned char bestW;
    unsigned char bestL;
    
    memset(&cfg, 0, sizeof(cfg));

    proc_args(argc, argv);

    if (0 == strcmp(cfg.in_fname, cfg.out_fname)
        && (0 != strcmp("-", cfg.in_fname))) {
        fprintf(stderr, "Refusing to overwrite file '%s' with itself.\n", cfg.in_fname);
        exit(1);
    }
    
    if (OP_ENC == cfg.cmd)
    {
        encode();
    }  
    if (OP_DEC == cfg.cmd)
    {
        decode();
    }
    else if (OP_UTEST == cfg.cmd)
    {
        unitTest();
    }
    else if (OP_OPTIMAL == cfg.cmd)
    {
        findOptimal(&bestW, &bestL);
    }
    return;
    
#if _WIN32
    /*
     * On Windows, stdin and stdout default to text mode. Switch them to
     * binary mode before sending data through them.
     */
    _setmode(STDOUT_FILENO, O_BINARY);
    _setmode(STDIN_FILENO, O_BINARY);
#endif
}
