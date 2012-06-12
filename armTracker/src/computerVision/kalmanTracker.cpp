#include "kalmanTracker.h"

CvKalman* kalman;
CvMat *w_k;



kalmanTracker::kalmanTracker(){
	bSetup = false;
}

float Mf(CvMat * r, int x, int y){
	return cvmGet(r, x, y);
}

void fM(CvMat * r, float val, int x, int y){
	cvmSet(r, x, y, val);
}

void kalmanTracker::setup( double processNoise, double measurementNoise ) 
{
	//cout << "kalmanTracker SETUP START " <<endl; 

	// Track time interval
	w_k = cvCreateMat(1, 1, CV_32FC1); // Measurement matrix

	// Setup Kalman tracker with 1 model variable, 1 measurement variable, and no control variables.
	kalman = cvCreateKalman( 1, 1, 0 );

	//cout << " after - cvCreateKalman address is " << (int)((void *)kalman )<< endl; 

	// Transition matrix F describes model parameters at and k and k+1
	cvSetIdentity( kalman->transition_matrix, cvRealScalar(1.0/30.0f));
	cvSetIdentity( kalman->measurement_matrix, cvRealScalar(1.0) );
	cvSetIdentity( kalman->process_noise_cov, cvRealScalar( processNoise ) );
	cvSetIdentity( kalman->measurement_noise_cov, cvRealScalar( measurementNoise ) );
	cvSetIdentity( kalman->error_cov_post, cvRealScalar(1) );
	bSetup = true;
	
	cout << "kalmanTracker SETUP DONE " <<endl; 

}

kalmanTracker::~kalmanTracker() 
{
	if ( kalman != NULL ) {
		cvReleaseMat( &w_k );
		cvReleaseKalman( &kalman );
		kalman = NULL;
	}
}

double kalmanTracker::setFirst( double dt )
{
	
	// Set initial state of Kalman filter
	fM(kalman->state_post, (float)dt, 0,0);
	
	return dt;
}

void kalmanTracker::print()
{
	double transition = Mf( kalman->transition_matrix, 0, 0 );
	double measurement = Mf( kalman->measurement_matrix, 0, 0 );
	double processNoise = Mf( kalman->process_noise_cov, 0, 0 );
	double measurementNoise = Mf( kalman->measurement_noise_cov, 0, 0 );
	double pre = Mf( kalman->state_pre, 0, 0 );
	double post = Mf( kalman->state_post, 0, 0 );
	printf("A:%lf H:%lf PN:%lf MN:%lf PRE:%lf POST:%lf\n", transition, measurement, processNoise, measurementNoise, pre, post );
}

double kalmanTracker::setNext( double dt )
{
	
	//print();

	// Actual Kalman steps
		
	cvKalmanPredict( kalman, NULL );
	
	fM(w_k,(float)dt, 0,0);
	cvKalmanCorrect( kalman, w_k );

	float post = Mf( kalman->state_post, 0, 0 );
						
	return post;
	
}

