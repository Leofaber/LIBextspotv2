#include "PerformanceEvaluator.h"



PerformanceEvaluator::PerformanceEvaluator(string _testSetPath, double _min_threshold, double _max_threshold, double _thresholdStep, double _CDELT1, double _CDELT2, double _PSF){

	testSetPath = _testSetPath;
	minThreshold = _min_threshold;
	maxThreshold = _max_threshold;
	thresholdStep = _thresholdStep;
	CDELT1 = _CDELT1;
	CDELT2 = _CDELT2;
	PSF = _PSF;

	outfilename_log_file = "output_for_log_from"+to_string(minThreshold)+"_to_"+to_string(maxThreshold)+"_psf_"+to_string(PSF)+"cdelt1_"+to_string(CDELT1)+"_time_"+to_string(time(&timer))+".txt";
	outfilename_log_for_plot = "output_for_plot"+to_string(minThreshold)+"_to_"+to_string(maxThreshold)+"_psf_"+to_string(PSF)+"cdelt1_"+to_string(CDELT1)+"_time_"+to_string(time(&timer))+".txt";

	if(maxThreshold > 100)
		maxThreshold = 100;
	//if(remove(outfilename_log_for_plot.c_str()) == 0 ) {
			//cout << "\n*Removing existing "<<outfilename_log_for_plot<<" file!" << endl;

	string header2write = "";
	header2write.append("Threshold Accuracy K_statistic FMeasure TPR FPR DistErrMean DistErrDev\n");
	FileWriter::write2File(outfilename_log_for_plot, header2write);
	//}

	//if(remove(outfilename_log_file.c_str()) == 0 ) {
			//cout << "*Removing existing "<<outfilename_log_file<<" file!" << endl;

	header2write = "";
	header2write.append("Threshold Total Instances\t Correctly Classified\t Incorrectly Classified\t K statistic\t FNR\t    FPR\t   Accuracy\t FMeasure\t EDM\n");
	FileWriter::write2File(outfilename_log_file, header2write);
	//}

}

void PerformanceEvaluator::evaluate(){

	// Riempio il vector<string> filenames con i nomi delle mappe contenute nel path
	filenames = FolderManager::getFileNamesFromFolder(testSetPath);

	cout << "*Analysis started: " <<filenames.size()<< " files need to be analyzed." << endl;

	// Crea il test set
	createTestSetMap();


	map< string, pair < Blob* , char > > predictions;

	// Per ogni soglia
	double currentThreshold = minThreshold;

	while( currentThreshold <= maxThreshold ){

		// Crea le previsioni
		predictions = createPredictionsMap(currentThreshold);

		// Calcola le performance
		computePerformance(predictions,currentThreshold);

		currentThreshold = currentThreshold + thresholdStep;

	}

}

