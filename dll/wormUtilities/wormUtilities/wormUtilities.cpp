// wormUtilities.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <iostream>
#include "Header.h"
#include <queue>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <gmmreg_lw_ext.h>
#include <dsmm.hpp>
#define PY_SSIZE_T_CLEAN
#include "Python.h"
#include <arrayobject.h>


void segment_singleframe(unsigned __int16 ImgIn[], 
	signed __int32 sizex, signed __int32 sizey, 
	float ArrC[], signed __int32 sizeC, 
	unsigned __int16 ArrA[], float ArrB[], float ArrBX[], float ArrBY[], 
	float ArrBth[], float ArrBdil[], float ArrUno[],
	unsigned __int16 NeuronXY[], unsigned __int16 &NeuronN, 
	double &maxX, double &maxY, double maxXInStack, double maxYInStack,
	float threshold = 0.25, bool resize = true) {

	unsigned __int16 k = 0;

	if (resize) {
		// Assume that both sizes of the image are even numbers.
		int sizex2 = sizex / 2;
		int sizey2 = sizey / 2;
		
		// OpenCV images, cv::Mat class. For most of them, just create a Mat 
		// header for memory that is already allocated in LabView, so that you 
		// save some time. This makes them also accessible from LabView for 
		// diagonstics, without having to copy a specific image to be 
		// "returned".
		cv::Mat A = cv::Mat(sizex2, sizey2, CV_16U, ArrA);
		cv::Mat B = cv::Mat(sizex2, sizey2, CV_32F, ArrB);
		cv::Mat Bth = cv::Mat(sizex2, sizey2, CV_32F, ArrBth);
		cv::Mat Bdil = cv::Mat(sizex2, sizey2, CV_32F, ArrBdil);
		cv::Mat K = cv::Mat(5, 5, CV_32F, cv::Scalar::all(1));

		// Resize image
		cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A, 
			A.size(), 0, 0, cv::INTER_AREA);

		// Apply Gaussian blur
		// 210 us
		//double blur = 0.65;
		//cv::GaussianBlur(A, A, cv::Size(5, 5), blur, blur);

		// Dilate
		// 60 us

		// 22nd July 2021
		for (int i = 0; i < sizex2*sizey2; i++) {
			ArrB[i] = (float) ArrA[i];
			ArrBth[i] = ArrB[i];
		}
		cv::dilate(B, Bdil, K);

		// Do a 1-dimensional scan over the arrays. Faster than two nested for
		// loops. You will have to calculate the xy coordinates from the single
		// index i.
		// 60 us
		/*THIS WORKS*/
		float tmpBi;
		float tmpBdili;
		uint16_t A_thresh = (uint16_t) threshold;
		int n_A_above_thresh;
		for (int i = 0; i < sizex2*sizey2; i++) {
			tmpBi = ArrB[i];
			tmpBdili = ArrBdil[i];
			n_A_above_thresh = 0; // 22nd July 2021
			if (tmpBi != 0.0 && tmpBi == tmpBdili && ArrA[i]>A_thresh) {
				// The following two ifs added on 22nd July 2021
				if (i>sizex2 && i<sizex2*(sizey2 - 1)) { 
					if (A.at<uint16_t>(i - sizex2)>A_thresh) { n_A_above_thresh += 1; }
					if (A.at<uint16_t>(i - 1)>A_thresh) { n_A_above_thresh += 1; }
					if (A.at<uint16_t>(i)>A_thresh) { n_A_above_thresh += 1; }
					if (A.at<uint16_t>(i + 1)>A_thresh) { n_A_above_thresh += 1; }
					if (A.at<uint16_t>(i + sizex2)>A_thresh) { n_A_above_thresh += 1; }
				}
				if (n_A_above_thresh > 4) {
					NeuronXY[k] = i;
					k++;
				}
				//before  22nd July 2021
				//NeuronXY[k] = i;
				//k++;
			}
			if (k > 200) { break; }
		}
	}
	NeuronN = k;
}

