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

*/
#ifndef __TIFFSRCFILE_H__
#define __TIFFSRCFILE_H__   1


#include <stdint.h>
#include <memory>

// Use libtiff - include header here.
#include "tiff.h"
#include <tiffio.h>

#include "TocErrors.h"

// Error Codes
#define	kErrTiff_Open	ERRNUM( ERRMOD_TIFF, 0x01 )
#define	kErrTiff_Close	ERRNUM( ERRMOD_TIFF, 0x02 )


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
    TIFF* mPTiff;           // ptr to libTiff file object
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
    TocErr_t NewFile();

    TocErr_t CloseFile();
    void ClearFile();

    bool IsMonoTiff() const {
        bool		isMono = false;
        if (mPTiff != NULL) {
            // Make sure we're monochrome
            if (mSampPerPixel == 1) {
                isMono = true;
            }
        }
        return(isMono);
    }

    // Read in a monochrome image - 
    TocErr_t ReadMonochrome(std::unique_ptr<uint16_t[]>& bufImg);


    // Test routine:
    TocErr_t TestIt();

    uint32_t getWidth() const {
        return mWidth;
    }

    uint32_t getHeight() const {
        return mHeight;
    }

};


#endif // __TIFFSRCFILE_H__
