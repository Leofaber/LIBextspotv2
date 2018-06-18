/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
 *
 * https://github.com/Leofaber/AG_extspot-v2-performance-evaluator
*/
#include <map>
#include <vector>
#include <iostream>
#include <iomanip>
#include <time.h>

#include "Blob.h"
#include "FolderManager.h"
#include "BlobsFinder.h"
#include "BayesianClassifierForBlobs.h"
#include "FileWriter.h"

#include "AgileMap.h"


class PerformanceEvaluator{

	public:
		PerformanceEvaluator(string testSetPath, double min_threshold, double max_threshold, double thresholdStep, double CDELT1, double CDELT2, double PSF);
		void evaluate();


	private:

		string testSetPath;

		double minThreshold;
		double maxThreshold;
		double thresholdStep;

		double CDELT1;
		double CDELT2;
		double PSF;

		time_t timer;

		BayesianClassifierForBlobs reverendBayes;

		string outfilename_log_file;
		string outfilename_log_for_plot;

		// Le immagini
		vector<string> filenames;

		// Il test set ->  <identificativo dell'istanza, pair<PuntatoreAlBlob,Etichetta di classificazione> >
		map< string, pair < Blob * , char > > testSet;

		// Prende la lista dei blobs e gli filtra scegliendo gli elementi che andranno a formare il test set
		void createTestSetMap();

		// Effettua la classificazione sul Blob * di ogni elemento di allBlobs. Ritorna una mappa <identificativo, coppia (centroide, etichetta di classificazione predetta)
		map< string, pair < Blob * , char > > createPredictionsMap(double currentThreshold);

		// Confronta ogni elemento di testSet con il corrispondente elemento di predictions.
		void computePerformance(map< string, pair < Blob * , char > >& predictions,double currentThreshold);

};
