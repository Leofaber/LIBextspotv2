/*
 * Copyright (c) 2018
 *     Leonardo Baroncelli, Giancarlo Zollino,
 *
 * Any information contained in this software
 * is property of the AGILE TEAM and is strictly
 * private and confidential.
*/


#include "BlobsFinder.h"

BlobsFinder::BlobsFinder()
{
    //ctor
}

///////////////////////////////////////////////////////////////////////////////
//
//		Extraction of blobs from FITS sky images
//

vector<Blob*> BlobsFinder::findBlobs(	string fitsfilePath,
										double PSF,
										double CDELT1,
										double CDELT2,
										bool VISUALIZATION_MODE
									) {


	bool debug = VISUALIZATION_MODE;

	vector<Blob*> blobs;

	// int ** data, int rows, int cols
	IntMatrixCustomMap * int_matrix_map = MapConverter::fitsMapToIntMatrix(fitsfilePath.c_str());


	int rows = int_matrix_map->rows;
	int cols = int_matrix_map->cols;



	if(debug){
		//Conversion IntMatrixCustomMap -> Mat8U (opencv)
		Mat photonsMap(rows, cols, CV_8UC1, Scalar(0));
		for(int y = 0; y < rows; y++){
			for(int x =0; x < cols; x++){
				photonsMap.at<uchar>(y,x) = (uchar) int_matrix_map->image[y][x];

			}
		}
		BlobsFinder::print01Image(photonsMap,"original");
	}




	/* Smoothing */
	Mat workingImage8U = BlobsFinder::gassusianSmoothing(int_matrix_map, PSF, CDELT1, CDELT2, debug);



	/* Thresholding */
	workingImage8U = BlobsFinder::thresholding(workingImage8U, debug);



	/* Add padding to image */
	Mat workingImage8UWithPadding = BlobsFinder::addPaddingToImage(workingImage8U);



	/* Find contours */
	vector<vector<Point> > contoursImage8UWithPadding;
    vector<Vec4i> hierarchy;
	findContours(workingImage8UWithPadding, contoursImage8UWithPadding, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);


	/* Blobs extraction */
	int indexx=0;
	for(vector<vector<Point> >::iterator i = contoursImage8UWithPadding.begin(); i < contoursImage8UWithPadding.end(); i++){

        vector<Point> currentContuorWithPadding = *i;

    	// Creating a blob if and only if it is not contained i another blob
    	if(hierarchy[indexx][3]==-1){

			vector<pair<CustomPoint,int>> pixelsOfBlobsNoPadding;
			vector<CustomPoint> photonsInBlobs;

			/* Compute blob's pixels */
			BlobsFinder::computePixelsAndPhotonsOfBlob(
															int_matrix_map,
															workingImage8UWithPadding,
															currentContuorWithPadding,
													 		pixelsOfBlobsNoPadding,
													 		photonsInBlobs
														);

			/* Remove padding from contour & Convert vector<Point> to vector<CustomPoint> */
			vector<CustomPoint> currentCustomContuorNoPadding;
			for(vector<Point> ::iterator ii = currentContuorWithPadding.begin(); ii < currentContuorWithPadding.end(); ii++){
				Point p = *ii;
				CustomPoint cp(p.y-1,p.x-1);
				currentCustomContuorNoPadding.push_back(cp);
			}

			/* Creating a Blob
				The following condition (photonsInBlobs.size() >= 2) is needed
				to avoid the photon closeness problem:
				a blob made by 1 photon has a photonCloseness = 1 (the minimum).
				In this context, both a background blob and a flux blob has the
				same feature. In addition, there are a lot of background blobs
				made by only 1 photon and this can alter the photon closeness distribution.
			*/
			if( photonsInBlobs.size() >= 2) {
				Blob* b = new Blob(fitsfilePath, currentCustomContuorNoPadding, pixelsOfBlobsNoPadding, photonsInBlobs, CDELT1, CDELT2);
				blobs.push_back(b);
			}
			/* Handling errors
			else if(photonsInBlobs.size() == 0){
				BlobsFinder::reportError(photonsInBlobs,pixelsOfBlobsNoPadding,currentCustomContuorNoPadding,fitsfilePath,int_matrix_map);
				exit(EXIT_FAILURE);
			}*/
		}
    	indexx++;
	}

 	if(debug){
		waitKey();
	}

	delete int_matrix_map;

    return blobs;
}