void PerformanceEvaluator::createTestSetMap() {

	cout << "*Creating test set.." << endl;
	int total = filenames.size();
	int count = 0;
	int step = total/10;
	if(step == 0)
		step = 1;
	int nextStep = step;

	for(vector<string>::const_iterator it = filenames.begin(); it < filenames.end(); ++it ) {

		string imageName = *it;

		string imagePathName = testSetPath;

		imagePathName.append("/" + imageName);

		AgileMap agileMapTool(imagePathName.c_str());


		// Trovo i blobs all'interno dell'imagine tramite Blobs finder
		vector<Blob*> blobs = BlobsFinder::findBlobs(imagePathName, PSF, agileMapTool.GetXbin() ,agileMapTool.GetYbin(), false);

		int countBlob = 0;
		bool fluxFound = false;

		// Creo il test set map < string  ,  pair<  Blob* , char > >

		for(vector<Blob *>::const_iterator it = blobs.begin(); it < blobs.end(); ++it ) {

			Blob * b = *it;

			string blobIdentifier = imagePathName;

			blobIdentifier.append("_BLOB" + to_string(countBlob));


			if( imageName.compare(0,1,"B") == 0 ){
				testSet.insert(make_pair(blobIdentifier, make_pair(b,'B')));
			}
			else if(b->getNumberOfPhotonsInBlob() > 1 && b->isCentered() && !fluxFound ){
				fluxFound = true;
				testSet.insert(make_pair(blobIdentifier, make_pair(b, 'F')));
			}

			countBlob++;
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

	}
	cout << "*Creating test set completed! Number of elements: "<<testSet.size() << endl;

}



map< string, pair < Blob * , char > > PerformanceEvaluator::createPredictionsMap(double currentThreshold) {

	cout << "\n*Creating predictions. Threshold: " << currentThreshold << endl;
	map< string, pair < Blob * , char > > predictions;

	map< string, pair < Blob * , char > >::iterator i;
	for(i = testSet.begin(); i != testSet.end(); i++){

		// unique blob identifier
		string blobIdentifier = i->first;

		// blob pointer
		Blob * blobPtr = i->second.first;

		CustomPoint centroid = blobPtr->getGalacticCentroid();

		vector<pair<string,double> > predicted = reverendBayes.classify(blobPtr);

		double fluxProbability = predicted[1].second;

		/*
		cout << "\nblob: " << blobIdentifier << endl;
		cout <<"["<< 100-blobPtr->getCentroid().y<<","<<blobPtr->getCentroid().x << "] flux prob. -> " <<fluxProbability*100<<endl;
		cout << "Photon closenss: " << blobPtr->getPhotonsCloseness() << endl;
		cout << "Photos: " << blobPtr->getNumberOfPhotonsInBlob() << endl;
		*/

		if(fluxProbability*100 >= currentThreshold)
			predictions.insert(make_pair(blobIdentifier, make_pair(blobPtr, 'F')));
		else
			predictions.insert(make_pair(blobIdentifier, make_pair(blobPtr, 'B')));

	}

	cout << "*Creating predictions completed! "<< endl;
	cout << "[";
	float stepsize = 1/thresholdStep;
	float steps = 101-maxThreshold;
	float numberOfSteps = steps*stepsize;
	for(int i = 0; i < currentThreshold; i++)
		cout << "#";
	for(int i = currentThreshold; i <= 100; i++)
		cout << "_";
	cout << "]"<<endl;

	return predictions;


}

void PerformanceEvaluator::computePerformance(map< string, pair < Blob* , char > >& predictions,double currentThreshold) {


	int TP = 0;
	int TN = 0;
	int FP = 0;
	int FN = 0;
	int totalInstances = 0;
	int totalFluxInstances = 0;
	int totalBackgroundInstances = 0;

	double falsePositiveRate = 0;
	double falseNegativeRate = 0;
	double truePositiveRate = 0;
	double trueNegativeRate = 0;

	double sensitivity = 0;
	double specificity = 0;
	double accuracy = 0;
	double f_measure = 0;
	double precision = 0;
	double k_choen = 0;

	vector<double> errorDistances;
	double errorDistancesTotal = 0;
	double errorDistancesMean = 0;
	double errorDistancesDeviation = 0;



	if(testSet.size() != predictions.size()){
		cout << "Different map size "<< endl;  // differing sizes, they are not the same
		exit(EXIT_FAILURE);
	}else{
		//cout << "Equal map size!" << endl;
	}

	map<string, pair < Blob *, char  > >::const_iterator i, j;

	for(i = testSet.begin(), j = predictions.begin(); i != testSet.end(); ++i, ++j)
	{
		pair <string, 	pair <  Blob *, char  > > testSetInstance    = *i;
		pair <string, 	pair <  Blob *, char  > > predictionInstance = *j;


		// Get unique blob identifiers
		string testSetInstanceIdentifier    = i->first;
		string predictionInstanceIdentifier = j->first;


		// Get Blob Ptr
		Blob * blob = i->second.first;


		//Get blob labels
		char realLabel      = i->second.second;
		char predictedLabel = j->second.second;


		//debug

	//	cout << "\nBLOB: " << predictionInstanceIdentifier << endl;
	//	cout <<"Blob: "<<100-blob->getCentroid().y << " , " << blob->getCentroid().x << endl;
	//	cout <<"real label: " << realLabel << " predictedLabel: "<<predictedLabel<<endl;




		if(testSetInstanceIdentifier != predictionInstanceIdentifier){
			cout << testSetInstanceIdentifier << " != " <<  predictionInstanceIdentifier << endl;
			exit(EXIT_FAILURE);
		}

		if(realLabel == 'F' && predictedLabel == 'F') {

			TP += 1;


		} else if (realLabel == 'B' && predictedLabel == 'F') {

			FP += 1;


		} else if (realLabel == 'B' && predictedLabel == 'B') {

			TN += 1;


		}else if (realLabel == 'F' && predictedLabel == 'B') {

			FN += 1;

		}





		/*
			DISTANCE ERROR EVALUATION
		*/
		if(realLabel == 'F' && predictedLabel == 'F'){
			// predicted centroid
			CustomPoint predictedCentroid = blob->getCentroid();

			string newPath = "";
			size_t foundPatternBLOB = predictionInstanceIdentifier.find("_BLOB");
			newPath = predictionInstanceIdentifier.substr(0,foundPatternBLOB);

			AgileMap agileMapTool(newPath.c_str());

			newPath.clear();


			double errorDistanceIstance = agileMapTool.SrcDist(predictedCentroid.x, predictedCentroid.y, agileMapTool.GetMapCenterL(),agileMapTool.GetMapCenterB());
			//cout << "Error distance: [ "<< to_string(predictedCentroid.y) <<" , "<< to_string(predictedCentroid.x) <<" ] "<< errorDistanceIstance << endl;
			errorDistancesTotal += errorDistanceIstance;
			errorDistances.push_back(errorDistanceIstance);


		}



	}

	if(errorDistances.size() == 0) {

		errorDistancesMean = 0;
		errorDistancesDeviation = 0;

	}else {

		errorDistancesMean = errorDistancesTotal / errorDistances.size();



		for(vector<double> :: iterator i=errorDistances.begin(); i != errorDistances.end(); i++) {
				double term = pow(*i - errorDistancesMean, 2);
				errorDistancesDeviation += term;
			}
		errorDistancesDeviation = sqrt(errorDistancesDeviation/errorDistances.size());

	}


	totalInstances = TP + TN + FN + FP;

	totalFluxInstances = TP + FN;

	totalBackgroundInstances = TN + FP;


	if( TP == 0 && FP == 0) {

		precision = 0;

	}else {

		sensitivity = TP/(double)(TP+FN);

		specificity = TN/(double)(TN+FP);

		//accuracy = sensitivity * (TP/(double)totalInstances) + specificity * (TN/(double)totalInstances);
		accuracy = sensitivity * (totalFluxInstances/(double)totalInstances) + specificity * (totalBackgroundInstances/(double)totalInstances);

		precision = TP/(double)(TP+FP);

		f_measure = 2 * ((precision * sensitivity) / (precision + sensitivity));

		double pr_e =  ( (TP+FP)/(double)totalInstances) * (  (TP+FN)/(double)totalInstances);

		k_choen = ( ( (TP + TN) / (double)totalInstances ) -pr_e) / (1-pr_e);

		falsePositiveRate = FP/(double)totalBackgroundInstances;

		//falseNegativeRate = FN/(double)totalFluxInstances;

		truePositiveRate = sensitivity;
	}

	/// Output printing
	cout << "\n*Outuput printing" << endl;
	cout << "*Total instances: " << totalInstances << endl;
	cout << "*TP: " << TP << endl;
	cout << "*TN: " << TN << endl;
	cout << "*FP: " << FP << endl;
	cout << "*FN: " << FN << endl;
	cout << "*ErrorDistanceMean: " << errorDistancesMean <<" +- "<< errorDistancesDeviation << endl;
	cout << "*FalsePostiveRate: " << falsePositiveRate <<endl;
	cout << "*FalseNegativeRate: " << falseNegativeRate << endl;
	cout << "*Kappa statistic: " << k_choen << endl;
	cout << "*Accuracy: " << accuracy << endl;
	cout << "*FMeasure: " << f_measure << endl;
	cout << "\n" << endl;



	/*
		DATA FOR LOG FILE
	*/
	string output2write = "";
	output2write.append(to_string(currentThreshold)+" "+to_string(totalInstances)+" "+to_string( TP + TN )+" "+to_string( FP + FN )+" "+to_string(k_choen)+" "+to_string(falseNegativeRate)+" "+to_string(falsePositiveRate)+" "+to_string(accuracy)+" "+to_string(f_measure)+" "+to_string(errorDistancesMean)+"\n");
	FileWriter::write2FileAppend(outfilename_log_file, output2write);

	/*
		DATA FOR CHART
	*/
	string outputChart2write = "";
	outputChart2write.append(to_string(currentThreshold)+" "+to_string(accuracy) + " " +to_string(k_choen) + " " + to_string(f_measure)+ " " + to_string(truePositiveRate) + " " + to_string(falsePositiveRate) + " "+ to_string(errorDistancesMean) + " "+ to_string(errorDistancesDeviation) + "\n");
	FileWriter::write2FileAppend(outfilename_log_for_plot, outputChart2write);
}
