#pragma once 
#include "ofxOpenCv.h"

class kalmanTracker{

		
	public:
		bool bSetup;

		kalmanTracker();
		void setup( double processNoise = 0.000001, double measurementNoise = 0.000001 );
		virtual ~kalmanTracker();
		double setFirst( double dt );
		void print();
		double setNext( double dt );
};

//	void setup(){
//	
//		Mat img = Mat(500, 500, CV_8UC3);
//		KalmanFilter kalman = KalmanFilter(2, 1, 0);
//		Mat state = Mat(2, 1, CV_32FC1);  // (phi, delta_phi)
//		Mat process_noise = Mat(2, 1, CV_32FC1);
//		Mat measurement = Mat(1, 1, CV_32FC1);
//		Mat measurement_noise = Mat(1, 1, CV_32FC1);
//
//		measurement.setTo(Scalar(0));
//		namedWindow("kalman", 1);
//
//		float m[2][2] = {{1, 1}, {0, 1}};
//		kalman.transitionMatrix = Mat(2, 2, CV_32FC1, m);
//
//		setIdentity(kalman.measurementMatrix, Scalar(1));
//		setIdentity(kalman.processNoiseCov, Scalar(1e-5));
//		setIdentity(kalman.measurementNoiseCov, Scalar(1e-1));
//		setIdentity(kalman.errorCovPost, Scalar(1));
//	
//	}
//
//
//	float doKalman(float input){
//			state = input;
//
//            // predict a measurement
//            Mat prediction = kalman.predict();
//            float predict_angle = prediction.at<float>(0, 0);
//
//            measurement = kalman.measurementMatrix * state;
//
//            // draw stuff
//            line(img, state_pt, measurement_pt, CV_RGB(255, 0,0), 3,  CV_AA, 0);
//            line(img, state_pt, predict_pt, CV_RGB(255, 255, 0), 3,  CV_AA, 0);
//            draw_cross(img, state_pt, CV_RGB(255, 255, 255), 3);
//            draw_cross(img, measurement_pt, CV_RGB(255, 0,0), 3);
//            draw_cross(img, predict_pt, CV_RGB(0, 255, 0), 3);
//
//            // correct kalman internal state with measurement
//            kalman.correct(measurement);
//            state = kalman.transitionMatrix * state + process_noise;	
//	}
//


