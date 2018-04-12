// wormUtilities.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include <iostream>
#include <vector>
#include "Header.h"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

void segment_singleframe(unsigned __int16 ImgIn[], signed __int32 sizex, 
	signed __int32 sizey, float ArrC[], signed __int32 sizeC, 
	unsigned __int16 ArrA[], float ArrB[], float ArrBX[], float ArrBY[], 
	float ArrBth[], float ArrBdil[], float ArrUno[], float threshold = 0.04, 
	bool resize = true) {

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
		cv::Mat Identity = cv::Mat(1, 1, CV_32F, cv::Scalar::all(1));
		cv::Mat K = cv::Mat(3, 3, CV_32F, cv::Scalar::all(1));
		
		// Other variables.
		std::vector<int> NeuronX, NeuronY;
		double minX, minY, maxX, maxY, threshX, threshY;

		// Resize image
		cv::resize(cv::Mat(sizex, sizey, CV_16U, ImgIn), A, cv::Size(), 
			0.5, 0.5, cv::INTER_AREA);

		// Apply Gaussian blur
		double blur = 0.65;
		cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);

		// Calculate -d2/dx2 and -d2/dy2 with the filter passed as C.
		// In this way, the CV_32F images will saturate the positive 
		// derivatives and not the negative ones, which we need.
		// Before summing the two derivatives, divide them by 2, so that they 
		// don't saturate.
		cv::sepFilter2D(A, BX, CV_32F, C, Identity);
		cv::sepFilter2D(A, BY, CV_32F, Identity, C);
		cv::divide(BX, 2, BX);
		cv::divide(BY, 2, BY);
		B = BX + BY;

		// Threshold out some noise below a fraction of the maximum values:
		// Keep only the points in which **both** derivatives are above
		// threshold (remember that BX and BY are -d2/dx2 and -d2/dy2).
		cv::minMaxLoc(BX, &minX, &maxX);
		cv::minMaxLoc(BY, &minY, &maxY);
		threshX = threshold * maxX;
		threshY = threshold * maxY;
		cv::threshold(BX, BX, threshX, 1, CV_THRESH_BINARY);
		cv::threshold(BY, BY, threshY, 1, CV_THRESH_BINARY);
		Bth = BX.mul(BY);
		Bth = Bth.mul(B);

		// Dilate
		cv::dilate(Bth, Bdil, K);

		// Find local minima
		int k = 0;
		for (int i = 0; i < sizex2; i++) {
			for (int j = 0; j < sizey2; j++) {
				if (B.at<float>(i, j) == -Bdil.at<float>(i, j)) {
					//NeuronX[k] = i;
					//NeuronY[k] = j;
					k++;
				}
			}
		}
	}
}

void segment_singleframe_old(unsigned __int16 Araw[], signed __int32 sizeAx, signed __int32 sizeAy, float C[], signed __int32 sizeC,
	float Aresized[], float threshold = 0.04, bool resize = true) {
	
	if (resize) {
		// Assume that both sizes of the image are even numbers.
		int sizeAx2 = sizeAx / 2;
		int sizeAy2 = sizeAy / 2;
		// Per tutte ste matrici (tranne che per B, che devi conservare fino ad avere tutto il volume) forse dovresti tenere allocata la memoria tramite uno shift register in LV
		// https://docs.opencv.org/3.4/d3/d63/classcv_1_1Mat.html Make a header for user-allocated data.
		cv::Mat A = cv::Mat(sizeAx2, sizeAy2, CV_16U, cv::Scalar::all(0));
		cv::Mat ImgOut = cv::Mat(sizeAx2, sizeAy2, CV_32F, Aresized);
		cv::Mat B = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(0));
		cv::Mat BX = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(0));
		cv::Mat BY = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(0));
		cv::Mat Bth = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(0));
		cv::Mat Bdil = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(0));
		cv::Mat Uno = cv::Mat(sizeAx2, sizeAy2, CV_32F, cv::Scalar::all(1.0));
		cv::Mat Ckern = cv::Mat(sizeC, 1, CV_32F, C);
		cv::Mat CkernIdentity = cv::Mat(1, 1, CV_32F, cv::Scalar::all(1));
		cv::Mat K = cv::Mat(3, 3, CV_32F, cv::Scalar::all(1));
		std::vector<int> NeuronX, NeuronY;
		double minX, minY, maxX, maxY, threshX, threshY;
		
		// Resize image
		cv::resize(cv::Mat(sizeAx, sizeAy, CV_16U, Araw), A, cv::Size(), 0.5, 0.5, cv::INTER_AREA);
		
		// Apply Gaussian blur
		double blur = 0.65;
		cv::GaussianBlur(A, A, cv::Size(3, 3), blur, blur);

		// Calculate -d2/dx2 and -d2/dy2 with the filter passed as C.
		// In this way, the CV_32F images will saturate the positive derivatives
		// and not the negative ones, which we need.
		// Before summing the two derivatives, divide them by 2, so that they don't saturate.
		cv::sepFilter2D(A, BX, CV_32F, Ckern, CkernIdentity);
		cv::sepFilter2D(A, BY, CV_32F, CkernIdentity, Ckern);
		cv::divide(BX, 2, BX);
		cv::divide(BY, 2, BY);
		B = BX + BY;
		
		// Threshold out some noise below a fraction of the maximum values:
		// Keep only the points in which **both** derivatives are above
		// threshold (remember that BX and BY are -d2/dx2 and -d2/dy2).
		cv::minMaxLoc(BX, &minX, &maxX);
		cv::minMaxLoc(BY, &minY, &maxY);
		threshX = threshold * maxX;
		threshY = threshold * maxY;
		cv::threshold(BX, BX, threshX, 1, CV_THRESH_BINARY);
		cv::threshold(BY, BY, threshY, 1, CV_THRESH_BINARY);
		Bth = BX.mul(BY);
		Bth = Bth.mul(B);

		// Dilate
		cv::dilate(Bth, Bdil, K);

		// Find local minima
		/**int k = 0;
		for (int i = 0; i < sizeAx2; i++) {
			for (int j = 0; j < sizeAy2; j++) {
				if (B.at<float>(i, j) == -Bdil.at<float>(i, j)) {
					//NeuronX[k] = i;
					//NeuronY[k] = j;
					//k++;
				}
			}
		}**/
		
		for (int i = 0; i < sizeAx2; i++) {
			for (int j = 0; j < sizeAy2; j++) {
				Aresized[sizeAx2*i + j] = Bdil.at<float>(i, j);
			}
		}
	
	}
}


unsigned __int16 sumarray(unsigned __int16 a[], signed __int32 size, unsigned __int16 *sum) {
	signed long i;

	if (a != NULL) {
		for (i = 0; i<size; i++) *sum += a[i];
	}
	else {
		return(1);
	}

	return(0);
}