void segment_singleframe_old(unsigned __int16 ImgIn[],
	signed __int32 sizex, signed __int32 sizey,
	float ArrC[], signed __int32 sizeC,
	unsigned __int16 ArrA[], float ArrB[], float ArrBX[], float ArrBY[],
	float ArrBth[], float ArrBdil[], float ArrUno[],
	unsigned __int16 NeuronXY[], unsigned __int16 &NeuronN,
	double &maxX, double &maxY, double maxXInStack, double maxYInStack,
	float threshold = 0.25, bool resize = true) {

	unsigned __int16 k = 0;

	if (resize) {
		// Assume that both sizes of the image are even numbers.
		int sizex2 = sizex / 2;
		int sizey2 = sizey / 2;

		// OpenCV images, cv::Mat class. For most of them, just create a Mat 
		// header for memory that is already allocated in LabView, so that you 
		// save some time. This makes them also accessible from LabView for 
		// diagonstics, without having to copy a specific image to be 
		// "returned".
		cv::Mat A = cv::Mat(sizex2, sizey2, CV_16U, ArrA);
		cv::Mat B = cv::Mat(sizex2, sizey2, CV_32F, ArrB);
		cv::Mat BX = cv::Mat(sizex2, sizey2, CV_32F, ArrBX);
		cv::Mat BY = cv::Mat(sizex2, sizey2, CV_32F, ArrBY);
		cv::Mat Bth = cv::Mat(sizex2, sizey2, CV_32F, ArrBth);
		cv::Mat Bdil = cv::Mat(sizex2, sizey2, CV_32F, ArrBdil);
		cv::Mat Uno = cv::Mat(sizex2, sizey2, CV_32F, ArrUno);
		cv::Mat C = cv::Mat(sizeC, 1, CV_32F, ArrC);
		cv::Mat OneHalf = cv::Mat(1, 1, CV_32F, cv::Scalar::all(0.5));
		//cv::Mat K = cv::Mat(3, 3, CV_32F, cv::Scalar::all(1));
		cv::Mat K = cv::Mat(7, 7, CV_32F, cv::Scalar::all(1));

		// Other variables.
		double minX, minY, threshX, threshY;

		// Resize image
		cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A,
			A.size(), 0, 0, cv::INTER_AREA);

		// Apply Gaussian blur
		// 210 us
		double blur = 0.65;
		//cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);
		cv::GaussianBlur(A, A, cv::Size(5, 5), blur, blur);

		// Calculate -d2/dx2 and -d2/dy2 with the filter passed as C.
		// In this way, the CV_32F images will saturate the positive 
		// derivatives and not the negative ones, which we need.
		// Before summing the two derivatives, they need to be divided by 2, 
		// so that they don't saturate. Instead of separately divide them,
		// use 1/2 instead of the 1 as second filter in the separable filter
		// function. (Saves 100 microseconds)
		// 300 us
		
		cv::sepFilter2D(A, BX, CV_32F, C, OneHalf);
		cv::sepFilter2D(A, BY, CV_32F, OneHalf, C);
		cv::add(BX,BY,B);

		// Threshold out some noise below a fraction of the maximum values:
		// Keep only the points in which **both** derivatives are above
		// threshold separately
		// (remember that BX and BY are -d2/dx2 and -d2/dy2).
		// The maxima used to calculate the threshold are either the one from
		// the current frame, or an estimate of the maximum in the stack
		// calculated from the previous stack (something like the n-th brightest
		// Hessian, so that you don't get too large threshold because of very
		// sharp neurons.
		// 100 us
		//cv::minMaxIdx(BX, &minX, &maxX);
		//cv::minMaxIdx(BY, &minY, &maxY);
		maxX = kthlargest((float*)BX.data, sizex2*sizey2, 10);
		maxY = kthlargest((float*)BY.data, sizex2*sizey2, 10);
		if ( (maxXInStack == -1.0 && maxYInStack == -1.0) || (maxXInStack < maxX || maxYInStack < maxY) ) {
		threshX = threshold * maxX;
		threshY = threshold * maxY;
		}
		else {
		threshX = threshold * maxXInStack;
		threshY = threshold * maxYInStack;
		}
		cv::threshold(BX, BX, threshX, 1, CV_THRESH_BINARY);
		cv::threshold(BY, BY, threshY, 1, CV_THRESH_BINARY);
		Bth = BX.mul(BY);
		Bth = Bth.mul(B);
		
		// Dilate
		// 60 us
		cv::dilate(B, Bdil, K);

		// Do a 1-dimensional scan over the arrays. Faster than two nested for
		// loops. You will have to calculate the xy coordinates from the single
		// index i.
		// 60 us
		/*THIS WORKS*/
		float tmpBi;
		float tmpBdili;
		for (int i = 0; i < sizex2*sizey2; i++) {
			tmpBi = ArrB[i];
			tmpBdili = ArrBdil[i];
			if (tmpBi != 0.0 && tmpBi == tmpBdili) {
				NeuronXY[k] = i;
				k++;
			}
			if (k > 200) { break; }
		}

		// This version removes a bit of the for loop overhead by doing the
		// comparison eight elements at a time. It runs 10 us faster.
		/*float tmpB0, tmpB1, tmpB2, tmpB3, tmpB4, tmpB5, tmpB6, tmpB7;
		float tmpBdil0, tmpBdil1, tmpBdil2, tmpBdil3, tmpBdil4, tmpBdil5, tmpBdil6, tmpBdil7;
		int ii = 0;
		for (int i = 0; i < sizex2*sizey2/8; i++) {
		tmpB0 = ArrB[ii];
		tmpB1 = ArrB[ii+1];
		tmpB2 = ArrB[ii+2];
		tmpB3 = ArrB[ii+3];
		tmpB4 = ArrB[ii+4];
		tmpB5 = ArrB[ii+5];
		tmpB6 = ArrB[ii+6];
		tmpB7 = ArrB[ii+7];
		tmpBdil0 = ArrBdil[ii];
		tmpBdil1 = ArrBdil[ii+1];
		tmpBdil2 = ArrBdil[ii+2];
		tmpBdil3 = ArrBdil[ii+3];
		tmpBdil4 = ArrBdil[ii+4];
		tmpBdil5 = ArrBdil[ii+5];
		tmpBdil6 = ArrBdil[ii+6];
		tmpBdil7 = ArrBdil[ii+7];

		if (tmpB0 != 0.0 && tmpB0 == tmpBdil0) {
		NeuronXY[k] = ii;
		k++;
		}
		if (tmpB1 != 0.0 && tmpB1 == tmpBdil1) {
		NeuronXY[k] = ii+1;
		k++;
		}
		if (tmpB2 != 0.0 && tmpB2 == tmpBdil2) {
		NeuronXY[k] = ii+2;
		k++;
		}
		if (tmpB3 != 0.0 && tmpB3 == tmpBdil3) {
		NeuronXY[k] = ii+3;
		k++;
		}
		if (tmpB4 != 0.0 && tmpB4 == tmpBdil4) {
		NeuronXY[k] = ii+4;
		k++;
		}
		if (tmpB5 != 0.0 && tmpB5 == tmpBdil5) {
		NeuronXY[k] = ii+5;
		k++;
		}
		if (tmpB6 != 0.0 && tmpB6 == tmpBdil6) {
		NeuronXY[k] = ii+6;
		k++;
		}
		if (tmpB7 != 0.0 && tmpB7 == tmpBdil7) {
		NeuronXY[k] = ii+7;
		k++;
		}

		ii += 8;
		}*/

		/**float tmpBij;
		// Find local maxima of -d2/dx2-d2/dy2
		for (unsigned __int16 i = 0; i < sizex2; i++) {
		for (unsigned __int16 j = 0; j < sizey2; j++) {
		tmpBij = B.at<float>(i, j);
		if (tmpBij != 0.0 && tmpBij == Bdil.at<float>(i, j)) {
		NeuronXY[k] = i;
		NeuronXY[k + 1] = j;
		k++;
		}
		}
		}**/
	}
	NeuronN = k;
}

