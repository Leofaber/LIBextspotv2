/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/
#include "BayesianClassifierForBlobs.h"
#include "BlobsFinder.h"
#include "FileWriter.h"
#include "ExpRatioEvaluator.h"

#include <AgileMap.h>


using namespace std;

class GammaRayDetector
{
public:

    /**
        User has to specify the path to the FITS file, the name of the output file, the classification threshold as an interger or a floating point number
    */
	GammaRayDetector(double PSF, const char * imageCtsPath, const char * outputLogName, double classificationThreshold, const char * imageExpPath, bool isExpMapNormalizedBool, bool createExpNormalizedMap,bool createExpRatioMap, double minTreshold, double maxTreshold, double squareSize, bool visualizationMode);

	~GammaRayDetector();
	/**
        Open the fits file, convert it to Mat image, extract blobs, classify them with bayesian classifier.
    */
    void detect();


private:


    /**
        Given a blob call Reverend Bayes to predict the probabilities.
    */
	double classifyBlob(Blob* b);
	string extractFileNameFromImagePath(string imagePath);
	string computeOutputLogName(string filename, string outputLogName, double minThreshold, double maxThreshold, double squareSize);



	double PSF;
	string imagePath;
	string outputLogName;
	string fileName;
	float classificationThreshold;
	bool evaluateExpRatio;
	bool visualizationMode;

	const char *imageExpPath;
	int** ctsMap;
	int rows;
	int cols;

	vector<Blob*> blobs;

	BayesianClassifierForBlobs reverendBayes;
	AgileMap agileMapTool;
	ExpRatioEvaluator* exp;
 };
