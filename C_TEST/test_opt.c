#include <stdio.h>   
  
#include <unistd.h>   
  
#include <getopt.h>   
#include <string.h>

struct globalArgs_t {
    int noIndex;                /* -I option */
    char *langCode;             /* -l option */
    const char *outFileName;    /* -o option */
    FILE *outFile;
    int verbosity;              /* -v option */
    char **inputFiles;          /* input files */
    int numInputFiles;          /* # of input files */
} globalArgs;

static const char *optString = "Il:o:vh?";
void display_usage( void )
{
    puts( "doc2html - convert documents to HTML" );
    /* ... */
    exit( -1 );
}

void convert_document( void )
{
    /* ... */
}

int main( int argc, char *argv[] )
{
    int opt = 0;
    
    /* Initialize globalArgs before we get to work. */
    globalArgs.noIndex = 0;     /* false */
    globalArgs.langCode = NULL;
    globalArgs.outFileName = NULL;
    globalArgs.outFile = NULL;
    globalArgs.verbosity = 0;
    globalArgs.inputFiles = NULL;
    globalArgs.numInputFiles = 0;

    opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
        switch( opt ) {
            case 'I':
                globalArgs.noIndex = 1; /* true */
                break;
                
            case 'l':
                globalArgs.langCode = optarg;
                break;
                
            case 'o':
                globalArgs.outFileName = optarg;
                break;
                
            case 'v':
                globalArgs.verbosity++;
                break;
                
            case 'h':   /* fall-through is intentional */
            case '?':
                display_usage();
                break;
                
            default:
                /* You won't actually get here. */
                break;
        }
        
        opt = getopt( argc, argv, optString );
    }
    
    globalArgs.inputFiles = argv + optind;
    globalArgs.numInputFiles = argc - optind;

    convert_document();
    printf("index -I %d,langCode -l %s ,outFileName -o %s ,verbosity -v %d \n,",globalArgs.noIndex,globalArgs.langCode,globalArgs.outFileName,globalArgs.verbosity);

    return 0;
}

