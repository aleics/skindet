#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/background_segm.hpp>


using namespace std;
using namespace cv;

// Define the Trackbars used
void define_trackbars();

// Detect the color of the hand
Mat color_detection(const Mat& src);

// Extracts the Hue (different type of colors) of the image
Mat hsv_filter(const Mat& src);

// Realize a median Filter
void median_filter(Mat in);

// Get the biggest contour
Mat biggest_contour(const Mat& in);

// Source image
Mat src;

// Output image
Mat out;

Mat fgMaskMOG2;

// Margin of the ROI
int margin = 20;

// File name of the input video (if any)
string filename;

// Filter size applied on the Median filter
int median_filter_size = 1;

// Thresh used on the Threshold filter
int threshold_thresh = 22;

// Source window name
string srcWindowName = "src";

// Hand window name
string handWindowName = "hand";

// Output window name
string outWindowName = "output";

//int canny_thresh = 35;
//Scalar lower = Scalar(0, 10, 60);
//Scalar upper = Scalar(20, 150, 255);

int main(int argc, char const *argv[]) {
  VideoCapture vidCap;

  char option;
  cout << "video or cam (v/c): ";
  cin >> option; // define if video or camera as input

  if(option == 'v') {
    cout << "introduce the video to play: ";
    cin >> filename;

    vidCap = VideoCapture(filename); // read file specified
  } else if(option == 'c') {
    vidCap = VideoCapture(0); // read camera
  } else {
    cerr << "option not found" << endl;
    return 1;
  }

  //Ptr<BackgroundSubtractor> pMOG2;
  //pMOG2 = createBackgroundSubtractorMOG2();  

  if(!vidCap.isOpened()) { // check if it's available the video capture source
    cerr << "Could not open video capture!" << endl;
    return 1;
  }

  // declare windows
  namedWindow(srcWindowName);
  namedWindow(handWindowName);
  namedWindow(outWindowName);

  // define the trackbars
  define_trackbars();

  bool done = false;
  while(!done) {
    if(!vidCap.read(src)) { // if error reading input image
      break;
    }

    if(option == 'c') { // apply a resizing for camera
      resize(src, src, Size(640, 360));
    }

    // define roi
    const Mat roi(src, 
                  Rect(margin, margin, src.cols - margin*2, src.rows - margin*2));

    char key = waitKey(1);
    switch(key) {
      case 27: // if 'ESC' selected
        done = true;
        break;
    }

    out = color_detection(roi); // detect the color of the hand
    out = biggest_contour(out); // detect the biggest contour of the frame

    //pMOG2->apply(roi, fgMaskMOG2);


    imshow(srcWindowName, src);
    //imshow("mog2", fgMaskMOG2);
    imshow(outWindowName, out);
  }

  return 0;
}

// Define the Trackbars used
void define_trackbars() {
  //createTrackbar("canny thresh", outWindowName, &canny_thresh, 200);
  createTrackbar("threshold thresh", outWindowName, &threshold_thresh, 50);
  createTrackbar("median filter", outWindowName, &median_filter_size, 15);
}

// Detect the color of the hand
Mat color_detection(const Mat& src) {
  Mat out;

  out = hsv_filter(src); // extract the hue of the source image

  imshow(handWindowName, out);

  threshold(out, out, threshold_thresh, 255, CV_THRESH_BINARY_INV); // apply a threshold filter (binary inverse)

  imshow("threshold", out);

  median_filter(out); // apply a median filter on the image

  imshow("median", out);

  //Canny(out, out, canny_thresh, canny_thresh*2);

  return out;
}

// Extracts the Hue (different type of colors) of the image
Mat hsv_filter(const Mat& src) {
  Mat hsv_out;

  vector<Mat> channels;

  cvtColor(src, hsv_out, CV_BGR2HSV); // transform image to hsv
  split(hsv_out, channels); // split it into channels

  return channels[0]; // return Hue
}

// Realize a median Filter
void median_filter(Mat in) {
  if(median_filter_size < 3) { // filter size must be higher than 3
    return;
  } else if(median_filter_size % 2 == 0) { // filter size must be odd
    median_filter_size = median_filter_size - 1;
  }

  medianBlur(in, in, median_filter_size);
}

// Get the biggest contour
Mat biggest_contour(const Mat& in) {
  vector< vector<Point> > contours;
  vector<Vec4i> hierarchy;
  double biggest_area = 0.0;
  int index_biggest_contour = 0;
  Rect rect_hand;

  Mat out = Mat::zeros( in.size(), CV_8UC3 );

  // get the contours of the input image
  findContours(in, 
               contours, 
               hierarchy, 
               CV_RETR_EXTERNAL, 
               CV_CHAIN_APPROX_SIMPLE);
  
  // for all contours save the one with biggest area
  for(int i = 0; i < contours.size(); i++) { 
    double area = contourArea(contours[i]);
    if(area > biggest_area) {      
      biggest_area = area;
      index_biggest_contour = i;
      rect_hand = boundingRect(contours[i]);      
    }
  }
  
  // draw in the output image the biggest contour
  drawContours(out, 
               contours, 
               index_biggest_contour, 
               Scalar(255, 255, 255), 
               CV_FILLED, 
               8, 
               hierarchy);

  // add a bounding box of the hand in the source
  rectangle(src, rect_hand,  Scalar(0,255,0), 1, 8, 0);

  return out;
}