Mat BlobsFinder::gassusianSmoothing(IntMatrixCustomMap * int_matrix_map, double PSF, double CDELT1, double CDELT2, bool debug){

	int rows = int_matrix_map->rows;
	int cols = int_matrix_map->cols;

	/* For Smoothing operation we use a float pixels matrix. */
	Mat workingImage32F(rows, cols, CV_32FC1, Scalar(0));
	for(int y = 0; y < rows; y++){
		for(int x =0; x < cols; x++){
			workingImage32F.at<float>(y,x) = (float) int_matrix_map->image[y][x];

		}
	}


	/* Gaussian smoothing */
	int kernelSize = (2 * PSF/CDELT2) + 1;
	Mat workingImage32FSmoothed;
	GaussianBlur(workingImage32F, workingImage32FSmoothed, Size(kernelSize, kernelSize), PSF, 0, 0); // 17x17 2.5    23x23 3	21x21 4


	/* Convert back to 8UC1 with linear stretching */
	Mat workingImage8U;
	double minVal, maxVal;
	minMaxLoc(workingImage32FSmoothed, &minVal, &maxVal); //find minimum and maximum intensities
	workingImage32FSmoothed.convertTo(workingImage8U, CV_8U, 255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));


	if(debug){
		BlobsFinder::printImage(workingImage32FSmoothed, "gaussianBlur32F", "32F");
	}

	return workingImage8U;
}

Mat BlobsFinder::thresholding(Mat workingImage8U, bool debug){

	int rows = workingImage8U.rows;
	int cols = workingImage8U.cols;

	/* Computing Image Histogram and Thresholding (mean value) */
	int nimages = 1;
  	int histSize = 256;
	float range[] = { 0, 256 };
 	const float* histRange = { range };
  	bool uniform = true; bool accumulate = false;
	Mat hist;

  	calcHist(&workingImage8U, nimages, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );


	Mat workingImageThresholded8U = workingImage8U.clone();
	int threshold = 0, mean = 0;
	for( int i = 1; i < histSize; i++ ){
		 mean += hist.at<float>(i-1);
	}

	threshold = mean/histSize-1;

	for(int y = 0; y < rows; y++){
		for(int x =0; x < cols; x++){
			unsigned short int gl = (unsigned short int) workingImageThresholded8U.at<uchar>(y,x);
			if(gl <= threshold)
				workingImageThresholded8U.at<uchar>(y,x) = 0;

		}
	}
	if(debug){
		//BlobsFinder::drawImageHistogram(hist,histSize);
		BlobsFinder::printImage(workingImageThresholded8U, "workingImageThresholded8U Thresholded", "8U");
  		calcHist(&workingImageThresholded8U, nimages, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );
		BlobsFinder::drawImageHistogram(hist,histSize);
	}
	return workingImageThresholded8U;
	/* ------------------  */
}



Mat BlobsFinder::addPaddingToImage(Mat image8U){
	int rows = image8U.rows;
	int cols = image8U.cols;

	// Adding 1x1 padding (the function does not take into account 1-pixel border of the image, so I dont want a photon pixel border to be clipped)
	Mat workingImage8UWithPadding(rows+2, cols+2, CV_8UC1, Scalar(0));
	for(int y = 0; y < rows; y++){
		for(int x = 0; x < cols; x++){
			workingImage8UWithPadding.at<uchar>(y+1,x+1) = image8U.at<uchar>(y,x);
		}
	}
	return workingImage8UWithPadding;

}

