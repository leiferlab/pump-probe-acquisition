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
	unsigned __int16 &NeuronN,
	double &maxX, double &maxY, double maxXInStack, double maxYInStack,
	float threshold, bool resize);

float kthlargest(float array[], int size, int k);

extern "C" WORMUTILITIES_API void segment_check2dcandidates_5planes(
	float ArrB0[], float ArrB1[], float ArrB2[],
	float ArrB3[], float ArrB4[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], unsigned __int16 NeuronNin,
	unsigned __int16 NeuronXYout[], unsigned __int16 &NeuronNout);             

extern "C" WORMUTILITIES_API void find_peak(unsigned __int16 ImgIn[],
	signed __int32 sizex, signed __int32 sizey, 
	double blur, signed __int32 &maxLocX, signed __int32 &maxLocY);

extern "C" WORMUTILITIES_API void gmmreg_transformation(
	int dim, double param0[],
	int nctrlpts, double coordCtrlPts[],
	int npts, double coord[],
	int nout, double out[],
	double normParams[], double denormParams[]);

extern "C" WORMUTILITIES_API void gmmreg_registration(
	int dim, double param0[], double lambda[],
	int nscale, double scale[],
	int nctrlpts, double coordCtrlPts[],
	int npts, double coord[],
	int nptsScene, double coordScene[],
	int nOut, double out[],
	double normParams[], double denormParams[]);

extern "C" WORMUTILITIES_API void dsmm_registration(double *X, double *Y, int M, int N, int D,
	double beta, double lambda, double neighbor_cutoff,
	double alpha, double gamma0,
	double conv_epsilon, double eq_tol,
	double *pwise_dist, double *pwise_distYY,
	double *Gamma, double *CDE_term,
	double *w, double *F_t, double *wF_t, double *wF_t_sum,
	double *p, double *u, int *Match,
	double *hatP, double *hatPI_diag, double *hatPIG, double *hatPX, double *hatPIY,
	double *G, double *W, double *GW,
	double *sumPoverN, double *expAlphaSumPoverN);

extern "C" WORMUTILITIES_API void displayImg(uint16_t* ArrA, int sizex, int sizey, int refresh_time, float* stop);

extern "C" WORMUTILITIES_API uint16_t* stealPointer_uint16(uint16_t* A);
extern "C" WORMUTILITIES_API float* stealPointer_float(float* A);
extern "C" WORMUTILITIES_API float* stealPointer_float(float* A);
extern "C" WORMUTILITIES_API uint16_t* allocateArray_uint16(int size);
extern "C" WORMUTILITIES_API void deallocateArray_uint16(uint16_t* ptr);
extern "C" WORMUTILITIES_API void writeInArray_uint16(uint16_t* src, uint16_t* dst, int size);

extern "C" WORMUTILITIES_API double displayImg_py(uint16_t* ArrA, int sizex, int sizey, int refresh_time, float* stop);
extern "C" WORMUTILITIES_API double hyperstack(uint16_t* ArrA, int shape0, int shape1, int shape2, int shape3, float* stop, float* out, int out_n, double refresh_time);
extern "C" WORMUTILITIES_API double python_emb();