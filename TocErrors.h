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

*/
#ifndef __TOCERRORS_H__
#define __TOCERRORS_H__		1


typedef int		TocErr_t;

#define kNoError		(0)

// -1 is used to set the "sign" bit so all error codes are negative.
#define ERRSIGN_ON                  ( static_cast<int>(-1 ) * (0x10000000) )
#define	ERRNUM( errMod, errNum)		( (ERRSIGN_ON) + (errMod ) + (errNum))

// List error code modules
//  These values are "errMod" in ERRNUM.
#define ERRMOD_SYS      (0x1000)            // basis system (memory ect)

#define	ERRMOD_TIFF		(0x4000)            // Tiff
#define	ERRMOD_HDF5		(0x5000)            // Hdf5FileSrc
#define	ERRMOD_PROF		(0x6000)            // SCProfFile



// Error Codes for some basic system problems
// Memory allocation errors:
#define	kErrSys_Alloc		    ERRNUM( ERRMOD_SYS, 0x01 )      // allocation error
#define	kErrSys_Free		    ERRNUM( ERRMOD_SYS, 0x02 )      // cannot free bogus ptr
#define	kErrSys_BadArg		    ERRNUM( ERRMOD_SYS, 0x03 )      // illegal argument



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

