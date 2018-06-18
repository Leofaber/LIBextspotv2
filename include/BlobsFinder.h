/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#ifndef BLOBSFINDER_H
#define BLOBSFINDER_H

#include <map>
#include <iostream>
#include <math.h>  // ceil ( )
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "Blob.h"
#include "MapConverter.h"



using namespace cv;


class BlobsFinder
{
    public:

        static vector<Blob*> findBlobs(  string fitsfilePath,
                        								 double PSF,
                        								 double CDELT1,
                        								 double CDELT2,
                                         bool VISUALIZATION_MODE
                                      );

    private:

        BlobsFinder();



        // EXTRACTION OPERATIONS

        static Mat gassusianSmoothing(  IntMatrixCustomMap * int_matrix_map,
                                        double PSF,
                                        double CDELT1,
                                        double CDELT2,
                                        bool debug
                                     );

        static Mat thresholding(Mat image, bool debug);


        static Mat addPaddingToImage(Mat image8U);


        static void computePixelsAndPhotonsOfBlob(	IntMatrixCustomMap * int_matrix_map_original,
                                                    Mat& smoothed_and_thresholded_image,
                                                    vector<Point>& contour,
                                      							vector<pair<CustomPoint,int>>& pixelsOfBlobs,
                                      							vector<CustomPoint>& photonsOfBlobs
                                                 );


        // DEBUGGIN

        static void reportError(vector<CustomPoint>& photonsOfBlobs, vector<pair<CustomPoint,int>>& pixelsOfBlobs, vector<CustomPoint>& contour, string fitsFilePath, IntMatrixCustomMap * int_matrix_map);

        static void print01Image(Mat& image,string windowName);

        static void printImage(Mat& image,string windowName, string type);

        static void printImageInConsole(Mat& image, string type);

        static void printImageBlobs(int rows,int cols, vector<Blob>& blobs, string windowName);

        static void printImageBlob(Mat& inputImage, Blob& b, string windowName);

        static void drawImageHistogram(Mat& hist, int histSize);
};


#endif // BLOBSFINDER_H
