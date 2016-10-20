#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video/background_segm.hpp>


using namespace std;
using namespace cv;

void define_trackbars();
Mat color_detection(const Mat& src);
Mat hsv_filter(const Mat& src);
void median_filter(Mat in);

Mat biggest_contour(const Mat& in);

Mat src;
Mat out;
Mat fgMaskMOG2;

int margin = 20;
string filename;

int median_filter_size = 1;
int threshold_thresh = 22;
//int canny_thresh = 35;

Scalar lower = Scalar(0, 10, 60);
Scalar upper = Scalar(20, 150, 255);

string srcWindowName = "src";
string handWindowName = "hand";
string outWindowName = "output";

int main(int argc, char const *argv[]) {
  VideoCapture vidCap;

  char option;
  cout << "video or cam (v/c): ";
  cin >> option;

  if(option == 'v') {
    cout << "introduce the video to play: ";
    cin >> filename;

    vidCap = VideoCapture(filename);
  } else if(option == 'c') {
    vidCap = VideoCapture(0);
  } else {
    cerr << "option not found" << endl;
    return 1;
  }

  //Ptr<BackgroundSubtractor> pMOG2;
  //pMOG2 = createBackgroundSubtractorMOG2();  

  if(!vidCap.isOpened()) {
    cerr << "Could not open video capture!" << endl;
    return 1;
  }

  namedWindow(srcWindowName);
  namedWindow(handWindowName);
  namedWindow(outWindowName);

  define_trackbars();

  bool done = false;
  while(!done) {
    if(!vidCap.read(src)) {
      break;
    }

    if(option == 'c') {
      resize(src, src, Size(640, 360));
    }

    const Mat roi(src, 
                  Rect(margin, margin, src.cols - margin*2, src.rows - margin*2));

    char key = waitKey(1);
    switch(key) {
      case 27: 
        done = true;
        break;
    }

    out = color_detection(roi);    
    Mat contour = biggest_contour(out);
    imshow("contour", contour);

    //pMOG2->apply(roi, fgMaskMOG2);


    imshow(srcWindowName, src);
    //imshow("mog2", fgMaskMOG2);
    imshow(outWindowName, out);
  }

  return 0;
}

void define_trackbars() {
  //createTrackbar("canny thresh", outWindowName, &canny_thresh, 200);
  createTrackbar("threshold thresh", outWindowName, &threshold_thresh, 50);
  createTrackbar("median filter", outWindowName, &median_filter_size, 15);
}

Mat color_detection(const Mat& src) {
  Mat out;

  out = hsv_filter(src);

  imshow(handWindowName, out);

  threshold(out, out, threshold_thresh, 255, CV_THRESH_BINARY_INV);

  imshow("threshold", out);

  median_filter(out);

  imshow("median", out);

  //Canny(out, out, canny_thresh, canny_thresh*2);

  return out;
}

// Extracts the Hue (different type of colors) of the image
Mat hsv_filter(const Mat& src) {
  vector<Mat> channels;

  cvtColor(src, out, CV_BGR2HSV); 
  split(out, channels);

  return channels[0];
}

void median_filter(Mat in) {
  if(median_filter_size <= 3) {
    return;
  } else if(median_filter_size % 2 == 0) {
    median_filter_size = median_filter_size - 1;
  }

  medianBlur(in, in, median_filter_size);
}

Mat biggest_contour(const Mat& in) {
  vector< vector<Point> > contours;
  vector<Vec4i> hierarchy;
  double biggest_area = 0.0;
  int index_biggest_contour = 0;
  Rect rect_hand;

  Mat out = Mat::zeros( in.size(), CV_8UC3 );

  findContours(in, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
  
  for(int i = 0; i < contours.size(); i++) {
    double area = contourArea(contours[i]);
    if(area > biggest_area) {      
      biggest_area = area;
      index_biggest_contour = i;
      rect_hand = boundingRect(contours[i]);      
    }
  }
  
  drawContours(out, contours, index_biggest_contour, Scalar(255, 0, 0), CV_FILLED, 8, hierarchy);
  rectangle(src, rect_hand,  Scalar(0,255,0), 1, 8, 0);

  return out;
}