float kthlargest(float array[], int size, int k) {
	// restricting it to the central third of the image, to save time
	std::priority_queue<std::pair<float, int>> q;
	int ii = size / 3;
	for (int i = 0; i < size / 3; ++i) {
		q.push(std::pair<float, int>(array[ii], i));
		ii++;
	}
	int ki;
	for (int i = 0; i < k; ++i) {
		ki = q.top().second;
		q.pop();
	}
	return array[ki+ size / 3];
}

void segment_check2dcandidates_5planes(
	float ArrB0[], float ArrB1[], float ArrB2[],
	float ArrB3[], float ArrB4[], signed __int32 sizeBx, signed __int32 sizeBy, 
	unsigned __int16 NeuronXYin[], unsigned __int16 NeuronNin,
	unsigned __int16 NeuronXYout[], unsigned __int16 &NeuronNout) {

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.
	
	unsigned __int16 x, y;
	float Bxy;
	unsigned __int16 k = 0;
	unsigned __int16 index;


	for (int i = 0; i < NeuronNin; i++) {
		index = NeuronXYin[i];
		x = index / sizeBx;
		y = index % sizeBx;
		if ((x > 75) && (x < 175) && (y > 75) && (y < 175)) { 

			Bxy = ArrB2[index];

			bool ok = true;
			// For stabilization
			float dx = 0.0;
			float dy = 0.0;
			float sum = 0.0;
			signed __int16 di = 0;

			// I will hard code the regions where to look.
			// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.

			///////////////////////////////
			///// - do sizeBxy*x just once and save in tmp variable
			///// - maybe do a for loop over the y-1,y,y+1, so that the compiler
			/////	can vectorize at least that.
			///////////////////////////////

			/**
			//ArrB0 and ArrB4 (single pixel)
			if (Bxy <= ArrB0[sizeBx*x + y]) ok = false;
			if (Bxy <= ArrB4[sizeBx*x + y]) ok = false;

			//ArrB1 and ArrB3 (cross shaped ROI)
			if (Bxy <= ArrB1[sizeBx*(x - 1) + y]) ok = false;
			if (Bxy <= ArrB1[sizeBx*x + y - 1]) ok = false;
			if (Bxy <= ArrB1[sizeBx*x + y]) ok = false;
			if (Bxy <= ArrB1[sizeBx*x + y + 1]) ok = false;
			if (Bxy <= ArrB1[sizeBx*(x + 1) + y]) ok = false;

			if (Bxy <= ArrB3[sizeBx*(x - 1) + y]) ok = false;
			if (Bxy <= ArrB3[sizeBx*x + y - 1]) ok = false;
			if (Bxy <= ArrB3[sizeBx*x + y]) ok = false;
			if (Bxy <= ArrB3[sizeBx*x + y + 1]) ok = false;
			if (Bxy <= ArrB3[sizeBx*(x + 1) + y]) ok = false;
			**/

			//ArrB0 and ArrB4 (single pixel)
			if (Bxy < ArrB0[index]) ok = false;
			if (Bxy < ArrB4[index]) ok = false;

			//ArrB1 and ArrB3 (cross shaped ROI)
			if (Bxy < ArrB1[index - sizeBx]) ok = false;
			if (Bxy < ArrB1[index - 1]) ok = false;
			if (Bxy < ArrB1[index]) ok = false;
			if (Bxy < ArrB1[index + 1]) ok = false;
			if (Bxy < ArrB1[index + sizeBx]) ok = false;

			if (Bxy < ArrB3[index - sizeBx]) ok = false;
			if (Bxy < ArrB3[index - 1]) ok = false;
			if (Bxy < ArrB3[index]) ok = false;
			if (Bxy < ArrB3[index + 1]) ok = false;
			if (Bxy < ArrB3[index + sizeBx]) ok = false;

			if (ok == true) {
				// Compute stabilization
				//dx = (-2.0*ArrB2[index - 2] - 1.0*ArrB2[index - 1] + ArrB2[index + 1] + 2.0*ArrB2[index + 2]) / (ArrB2[index - 2] + ArrB2[index - 1] + ArrB2[index] + ArrB2[index + 1] + ArrB2[index + 2]);
				//dy = (-2.0*ArrB2[index - 2 * sizeBx] - 1.0*ArrB2[index - sizeBx] + ArrB2[index + sizeBx] + 2.0*ArrB2[index + 2 * sizeBx]) / (ArrB2[index - 2 * sizeBx] + ArrB2[index - sizeBx] + ArrB2[index] + ArrB2[index + sizeBx] + ArrB2[index + 2 * sizeBx]);
				dx = 0.0;
				for (int m = 0; m < 5; ++m) {
					//per ogni riga
					for (int l = 0; l < 5; ++l) {
						//per ogni colonna
						dx = dx + (l - 2)*ArrB2[index + (l - 2) + sizeBx * (m - 2)]; 
						dy = dy + (m - 2)*ArrB2[index + (l - 2) + sizeBx * (m - 2)];
						sum = sum + ArrB2[index + (l - 2) + sizeBx * (m - 2)];
					}
				}
				dx = dx / sum;
				dy = dy / sum;
				di = int(dx) + sizeBx * int(dy);
				NeuronXYout[k] = index + di;
				k++;
			}
		}
	}
	
	NeuronNout = k;
}

