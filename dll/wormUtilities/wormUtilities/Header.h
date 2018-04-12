// wormUtilites.h - Contains declarations of the utilites for the pump-probe instrument
#pragma once

#ifdef WORMUTILITIES_EXPORTS
#define WORMUTILITIES_API __declspec(dllexport)
#else
#define WORMUTILITIES_API __declspec(dllimport)
#endif

#include <opencv2/core.hpp>

extern "C" WORMUTILITIES_API unsigned __int16 sumarray(unsigned __int16 a[], signed __int32 size, unsigned __int16 *sum);

extern "C" WORMUTILITIES_API void segment_singleframe(unsigned __int16 ImgIn[],
	signed __int32 sizex, signed __int32 sizey, float C[], 
	signed __int32 sizeC, unsigned __int16 ArrA[], float ArrB[], float ArrBX[],
	float ArrBY[], float ArrBth[], float ArrBdil[], float ArrUno[],
	float threshold, bool resize);