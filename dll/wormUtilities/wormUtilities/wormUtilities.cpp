// wormUtilities.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <iostream>
#include "Header.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <gmmreg_lw.h>

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
		cv::Mat BX = cv::Mat(sizex2, sizey2, CV_32F, ArrBX);
		cv::Mat BY = cv::Mat(sizex2, sizey2, CV_32F, ArrBY);
		cv::Mat Bth = cv::Mat(sizex2, sizey2, CV_32F, ArrBth);
		cv::Mat Bdil = cv::Mat(sizex2, sizey2, CV_32F, ArrBdil);
		cv::Mat Uno = cv::Mat(sizex2, sizey2, CV_32F, ArrUno);
		cv::Mat C = cv::Mat(sizeC, 1, CV_32F, ArrC);
		cv::Mat OneHalf = cv::Mat(1, 1, CV_32F, cv::Scalar::all(0.5));
		cv::Mat K = cv::Mat(3, 3, CV_32F, cv::Scalar::all(1));
		
		// Other variables.
		double minX, minY, threshX, threshY;

		// Resize image
		cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A, 
			A.size(), 0, 0, cv::INTER_AREA);

		// Apply Gaussian blur
		// 210 us
		double blur = 0.65;
		cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);

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
		// shar
		// 100 us
		cv::minMaxIdx(BX, &minX, &maxX);
		cv::minMaxIdx(BY, &minY, &maxY);
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
		cv::dilate(Bth, Bdil, K);

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
		Bxy = ArrB2[index];

		bool ok = true;

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
			NeuronXYout[k] = index;
			k++;
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
	cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);
	
	// Find maximum position
	cv::minMaxLoc(A, &min, &max, &minloc, &maxloc);
	maxLocX = 2*maxloc.x;
	maxLocY = 2*maxloc.y;
}

void gmmreg_transform(__int32 dim, double param0[], double scale, 
					__int32 nctrlpts, double coordCtrlPts[],
					__int32 npts, double coord[],
					__int32 nout, double out[]) 
	{

	// Initialize both transformation and pointset objects
	gmmreg_lw::regTransformation transformation(dim, nctrlpts, coordCtrlPts, param0);
	gmmreg_lw::pointset points(dim, nctrlpts, npts, coord);
	transformation.SetScale(scale);
	transformation.SetLambda(0.0);

	// Prepare the basis and the kernel for the transformation (the kernel is
	// not really needed here, but I still have to split the functions).
	transformation.PrepareBasisKernel(points.pts, points.basis);

	// Transform the points
	transformation.PerformTransform(points.trpts, points.basis);

	points.getTrpts(out);
}