#include "stdio.h" // getchar
#include <iostream>
#include <vector>
#include <random> // for normal_distribution<T>

#include "FolderManager.h"
#include "BlobsFinder.h"
#include "MapConverter.h"



class BayesianModelEvaluator{

	public:
		static void computeModel(string trainingSetPath, double CDELT1, double CDELT2, double PSF);


	private:

		static void computeModelFromTrainingSet(string type,
												vector<string> fileNames,
												string trainingSetPath,
												double CDELT1,
												double CDELT2,
												double PSF
												);

		static pair<vector<Blob *>,vector<Blob *>> getAllBlobsFromTrainingSet(	vector<string> bgFileNames,
																				 vector<string> fluxFileNames,
																				 string trainingSetBackgroundPath,
																				 string trainingSetFluxPath,
																				 double CDELT1,
																				 double CDELT2,
																				 double PSF
																			 );


		static void printMeanAndDeviation(string type, vector<Blob *>& allBlobs);

		static normal_distribution<double> computeNormalDistribution(vector<double>& attributeValues);

		static void cleanMemory(vector<Blob*>& blobs);

		BayesianModelEvaluator();

};
