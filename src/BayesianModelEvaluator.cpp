#include "BayesianModelEvaluator.h"

BayesianModelEvaluator::BayesianModelEvaluator(){

}


void BayesianModelEvaluator::computeModel(string trainingSetPath, double CDELT1, double CDELT2, double PSF){


	string trainingSetBackgroundPath = trainingSetPath+"/bg";

	string trainingSetFluxPath = trainingSetPath+"/flux";


	// Vector that contains all the file names of the training set background images.
	vector<string> fileNamesBackground = FolderManager::getFileNamesFromFolder(trainingSetBackgroundPath);

	// Vector that contains all the file names of the training set flux images.
	vector<string> fileNamesFlux = FolderManager::getFileNamesFromFolder(trainingSetFluxPath);




    cout << "\nAnalysis started: " << fileNamesBackground.size() + fileNamesFlux.size() << " files need to be analyzed." << endl;

	// Get all training instances
	pair<vector<Blob *> , vector<Blob *> > bgAndfluxBlobs = BayesianModelEvaluator::getAllBlobsFromTrainingSet(fileNamesBackground,fileNamesFlux,  trainingSetBackgroundPath, trainingSetFluxPath, CDELT1, CDELT2, PSF);


	cout << "* Computing distributions.. " << endl;
	// Compute and print distributions (Bayesian Model)
	BayesianModelEvaluator::printMeanAndDeviation("BACKGROUND", bgAndfluxBlobs.first);

	BayesianModelEvaluator::printMeanAndDeviation("FLUX", bgAndfluxBlobs.second);

	cout << "\nLegend: "<< endl;
	cout << "AR  "<< "AREA (degrees)" << endl;
	cout << "PH  "<< "PHOTONS (number of photons)" << endl;
	cout << "PC  "<< "PHOTONS CLOSENESS (degrees)" << endl;
	//cout << "PM  "<< "PIXELS MEAN (grey level)" << endl;


	cout << "\n\n" << endl;


	// Clean heap
	BayesianModelEvaluator::cleanMemory(bgAndfluxBlobs.first);
	BayesianModelEvaluator::cleanMemory(bgAndfluxBlobs.second);




}


pair<vector<Blob *>,vector<Blob *>> BayesianModelEvaluator::getAllBlobsFromTrainingSet(
							vector<string> bgFileNames,
							vector<string> fluxFileNames,
							string trainingSetBackgroundPath,
							string trainingSetFluxPath,
							double CDELT1,
							double CDELT2,
							double PSF
){


	// bg blobs
	vector<Blob *> bgBlobs;

	// flux blobs
	vector<Blob *> fluxBlobs;

	cout << "* Looking up for blobs in training set backgroud images.. ["<< bgFileNames.size() <<"] files"<< endl;
	int total = bgFileNames.size();
	int count = 0;
	int step = total/10;
	int nextStep = step;
	// look up for blobs in trainingSetBackgroundPath
	for(vector<string>::iterator it=bgFileNames.begin() ; it < bgFileNames.end(); it++){

		// filepath
		string filename = *it;
		string fitsFilePath = trainingSetBackgroundPath + "/" + filename;


		// search for blobs
		vector<Blob *> blobs = BlobsFinder::findBlobs(fitsFilePath, PSF, CDELT1, CDELT2, false);


		// add blobs to all blobs list
		bgBlobs.insert( bgBlobs.end(), blobs.begin(), blobs.end() );



		count++;
		if( count > nextStep ){
			cout << "[";
			for(int i = 0; i < count/step; i++)
				cout << "#";
			for(int i = 0; i < total/step - count/step; i++)
				cout <<"_";
			cout << "]\n";
			cout << flush;
			nextStep += step;
		}

	}


	cout << "* Looking up for blobs in training set flux images.. ["<< fluxFileNames.size() <<"] files"<< endl;
	total = fluxFileNames.size();
	count = 0;
	step = total/10;
	nextStep = step;
	// look up for blobs in trainingSetBackgroundPath
	for(vector<string>::iterator it=fluxFileNames.begin() ; it < fluxFileNames.end(); it++)
    	{

		// filepath
		string filename = *it;
		string fitsFilePath = trainingSetFluxPath + "/" + filename;

		// search blobs
		vector<Blob *> blobs = BlobsFinder::findBlobs(fitsFilePath, PSF, CDELT1, CDELT2, false);

		int countFlux = 0;

		// handling background blobs found in flux images
		for(vector<Blob *>::iterator it = blobs.begin() ; it < blobs.end(); it++){

			Blob * b = *it;

			if( b->isCentered() && ( b->getNumberOfPhotonsInBlob()>1 ) && countFlux==0 ){
				// add blobs to flux blobs list
				fluxBlobs.push_back(b);
				countFlux++;
			}
			//else
				// add blobs to bg blobs list
				//bgBlobs.push_back(b);

		}

		count++;
		if( count > nextStep ){
			cout << "[";
			for(int i = 0; i < count/step; i++)
				cout << "#";
			for(int i = 0; i < total/step - count/step; i++)
				cout <<"_";
			cout << "]\n";
			cout << flush;
			nextStep += step;
		}

		/* DEBUGGIN
		// sommo tutti i fotoni dell'immagine
		int countTotalPhotons = 0;
		for(vector<Blob *>::iterator it = blobs.begin() ; it < blobs.end(); it++){
			Blob * b = *it;
			countTotalPhotons += b->getNumberOfPhotonsInBlob();
		}

		// nell'immagine non si trova flusso perchè tutti i blob han solo 1 fotone
		if(countFlux == 0 && countTotalPhotons == blobs.size() ){
			// NIENTE.. TUTTO OK..
		}
		// nell'immagine non si trova flusso (esiste un blob con più di un fotone..)
		else if(countFlux == 0 && countTotalPhotons > blobs.size()){
			cout << "\n" << endl;
			cout << "NO FLUX!" << endl;
			cout << "Count flux: " << countFlux << " in " << fitsFilePath << endl;
			for(vector<Blob *>::iterator it = blobs.begin() ; it < blobs.end(); it++){
				Blob * b = *it;
				cout << b->getCentroid().y << "," << b->getCentroid().x <<"#f: "<<b->getNumberOfPhotonsInBlob() << " is centered? " <<   b->isCentered() << endl;

			}
		// nell'immagine troviamo un blob di flusso!
		} else if(countFlux == 1){
			// OK PERFETTO!
		}
		// nell'immagine troviamo più di un blob di flusso!
		else if(countFlux > 1){
			cout << "\n" << endl;
			cout << "MANY FLUX! "<< endl;
			cout << "Count flux: " << countFlux << " in " << fitsFilePath << endl;
			for(vector<Blob *>::iterator it = blobs.begin() ; it < blobs.end(); it++){
				Blob * b = *it;
				cout << b->getCentroid().y << "," << b->getCentroid().x <<"#f: "<<b->getNumberOfPhotonsInBlob() << " is centered? " <<   b->isCentered() << endl;
			}
 			getchar();

		}

 		*/
	}



	cout << "* Number of background blobs: " << bgBlobs.size() <<endl;
	cout << "* Number of flux blobs: " << fluxBlobs.size() <<endl;

	return make_pair(bgBlobs,fluxBlobs);


}

