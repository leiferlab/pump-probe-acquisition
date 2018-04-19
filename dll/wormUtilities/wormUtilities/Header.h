// wormUtilites.h - Contains declarations of the utilites for the pump-probe instrument
#pragma once

#ifdef WORMUTILITIES_EXPORTS
#define WORMUTILITIES_API __declspec(dllexport)
#else
#define WORMUTILITIES_API __declspec(dllimport)
#endif

extern "C" WORMUTILITIES_API void segment_singleframe(
	unsigned __int16 ImgIn[], signed __int32 sizex, signed __int32 sizey, 
	float C[], signed __int32 sizeC, unsigned __int16 ArrA[], float ArrB[], 
	float ArrBX[], float ArrBY[], float ArrBth[], float ArrBdil[], 
	float ArrUno[], unsigned __int16 NeuronXY[], 
	unsigned __int16 &NeuronN, float threshold, bool resize);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes(
	float ArrB0[], float ArrB1[], float ArrB2[],
	float ArrB3[], float ArrB4[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes_0(
	float ArrB2[], float ArrB3[], float ArrB4[],
	signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes_1(
	float ArrB1[], float ArrB2[],
	float ArrB3[], float ArrB4[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes_3(
	float ArrB0[], float ArrB1[], float ArrB2[],
	float ArrB3[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes_4(
	float ArrB0[], float ArrB1[], float ArrB2[],
	signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout);