#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

unsigned char *buffer;
unsigned int offs;
unsigned int startpos;
uint16_t cur_m;

uint32_t vertices;
uint32_t normals;
uint32_t uvs;
uint32_t triflat;
uint32_t trismooth;
uint32_t triflatuvs;
uint32_t trismoothuvs;
uint32_t quadflat;
uint32_t quadsmooth;
uint32_t quadflatuvs;
uint32_t quadsmoothuvs;

unsigned char parseUChar8(unsigned int offset)
{
  return buffer[offset];
}

char parseChar8(unsigned int offset)
{
  return (char)buffer[offset];
}

uint32_t parseUInt32(unsigned int offset)
{
  return ((buffer[offset+3]<<24)|(buffer[offset+2]<<16)|(buffer[offset+1]<<8)|buffer[offset]);
}

float parseFloat32(unsigned int offset)
{
  union { float f; unsigned char b[4]; } u;

  u.b[3]=buffer[offset+3];
  u.b[2]=buffer[offset+2];
  u.b[1]=buffer[offset+1];
  u.b[0]=buffer[offset+0];

  return u.f;
}

uint16_t parseUShort16(unsigned int offset)
{
  return ((buffer[offset+1]<<8)|buffer[offset]);
}

unsigned char parsemetadata()
{
  int i;

  fprintf(stderr, "signature = \"");
  for (i=0; i<12; i++)
    fprintf(stderr, "%c", buffer[i]);
  fprintf(stderr, "\"\n");

  // All data is little-endian

  fprintf(stderr, "header_bytes = %d\n\n", parseUChar8(offs+12)); // 64

  fprintf(stderr, "vertex_coordinate_bytes = %d\n", parseUChar8(offs+13)); // 4
  fprintf(stderr, "normal_coordinate_bytes = %d\n", parseUChar8(offs+14)); // 1
  fprintf(stderr, "uv_coordinate_bytes = %d\n\n", parseUChar8(offs+15)); // 4

  fprintf(stderr, "vertex_index_bytes = %d\n", parseUChar8(offs+16)); // 4
  fprintf(stderr, "normal_index_bytes = %d\n", parseUChar8(offs+17)); // 4
  fprintf(stderr, "uv_index_bytes = %d\n", parseUChar8(offs+18)); // 4
  fprintf(stderr, "material_index_bytes = %d\n\n", parseUChar8(offs+19)); // 2

  fprintf(stderr, "nvertices = 12 x %d\n", parseUInt32(offs+20)); vertices=parseUInt32(offs+20);
  fprintf(stderr, "nnormals = 3 x %d\n", parseUInt32(offs+20+(4*1))); normals=parseUInt32(offs+20+(4*1));
  fprintf(stderr, "nuvs = 8 x %d\n\n", parseUInt32(offs+20+(4*2))); uvs=parseUInt32(offs+20+(4*2));

  fprintf(stderr, "ntri_flat = 14 x %d\n", parseUInt32(offs+20+(4*3))); triflat=parseUInt32(offs+20+(4*3));
  fprintf(stderr, "ntri_smooth = 26 x %d\n", parseUInt32(offs+20+(4*4))); trismooth=parseUInt32(offs+20+(4*4));
  fprintf(stderr, "ntri_flat_uv = 26 x %d\n", parseUInt32(offs+20+(4*5))); triflatuvs=parseUInt32(offs+20+(4*5));
  fprintf(stderr, "ntri_smooth_uv = 38 x %d\n\n", parseUInt32(offs+20+(4*6))); trismoothuvs=parseUInt32(offs+20+(4*6));

  fprintf(stderr, "nquad_flat = 18 x %d\n", parseUInt32(offs+20+(4*7))); quadflat=parseUInt32(offs+20+(4*7));
  fprintf(stderr, "nquad_smooth = 34 x %d\n", parseUInt32(offs+20+(4*8))); quadsmooth=parseUInt32(offs+20+(4*8));
  fprintf(stderr, "nquad_flat_uv = 34 x %d\n", parseUInt32(offs+20+(4*9))); quadflatuvs=parseUInt32(offs+20+(4*9));
  fprintf(stderr, "nquad_smooth_uv = 50 x %d\n\n", parseUInt32(offs+20+(4*10))); quadsmoothuvs=parseUInt32(offs+20+(4*10));

  return parseUChar8(12);
}