void find_peak(unsigned __int16 ImgIn[], 
	signed __int32 sizex, signed __int32 sizey, 
	double blur, signed __int32 &maxLocX, signed __int32 &maxLocY) {
	
	int sizex2 = sizex / 2;
	int sizey2 = sizey / 2;
	cv::Point minloc, maxloc;
	double min, max;

	// Create intermediate image
	cv::Mat A = cv::Mat(sizex2, sizey2, CV_16U, cv::Scalar::all(0));

	// Resize image
	cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A,
		A.size(), 0, 0, cv::INTER_AREA);
	
	// Apply Gaussian blur
	cv::GaussianBlur(A, A, cv::Size(5, 5), blur, blur);
	
	// Find maximum position
	cv::minMaxLoc(A, &min, &max, &minloc, &maxloc);
	maxLocX = 2*maxloc.x;
	maxLocY = 2*maxloc.y;
}

void gmmreg_transformation(int dim, double param0[], 
					int nctrlpts, double coordCtrlPts[],
					int npts, double coord[],
					int nout, double out[],
					double normParams[], double denormParams[])
	{

	gmmreg_lw::transformation(dim, param0,
		 nctrlpts, coordCtrlPts,
		 npts, coord,
		 nout, out,
		 normParams, denormParams);
}

void gmmreg_registration(int dim, double param0[], double lambda[],
	int nscale, double scale[],
	int nctrlpts, double coordCtrlPts[],
	int npts, double coord[],
	int nptsScene, double coordScene[],
	int nOut, double out[],
	double normParams[], double denormParams[]) {
	
	gmmreg_lw::registration(dim, param0, lambda,
		nscale, scale,
		nctrlpts, coordCtrlPts,
		npts, coord,
		nptsScene, coordScene,
		nOut, out,
		normParams, denormParams);
}

