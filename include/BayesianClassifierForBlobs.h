/*
 * Copyright (c) 2017
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#ifndef BAYESIANCLASSIFIERFORBLOBS_H
#define BAYESIANCLASSIFIERFORBLOBS_H

#include "Blob.h"
#include <random>



class BayesianClassifierForBlobs
{
    public:

        // Uses already computed distribution values, hard-coded as class attributes.
        BayesianClassifierForBlobs();


        // Compute the probability for a Blob to be background and flux. Returns [ ("background", x%) , ("flux",1-x%) ]
        vector<pair<string,double> > classify(Blob* b);



    private:

        double bgFrequency; // Assuming 0.5
        double fluxFrequency; // Assuming 0.5

        // Compute the probability of an attribute value, given a distribution, from the Gaussian Equation.
        // This probability will be inserted in the Bayesian Theorem in the classify() method.
        double computeProbabilityFromDistribution(double x,normal_distribution<double> distribution);


     //////////////////////////////////////////////////////////////////////////////////////////////////////
     //
     // Distribution 1
     //
     // Observables:
     //     - PhotonsInBlob : number of photons inside the blobs
     //     - PhotonsCloseness : closeness index among photons inside the blob
     //     - Area : the blob's area (degree)
     //
     // Generated with https://github.com/Leofaber/AG_extspot-v2-bayesian-model-evaluator
     //
     // Dataset:
     //
     //    - TODO: NEED DATASET GENERATION PARAMETERs !!!
     //
     //    - 972 background images -> 2152 backgroud blobs
     //    - 4441 flux images      -> 4357 flux blobs
     //    - 5413 total images     -> 6509 total blobs
     //
     // Blobs Finder parameters:
     //     - Smoothing [31x31 sigma=7.5]
     //     - Thresholding (on the grey level's average)
     //     - Post filtering => extract a blob iff contains a number of photons >= 2
     //
     // Bayesian Model Evaluator Post Filtering on FLUX images:
     //     - extract the i-blob in the j-image iff:
     //         - IS CENTERED (15 pixels offset)
     //         - CONTAINS A NUMBER OF PHOTONs > 1
     //         - IS THE FIRST FLUX FOUND IN j
     //
     // background
        normal_distribution<double> bgAreaDistribution    = normal_distribution<double>( 259.313, 114.337 );
        normal_distribution<double> bgPhotonsInBlob    = normal_distribution<double>( 2.70782, 1.06004 );
        normal_distribution<double> bgPhotonsCloseness = normal_distribution<double>( 2.8795, 1.41748 );
     //
     // flux
        normal_distribution<double> fluxAreaDistribution    = normal_distribution<double>( 240.756, 66.9674 );
        normal_distribution<double> fluxPhotonsInBlob    = normal_distribution<double>( 4.26807, 1.97511 );
        normal_distribution<double> fluxPhotonsCloseness = normal_distribution<double>( 1.7331, 0.873429 );
     //
     ////////////////////////////////////////////////////////////////////////////////////////////

     /*
     // background
        normal_distribution<double> bgPhotonsInBlob    = normal_distribution<double>( 2.92556, 1.30257 );
        normal_distribution<double> bgPhotonsCloseness = normal_distribution<double>( 3.50439, 1.63845 );
     //
     // flux
        normal_distribution<double> fluxPhotonsInBlob    = normal_distribution<double>( 4.33456, 2.03592 );
        normal_distribution<double> fluxPhotonsCloseness = normal_distribution<double>( 1.96242, 0.999395 );
     //
     */
};

#endif // BAYESIANCLASSIFIERFORBLOBS_H
