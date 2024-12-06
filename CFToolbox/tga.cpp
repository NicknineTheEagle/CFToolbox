#include "tga.h"
#pragma pack(push, 1)
/* TGA header */
typedef struct
{
  BYTE id_lenght;          /* size of image id */
  BYTE colormap_type;      /* 1 is has a colormap */
  BYTE image_type;         /* compression type */

  WORD	cm_first_entry;       /* colormap origin */
  WORD	cm_length;            /* colormap length */
  BYTE cm_size;            /* colormap size */

  WORD	x_origin;             /* bottom left x coord origin */
  WORD	y_origin;             /* bottom left y coord origin */

  WORD	width;                /* picture width (in pixels) */
  WORD	height;               /* picture height (in pixels) */

  BYTE pixel_depth;        /* bits per pixel: 8, 16, 24 or 32 */
  BYTE image_descriptor;   /* 24 bits = 0x00; 32 bits = 0x80 */
} TGA_header_t;
#pragma pack(pop)





static void
ReadTGA8bits (const BYTE *data, const BYTE *colormap,
	        TGA *tgaData)
{
  DWORD i;
  BYTE color;

  for (i = 0; i < tgaData->width * tgaData->height; ++i)
    {
      /* Read index color byte */
      color = data[i];

      /* Convert to RGB 24 bits */
	  tgaData->pixels[(i * 4) + 3] = 0xFF;
      tgaData->pixels[(i * 4) + 2] = colormap[(color * 3) + 0];
      tgaData->pixels[(i * 4) + 1] = colormap[(color * 3) + 1];
      tgaData->pixels[(i * 4) + 0] = colormap[(color * 3) + 2];
    }
}

static void
ReadTGA16bits (const BYTE *data, TGA *tgaData)
{
  DWORD i, j = 0;
  unsigned short color;

  for (i = 0; i < tgaData->width * tgaData->height; ++i, j += 2)
    {
      /* Read color word */
      color = data[j] + (data[j + 1] << 8);

      /* Convert BGR to RGB */
      tgaData->pixels[(i * 4) + 0] = (BYTE)(((color & 0x7C00) >> 10) << 3);
      tgaData->pixels[(i * 4) + 1] = (BYTE)(((color & 0x03E0) >>  5) << 3);
      tgaData->pixels[(i * 4) + 2] = (BYTE)(((color & 0x001F) >>  0) << 3);
	  tgaData->pixels[(i * 4) + 3] = 0xFF;
    }
}

static void
ReadTGA24bits (const BYTE *data, TGA *tgaData)
{
  DWORD i, j = 0;

  for (i = 0; i < tgaData->width * tgaData->height; ++i, j += 3)
    {
      /* Read and convert BGR to RGB */
	  tgaData->pixels[(i * 4) + 3] = 0xFF; // emulated alpha
      tgaData->pixels[(i * 4) + 2] = data[j + 0];
      tgaData->pixels[(i * 4) + 1] = data[j + 1];
      tgaData->pixels[(i * 4) + 0] = data[j + 2];
    }
}

static void
ReadTGA32bits (const BYTE *data, TGA *tgaData)
{
  DWORD i, j = 0;

  for (i = 0; i < tgaData->width * tgaData->height; ++i, j += 4)
    {
      /* Read and convert BGRA to RGBA */
      tgaData->pixels[(i * 4) + 2] = data[j + 0];
      tgaData->pixels[(i * 4) + 1] = data[j + 1];
      tgaData->pixels[(i * 4) + 0] = data[j + 2];
      tgaData->pixels[(i * 4) + 3] = data[j + 3];
    }
}

static void
ReadTGAgray8bits (const BYTE *data, TGA *tgaData)
{
  memcpy (tgaData->pixels, data, sizeof (BYTE) *
	  tgaData->width * tgaData->height);
}

static void
ReadTGAgray16bits (const BYTE *data, TGA *tgaData)
{
  memcpy (tgaData->pixels, data, sizeof (BYTE) *
	  tgaData->width * tgaData->height * 2);
}

static void
ReadTGA8bitsRLE (const BYTE *data, const BYTE *colormap,
		 TGA *tgaData)
{
  DWORD i, size, j = 0;
  BYTE color;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height) * 3)
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  color = data[j++];

	  for (i = 0; i < size; ++i, ptr += 3)
	    {
	      ptr[0] = colormap[(color * 3) + 2];
	      ptr[1] = colormap[(color * 3) + 1];
	      ptr[2] = colormap[(color * 3) + 0];
	    }
	}
      else
	{
	  /* Non run-length packet */
	  for (i = 0; i < size; ++i, ptr += 3)
	    {
	      color = data[j++];

	      ptr[0] = colormap[(color * 3) + 2];
	      ptr[1] = colormap[(color * 3) + 1];
	      ptr[2] = colormap[(color * 3) + 0];
	    }
	}
    }
}

static void
ReadTGA16bitsRLE (const BYTE *data, TGA *tgaData)
{
  DWORD i, size, j = 0;
  unsigned short color;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height) * 3)
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  color = data[j] + (data[j + 1] << 8);
	  j += 2;

	  for (i = 0; i < size; ++i, ptr += 3)
	    {
	      ptr[0] = (BYTE)(((color & 0x7C00) >> 10) << 3);
	      ptr[1] = (BYTE)(((color & 0x03E0) >>  5) << 3);
	      ptr[2] = (BYTE)(((color & 0x001F) >>  0) << 3);
	    }
	}
      else
	{
	  /* Non run-length packet */
	  for (i = 0; i < size; ++i, ptr += 3, j += 2)
	    {
	      color = data[j] + (data[j + 1] << 8);

	      ptr[0] = (BYTE)(((color & 0x7C00) >> 10) << 3);
	      ptr[1] = (BYTE)(((color & 0x03E0) >>  5) << 3);
	      ptr[2] = (BYTE)(((color & 0x001F) >>  0) << 3);
	    }
	}
    }
}