void BlobsFinder::computePixelsAndPhotonsOfBlob( 	IntMatrixCustomMap * int_matrix_map_original,
													Mat& padded_smoothed_and_thresholded_image,
													vector<Point>& contour,
													vector<pair<CustomPoint,int>>& pixelsOfBlobs,
													vector<CustomPoint>& photonsOfBlobs
){



	for(int y = 0; y < padded_smoothed_and_thresholded_image.rows; y++){
		for(int x = 0; x < padded_smoothed_and_thresholded_image.cols; x++){

			Point p(x,y);
			double isInside = pointPolygonTest(contour,p,false); /*opencv*/

			/* check if pixel is inside blob */
			if( isInside >= 0 ){

				/* Removing padding */
				CustomPoint cp(p.y - 1, p.x - 1);
				pixelsOfBlobs.push_back(make_pair(cp,(int)padded_smoothed_and_thresholded_image.at<uchar>(y,x)));

				/* check if pixel is a photon */
				int greyLevel = int_matrix_map_original->image[cp.y][cp.x];


				for(int j = 0; j < greyLevel; j++){
					photonsOfBlobs.push_back(cp);
				}

			}
		}
	}

}





///////////////////////////////////////////////////////////////////////////
//
//	DEBUGGING


void BlobsFinder::reportError(vector<CustomPoint>& photonsOfBlobs, vector<pair<CustomPoint,int>>& pixelsOfBlobs, vector<CustomPoint>& contour, string filePath, IntMatrixCustomMap * int_matrix_map){

	int rows = int_matrix_map->rows;
	int cols = int_matrix_map->cols;


	cout << "*BlobsFinder Error: 0 fotoni nel blob! File: "<< filePath << endl;
	cout << "*Number of pixels: " << pixelsOfBlobs.size() << endl;
	cout << "*Number of contour pixels: " << contour.size() << endl;


	for(vector<pair<CustomPoint,int>>::iterator i = pixelsOfBlobs.begin(); i != pixelsOfBlobs.end(); i++){
		pair<CustomPoint,int> p = *i;
		cout << "*(" << p.first.y << "," << p.first.x <<")"<<" gl: "<<p.second<<" photon:" <<int_matrix_map->image[p.first.y][p.first.x]<<endl;
	}
	cout <<"\n*Contorno:" << endl;
	for(vector<CustomPoint>::iterator i = contour.begin(); i != contour.end(); i++){
		CustomPoint p = *i;
		cout << "*(" << p.y << "," << p.x <<")"<<" photon:" <<int_matrix_map->image[p.y][p.x]<<endl;
	}


	Mat workingImage8U3C(rows, cols, CV_8UC3, Scalar(0,0,0));

	Vec3b color( rand()&255, rand()&255, rand()&255 );

	for(vector<CustomPoint>::iterator i = contour.begin(); i != contour.end(); i++)		{
		CustomPoint p = *i;
		workingImage8U3C.at<Vec3b>(p.y,p.x) = color;
	}
	///DRAW CENTROID
	//CustomPoint centroid = b->getCentroid();
	//workingImage8U3C.at<Vec3b>(centroid.y,centroid.x) = Vec3b(0,0,255);
	BlobsFinder::printImage(workingImage8U3C, "Debug", "8U");
	waitKey();


}





void BlobsFinder::printImageInConsole(Mat& image, string type){
	cout << "\n\n" << endl;
	for(int i = 0; i < image.rows; i++){
		for(int j =0; j < image.cols; j++){
			if(type == "8U")
				cout << ceil((unsigned short int) image.at<uchar>(i,j));
			else if(type == "32F")
				cout << fixed << image.at<float>(i,j)<<" ";
		}
		cout << "\n\n";
	}
}


