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
Description:
Basic implementation of tiff read.

*/
#include "TiffSrcFile.h"

#include <sstream>

/**
 *  Open a .tiff image.
*/
TocErr_t
TiffSrcFile::OpenFile(const char* pFilename)
{
    TocErr_t			nReturn = 0;

    CloseFile();

    mPTiff = TIFFOpen(pFilename, "r");

    if (mPTiff != NULL) {
        TIFFGetField(mPTiff, TIFFTAG_IMAGEWIDTH, &mWidth);
        TIFFGetField(mPTiff, TIFFTAG_IMAGELENGTH, &mHeight);
        TIFFGetField(mPTiff, TIFFTAG_BITSPERSAMPLE, &mBPP);
        TIFFGetField(mPTiff, TIFFTAG_SAMPLESPERPIXEL, &mSampPerPixel);

        // Resolution
        TIFFGetField(mPTiff, TIFFTAG_RESOLUTIONUNIT, &mResolutionUnits);
        TIFFGetField(mPTiff, TIFFTAG_XRESOLUTION, &mResolutionX );
        TIFFGetField(mPTiff, TIFFTAG_YRESOLUTION, &mResolutionY );
    }
    else {
        nReturn = kErrTiff_Open;
        mPTiff = NULL;
    }

    return(nReturn);
}


/**
 *  close a .tiff image.
*/
TocErr_t
TiffSrcFile::CloseFile( )
{
    if (mPTiff != NULL) {
        TIFFClose(mPTiff);
        mPTiff = NULL;
    }
    ClearFile();

    return(kNoError);
}

void 
TiffSrcFile::ClearFile()
{
    mPTiff  = NULL;
    mWidth  = 0;
    mHeight = 0;
    mBPP = 0;
    mSampPerPixel = 0;

    mResolutionX = 0.0f;
    mResolutionY = 0.0f;
}


/**
 * \brief Read in a monochrome image.
 *
 * Tiff must be open before reading.
 * Detailed sequence:
    const char* pFilename = "MyImage.tif";

    nReturn = OpenFile(pFilename);

    std::vector<uint16_t>    bufImg
    int			nEc = ReadMonochrome16( bufImg );
    CloseFile();
 *
 *
 * @param  bufImg = ref to unique_ptr for uint16 values.
 * @return Error Code
 */
int
TiffSrcFile::ReadMonochrome( std::vector<uint16_t>& bufImg )
{
    int			ec = kErrTiff_PTiff;		    // return number of scans read

    if (mPTiff != NULL)
    {
        // Accept only:
        //	16 bit
        //	1 sample per pixel
        //	Round scaneline out to uint16 - this is the widthRead
        //  Readscanline and advance by widthRead.
        //
        size_t			nLenBuffer = (mWidth * mHeight);

        // CHeck to see that we're a uint16 monochrome image.
        if (IsMonoTiff()) {
            if (mBPP == 16) {
                bufImg.resize(nLenBuffer);
            }

            if (bufImg.size() >= nLenBuffer)
            {
                // Read in the tiff image
                size_t		nNext = 0;

                for (unsigned nRow = 0; nRow < mHeight; nRow++)
                {
                    //                uint16_t* pScan = (bufImg.begin() + nNext);
                    uint16_t* pScan = (bufImg.data() + nNext);

                    TIFFReadScanline(mPTiff, pScan, nRow);

                    nNext += mWidth;
                }

                // set return value
                ec = kNoError;
            }
        }
    }

    return(ec);
}


TocErr_t 
TiffSrcFile::ReadMonochrome( CTocMatrix<uint16_t> & bufImg )
{
    int			ec = kErrTiff_PTiff;		    // return number of scans read

    if (mPTiff != NULL)
    {
        // Accept only:
        //	16 bit
        //	1 sample per pixel
        //	Round scaneline out to uint16 - this is the widthRead
        //  Readscanline and advance by widthRead.
        //
        size_t			nLenBuffer = (mWidth * mHeight);

        // CHeck to see that we're a uint16 monochrome image.
        if (IsMonoTiff()) {
            if (mBPP == 16) {
                ec = bufImg.Alloc( mWidth, mHeight );
            }

            if (ec == kNoError )
            {
                // Read in the tiff image
                size_t		nNext = 0;

                for (unsigned nRow = 0; nRow < mHeight; nRow++)
                {
                    //                uint16_t* pScan = (bufImg.begin() + nNext);
                    uint16_t* pScan = (bufImg.data() + nNext);

                    TIFFReadScanline(mPTiff, pScan, nRow);

                    nNext += mWidth;
                }

                // set return value
                ec = kNoError;
            }
        }
    }

    return(ec);
}