void dsmm_registration(double *X, double *Y, int M, int N, int D,
	double beta, double lambda, double neighbor_cutoff,
	double alpha, double gamma0, 
	double conv_epsilon, double eq_tol,
	double *pwise_dist, double *pwise_distYY,
	double *Gamma, double *CDE_term,
	double *w, double *F_t, double *wF_t, double *wF_t_sum,
	double *p, double *u, int *Match,
	double *hatP, double *hatPI_diag, double *hatPIG, double *hatPX, double *hatPIY,
	double *G, double *W, double *GW,
	double *sumPoverN, double *expAlphaSumPoverN) {

	dsmm::_dsmm(X,Y,M,N,D,
		beta,lambda,neighbor_cutoff,
		alpha,gamma0,
		conv_epsilon,eq_tol,
		pwise_dist,pwise_distYY,
		Gamma,CDE_term,
		w,F_t,wF_t,wF_t_sum,
		p,u,Match,
		hatP,hatPI_diag,hatPIG,hatPX,hatPIY,
		G,W,GW,
		sumPoverN,expAlphaSumPoverN);
}


void displayImg(uint16_t* ArrA, int sizex, int sizey, int refresh_time, float* stop) {
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::Mat A = cv::Mat(sizex, sizey, CV_16U, ArrA);
	
	while (true) {
		cv::imshow("Display window", A);
		cv::waitKey(refresh_time);
		if (stop[0] > 0) { break; }
	}

	cv::destroyAllWindows();
}