void BlobsFinder::print01Image(Mat& image, string windowName){
	Mat toShow = image.clone();
	image.convertTo(toShow, CV_8U, 255.0, 0);
	resize(toShow, toShow, Size(), 4, 4, INTER_LINEAR);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, toShow);
}
void BlobsFinder::printImage(Mat& image, string windowName, string type){

	Mat toShow;
	if( type == "8U" ){
		toShow = image.clone();
	}
	else if ( type == "32F" ){

		/*need conversion from 32f to 8u*/
		double minVal, maxVal;
		minMaxLoc(image, &minVal, &maxVal); //find minimum and maximum intensities
		image.convertTo(toShow, CV_8U, 255.0/(maxVal - minVal), -minVal * 255.0/(maxVal - minVal));
	}

	resize(toShow, toShow, Size(), 4, 4, INTER_LINEAR);
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, toShow);
	//waitKey();

}

void BlobsFinder::printImageBlobs(int rows,int cols, vector<Blob>& blobs, string windowName){
	Mat temp3ChannelImage(rows,cols, CV_32FC3, Scalar(0,0,0));
	for(vector<Blob>::iterator i = blobs.begin(); i != blobs.end(); i++){
		printImageBlob(temp3ChannelImage,*i,windowName);
	}
//	waitKey(0);
}
void BlobsFinder::printImageBlob(Mat& inputImage, Blob& b, string windowName) {

	Mat tempInputImage = inputImage.clone();

	///DRAW CONTOUR
	vector<CustomPoint> contour = b.getContour();
	Vec3b color( rand()&255, rand()&255, rand()&255 );
	for(vector<CustomPoint>::iterator i = contour.begin(); i != contour.end(); i++){
		CustomPoint p = *i;
		tempInputImage.at<Vec3b>(p.y,p.x) = color;
	}

	///DRAW CENTROID
	CustomPoint centroid = b.getCentroid();
	tempInputImage.at<Vec3b>(centroid.y, centroid.x) = Vec3b(0,0,255);

	namedWindow( windowName, CV_WINDOW_AUTOSIZE );
	resize(tempInputImage, tempInputImage, Size(0, 0), 3, 3, INTER_LINEAR);
	imshow( windowName, tempInputImage );

}


void BlobsFinder::drawImageHistogram(Mat& hist, int histSize){

	int hist_w = 512;
	int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar(0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	//normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );



	for( int i = 1; i < histSize; i++ ){
		line( histImage,
		Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
		Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
		Scalar( 255, 255, 255), 2, 8, 0  );
	}

	/// Display
	namedWindow("Image Histogram", CV_WINDOW_AUTOSIZE );
	imshow("Image Histogram", histImage );

}

/*
Unsigned 8bits uchar 0~255
IplImage: IPL_DEPTH_8U
Mat: CV_8UC1, CV_8UC2, CV_8UC3, CV_8UC4

Signed 8bits char -128~127
IplImage: IPL_DEPTH_8S
Mat: CV_8SC1，CV_8SC2，CV_8SC3，CV_8SC4

Unsigned 16bits ushort 0~65535
IplImage: IPL_DEPTH_16U
Mat: CV_16UC1，CV_16UC2，CV_16UC3，CV_16UC4

Signed 16bits short -32768~32767
IplImage: IPL_DEPTH_16S
Mat: CV_16SC1，CV_16SC2，CV_16SC3，CV_16SC4

Signed 32bits int -2147483648~2147483647
IplImage: IPL_DEPTH_32S
Mat: CV_32SC1，CV_32SC2，CV_32SC3，CV_32SC4

Float 32bits float -1.18*10-38~3.40*10-38
IplImage: IPL_DEPTH_32F
Mat: CV_32FC1，CV_32FC2，CV_32FC3，CV_32FC4

Double 64bits double
Mat: CV_64FC1，CV_64FC2，CV_64FC3，CV_64FC4

Unsigned 1bit bool
IplImage: IPL_DEPTH_1U
*/
