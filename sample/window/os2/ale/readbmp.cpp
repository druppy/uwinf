/* Note: This code is a slightly modified version of the ReadBitmap function 
 * in the Toolkit JIGSAW sample program. It has been modified so it compiles as 
 * C++, suits our needs and the goto-fobia has been removed, but anyway, here is 
 * the original disclaimer:
 */
/****************************************************************************
*  Copyright (C) 1992 IBM Corporation
*
*      DISCLAIMER OF WARRANTIES.  The following [enclosed] code is
*      sample code created by IBM Corporation. This sample code is not
*      part of any standard or IBM product and is provided to you solely
*      for  the purpose of assisting you in the development of your
*      applications.  The code is provided "AS IS", without
*      warranty of any kind.  IBM shall not be liable for any damages
*      arising out of your use of the sample code, even if they have been
*      advised of the possibility of such damages.
*
******************************************************************************/
#define INCL_GPI
#define INCL_DOSFILEMGR
#include <os2.h>
#include "readbmp.hpp"

#include <mem.h>


HBITMAP ReadBitmapFromFile(const char *filename, HPS hpsBitmapFile)
{
   HBITMAP      hbmBitmapFile=NULL;
   APIRET     rc;                                             /* API return code */
   BOOL              fRet = FALSE;                      /* Function return code. */
   FILESTATUS fsts;
   PBITMAPFILEHEADER2 pbfh2;                       /* can address any file types */
   PBITMAPINFOHEADER2 pbmp2;                         /* address any info headers */
   BITMAPINFOHEADER2 bmp2BitmapFile;
   PBYTE  pFileBegin = NULL;                    /* beginning of bitmap file data */
   ULONG  cbRead;                            /* Number of bytes read by DosRead. */
   ULONG  cScans, cScansRet;              /* number of scan lines in bitmap (cy) */


   HFILE hfile;
   ULONG dontCare;
   rc = DosOpen((PSZ)filename,
                &hfile,
                &dontCare,
                0,
                0,
                OPEN_ACTION_FAIL_IF_NEW|OPEN_ACTION_OPEN_IF_EXISTS,
                OPEN_FLAGS_SEQUENTIAL|OPEN_SHARE_DENYWRITE|OPEN_ACCESS_READONLY,
                NULL
               );
   if(rc)
      return NULL;



   /*
    *   Find out how big the file is, allocate that much memory, and read
    *   in the entire bitmap.
    */

   rc = DosQueryFileInfo(hfile, 1, &fsts, sizeof(fsts));
   if(rc)
      goto fail;                        /* jump to error code outside of loop */

   rc = DosAllocMem((PPVOID) &pFileBegin,
                    (ULONG)  fsts.cbFile,
                    (ULONG)  PAG_READ | PAG_WRITE | PAG_COMMIT);

   if (rc)
      goto fail;                        /* jump to error code outside of loop */

   if (DosRead( hfile, (PVOID)pFileBegin, fsts.cbFile, &cbRead))
      goto fail;                        /* jump to error code outside of loop */

   /*
    *   If it's a bitmap-array, point to common file header.  Otherwise,
    *   point to beginning of file.
    */

   pbfh2 = (PBITMAPFILEHEADER2) pFileBegin;
   pbmp2 = NULL;                   /* only set this when we validate type */

   switch (pbfh2->usType)
   {
      case BFT_BITMAPARRAY:

         /*
          *  If this is a Bitmap-Array, adjust pointer to the normal
          *  file header.  We'll just use the first bitmap in the
          *  array and ignore other device forms.
          */

         pbfh2 = &(((PBITMAPARRAYFILEHEADER2) pFileBegin)->bfh2);
         pbmp2 = &pbfh2->bmp2;    /* pointer to info header (readability) */
         break;

      case BFT_BMAP:

         pbmp2 = &pbfh2->bmp2;    /* pointer to info header (readability) */
         break;

      default:      /* these formats aren't supported; don't set any ptrs */
      case BFT_ICON:
      case BFT_POINTER:
      case BFT_COLORICON:
      case BFT_COLORPOINTER:

         break;

   }          /* end switch (pbfh2->usType) */

   if (pbmp2 == NULL)
      goto fail;    /* File format NOT SUPPORTED: break out to error code */

   /*
    *   Check to see if BMP file has an old structure, a new structure, or
    *   Windows structure.  Capture the common data and treat all bitmaps
    *   generically with pointer to new format.  API's will determine format
    *   using cbFixed field.
    *
    *   Windows bitmaps have the new format, but with less data fields
    *   than PM.  The old strucuture has some different size fields,
    *   though the field names are the same.
    *
    *
    *   NOTE: bitmap data is located by offsetting the beginning of the file
    *         by the offset contained in pbfh2->offBits.  This value is in
    *         the same relatie location for different format bitmap files.
    */

   if (pbmp2->cbFix == sizeof(BITMAPINFOHEADER))           /* old format? */
   {
      cScans = (ULONG) ((PBITMAPINFOHEADER)pbmp2)->cy;
   }
   else                                  /* new PM format, Windows, or other */
   {
      cScans = pbmp2->cy;
   }

   memcpy(&bmp2BitmapFile,         /* copy bitmap info into global structure */
          pbmp2,
          pbmp2->cbFix);     /* only copy specified size (varies per format) */

   hbmBitmapFile=GpiCreateBitmap(hpsBitmapFile,   /*presentation-space handle*/
                                 &bmp2BitmapFile,/* structure for format data*/
                                 0L,                              /* options */
                                 NULL,               /* buffer of image data */
                                 NULL);    /* structure for color and format */

   if (!hbmBitmapFile)
      goto fail;

   if (GpiSetBitmap( hpsBitmapFile, hbmBitmapFile) == (HBITMAP)BMB_ERROR)
      goto fail;


   /*
    *   Tell GPI to put the bits into the thread's PS. The function returns
    *   the number of scan lines of the bitmap that were copied.  We want
    *   all of them at once.
    */

   cScansRet =
   GpiSetBitmapBits(
         hpsBitmapFile,                         /* presentation-space handle */
         0L,                                     /* index of first scan line */
         cScans,                                     /* number of scan lines */
         pFileBegin + pbfh2->offBits,              /* address of bitmap data */
         (PBITMAPINFO2) pbmp2);            /* address of bitmap header table */

   if (cScansRet != cScans)                          /* original # of scans? */
      goto fail;

    DosFreeMem( pFileBegin);
    DosClose( hfile);
    return hbmBitmapFile;                             /* function successful */


fail:
    /*
     *  Close the file, free the buffer space and leave.  This is an error exit
     *  point from the function.  Cleanup code is here to avoid duplicate code
     *  after each operation.
     */

    if (pFileBegin != NULL)
       DosFreeMem( pFileBegin);
    DosClose( hfile);


    return NULL;                                          /* function failed */

}   /* end ReadBitmap() */


