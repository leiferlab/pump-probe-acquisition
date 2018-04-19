// wormUtilities.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <iostream>
#include "Header.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

void segment_singleframe(unsigned __int16 ImgIn[], 
	signed __int32 sizex, signed __int32 sizey, 
	float ArrC[], signed __int32 sizeC, 
	unsigned __int16 ArrA[], float ArrB[], float ArrBX[], float ArrBY[], 
	float ArrBth[], float ArrBdil[], float ArrUno[],
	unsigned __int16 NeuronXY[], unsigned __int16 &NeuronN, 
	float threshold = 0.04, bool resize = true) {

	// Adapt threshold to simplified kernel for the derivative
	threshold = 0.25;

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
		double minX, minY, maxX, maxY, threshX, threshY;

		// Resize image
		cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A, 
			cv::Size(sizex2, sizey2), 0.5, 0.5, cv::INTER_AREA);

		// Apply Gaussian blur
		double blur = 0.65;
		cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);

		// Calculate -d2/dx2 and -d2/dy2 with the filter passed as C.
		// In this way, the CV_32F images will saturate the positive 
		// derivatives and not the negative ones, which we need.
		// Before summing the two derivatives, they need to be divided by 2, 
		// so that they don't saturate. Instead of separately divide them,
		// use 1/2 instead of the 1 as second filter in the separable filter
		// function. (Saves 100 microseconds)
		cv::sepFilter2D(A, BX, CV_32F, C, OneHalf);
		cv::sepFilter2D(A, BY, CV_32F, OneHalf, C);
		B = BX + BY;

		// Threshold out some noise below a fraction of the maximum values:
		// Keep only the points in which **both** derivatives are above
		// threshold separately
		// (remember that BX and BY are -d2/dx2 and -d2/dy2).
		cv::minMaxIdx(BX, &minX, &maxX);
		cv::minMaxIdx(BY, &minY, &maxY);
		threshX = threshold * maxX;
		threshY = threshold * maxY;
		cv::threshold(BX, BX, threshX, 1, CV_THRESH_BINARY);
		cv::threshold(BY, BY, threshY, 1, CV_THRESH_BINARY);
		Bth = BX.mul(BY);
		Bth = Bth.mul(B);

		// Dilate
		cv::dilate(Bth, Bdil, K);

		// Do a 1-dimensional scan over the arrays. Faster than two nested for
		// loops. You will have to calculate the xy coordinates from the single
		// index i.
		float tmpBij;
		for (int i = 0; i < sizex2*sizey2; i++) {
			tmpBij = ArrB[i];
			if (tmpBij != 0.0) {
				if (tmpBij == ArrBdil[i]) {
					NeuronXY[k] = i;
					k++;
				}
			}
		}

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
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout) {

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.
	
	unsigned __int16 x, y;
	signed __int32 sizeBxy = sizeBx * sizeBy;
	float Bxy;
	signed __int32 k = 0;


	for (int i = 0; i < NeuronNin; i++) {
		x = NeuronXYin[2 * i];
		y = NeuronXYin[2 * i + 1];
		Bxy = ArrB2[sizeBxy*x + y];

		bool ok = true;

		// I will hard code the regions where to look.
		// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.
		
		//ArrB0 and ArrB4 (single pixel)
		if (Bxy <= ArrB0[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB4[sizeBxy*x + y]) ok = false;

		//ArrB1 and ArrB3 (cross shaped ROI)
		if (Bxy <= ArrB1[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*(x + 1) + y]) ok = false;

		if (Bxy <= ArrB3[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*(x + 1) + y]) ok = false;

		if (ok == true) {
			NeuronXYout[k] = x;
			NeuronXYout[k + 1] = y;
			k++;
		}
	}
	
	NeuronNout = k;
}

void segment_check2dcandidates_5planes_0(
	float ArrB2[], float ArrB3[], float ArrB4[], 
	signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout) {

	// For the plane at the edge of the volume. Bottom plane.

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.

	unsigned __int16 x, y;
	signed __int32 sizeBxy = sizeBx * sizeBy;
	float Bxy;
	signed __int32 k = 0;


	for (int i = 0; i < NeuronNin; i++) {
		x = NeuronXYin[2 * i];
		y = NeuronXYin[2 * i + 1];
		Bxy = ArrB2[sizeBxy*x + y];

		bool ok = true;

		// I will hard code the regions where to look.
		// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.

		//ArrB0 and ArrB4 (single pixel)
		if (Bxy <= ArrB4[sizeBxy*x + y]) ok = false;

		//ArrB1 and ArrB3 (cross shaped ROI)
		if (Bxy <= ArrB3[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*(x + 1) + y]) ok = false;

		if (ok == true) {
			NeuronXYout[k] = x;
			NeuronXYout[k + 1] = y;
			k++;
		}
	}

	NeuronNout = k;
}

void segment_check2dcandidates_5planes_1(
	float ArrB1[], float ArrB2[],
	float ArrB3[], float ArrB4[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout) {

	// For the plane at distance 1 from the edge of the volume. Bottom.

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.

	unsigned __int16 x, y;
	signed __int32 sizeBxy = sizeBx * sizeBy;
	float Bxy;
	signed __int32 k = 0;


	for (int i = 0; i < NeuronNin; i++) {
		x = NeuronXYin[2 * i];
		y = NeuronXYin[2 * i + 1];
		Bxy = ArrB2[sizeBxy*x + y];

		bool ok = true;

		// I will hard code the regions where to look.
		// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.

		//ArrB4 (single pixel)
		if (Bxy <= ArrB4[sizeBxy*x + y]) ok = false;

		//ArrB1 and ArrB3 (cross shaped ROI)
		if (Bxy <= ArrB1[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*(x + 1) + y]) ok = false;

		if (Bxy <= ArrB3[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*(x + 1) + y]) ok = false;

		if (ok == true) {
			NeuronXYout[k] = x;
			NeuronXYout[k + 1] = y;
			k++;
		}
	}

	NeuronNout = k;
}

void segment_check2dcandidates_5planes_3(
	float ArrB0[], float ArrB1[], float ArrB2[],
	float ArrB3[], signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout) {

	// For the plane at distance 1 from the edge of the volume. Top.

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.

	unsigned __int16 x, y;
	signed __int32 sizeBxy = sizeBx * sizeBy;
	float Bxy;
	signed __int32 k = 0;


	for (int i = 0; i < NeuronNin; i++) {
		x = NeuronXYin[2 * i];
		y = NeuronXYin[2 * i + 1];
		Bxy = ArrB2[sizeBxy*x + y];

		bool ok = true;

		// I will hard code the regions where to look.
		// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.

		//ArrB0 and ArrB4 (single pixel)
		if (Bxy <= ArrB0[sizeBxy*x + y]) ok = false;

		//ArrB1 and ArrB3 (cross shaped ROI)
		if (Bxy <= ArrB1[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*(x + 1) + y]) ok = false;

		if (Bxy <= ArrB3[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB3[sizeBxy*(x + 1) + y]) ok = false;

		if (ok == true) {
			NeuronXYout[k] = x;
			NeuronXYout[k + 1] = y;
			k++;
		}
	}

	NeuronNout = k;
}

void segment_check2dcandidates_5planes_4(
	float ArrB0[], float ArrB1[], float ArrB2[],
	signed __int32 sizeBx, signed __int32 sizeBy,
	unsigned __int16 NeuronXYin[], signed __int32 NeuronNin,
	unsigned __int16 NeuronXYout[], signed __int32 &NeuronNout) {

	// For the plane at the edge of the volume. Top.

	// Brute force check if the candidate neurons found on each plane are
	// actually maxima in -d2/dx2-d2/dy2 also in a sphere around them and not
	// just in their plane.
	// This function can run as soon as the 5 planes are available, so that
	// you don't have to wait until the full volume is ready before starting
	// this brute force check.

	unsigned __int16 x, y;
	signed __int32 sizeBxy = sizeBx * sizeBy;
	float Bxy;
	signed __int32 k = 0;


	for (int i = 0; i < NeuronNin; i++) {
		x = NeuronXYin[2 * i];
		y = NeuronXYin[2 * i + 1];
		Bxy = ArrB2[sizeBxy*x + y];

		bool ok = true;

		// I will hard code the regions where to look.
		// Since B is -d2/dx2-d2/dy2, I have to check if it's the maximum.

		//ArrB0 and ArrB4 (single pixel)
		if (Bxy <= ArrB0[sizeBxy*x + y]) ok = false;

		//ArrB1 and ArrB3 (cross shaped ROI)
		if (Bxy <= ArrB1[sizeBxy*(x - 1) + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y - 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*x + y + 1]) ok = false;
		if (Bxy <= ArrB1[sizeBxy*(x + 1) + y]) ok = false;

		if (ok == true) {
			NeuronXYout[k] = x;
			NeuronXYout[k + 1] = y;
			k++;
		}
	}

	NeuronNout = k;
}