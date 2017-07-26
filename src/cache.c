#include <options.h>
#include "dbg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "zlib.h"
#include <assert.h>
#include <http.h>

// GZIP compression
#define windowBits 15
#define GZIP_ENCODING 16
/* #define CHUNK 0x4000 */
#define CHUNK 16384*2

int compress_cache();

void cache(const char *postData)
{

  if (options.debug) {
    debug("Caching the datas!");
  }

  FILE *out = fopen(options.cache, "a");

  if(out == NULL) {
    perror("Error opening file.");
  }

  fseek(out, 0L, SEEK_END);
  unsigned long sz = (unsigned long)ftell(out);
  fseek(out, 0, SEEK_SET);

  /* debug("FILE SIZE: %ld", sz); */

  if (sz > 5000000) {
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

void send_cached()
{

  if( access( options.cache, F_OK ) == -1 ) {
    debug("No cache file, not sending...");
    return;
  }

  int ret = compress_cache();

  if (ret < Z_OK) {
    debug("Could not compress the file, returning.");
    return;
  }

  int run = post_cache(options.archive);

  // Remove the archive whatever the result
  int del = unlink(options.archive);
  if (del != 0) {
    printf("Archive could not be deleted!");
  }

  // run == the returned value from the api
  if (run == 0) {
    return;
  }

  // delete the archive every run but leave the cache
  // in place just in case it fails
  del = unlink(options.cache);
  if(del != 0) {
    printf("File can not be deleted!");
  }

  return;
}

int compress_cache()
{
  int ret = Z_OK;
  z_stream strm;

  Byte *ibuf, *obuf;
  FILE *ifp = fopen(options.cache, "r");
  FILE *ofp = fopen(options.archive, "w");

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

  (void)deflateEnd(&strm);
  fclose(ofp);
  fclose(ifp);

  free(ibuf);
  free(obuf);

  return ret;
}
