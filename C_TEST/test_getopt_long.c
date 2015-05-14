#include <stdio.h>   
  
#include <unistd.h>   
  
#include <getopt.h>   
#include <string.h>

char *para = ":ab:cf:v";   
  
int do_all = 0;   
  
int do_help = 0;    
  
int do_version = 0;   
  
char *file = NULL;   
  
struct option longopt[] =    
  
{   
  
     {"all", no_argument, &do_all, 1},   
  
     {"file", required_argument, NULL, 'f'},   
  
     {"help", no_argument, &do_help, 1},   
  
     {"version", no_argument, &do_version, 1},   
  
     {"bob", required_argument, NULL, 'b'},   
  
     {0, 0, 0, 0},   
  
};   
  
struct globalArgs_t {
    int noIndex;                /* -I option */
    char *langCode;             /* -l option */
    const char *outFileName;    /* -o option */
    FILE *outFile;
    int verbosity;              /* -v option */
    char **inputFiles;          /* input files */
    int numInputFiles;          /* # of input files */
    int randomized;             /* --randomize option */
} globalArgs;

static const char *optString = "Il:o:vh?";

static const struct option longOpts[] = {
    { "no-index", no_argument, NULL, 'I' },
    { "language", required_argument, NULL, 'l' },
    { "output", required_argument, NULL, 'o' },
    { "verbose", no_argument, NULL, 'v' },
    { "randomize", no_argument, NULL, 0 },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};

void display_usage(void)
{
    printf("exe : -I <index> -l <str:langCode> -o <str:outFile> -v <verbosity>\n");
}

int main(int argc,char *argv[])
{
    int opt = -1;
    int longIndex;
    struct globalArgs_t globalArgs;
    memset(&globalArgs,0,sizeof(globalArgs));

    opt = getopt_long( argc, argv, optString, longOpts, &longIndex );
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

            case 0:     /* long option without a short arg */
                if( strcmp( "randomize", longOpts[longIndex].name ) == 0 ) {
                    globalArgs.randomized = 1;
                }
                break;
                
            default:
                /* You won't actually get here. */
                break;
        }
        
        opt = getopt_long( argc, argv, optString, longOpts, longIndex );
    }
    printf("index -I %d,langCode -l %s ,outFileName -o %s ,verbosity -v %d \n,",globalArgs.noIndex,globalArgs.langCode,globalArgs.outFileName,globalArgs.verbosity);
}

int _main(int argc, char *argv[])   
  
{   
  
    int oc = -1;   
  
    char *b_input = NULL;   
  
    while((oc = getopt_long(argc, argv, para, longopt, NULL)) != -1)   
  
    {   
  
         switch(oc)   
  
         {   
  
         case 'a':   
  
               printf("input para is a\n");   
  
              break;   
  
         case 'b':   
  
              b_input = optarg;   
  
              printf("input para is b,and optarg is %s\n", b_input);   
  
             break;   
  
        case 'c':   
  
             printf("input para is c\n");   
  
            break;   
  
        case 'v':   
  
            printf("input para is v\n");   
  
            break;   
  
        case 'f':   
  
            printf("input para is f\n");   
  
            file = "hello world";  
            b_input = optarg;
            printf("input -f arg  is %s \n",b_input); 
  
            break;   
  
        case 0:   
  
           break;   
  
        case ':':   
  
             printf("option %c requires an argument\n",optopt);   
  
             break;   
  
         case '?':   
  
         default:   
  
            printf("option %c is invalid:ignored\n",optopt);   
  
            break;   
  
         }   
  
     }   
  
     printf("do_all is %d\n",do_all);   
  
     printf("do_help is %d\n",do_help);   
  
     printf("do_version is %d\n",do_version);   
  
     printf("do_file is %s\n",file);   
  
     printf("bob is %s\n", b_input);   
  
     return 0;   
  
}
  


