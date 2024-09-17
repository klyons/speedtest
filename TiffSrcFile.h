/*
Copyright(c) 2024 Transformative Optics.All rights reserved.

This software and its documentation are considered to be
proprietary and confidential information of Transformative Optics,
and may not be disclosed to unauthorized individuals
or used in any way not expressly authorized
by the license agreement accompanying this product.

Unauthorized copying of this file, via any medium,
is strictly prohibited.Modification, reverse engineering, disassembly,
or decompilation of this software is prohibited unless expressly permitted
by a written agreement with Transformative Optics.

----------------------------------------------------------
Descriptinon:

UnitTestApp.cpp - Unit test has examples on usage.

*/
#ifndef __TIFFSRCFILE_H__
#define __TIFFSRCFILE_H__   1


#include <stdint.h>
#include <vector>
#include <memory>

// Use libtiff - include header here.
#include "tiff.h"
#include <tiffio.h>

#include "TocErrors.h"
#include "TocMatrix.h"


// Error Codes
#define	kErrTiff_Open	    ERRNUM( ERRMOD_TIFF, 0x01 )     // cannot open tiff image
#define	kErrTiff_Create	    ERRNUM( ERRMOD_TIFF, 0x02 )     // cannot open tiff image
#define	kErrTiff_Close	    ERRNUM( ERRMOD_TIFF, 0x03 )     // cannot close tiff image
#define	kErrTiff_PTiff	    ERRNUM( ERRMOD_TIFF, 0x04 )     // PTiff is incorrect (e.g. NULL)
#define	kErrTiff_Read	    ERRNUM( ERRMOD_TIFF, 0x05 )     // Read error
#define	kErrTiff_Write	    ERRNUM( ERRMOD_TIFF, 0x06 )     // write error

#define	kErrTiff_NotImpl	ERRNUM( ERRMOD_TIFF, 0x05 )     // not yet implemented


/**
 * \brief TiffSrcFile is a TIFF source file with read operations.
 * 
 * TiffSrcFile contains routines to:
 *   Open, Close and Read tiff images.
 *   Writing is not supported in this class.
 * 
 * mPTiff = ptr to the libTiff file object.
*/
class TiffSrcFile
{
    TIFF			* mPTiff;           // ptr to libTiff file object
    uint32_t		mWidth;
    uint32_t		mHeight;			// height of tiff image
    uint32_t		mBPP;				// bits per pixel
    uint16_t		mSampPerPixel;		// samples per pixel

    uint16_t		mResolutionUnits;	// res units
    float			mResolutionX;		// resolution in x-direction
    float			mResolutionY;		// resolution in x-direction

public:
    TiffSrcFile() {
        mPTiff = NULL;

        ClearFile();
    }


    TocErr_t OpenFile(const char* pFilename);

    TocErr_t CloseFile();
    void ClearFile();

// Access Data Elements
public:
    uint32_t getWidth() const   { return(mWidth); }
    uint32_t getHeight() const  { return(mHeight); }
    uint32_t getBPP() const     { return(mBPP); }
    uint16_t getSampPerPixel() const    { return(mSampPerPixel); }

    bool IsMonoTiff() const {
        bool		isMono = false;
        if( mPTiff != NULL ) {
            // Make sure we're monochrome
            if (mSampPerPixel == 1 ) {
                isMono = true;
            }
        }
        return(isMono);
    }

    // Read in a monochrome image - 
//    TocErr_t ReadMonochrome( std::unique_ptr<uint16_t[]> & bufImg );

    TocErr_t ReadMonochrome( std::vector<uint16_t> & bufImg);

    TocErr_t ReadMonochrome(CTocMatrix<uint16_t> & bufImg);


// Write Routines
public:
//    TocErr_t WriteMultiChan( CTocMatrix<uint16)
// Test routine:
//    TocErr_t TestIt();

};


/**
*  Write a multi-channel TIFF to the given file.
*/
template< class _TChan>
TocErr_t 
TiffWriteMultiChan(CTocMatrix<_TChan> & tiffData, const char * pFNameTiff )
{
    TocErr_t    ec    = kErrTiff_Create;    // create TIFF 
    TIFF *      pTiff = NULL;               // ptr to libTiff file object

    pTiff = TIFFOpen(pFNameTiff, "w");
    if (pTiff != NULL) 
    {
        int         nSetRet = 0;

        // Set the TIFF fields
        size_t      nWidth  = tiffData.getWidth();
        size_t      nHeight = tiffData.getHeight();
        unsigned    nBitsPerSample = tiffData.GetBitsPerSamp();

        TIFFSetField(pTiff, TIFFTAG_IMAGEWIDTH, nWidth);
        TIFFSetField(pTiff, TIFFTAG_IMAGELENGTH, nHeight);
        TIFFSetField(pTiff, TIFFTAG_BITSPERSAMPLE, nBitsPerSample );
        TIFFSetField(pTiff, TIFFTAG_SAMPLESPERPIXEL, tiffData.GetSampsPerPixel( ) );
        TIFFSetField(pTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(pTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
        TIFFSetField(pTiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
        TIFFSetField(pTiff, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

        // We have 1 row per strip
        //  This is not the fastest but it's the easiest,
        //  If we use nHeight then we must write the entire frame on one write.
        //    TIFFSetField(pTiff, TIFFTAG_ROWSPERSTRIP, height); // One strip for the whole image
        TIFFSetField(pTiff, TIFFTAG_ROWSPERSTRIP, 1);       // One strip for the whole image

        // Write out the tiff data
        size_t          nRetWrite = 0;

        // Write out the tiff data scanline by scanline
        //   If there is an error while we write we mark an error
        size_t      nScanLen = sizeof(_TChan) * nWidth * tiffData.GetSampsPerPixel();
        ec = kNoError;

        for (unsigned nRow = 0; nRow < nHeight; nRow += 1)
        {
            _TChan * pNext = tiffData.GetRowPtr(nRow);

            //            nRetWrite = TIFFWriteScanline(pTiff, tiffData.GetRowPtr( nRow ), nRow, nScanLen );
            nRetWrite = TIFFWriteEncodedStrip(pTiff, nRow, pNext, nScanLen );
            if ((nRetWrite == -1 ) || (nRetWrite < nScanLen)) {
                // Mark an error
                ec = kErrTiff_Write;
            }
        }

        // Close the file
        TIFFClose(pTiff);
        pTiff = NULL;
    }

    return( ec );
}

#endif // __TIFFSRCFILE_H__
