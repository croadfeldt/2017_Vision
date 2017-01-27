
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>

using namespace cv;

int main(int argc, char** argv )
{
    Mat image;
    RNG rng(12345);
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    Scalar ColorMin = Scalar(56, 193, 21);
    Scalar ColorMax = Scalar(91, 255, 156);


    sl::zed::Camera* zed;
    zed = new sl::zed::Camera(sl::zed::HD720);

    sl::zed::InitParams parameters;
    parameters.mode = sl::zed::MODE::PERFORMANCE;
    parameters.unit = sl::zed::UNIT::METER; // Viewer scaled for processing METERs
parameters.verbose = true;
    

    sl::zed::ERRCODE err = zed->init(parameters);
    std::cout << "Error code : " << sl::zed::errcode2str(err) << std::endl;
    if (err != sl::zed::SUCCESS) {
        // Exit if an error occurred
        delete zed;
        return 1;
}

int width = zed->getImageSize().width;
int height = zed->getImageSize().height;

cv::Size displaySize(720, 404);
/*

int y_int = (y * data->_image.height / data->_resize.height);
        int x_int = (x * data->_image.width / data->_resize.width);

        float* ptr_image_num = (float*) ((int8_t*) data->data + y_int * data->step);
	float dist = ptr_image_num[x_int];*/

	  
sl::zed::SENSING_MODE dm_type = sl::zed::STANDARD;


sl::zed::Mat depth;
    
depth = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH); // Get the pointer


sl::zed::Camera::sticktoCPUCore(2);

char key = ' ';

cv::Mat disp(height, width, CV_8UC4);

 int viewID=0;




  while (key != 'q'){

zed->grab(dm_type);

 slMat2cvMat(zed->retrieveImage(static_cast<sl::zed::SIDE> (viewID))).copyTo(disp);
 

 /*


    GaussianBlur(image, image, Size(1,1), 2, 2);
    cvtColor(image, image, COLOR_BGR2HSV);
    inRange(image, ColorMin, ColorMax, image);
    erode(image, image, Mat(), Point(-1,-1), 2);
    dilate(image, image, Mat(), Point(-1,-1), 2);

    
    

    findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    
    /// Get the moments
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mu[i] = moments( contours[i], false ); }

  ///  Get the mass centers:
  vector<Point2f> mc( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }


     for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours(image, contours, i, color, 2, 8, hierarchy, 0, Point() );
       circle(image, mc[i], 4, color, -1, 8, 0);
     }

    */
    namedWindow("Display Image", WINDOW_NORMAL);
    resizeWindow("Display Image", 720, 440);
    imshow("Display Image", disp);
 }

    waitKey(0);

    return 0;
}
