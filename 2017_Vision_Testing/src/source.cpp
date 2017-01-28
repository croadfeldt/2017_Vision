#include <iostream>
 
// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

 
// ZED
#include <zed/Camera.hpp>
using namespace cv;
using namespace std;
 
// Input from keyboard
char keyboard = ' ';

void objectProc(Mat source, Scalar hsvMin, Scalar hsvMax );
const char* tostr (int);

int main(int argc, char** argv)
{
  RNG rng(12345);
  int contour_height[20];
  int contour_width[20];
  float aspect_ratio[20];
  vector<vector<Point> > contours;
  vector<vector<Point> > filtered;
  vector<Vec4i> hierarchy;
  // Initialize ZED color stream in HD and depth in Performance mode
  sl::zed::Camera* zed = new sl::zed::Camera(sl::zed::HD720);
  sl::zed::InitParams parameters;
  parameters.mode = sl::zed::MODE::PERFORMANCE;
  parameters.unit = sl::zed::UNIT::METER; // Viewer scaled for processing METERs
  parameters.verbose=true;

  // HSV Variables
  int hueMin = 0;
  int hueMax = 179;
  int satMin = 0;
  int satMax = 255;
  int valMin = 0;
  int valMax = 255;
  Scalar hsvMin = Scalar(hueMin, satMin, valMin, 0);
  Scalar hsvMax = Scalar(hueMax, satMax, valMax, 0);
  Scalar gearMin = Scalar(15, 123, 0);
  Scalar gearMax = Scalar(39, 255, 255);

  // Countour index
  int largest_area=0;
  int minimum_area=0;
  int largest_contour_index=0;
  cv::Rect bounding_rect;
  

  sl::zed::ERRCODE err = zed->init(parameters);
 
  // Quit if an error occurred
  if (err != sl::zed::SUCCESS) {
    std::cout << "Unable to init the ZED:" << errcode2str(err) << std::endl;
    delete zed;
    return 1;
  }
 
  // Initialize color image and depth
  int width = zed->getImageSize().width;
  int height = zed->getImageSize().height;
  Mat image(height, width, CV_8UC4,1);
  Mat depth(height, width, CV_8UC4,1);
  Mat blur(height, width, CV_8UC4,1);

  // Create OpenCV windows
  namedWindow("Image", WINDOW_AUTOSIZE);
  namedWindow("Depth", WINDOW_AUTOSIZE);
  namedWindow("Output", WINDOW_AUTOSIZE);
  namedWindow("HSV", 1);
  namedWindow("BoatsHSV", WINDOW_AUTOSIZE);


  // Trackbars
  cvCreateTrackbar("Hue Min","HSV",&hueMin,179);
  cvCreateTrackbar("Hue Max","HSV",&hueMax,179);
  cvCreateTrackbar("Sat Min","HSV",&satMin,255);
  cvCreateTrackbar("Sat Max","HSV",&satMax,255);
  cvCreateTrackbar("Val Min","HSV",&valMin,255);
  cvCreateTrackbar("Val Max","HSV",&valMax,255);
  cvCreateTrackbar("Min Area","HSV",&minimum_area,5000);

  // Settings for windows
  Size displaySize(720, 404);
  Mat imageDisplay(displaySize, CV_8UC4);
  Mat depthDisplay(displaySize, CV_8UC4);
  Mat outputDisplay(displaySize, CV_8UC4); 
  Mat BoatsHSV(displaySize, CV_8UC4);
  Mat Same(height, width, CV_8UC4,1);

  // Loop until 'q' is pressed
  while (keyboard != 'q') {

    hsvMin = Scalar(hueMin, satMin, valMin, 0);
    hsvMax = Scalar(hueMax, satMax, valMax, 0);
 
    filtered.clear();

    // Grab frame and compute depth in FILL sensing mode
    if (!zed->grab(sl::zed::SENSING_MODE::FILL))
      {
 
	// Retrieve left color image
	sl::zed::Mat left = zed->retrieveImage(sl::zed::SIDE::LEFT);
	memcpy(image.data,left.data,width*height*4*sizeof(uchar));
 
	// Retrieve depth map
	sl::zed::Mat depthmap = zed->normalizeMeasure(sl::zed::MEASURE::DEPTH);
	memcpy(depth.data,depthmap.data,width*height*4*sizeof(uchar));
	GaussianBlur(image, blur, Size(1,1), 2, 2);
   
	cvtColor(blur, blur, COLOR_BGR2HSV);
	inRange(blur, hsvMin, hsvMax, blur);
	blur.copyTo(Same);
	erode(blur, blur, Mat(), Point(-1,-1), 2);
	dilate(blur, blur, Mat(), Point(-1,-1), 2);
    
	findContours(blur, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	

	largest_area = minimum_area;
	// Find areas of contours, find largest areas, save the largest area
	for( int i = 0; i< contours.size(); i++ ) // iterate through each contour.
	  {
	    double a=contourArea( contours[i],false);  //  Find the area of contour
	    if(a>largest_area){
	      largest_area=a;
	      largest_contour_index=i;                //Store the index of largest contour
	      bounding_rect=boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
	      contour_height[i]=bounding_rect.height;
	      contour_width[i]=bounding_rect.width;
	      filtered.push_back(contours[i]);
	      aspect_ratio[i] = (float) contour_width[i]/(float) contour_height[i];
	      cout<< "largest area: " << largest_area <<endl;
	      cout<< "contour height: " << contour_height[i] << " contour width: " << contour_width[i] <<endl;
	      cout<< "aspect ratio: " << aspect_ratio[i] <<endl;
	      
	    }    
	  }

	//	filtered.push_back(contours[largest_contour_index]);

	
	/*	Scalar color( 255,255,255);
	drawContours( image, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
	rectangle(image, bounding_rect,  Scalar(0,255,0),1, 8,0);
	*/
    
	/// Get the moments
	vector<Moments> mu(filtered.size() );
	for( int i = 0; i < filtered.size(); i++ )
	  { mu[i] = moments( filtered[i], false ); }

	///  Get the mass centers:
	vector<Point2f> mc( filtered.size() );
	for( int i = 0; i < filtered.size(); i++ )
	  { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

	// Draws largest contours
	for( int i = 0; i< filtered.size(); i++ )
	  {
	    Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	    drawContours(image, filtered, i, color, 2, 8, hierarchy, 0, Point() );
	    circle(image, mc[i], 4, color, -1, 8, 0);
	  }

	
	
	// I don't think this is helping me at all

	  void DisplayOverlay(const char* Output, const char* Width, int delayms=25);
	 
	 

	// Display image in OpenCV window
	resize(image, imageDisplay, displaySize);
	imshow("Image", imageDisplay);
	resize(blur, outputDisplay, displaySize);
	imshow("Output", outputDisplay);
	resize(Same, BoatsHSV, displaySize);
	imshow("BoatsHSV", BoatsHSV);
	// Display depth map in OpenCV window
	resize(depth, depthDisplay, displaySize);
	imshow("Depth", depthDisplay);
	
      }
 
    keyboard = waitKey(30);
 
  }
 
  delete zed;
 
}
