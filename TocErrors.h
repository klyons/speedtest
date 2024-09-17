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

---------------------------------------------------------- -
Description:
    Error numbers

    Use macro "ERRNUM" to define errors for each module.
    We do not use assertions in this library.
*/
#ifndef __TOCERRORS_H__
#define __TOCERRORS_H__		1

// Define CPLUSPLUS_VERS to be 1998, 2011, 2014, 2017 or 2020.
#if (__cplusplus >= 201703 )
// We only use 2017 features
#define CPLUSPLUS_VERS      2017
#else

// Visual studio does not set __cplusplus correctly,
//  So we cannot use it to define C++17.
//  That is the reason we include our own equate for this.
//  If running in visual studio we assume C++17.
#if defined( _MSC_VER  )
#define CPLUSPLUS_VERS      2017
#else
#define CPLUSPLUS_VERS      1998
#endif // defined( _MSC_VER  )

#endif // (__cplusplus >= 201703 )


//
// ERROR CODES
//
typedef int		TocErr_t;

#define kNoError		(0)

// -1 is used to set the "sign" bit so all error codes are negative.
#define ERRSIGN_ON                  ( static_cast<int>(-1 ) * (0x10000000) )
#define ERR_MOD_MASK                (0x3FFF0000)
#define ERR_NUM_MASK                (0x0000FFFF)                        // mask off the error number
#define ERR_MASKS                   ( ERR_MOD_MASK | ERR_NUM_MASK )     // both masks
#define	ERRNUM( errMod, errNum)		( (ERRSIGN_ON & ~ERR_MASKS ) + (errMod & ERR_MOD_MASK ) + (errNum & ERR_NUM_MASK))

#define ERR_NUM( errTotal )         ( ERR_NUM_MASK & (errTotal ))
#define ERR_MOD( errTotal )         ( ERR_MOD_MASK & (errTotal) )

// List error code modules
//  These values are "errMod" in ERRNUM.

#define ERRMOD_SYS          (0x0010000)     // basis system (memory ect)
#define ERRMOD_FILES        (0x0020000)     // basis system (memory ect)


// MOdules 
#define	ERRMOD_TIFF		    (0x0140000)     // Tiff
#define	ERRMOD_HDF5		    (0x0150000)     // Hdf5FileSrc
#define	ERRMOD_PROF		    (0x0160000)     // SCProfFile file I/O

// ShadowChrome applications:
#define ERRMOD_SCAPP        (0x0200000)     // Test app for ShadowChrome App
#define ERRMOD_SCPROFILE    (0x0210000)     // Test app for ShadowChrome Profile
#define ERRMOD_SCTEST       (0x0220000)     // Test app for ShadowChrome Test App


// Error Codes for some basic system problems
// Memory allocation errors:
#define	kErrSys_Alloc		    ERRNUM( ERRMOD_SYS, 0x01 )      // allocation error
#define	kErrSys_Free		    ERRNUM( ERRMOD_SYS, 0x02 )      // cannot free bogus ptr
#define kErrSys_BadPtr          ERRNUM( ERRMOD_SYS, 0x03 )      // bad ptr 
#define	kErrSys_BadArg		    ERRNUM( ERRMOD_SYS, 0x04 )      // illegal argument
#define	kErrSys_MissArg	        ERRNUM( ERRMOD_SYS, 0x05 )      // missing arg

#define	kErrSys_FileOpen	    ERRNUM( ERRMOD_FILES, 0x01 )    // can't open file
#define	kErrSys_FileClose	    ERRNUM( ERRMOD_FILES, 0x02 )    // can't close file
#define	kErrSys_FileEOF	        ERRNUM( ERRMOD_FILES, 0x03 )    // reached the end-of-file
#define	kErrSys_NoFile	        ERRNUM( ERRMOD_FILES, 0x04 )    // name doesn't correspond to a file


/* Redefine max/min
*/
template<class T>
const T& TMax(const T& a, const T& b)
{
    return (a < b) ? b : a;
}

template<class T>
const T& TMin(const T& a, const T& b)
{
    return (a > b) ? b : a;
}

#endif // __TOCERRORS_H__