void BayesianModelEvaluator::printMeanAndDeviation(string type, vector<Blob *>& allBlobs){

	// the attribute values vectors
	vector<double> area;
	vector<double> photons;
	vector<double> photons_closeness;
	//vector<double> pixel_mean;
	// ** add attribute


	// population the attribute values vectors
	for(vector<Blob*>::iterator i = allBlobs.begin(); i != allBlobs.end(); i++){

		Blob * b = *i;

 		area.push_back(b->getArea());
		photons.push_back(b->getNumberOfPhotonsInBlob());
		photons_closeness.push_back(b->getPhotonsCloseness());
		//pixel_mean.push_back(b->getPixelsMean());
		// ** add attribute



		/*
		cout << "\n" << endl;
		cout << "file " << b->getFilePath() << endl;
		cout <<"getArea "<< b->getArea() << endl;
		cout <<"getNumberOfPhotonsInBlob "<< b->getNumberOfPhotonsInBlob() << endl;
		cout <<"getPhotonsCloseness "<< b->getPhotonsCloseness() << endl;
		cout <<"getPixelsMean "<< b->getPixelsMean() << endl;

		getchar();
		*/
	}


	// computing normal distributions
	normal_distribution<double> AREA = computeNormalDistribution(area);

	normal_distribution<double> PHOTONS = computeNormalDistribution(photons);

	normal_distribution<double> PHOTONS_CLOSENESS = computeNormalDistribution(photons_closeness);

    //	normal_distribution<double> PIXEL_MEAN = computeNormalDistribution(pixel_mean);
	// ** add attribute


	// print mean and deviation
	cout << "\n*"<<type<<endl;
	cout << "AR  "<< AREA.mean() 			<< " " << AREA.stddev() 		<< endl;
	cout << "PH  "<< PHOTONS.mean() 		<< " " << PHOTONS.stddev() 		<< endl;
	cout << "PC  "<< PHOTONS_CLOSENESS.mean() 	<< " " << PHOTONS_CLOSENESS.stddev() 	<< endl;
	//cout << "PM  "<< PIXEL_MEAN.mean() 		<< " " << PIXEL_MEAN.stddev() 		<< endl;
	// ** add attribute


}

normal_distribution<double>  BayesianModelEvaluator::computeNormalDistribution(vector<double>& attributeValues){

	double mean = 0;
	double deviation = 0;
	int total = attributeValues.size();

	for(vector<double>::iterator i = attributeValues.begin(); i != attributeValues.end(); i++){
		mean += *i;
	}
	mean = mean/(double)total;


	for(vector<double>::iterator i=attributeValues.begin() ; i < attributeValues.end(); i++) {

		double term = pow(*i - mean, 2);
	        deviation += term;
        }
        deviation = sqrt(deviation/total);

	normal_distribution<double> nd(mean,deviation);
	return nd;
}



void BayesianModelEvaluator::cleanMemory(vector<Blob*>& blobs){

	for(vector<Blob*>::iterator i = blobs.begin(); i != blobs.end(); i++){
		Blob * b = *i;
		delete b;
	}
	blobs.clear();
}
