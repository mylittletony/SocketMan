#include <string.h>
#include <stddef.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <unistd.h>
#include "dbg.h"
#include "options.h"
#include <ctype.h>
#include <stdlib.h>
#include "http.h"

#define NCHAR 64

char *strrev(char *str)
{
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

int valid_mac(char *mac)
{
  int i = 0;
  if (strlen(mac) != 17)
    return 0;

  if(mac[17] != '\0')
    return 0;

  for(i = 0; i < 17; i++) {
    if(i % 3 != 2 && !isxdigit(mac[i]))
      return 0;
    if(i % 3 == 2 && (mac[i] != '-'))
      return 0;
  }
  return 1;
}

char *read_config()
{
  FILE *fp;
  long lSize;
  char *buffer = NULL;

  fp = fopen (options.config, "r");
  if( fp ) {
    debug("Reading config from %s", options.config);
    if( fseek( fp , 0L , SEEK_END) == 0 ) {
      lSize = ftell( fp );
      rewind( fp );

      if(lSize >= 0) {
        buffer = calloc( 1, lSize+1 );
        if( !buffer ) {
          fclose(fp);
          fputs("memory alloc fails",stderr);
          return buffer;
        }

        if( 1!=fread( buffer , lSize, 1 , fp) ) {
          fclose(fp);
          free(buffer);
          buffer = NULL;
          fputs("entire read fails",stderr);
          return buffer;
        }
      }
    }
    fclose(fp);
  } else {
    debug("No config file found at %s", options.config);
  }
  return buffer;
}

// Replace config above with this
void read_file(char *file, char *buf)
{
  FILE *fp;
  long lSize;
  char *buffer = NULL;

  fp = fopen (file, "r");
  if( !fp ) {
    debug("No file found at %s", file);
    return;
  }

  debug("Reading from %s", file);
  if( fseek( fp , 0L , SEEK_END) == 0 ) {
    lSize = ftell( fp );
    rewind( fp );

    if(lSize >= 0) {
      buffer = calloc( 1, lSize+1 );
      if( !buffer ) {
        fclose(fp);
        fputs("memory alloc fails",stderr);
        return;
      }

      if( 1!=fread( buffer , lSize, 1 , fp) ) {
        fclose(fp);
        free(buffer);
        buffer = NULL;
        fputs("entire read fails",stderr);
        return;
      }
    }
  }
  strcpy(buf, buffer);
  free(buffer);
  fclose(fp);
  return;
}

int file_present(char *file)
{
  return access( file, F_OK ) != -1;
}

void readlineToBuffer(char *file, char *buffer) {

  if( access(file, F_OK ) == -1 ) {
    strcpy(buffer, "DNE");
    return;
  }

  FILE *sinfile;
  char *sbuffer = NULL;
  long snumbytes;
  sinfile = fopen(file, "r");
  if(sinfile != NULL) {
    if(fseek(sinfile, 0L, SEEK_END)==0) {
      snumbytes = ftell(sinfile);
      if(fseek(sinfile,0L,SEEK_SET)==0) {
        if(snumbytes >= 0)
          sbuffer = (char*)calloc(snumbytes+1,sizeof(char));

        if(sbuffer != NULL) {
          fread(sbuffer,sizeof(char),snumbytes,sinfile);
          char *ret = strpbrk(sbuffer, "\n");
          if(ret) {
            strcpy(buffer, strtok(sbuffer,"\n"));
          } else {
            strcpy(buffer, sbuffer);
          }
          free(sbuffer);
        }
      }
    }
    fclose(sinfile);
  }
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

int check_certificates()
{

  unsigned char c[MD5_DIGEST_LENGTH];
  int i;
  FILE *inFile = fopen (options.cacrt, "rb");
  MD5_CTX mdContext;
  int bytes;
  unsigned char data[512];

  if (inFile == NULL) {
    printf ("%s can't be opened.\n", options.cacrt);
    goto update;
  }

  char md[33];
  md[0] = '\0';

  MD5_Init (&mdContext);
  while ((bytes = fread (data, 1, 512, inFile)) != 0)
    MD5_Update (&mdContext, data, bytes);

  MD5_Final (c, &mdContext);

  for (i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    snprintf(&(md[i*2]), 16*2, "%02x", (unsigned int)c[i]);
  }

  fclose (inFile);

  debug("MD5 (%s): %s", options.cacrt, md);

  if (md[0] == '\0') {
    debug("Not checking MD5 against server...");
    goto update;
  }

  char current[33];
  current[0] = '\0';
  fetch_ca(current);

  strtok(current, "\n");

  // Missing CA or not connected to Internet
  if (current[0] == '\0') {
    debug("CA empty, continuing without updating.");
    return 0;
  }

  // CA matches our one!
  if (current[0] != '\0' && strcmp(current, md) == 0) {
    debug("CA matches current, not updating!");
    return 0;
  }

  // CA doesn't match, we shall update it
  debug("CA not matching, updating");
  goto update;

update:
  debug("Installing new CA");
  install_ca();
  return 0;
}
