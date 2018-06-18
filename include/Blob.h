/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#ifndef BLOB_H
#define BLOB_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <string>
#include <iostream>
#include "AgileMap.h"

using namespace std;

struct CustomPoint{
	int y;  // righe
	int x;	//colonne
	CustomPoint(){
		y = 0;
		x = 0;
	}
	CustomPoint(int _y, int _x){
		y = _y;
		x = _x;
	}
};






class Blob
{
    public:

        /**
          Create a new Blob starting from the contour pixels.
          Computes the centroid of the blob.
          Finds the pixels that are inside the blob.
          Finds the number of photons inside the blob.
          Compute the gray level pixel mean of the blob.
          Compute the photon's closeness
        */
        Blob(string filePath, vector<CustomPoint>& _contourPixels, vector<pair<CustomPoint,int>>& _blobPixels, vector<CustomPoint> photonsInBlob, double CDELT1, double CDELT2);



	       string getFilePath();

        /**
            Return the centroid of the blob
        */
        CustomPoint getCentroid();

 	      /**
            Return the galactic centroid of the blob
        */
        CustomPoint getGalacticCentroid();

      	double getGalacticCentroidL();
      	double getGalacticCentroidB();


        /**
            Return the sum of all the grey levels of the pixels of the blob divided by the number of those pixels.
        */
        double getPixelsMean();

        /**
            Return all the contour points
        */
        vector<CustomPoint> getContour();

        /**
            Return the number of pixels inside the blob
        */
        int getNumberOfPixels();

        /**
        	Return the blob's area (degree)
        */
        double getArea();

        /**
            Return the number of photons inside the blob (before any stretching or smoothing)
        */
        int getNumberOfPhotonsInBlob();


	       vector<CustomPoint> getPhotonsInBlob();


        /**
            Return all the Pixels (a CustomPoint with a grey level) that are inside the blob
        */
        vector<pair<CustomPoint,int>> getBlobPixels();


        /**
            Return the sum of the distances of each photons from the centroid divided by the number of photons
        */
        double getPhotonsCloseness();

        /**
            Check if the blob's centroid is included in a range
        */
        bool isCentered();


     private:

      	//static int ID;

      	//const int blobId;

      	AgileMap agileMapTool;

      	string filePath;

        vector<CustomPoint> contour;

        vector<pair<CustomPoint,int>> blobPixels;

        int numberOfPixels;

      	double pixelArea;

      	double blobArea;

        vector<CustomPoint> photonsInBlob;

        CustomPoint centroid;

      	CustomPoint galacticCentroid;

      	double pixelMean;

        double photonsCloseness;

	       //vector<CustomPoint> computePhotonsBlob();

        CustomPoint computeCentroid();

        CustomPoint computeGalacticCentroid();

        double computePixelMean();

        double computePhotonsCloseness();

        double getDistanceFromCentroid(CustomPoint p);

	      double getSphericalDistanceFromCentroid(CustomPoint p);


};
#endif // BLOB_H
