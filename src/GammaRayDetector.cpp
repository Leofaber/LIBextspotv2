/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#include "GammaRayDetector.h"


GammaRayDetector::GammaRayDetector(double _PSF,
								   const char * _imagePath,
								   const char * _outputLogName,
								   double _classificationThreshold,
								   const char * _imageExpPath,
								   bool isExpMapNormalizedBool,
								   bool createExpNormalizedMap,
								   bool createExpRatioMap,
								   double minTreshold,
								   double maxTreshold,
								   double squareSize,
							   	   bool _visualizationMode ) :

	reverendBayes(),
	agileMapTool(_imagePath)


{
	PSF = _PSF;
	imagePath = _imagePath;
	imageExpPath = _imageExpPath;

	visualizationMode = _visualizationMode;

	string imageExpPathString = _imageExpPath;
	string outPutLogNameString = _outputLogName;

   	if(imageExpPathString == "None"){
		evaluateExpRatio = false;
		cout << "\n*No exp-ratio evaluation will be performed\n"<<endl;
	}
	else{
		cout << "\n*Exp-ratio evaluation will be performed\n"<<endl;
		evaluateExpRatio = true;
		exp = new ExpRatioEvaluator(_imageExpPath, isExpMapNormalizedBool, createExpNormalizedMap, createExpRatioMap, minTreshold, maxTreshold, squareSize);
	}


	/*Check AgileMap*/
	if(agileMapTool.Read(_imagePath) == 202){
		cout << "*ERROR: File "<< _imagePath << " has not AgileMap format." << endl;
		exit(EXIT_FAILURE);
	}

	rows = agileMapTool.Rows();
	cols = agileMapTool.Cols();



   	fileName = extractFileNameFromImagePath(imagePath);

	outputLogName = computeOutputLogName(fileName,outPutLogNameString,minTreshold,maxTreshold,squareSize);

   	classificationThreshold = _classificationThreshold/100;

}


GammaRayDetector::~GammaRayDetector(){
	for(vector<Blob*>::iterator i = blobs.begin(); i != blobs.end(); i++){
		Blob * b = *i;
		delete b;
	}
	blobs.clear();
}



void GammaRayDetector::detect()
{


	const char * observationDateUTCtemp = agileMapTool.GetStartDate();
	string observationDateUTC = observationDateUTCtemp;
	double observationDateTT = agileMapTool.GetTstart();


	/// tira fuori una lista con tutti i BLOBS
	blobs = BlobsFinder::findBlobs(imagePath, PSF, agileMapTool.GetXbin() ,agileMapTool.GetYbin(), visualizationMode);

	//cout << "SIZE: " << blobs.size()<<endl;

	string information2PrintForSources = "";

	string expRatioString = "";

	int index = 1;
	if(blobs.size() > 0)
	{
		for(vector<Blob*>::iterator i = blobs.begin(); i != blobs.end(); i++)
		{
			information2PrintForSources += to_string(index)+" ";
			index++;
			Blob* b = *i;

			// Classification
			double fluxProbability = classifyBlob(b);


			double gaLong = b->getGalacticCentroidL();
			double gaLat  = b->getGalacticCentroidB();

			cout <<"Blob "<< index-1 <<":\n * centroid (pixel): [ "<<b->getCentroid().y<<" , "<<b->getCentroid().x<<" ]\n * centroid (degree): [ "<<gaLong<<" , "<< gaLat<<" ]\n * flux prob -> " << fluxProbability*100 << "%\n" <<endl;




			// ExpRatioEvaluation
			expRatioString="-1 ";
			if(evaluateExpRatio){
				expRatioString = to_string(exp->computeExpRatioValues(gaLong,gaLat))+" ";
			}

			// Building of output
			string tempString = to_string(gaLong)+" "+to_string(gaLat)+" "+to_string(fluxProbability*100)+" "+observationDateUTC+" "+to_string(observationDateTT)+" "+to_string(classificationThreshold*100)+" "+fileName+" "+expRatioString+"\n";

			/// Labeling
			if(fluxProbability >= classificationThreshold){
				information2PrintForSources += "SOURCE "+tempString;
			}else{
				information2PrintForSources += "BG "+tempString;

			}



		}
	}else{

		information2PrintForSources += "NO_BLOBS "+observationDateUTC+" "+to_string(observationDateTT)+" "+to_string(classificationThreshold*100)+" "+fileName+"\n";
	}



	FileWriter::write2File(outputLogName,information2PrintForSources);

	cout << "\n*Created Log File: " << outputLogName << "\n" <<endl;

}




double GammaRayDetector::classifyBlob(Blob* b)
{
    vector<pair<string,double> > predicted = reverendBayes.classify(b);
    //double bgProbability   = predicted[0].second;
    double fluxProbability = predicted[1].second;

    return fluxProbability;
}


///   /ANALYSIS3/NGC4993_SHORT/output/20s_428630400.0_431308800.0/SCAN20_428630420.0_428630440.0_1.cts.gz
string GammaRayDetector::extractFileNameFromImagePath(string imagePath){


    int firstIndex=0;
    int secondIndex=imagePath.size();

    /// FINDING .cts.gz  OR .cts in imagePath
    size_t foundCtsGz = imagePath.find(".cts.gz");

    if (foundCtsGz!=string::npos){
        //cout << ".cts.gz foundCtsGz at: " << foundCtsGz <<endl;
        secondIndex = foundCtsGz;
        imagePath = imagePath.substr(firstIndex, secondIndex);
    } else {

        size_t foundCts = imagePath.find(".cts");
        if (foundCts!=string::npos){
            secondIndex = foundCts;
            imagePath = imagePath.substr(firstIndex, secondIndex);
        }
    }

    /// FINDING SLASH in imagePath
    size_t foundSlash;


    char toDelete2 = '/';
    do{
        foundSlash = imagePath.find(toDelete2);
        //cout << "\nSlash foundSlash at: " << foundSlash << " NPOS: "<<string::npos << endl;
        firstIndex = foundSlash;
        imagePath = imagePath.substr(firstIndex+1, secondIndex);
      //  cout << "\nIMAGE PATH SUBSTRING: " << imagePath << endl;
    }
    while (foundSlash!=string::npos);



    return imagePath;
}

string GammaRayDetector::computeOutputLogName(string _filename, string _outputLogName, double minThreshold, double maxThreshold, double squareSize){
	string outputlogname;

	// FIND .txt in outputLogName.
	size_t foundTxt = _outputLogName.find(".txt");
	if(foundTxt != string::npos)
		outputlogname = _outputLogName.substr(0,foundTxt);
	else
		outputlogname = _outputLogName;


   	outputlogname +="_"+_filename+".txt";

	return outputlogname;


}