unsigned int handlePadding(unsigned int n)
{
  return ( n % 4 ) ? ( 4 - n % 4 ) : 0;
}

int main(int argc, char **argv)
{
  FILE *fp;
  size_t flen;
  unsigned char headerlen;
  uint32_t i;
  uint32_t vt;
  struct stat st;

  if (argc!=2)
  {
    fprintf(stderr, "Specify .bin file on command line\n");
    return 1;
  }

  if (stat(argv[1], &st)!=0)
  {
    fprintf(stderr, "Unable to check file '%s'\n", argv[1]);
    return 1;
  }

  buffer=malloc(st.st_size);

  if (buffer==NULL)
  {
    fprintf(stderr, "Unable to allocate %ld bytes for buffer\n", st.st_size);
    return 1;
  }

  fp=fopen(argv[1], "r");
  if (fp!=NULL)
  {
    flen=fread(buffer, 1, st.st_size, fp);
    fclose(fp);
  }
  else
  {
    fprintf(stderr, "Unable to open .bin file\n");
    free(buffer);
    return 2;
  }

  fprintf(stderr, "File size : %d\n", flen);

  offs=0;
  headerlen=parsemetadata();
  offs+=headerlen;

  cur_m=UINT16_MAX;
  fprintf(stdout, "mtllib tex.mtl\n");

  // 1. vertices
  // ------------
  // x float   4
  // y float   4
  // z float   4
  for (i=0; i<vertices; i++)
  {
    float x, y, z;

    x=parseFloat32(offs+0);
    y=parseFloat32(offs+4);
    z=parseFloat32(offs+8);

    fprintf(stdout, "v %f %f %f\n", x, y, z);
    offs+=(3*4);
  }

  fprintf(stderr, "Processed %d vertices [%d]\n", vertices, offs);

  // 2. normals
  // ---------------
  // x signed char 1
  // y signed char 1
  // z signed char 1
  for (i=0; i<normals; i++)
  {
    char x, y, z;

    x=parseChar8(offs+0);
    y=parseChar8(offs+1);
    z=parseChar8(offs+2);

    fprintf(stdout, "vn %d %d %d\n", x, y, z);
    offs+=3;
  }

  fprintf(stderr, "Processed %d normals [%d]\n", normals, offs);

  // Handle padding
  if (normals>0)
    offs+=handlePadding(normals*3);

  // 3. uvs
  // -----------
  // u float   4
  // v float   4
  for (i=0; i<uvs; i++)
  {
    float u, v;

    u=parseFloat32(offs+0);
    v=parseFloat32(offs+4);

    fprintf(stdout, "vt %f %f\n", u, 1-v); // Flipped Y
    offs+=8;
  }

  fprintf(stderr, "Processed %d uvs [%d]\n", uvs, offs);

  // 4. flat triangles (vertices + materials)
  // ------------------
  // a unsigned int   4
  // b unsigned int   4
  // c unsigned int   4
  // ------------------
  // m unsigned short 2   // TODO
  if (triflat>0)
  {
    startpos=offs;
    for (i=0; i<triflat; i++)
    {
      uint32_t a, b, c;
      uint16_t m;

      a=parseUInt32(startpos+(i*12)+0);
      b=parseUInt32(startpos+(i*12)+4);
      c=parseUInt32(startpos+(i*12)+8);

      m=parseUShort16(startpos+(triflat*12)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d %d %d\n", a+1, b+1, c+1);
    }

    offs+=((12+2)*triflat);
    fprintf(stderr, "Processed %d tri flat [%d]\n", triflat, offs);
    offs+=handlePadding(triflat*2);
  }

  // 5. smooth triangles (vertices + materials + normals)
  // -------------------
  // a  unsigned int   4
  // b  unsigned int   4
  // c  unsigned int   4
  // -------------------
  // na unsigned int   4
  // nb unsigned int   4
  // nc unsigned int   4
  // -------------------
  // m  unsigned short 2  TODO
  if (trismooth>0)
  {
    startpos=offs;
    for (i=0; i<trismooth; i++)
    {
      uint32_t a, b, c;
      uint32_t na, nb, nc;
      uint16_t m;

      a=parseUInt32(startpos+(i*12)+0);
      b=parseUInt32(startpos+(i*12)+4);
      c=parseUInt32(startpos+(i*12)+8);

      na=parseUInt32(startpos+(trismooth*12)+(i*12)+0);
      nb=parseUInt32(startpos+(trismooth*12)+(i*12)+4);
      nc=parseUInt32(startpos+(trismooth*12)+(i*12)+8);

      m=parseUShort16(startpos+(trismooth*12)+(trismooth*12)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d//%d %d//%d %d//%d\n", a+1, na+1, b+1, nb+1, c+1, nc+1);
    }

    offs+=((12+12+2)*trismooth);

    fprintf(stderr, "Processed %d tri smooth [%d]\n", trismooth, offs);

    offs+=handlePadding(trismooth*2);
  }

  // 6. flat triangles uv (vertices + materials + uvs)
  // --------------------
  // a  unsigned int    4
  // b  unsigned int    4
  // c  unsigned int    4
  // --------------------
  // ua unsigned int    4
  // ub unsigned int    4
  // uc unsigned int    4
  // --------------------
  // m  unsigned short  2  TODO
  if (triflatuvs>0)
  {
    startpos=offs;
    for (i=0; i<triflatuvs; i++)
    {
      uint32_t a, b, c;
      uint32_t ua, ub, uc;
      uint16_t m;

      a=parseUInt32(startpos+(i*12)+0);
      b=parseUInt32(startpos+(i*12)+4);
      c=parseUInt32(startpos+(i*12)+8);

      ua=parseUInt32(startpos+(triflatuvs*12)+(i*12)+0);
      ub=parseUInt32(startpos+(triflatuvs*12)+(i*12)+4);
      uc=parseUInt32(startpos+(triflatuvs*12)+(i*12)+8);

      m=parseUShort16(startpos+(triflatuvs*12)+(triflatuvs*12)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d/%d %d/%d %d/%d\n", a+1, ua+1, b+1, ub+1, c+1, uc+1);
    }

    offs+=((12+12+2)*triflatuvs);

    fprintf(stderr, "Processed %d tri flat uvs [%d]\n", triflatuvs, offs);

    offs+=handlePadding(triflatuvs*2);
  }

  // 7. smooth triangles uv (vertices + materials + normals + uvs)
  // ----------------------
  // a  unsigned int    4
  // b  unsigned int    4
  // c  unsigned int    4
  // --------------------
  // na unsigned int    4
  // nb unsigned int    4
  // nc unsigned int    4
  // --------------------
  // ua unsigned int    4
  // ub unsigned int    4
  // uc unsigned int    4
  // --------------------
  // m  unsigned short  2  TODO
  if (trismoothuvs>0)
  {
    startpos=offs;
    for (i=0; i<trismoothuvs; i++)
    {
      uint32_t a, b, c;
      uint32_t na, nb, nc;
      uint32_t ua, ub, uc;
      uint16_t m;

      a=parseUInt32(startpos+(i*12)+0);
      b=parseUInt32(startpos+(i*12)+4);
      c=parseUInt32(startpos+(i*12)+8);

      na=parseUInt32(startpos+(trismoothuvs*12)+(i*12)+0);
      nb=parseUInt32(startpos+(trismoothuvs*12)+(i*12)+4);
      nc=parseUInt32(startpos+(trismoothuvs*12)+(i*12)+8);

      ua=parseUInt32(startpos+(trismoothuvs*12)+(trismoothuvs*12)+(i*12)+0);
      ub=parseUInt32(startpos+(trismoothuvs*12)+(trismoothuvs*12)+(i*12)+4);
      uc=parseUInt32(startpos+(trismoothuvs*12)+(trismoothuvs*12)+(i*12)+8);

      m=parseUShort16(startpos+(trismoothuvs*12)+(trismoothuvs*12)+(trismoothuvs*12)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", a+1, ua+1, na+1, b+1, ub+1, nb+1, c+1, uc+1, nc+1);
    }

    offs+=((12+12+12+2)*trismoothuvs);

    fprintf(stderr, "Processed %d tri smooth uvs [%d]\n", trismoothuvs, offs);

    offs+=handlePadding(trismoothuvs*2);
  }

  // 8. flat quads (vertices + materials)
  // ------------------
  // a unsigned int   4
  // b unsigned int   4
  // c unsigned int   4
  // d unsigned int   4
  // --------------------
  // m unsigned short 2  // TODO
  if (quadflat>0)
  {
    startpos=offs;
    for (i=0; i<quadflat; i++)
    {
      uint32_t a, b, c, d;
      uint16_t m;

      a=parseUInt32(startpos+(i*16)+0);
      b=parseUInt32(startpos+(i*16)+4);
      c=parseUInt32(startpos+(i*16)+8);
      d=parseUInt32(startpos+(i*16)+12);

      m=parseUShort16(startpos+(16*quadflat)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d %d %d %d\n", a+1, b+1, c+1, d+1);
    }

    offs+=((16+2)*quadflat);
    fprintf(stderr, "Processed %d quad flat [%d]\n", quadflat, offs);
    offs+=handlePadding(quadflat*2);
  }

  // 9. smooth quads (vertices + materials + normals)
  // -------------------
  // a  unsigned int   4
  // b  unsigned int   4
  // c  unsigned int   4
  // d  unsigned int   4
  // --------------------
  // na unsigned int   4
  // nb unsigned int   4
  // nc unsigned int   4
  // nd unsigned int   4
  // --------------------
  // m  unsigned short 2  // TODO
  if (quadsmooth>0)
  {
    startpos=offs;
    for (i=0; i<quadsmooth; i++)
    {
      uint32_t a, b, c, d;
      uint32_t na, nb, nc, nd;
      uint16_t m;

      a=parseUInt32(startpos+(i*16)+0);
      b=parseUInt32(startpos+(i*16)+4);
      c=parseUInt32(startpos+(i*16)+8);
      d=parseUInt32(startpos+(i*16)+12);

      na=parseUInt32(startpos+(16*quadsmooth)+(i*16)+0);
      nb=parseUInt32(startpos+(16*quadsmooth)+(i*16)+4);
      nc=parseUInt32(startpos+(16*quadsmooth)+(i*16)+8);
      nd=parseUInt32(startpos+(16*quadsmooth)+(i*16)+12);

      m=parseUShort16(startpos+(16*quadsmooth)+(16*quadsmooth)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d//%d %d//%d %d//%d %d//%d\n", a+1, na+1, b+1, nb+1, c+1, nc+1, d+1, nd+1);
    }

    offs+=((16+16+2)*quadsmooth);
    fprintf(stderr, "Processed %d quad smooth [%d]\n", quadsmooth, offs);
    offs+=handlePadding(quadsmooth*2);
  }

  // 10. flat quads uv (vertices + materials + uvs)
  // ------------------
  // a unsigned int   4
  // b unsigned int   4
  // c unsigned int   4
  // d unsigned int   4
  // --------------------
  // ua unsigned int  4
  // ub unsigned int  4
  // uc unsigned int  4
  // ud unsigned int  4
  // --------------------
  // m unsigned short 2  // TODO
  if (quadflatuvs>0)
  {
    startpos=offs;
    for (i=0; i<quadflatuvs; i++)
    {
      uint32_t a, b, c, d;
      uint32_t ua, ub, uc, ud;
      uint16_t m;

      a=parseUInt32(startpos+(i*16)+0);
      b=parseUInt32(startpos+(i*16)+4);
      c=parseUInt32(startpos+(i*16)+8);
      d=parseUInt32(startpos+(i*16)+12);

      ua=parseUInt32(startpos+(16*quadflatuvs)+(i*16)+0);
      ub=parseUInt32(startpos+(16*quadflatuvs)+(i*16)+4);
      uc=parseUInt32(startpos+(16*quadflatuvs)+(i*16)+8);
      ud=parseUInt32(startpos+(16*quadflatuvs)+(i*16)+12);

      m=parseUShort16(startpos+(16*quadflatuvs)+(16*quadflatuvs)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d/%d %d/%d %d/%d %d/%d\n", a+1, ua+1, b+1, ub+1, c+1, uc+1, d+1, ud+1);
    }

    offs+=((16+16+2)*quadflatuvs);
    fprintf(stderr, "Processed %d quad flat uvs [%d]\n", quadflatuvs, offs);
    offs+=handlePadding(quadflatuvs*2);
  }

  // 11. smooth quads uv
  // -------------------
  // a  unsigned int   4
  // b  unsigned int   4
  // c  unsigned int   4
  // d  unsigned int   4
  // --------------------
  // na unsigned int   4
  // nb unsigned int   4
  // nc unsigned int   4
  // nd unsigned int   4
  // --------------------
  // ua unsigned int   4
  // ub unsigned int   4
  // uc unsigned int   4
  // ud unsigned int   4
  // --------------------
  // m  unsigned short 2  TODO
  if (quadsmoothuvs>0)
  {
    startpos=offs;
    for (i=0; i<quadsmoothuvs; i++)
    {
      uint32_t a, b, c, d;
      uint32_t na, nb, nc, nd;
      uint32_t ua, ub, uc, ud;
      uint16_t m;

      a=parseUInt32(startpos+(i*16)+0);
      b=parseUInt32(startpos+(i*16)+4);
      c=parseUInt32(startpos+(i*16)+8);
      d=parseUInt32(startpos+(i*16)+12);

      na=parseUInt32(startpos+(16*quadsmoothuvs)+(i*16)+0);
      nb=parseUInt32(startpos+(16*quadsmoothuvs)+(i*16)+4);
      nc=parseUInt32(startpos+(16*quadsmoothuvs)+(i*16)+8);
      nd=parseUInt32(startpos+(16*quadsmoothuvs)+(i*16)+12);

      ua=parseUInt32(startpos+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(i*16)+0);
      ub=parseUInt32(startpos+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(i*16)+4);
      uc=parseUInt32(startpos+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(i*16)+8);
      ud=parseUInt32(startpos+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(i*16)+12);

      m=parseUShort16(startpos+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(16*quadsmoothuvs)+(i*2));
      if (m!=cur_m)
      {
        fprintf(stdout, "usemtl Texture_%d\n", m);
        cur_m=m;
      }

      fprintf(stdout, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", a+1, ua+1, na+1, b+1, ub+1, nb+1, c+1, uc+1, nc+1, d+1, ud+1, nd+1);
    }

    offs+=((16+16+16+2)*quadsmoothuvs);
    fprintf(stderr, "Processed %d quad smooth uvs [%d]\n", quadsmoothuvs, offs);
    offs+=handlePadding(quadsmoothuvs*2);
  }

  fprintf(stderr, "Offset: %d\n", offs);

  free(buffer);
  return 0;
}
