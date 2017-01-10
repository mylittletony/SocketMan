/* #include <stdlib.h> */
/* #include <stddef.h> */
/* #include <stdio.h> */
/* #include <string.h> */
#include "dbg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "zlib.h"
#include <assert.h>
#include <http.h>

#define CACHE_FILE "/etc/sm-cache/cache"
#define CACHE_ARCHIVE "/tmp/data.gz"
#define CHUNK 0x4000
#define windowBits 15
#define GZIP_ENCODING 16

int compress_cache();

void cache(const char *postData)
{

  struct stat st = {0};

  if (stat("/etc/sm-cache", &st) == -1) {
    mkdir("/etc/sm-cache", 0755);
  }

  debug("save the datas! %s", postData);
  FILE *out = fopen(CACHE_FILE, "a");

  if(out == NULL) {
    perror("Error opening file.");
  }

  fseek(out, 0L, SEEK_END);
  unsigned long sz = (unsigned long)ftell(out);
  fseek(out, 0, SEEK_SET);

  debug("FILE SIZE: %ld", sz);

  if (sz > 100000) {
    debug("Cache is getting large, not writing...");
    fclose(out);
    return;
  }

  fprintf(out, "%s", postData);
  fprintf(out, "%s", "\n");
  fclose(out);
}

int strm_init (z_stream * strm)
{
  strm->zalloc = Z_NULL;
  strm->zfree  = Z_NULL;
  strm->opaque = Z_NULL;
  int ret = deflateInit2 (strm, Z_BEST_COMPRESSION, Z_DEFLATED,
        windowBits | GZIP_ENCODING, 8,
        Z_DEFAULT_STRATEGY);
  return ret;
}

void send_cache()
{
  if( access( CACHE_FILE, F_OK ) == -1 ) {
    return;
  }

  int ret = compress_cache();

  if (ret < Z_OK) {
    debug("Could not compress the file, returning.");
    return;
  }

  post_cache(CACHE_ARCHIVE);

  return;
}

int compress_cache()
{
  int ret = Z_OK;
  z_stream strm;

  Byte *ibuf, *obuf;
  FILE *ifp = fopen(CACHE_FILE, "r");
  FILE *ofp = fopen(CACHE_ARCHIVE, "w");

  ibuf = (Byte *)calloc(CHUNK, sizeof(Byte));
  obuf = (Byte *)calloc(CHUNK, sizeof(Byte));
  fread(ibuf, CHUNK, 1, ifp);

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  ret = strm_init (& strm);
  if (ret != Z_OK)
    return ret;

  strm.next_in = ibuf;
  strm.avail_in = CHUNK;
  strm.next_out = obuf;
  strm.avail_out = CHUNK;

  ret = deflate(&strm, Z_FINISH);

  fwrite(obuf, strm.total_out, 1, ofp);

  deflateEnd(&strm);
  fclose(ofp);
  fclose(ifp);

  return ret;
}