static void
ReadTGA24bitsRLE (const BYTE *data, TGA *tgaData)
{
  DWORD i, size, j = 0;
  const BYTE *rgb;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height) * 3)
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  rgb = &data[j];
	  j += 3;

	  for (i = 0; i < size; ++i, ptr += 3)
	    {
	      ptr[0] = rgb[2];
	      ptr[1] = rgb[1];
	      ptr[2] = rgb[0];
	    }
	}
      else
	{
	  /* Non run-length packet */
	  for (i = 0; i < size; ++i, ptr += 3, j += 3)
	    {
	      ptr[2] = data[j + 0];
	      ptr[1] = data[j + 1];
	      ptr[0] = data[j + 2];
	    }
	}
    }
}

static void
ReadTGA32bitsRLE (const BYTE *data, TGA *tgaData)
{
  DWORD i, size, j = 0;
  const BYTE *rgba;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height) * 4)
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  rgba = &data[j];
	  j += 4;

	  for (i = 0; i < size; ++i, ptr += 4)
	    {
	      ptr[0] = rgba[2];
	      ptr[1] = rgba[1];
	      ptr[2] = rgba[0];
	      ptr[3] = rgba[3];
	    }
	}
      else
	{
	  /* Non run-length packet */
	  for (i = 0; i < size; ++i, ptr += 4, j += 4)
	    {
	      ptr[2] = data[j + 0];
	      ptr[1] = data[j + 1];
	      ptr[0] = data[j + 2];
	      ptr[3] = data[j + 3];
	    }
	}
    }
}

static void
ReadTGAgray8bitsRLE (const BYTE *data, TGA *tgaData)
{
  DWORD i, size, j = 0;
  BYTE color;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height))
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  color = data[j++];

	  for (i = 0; i < size; ++i, ptr++)
	    *ptr = color;
	}
      else
	{
	  /* Non run-length packet */
	  memcpy (ptr, data + j, size * sizeof (BYTE));

	  ptr += size;
	  j += size;
	}
    }
}

static void
ReadTGAgray16bitsRLE (const BYTE *data, TGA *tgaData)
{
  DWORD i, size, j = 0;
  BYTE color, alpha;
  BYTE packet_header;
  BYTE *ptr = tgaData->pixels;

  while (ptr < tgaData->pixels + (tgaData->width * tgaData->height) * 2)
    {
      /* Read first byte */
      packet_header = data[j++];
      size = 1 + (packet_header & 0x7f);

      if (packet_header & 0x80)
	{
	  /* Run-length packet */
	  color = data[j++];
	  alpha = data[j++];

	  for (i = 0; i < size; ++i, ptr += 2)
	    {
	      ptr[0] = color;
	      ptr[1] = alpha;
	    }
	}
      else
	{
	  /* Non run-length packet */
	  memcpy (ptr, data + j, size * sizeof (BYTE) * 2);

	  ptr += size * 2;
	  j += size * 2;
	}
    }
}



TGA * parseTGA (char * buffer, long length)
{
 
  TGA * tgaData;
  TGA_header_t *header;
 
  BYTE *colormap = NULL;
  BYTE *ptr;
  
  ptr = (BYTE*)buffer;

  /* Extract header */
  header = (TGA_header_t *)ptr;
  ptr += sizeof (TGA_header_t);

  tgaData = (TGA *)malloc (sizeof (TGA));
  tgaData->height=header->height;
  tgaData->width=header->width;
//  GetTextureInfo (header, tgaData);
  ptr += header->id_lenght;

  /* Memory allocation for pixel data */
tgaData->pixels=(BYTE*)malloc(4*tgaData->width*tgaData->height);
 
  /* Read color map */
  if (header->colormap_type)
    {
      /* NOTE: color map is stored in BGR format */
      colormap = ptr;
      ptr += header->cm_length * (header->cm_size >> 3);
    }

  /* Read image data */
  switch (header->image_type)
    {
    case 0:
      /* No data */
      break;

    case 1:
      /* Uncompressed 8 bits color index */
      ReadTGA8bits (ptr, colormap, tgaData);
      break;

    case 2:
      /* Uncompressed 16-24-32 bits */
      switch (header->pixel_depth)
	{
	case 16:
	  ReadTGA16bits (ptr, tgaData);
	  break;

	case 24:
	  ReadTGA24bits (ptr, tgaData);
	  break;

	case 32:
	  ReadTGA32bits (ptr, tgaData);
	  break;
	}

      break;

    case 3:
      /* Uncompressed 8 or 16 bits grayscale */
      if (header->pixel_depth == 8)
	ReadTGAgray8bits (ptr, tgaData);
      else /* 16 */
	ReadTGAgray16bits (ptr, tgaData);

      break;

    case 9:
      /* RLE compressed 8 bits color index */
      ReadTGA8bitsRLE (ptr, colormap, tgaData);
      break;

    case 10:
      /* RLE compressed 16-24-32 bits */
      switch (header->pixel_depth)
	{
	case 16:
	  ReadTGA16bitsRLE (ptr, tgaData);
	  break;

	case 24:
	  ReadTGA24bitsRLE (ptr, tgaData);
	  break;

	case 32:
	  ReadTGA32bitsRLE (ptr, tgaData);
	  break;
	}

      break;

    case 11:
      /* RLE compressed 8 or 16 bits grayscale */
      if (header->pixel_depth == 8)
	ReadTGAgray8bitsRLE (ptr, tgaData);
      else /* 16 */
	ReadTGAgray16bitsRLE (ptr, tgaData);

      break;

 
  }

  return tgaData;
}