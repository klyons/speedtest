

#include "TiffSrcFile.h"

#include <sstream>

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
        TIFFGetField(mPTiff, TIFFTAG_XRESOLUTION, &mResolutionX);
        TIFFGetField(mPTiff, TIFFTAG_YRESOLUTION, &mResolutionY);
    }
    else {
        nReturn = -1;
    }

    return(nReturn);
}

TocErr_t
TiffSrcFile::CloseFile()
{
    if (mPTiff != NULL) {
        TIFFClose(mPTiff);
        mPTiff = NULL;

        ClearFile();
    }

    return(kNoError);
}

void
TiffSrcFile::ClearFile()
{
    mPTiff = NULL;
    mWidth = 0;
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
    std::unique_ptr<uint16_t[]>			bufImg;
    int			nEc = ReadMonochrome16( bufImg );
    CloseFile();
 *
 *
 * @param  bufImg = ref to unique_ptr for uint16 values.
 * @return Error Code
 */

int TiffSrcFile::ReadMonochrome(std::unique_ptr<uint16_t[]>& bufImg)
{
    int			nEc = -1;		// return number of scans read

    // Accept only:
    //	16 bit
    //	1 sample per pixel
    //	Round scaneline out to uint16 - this is the widthRead
    //  Readscanline and advance by widthRead.
    //
    tmsize_t		nScanLen = TIFFScanlineSize(mPTiff);
    size_t			nScan16Round = (nScanLen + sizeof(uint16_t) - 1) / sizeof(uint16_t);
    size_t			nLenBuffer = (nScan16Round * mHeight);

    // CHeck to see that we're a uint16 monochrome image.
    if (IsMonoTiff()) {
        if (mBPP == 16) {
            bufImg = std::unique_ptr<uint16_t[]>(new uint16_t[nLenBuffer]);
        }

        // Read in the tiff image
        size_t		nNext = 0;

        for (unsigned nRow = 0; nRow < mHeight; nRow++)
        {
            uint16_t* pScan = (bufImg.get() + nNext);

            TIFFReadScanline(mPTiff, pScan, nRow);

            nNext += nScan16Round;
        }

        // set return value
        nEc = 0;
    }

    return(nEc);
}

int TiffSrcFile::TestIt()
{
    int			nReturn = 0;
    const char* pFilename = "C:\\TOC_Data\\ImageCaptures\\NikonZ8_Captures\\ChromaBathCalFiles\\NikonZ8Lev0\\DSC_1605.TIFF";

    nReturn = OpenFile(pFilename);

#if 0
    uint32_t	config = 0;
    uint16_t	nsamples = 0;
    TIFFGetField(mPTiff, TIFFTAG_PLANARCONFIG, &config);
    TIFFGetField(mPTiff, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
#endif // 0

    std::unique_ptr<uint16_t[]>			bufImg;
    int			nEc = ReadMonochrome(bufImg);

    CloseFile();

    return(nReturn);
}

