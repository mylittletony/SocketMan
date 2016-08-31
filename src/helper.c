#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include "dbg.h"
#include "options.h"
#include <ctype.h>
#include <stdlib.h>

#define NCHAR 64

char *strrev(char *str) {
  char *p1, *p2;

  if (! str || ! *str)
    return str;
  for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
  {
    *p1 ^= *p2;
    *p2 ^= *p1;
    *p1 ^= *p2;
  }
  return str;
}

char *read_config(char *file) {
  FILE *fp;
  long lSize;
  char *buffer = NULL;

  fp = fopen (options.config, "r");
  if( fp ) {;
    debug("Reading config from %s", options.config);
    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    buffer = calloc( 1, lSize+1 );
    if( !buffer ) {
      fclose(fp);
      fputs("memory alloc fails",stderr);
      return buffer;
    }

    if( 1!=fread( buffer , lSize, 1 , fp) ) {
      fclose(fp);
      free(buffer);
      fputs("entire read fails",stderr);
      return buffer;
    }

    fclose(fp);
  } else {
    debug("No config file found at %s", options.config);
  }
  return buffer;
}

int file_present(char *file)
{
  return access( file, F_OK ) != -1;
}

void readlineToBuffer(char *file, char *buffer) {

  if( access(file, F_OK ) != -1 ) {
    FILE *sinfile;
    char *sbuffer;
    long snumbytes;
    sinfile = fopen(file, "r");
    if(sinfile != NULL) {
      fseek(sinfile, 0L, SEEK_END);
      snumbytes = ftell(sinfile);

      fseek(sinfile,0L,SEEK_SET);
      sbuffer = (char*)calloc(snumbytes,sizeof(char));

      if(sbuffer == NULL) {
        strcpy(buffer, sbuffer);
      } else {
        fread(sbuffer,sizeof(char),snumbytes,sinfile);
        char *ret = strpbrk(sbuffer, "\n");
        if(ret) {
          strcpy(buffer, strtok(sbuffer,"\n"));
        } else {
          strcpy(buffer, sbuffer);
        }
      }
      free(sbuffer);
    }
    fclose(sinfile);
    return;
  }

  strcpy(buffer, "DNE");
  return;
}

int in_array(int val, int *arr, int size){
  int i;
  for (i=0; i < size; i++) {
    if (arr[i] == val)
      return 1;
  }
  return 0;
}