uint16_t* stealPointer_uint16(uint16_t* A) {
	return A;
}

float* stealPointer_float(float* A) {
	return A;
}

uint16_t* allocateArray_uint16(int size) {
	uint16_t* ptr = new uint16_t[size];
	for (int i = 0; i < size; ++i) { ptr[i] = 0; }
	return ptr;
}

void deallocateArray_uint16(uint16_t* ptr) {
	delete[] ptr;
}

void writeInArray_uint16(uint16_t* src, uint16_t* dst, int size) {
	for(int i=0;i<size;i++){
		dst[i] = src[i];
	}
}

double displayImg_py(uint16_t* ArrA, int sizex, int sizey, int refresh_time, float* stop) {
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	// Do init stuff for Python and Numpy
	Py_Initialize();
	import_array();

	// Get the Py_ version of the name of the module
	pName = PyUnicode_DecodeFSDefault("labview");

	// Import the module
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		// Get the reference to the function
		pFunc = PyObject_GetAttrString(pModule, "display");

		// Check that you retrieved pFunc correctly and that it is callable.
		// All these checks can go when you've done the tests.
		if (pFunc && PyCallable_Check(pFunc)) {
			pArgs = PyTuple_New(3);

			// Create the numpy-array-wrapper to the array
			npy_intp shape[2] = { sizex, sizey };			
			PyObject* A_o = PyArray_SimpleNewFromData(2, shape, NPY_UINT16, ArrA);
			if (A_o == NULL) {
				Py_XDECREF(A_o); Py_DECREF(pArgs); Py_DECREF(pFunc); Py_DECREF(pModule);
				return -1.0;
				};

			PyObject* refresh_time_o = PyLong_FromLong(refresh_time);

			npy_intp stop_shape[1] = { 1 };
			PyObject* stop_o = PyArray_SimpleNewFromData(1, stop_shape, NPY_FLOAT32, stop);

			// Add it to the tuple of the arguments to be passed to the function
			PyTuple_SetItem(pArgs, 0, A_o);
			PyTuple_SetItem(pArgs, 1, refresh_time_o);
			PyTuple_SetItem(pArgs, 2, stop_o);

			// Call the function
			pValue = PyObject_CallObject(pFunc, pArgs);
			
			Py_DECREF(pArgs);
			
			if (pValue != NULL) {
				return PyLong_AsLong(pValue);
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				return -1.0;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
		}
		
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", "a");
		return 6.0;
	}
	if (Py_FinalizeEx() < 0) {
		return 120;
	}
	return 0;
}

double hyperstack(uint16_t* ArrA, int shape0, int shape1, int shape2, int shape3, float* stop, float* out, int out_n, double refresh_time) {
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;

	// Do init stuff for Python and Numpy
	Py_Initialize();
	import_array();

	// Get the Py_ version of the name of the module
	pName = PyUnicode_DecodeFSDefault("mistofrutta.plt");

	// Import the module
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		// Get the reference to the function
		pFunc = PyObject_GetAttrString(pModule, "hyperstack_live_min_interface");

		// Check that you retrieved pFunc correctly and that it is callable.
		// All these checks can go when you've done the tests.
		if (pFunc && PyCallable_Check(pFunc)) {
			pArgs = PyTuple_New(4);

			// Create the numpy-array-wrapper to the array
			npy_intp shape[4] = { shape0, shape1, shape2, shape3 };
			PyObject* A_o = PyArray_SimpleNewFromData(4, shape, NPY_UINT16, ArrA);
			if (A_o == NULL) {
				Py_XDECREF(A_o); Py_DECREF(pArgs); Py_DECREF(pFunc); Py_DECREF(pModule);
				return -1.0;
			};

			npy_intp out_shape[1] = { out_n };
			PyObject* out_o = PyArray_SimpleNewFromData(1, out_shape, NPY_FLOAT32, out);

			npy_intp stop_shape[1] = { 2 };
			PyObject* stop_o = PyArray_SimpleNewFromData(1, stop_shape, NPY_FLOAT32, stop);

			PyObject* refresh_time_o = PyFloat_FromDouble(refresh_time);

			// Add it to the tuple of the arguments to be passed to the function
			PyTuple_SetItem(pArgs, 0, A_o);
			PyTuple_SetItem(pArgs, 1, stop_o);
			PyTuple_SetItem(pArgs, 2, out_o);
			PyTuple_SetItem(pArgs, 3, refresh_time_o);
			

			// Call the function
			pValue = PyObject_CallObject(pFunc, pArgs);

			Py_DECREF(pArgs); 

			if (pValue != NULL) {
				return PyFloat_AsDouble(pValue);
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				return -2.5;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
		}

		Py_XDECREF(pFunc);
		Py_DECREF(pModule); 
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", "a");
		return -3.0;
	}
	if (Py_FinalizeEx() < 0) {
		return 120;
	}
	return 89.0;
}

double python_emb() {
	PyObject *pName, *pModule, *pFunc;
	PyObject *pArgs, *pValue;
	int i;

	Py_Initialize();
	pName = PyUnicode_DecodeFSDefault("labview");
	/* Error checking of pName left out */

	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule != NULL) {
		pFunc = PyObject_GetAttrString(pModule, "multiply");
		/* pFunc is a new reference */

		if (pFunc && PyCallable_Check(pFunc)) {
			pArgs = PyTuple_New(2);
			for (i = 0; i < 2; ++i) {
				pValue = PyLong_FromLong(3);
				if (!pValue) {
					Py_DECREF(pArgs);
					Py_DECREF(pModule);
					fprintf(stderr, "Cannot convert argument\n");
					return 1;
				}
				/* pValue reference stolen here: */
				PyTuple_SetItem(pArgs, i, pValue);
			}
			pValue = PyObject_CallObject(pFunc, pArgs);
			Py_DECREF(pArgs);
			if (pValue != NULL) {
				printf("Result of call: %ld\n", PyLong_AsLong(pValue));
				return PyLong_AsLong(pValue);
				Py_DECREF(pValue);
			}
			else {
				Py_DECREF(pFunc);
				Py_DECREF(pModule);
				PyErr_Print();
				fprintf(stderr, "Call failed\n");
				return 5.0;
			}
		}
		else {
			if (PyErr_Occurred())
				PyErr_Print();
			fprintf(stderr, "Cannot find function \"%s\"\n", "funzione");
		}
		Py_XDECREF(pFunc);
		Py_DECREF(pModule);
	}
	else {
		PyErr_Print();
		fprintf(stderr, "Failed to load \"%s\"\n", "a");
		return 6.0;
	}
	if (Py_FinalizeEx() < 0) {
		return 120;
	}
	return 0;
}