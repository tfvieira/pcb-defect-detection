#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

cv::Mat imgAlignment(Mat &img1, Mat &img2)
{
    const int warp_mode = MOTION_EUCLIDEAN;

    // Set a 2x3 or 3x3 warp matrix depending on the motion model.
    Mat warp_matrix;

    // Initialize the matrix to identity
    if ( warp_mode == MOTION_HOMOGRAPHY )
        warp_matrix = Mat::eye(3, 3, CV_32F);
    else
        warp_matrix = Mat::eye(2, 3, CV_32F);

    // Specify the number of iterations.
    int number_of_iterations = 5000;

    // Specify the threshold of the increment
    // in the correlation coefficient between two iterations
    double termination_eps = 1e-10;

    // Define termination criteria
    TermCriteria criteria (TermCriteria::COUNT+TermCriteria::EPS, number_of_iterations, termination_eps);

    // Run the ECC algorithm. The results are stored in warp_matrix.
    findTransformECC(
                     img1,
                     img2,
                     warp_matrix,
                     warp_mode,
                     criteria
                 );

    // Storage for warped image.
    Mat img2_aligned;

    if (warp_mode != MOTION_HOMOGRAPHY)
        // Use warpAffine for Translation, Euclidean and Affine
        warpAffine(img2, img2_aligned, warp_matrix, img1.size(), INTER_LINEAR + WARP_INVERSE_MAP);
    else
        // Use warpPerspective for Homography
        warpPerspective (img2, img2_aligned, warp_matrix, img1.size(),INTER_LINEAR + WARP_INVERSE_MAP);

    return img2_aligned;
}

cv::Mat imgPreprocessor(Mat &img)
{
    Mat result;
    threshold(img, result, 47, 255, CV_THRESH_BINARY);
    //threshold(img, result, 127, 255, CV_THRESH_BINARY);
    //result = Scalar(255) - result;
    return result;
}

vector<Mat> imgSegmentation(Mat &img)
{
    vector<Mat> segments;

    //Hole detection
    Mat kernel1 = getStructuringElement(0, Size(5, 5), Point(2, 2));
    Mat detHole1, detHole;
    morphologyEx(img, detHole1, MORPH_CLOSE, kernel1);
    //detHole1 = img;
    bitwise_xor(detHole1, img, detHole);

    //Square pad segmentation
    Mat kernel2 = getStructuringElement(0, Size(31, 31), Point(15, 15));
    Mat squarePad1, squarePad;
    morphologyEx(detHole1, squarePad1, MORPH_OPEN, kernel2);
    dilate(squarePad1, squarePad, img);

    //Hole pad segmentation
    Mat holePad1, holePad2, holePad;
    Mat kernel3 = getStructuringElement(0, Size(7, 7), Point(3, 3));
    bitwise_xor(detHole1, squarePad1, holePad1);
    morphologyEx(holePad1, holePad2, MORPH_OPEN, kernel3);
    dilate(holePad1, holePad, holePad2);

    //Rectangular pad segmentation
    Mat rectPad1, rectPad2, rectPad;
    bitwise_xor(holePad1, holePad2, rectPad1);
    morphologyEx(rectPad1, rectPad2, MORPH_OPEN, kernel1);
    dilate(rectPad2, rectPad, img);

    //Thick line segmentation
    Mat thickLine1, thickLine2;
    bitwise_xor(rectPad1, rectPad2, thickLine1);
    Mat kernel4 = getStructuringElement(0, Size(3, 3), Point(1, 1));
    morphologyEx(thickLine1, thickLine2, MORPH_OPEN, kernel4);
    imshow("img", thickLine1);
    imshow("img2", thickLine2);

    return segments;
}

int main()
{
    namedWindow("test", WINDOW_KEEPRATIO);
    namedWindow("reference", WINDOW_KEEPRATIO);
    namedWindow("img", WINDOW_KEEPRATIO);
    namedWindow("img2", WINDOW_KEEPRATIO);

    Mat testImg = imread("test_pcb.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat refImg = imread("ref_pcb.png", CV_LOAD_IMAGE_GRAYSCALE);
    Mat img, img2;

    if(!testImg.data)
    {
        cout <<  "Could not open or find the test image" << std::endl ;
        return -1;
    }
    if(!refImg.data)
    {
        cout <<  "Could not open or find the reference image" << std::endl ;
        return -1;
    }


    resize(refImg, refImg, testImg.size(), 0, 0, INTER_LINEAR);

    //refImg = imgAlignment(testImg, refImg);

    testImg = imgPreprocessor(testImg);
    refImg = imgPreprocessor(refImg);

    imgSegmentation(testImg);

    //-------Testing--------
    //*
    int x = 8;
    //createTrackbar( "x:", "img", &x, 55, 0);
    /*
    //Hole detection
    Mat kernel1 = getStructuringElement(0, Size(5, 5), Point(2, 2));
    Mat detHole1, detHole;
    morphologyEx(testImg, detHole1, MORPH_CLOSE, kernel1);
    //detHole1 = img;
    bitwise_xor(detHole1, testImg, detHole);

    //Square pad segmentation
    Mat kernel2 = getStructuringElement(0, Size(31, 31), Point(15, 15));
    Mat squarePad1, squarePad;
    morphologyEx(detHole1, squarePad1, MORPH_OPEN, kernel2);
    dilate(squarePad1, squarePad, testImg);

    //Hole pad segmentation
    Mat holePad1, holePad2, holePad;
    Mat kernel3 = getStructuringElement(0, Size(7, 7), Point(3, 3));
    bitwise_xor(detHole1, squarePad1, holePad1);
    morphologyEx(holePad1, holePad2, MORPH_OPEN, kernel3);
    dilate(holePad1, holePad, holePad2);

    //Rectangular pad segmentation
    Mat rectPad1, rectPad2, rectPad;
    bitwise_xor(holePad1, holePad2, rectPad1);
    morphologyEx(rectPad1, rectPad2, MORPH_OPEN, kernel1);
    dilate(rectPad2, rectPad, testImg);

    //Thick line segmentation
    Mat thickLine1, thickLine2, thickLine;
    bitwise_xor(rectPad1, rectPad2, thickLine1);
    Mat kernel4 = getStructuringElement(0, Size(3, 3), Point(1, 1));
    morphologyEx(thickLine1, thickLine2, MORPH_OPEN, kernel4);
    //*/

    for(;;)
    {
        //Mat kernel5 = getStructuringElement(2, Size(x*2 +1, x*2 +1), Point(x, x));
        //morphologyEx(testImg, detHole1, MORPH_CLOSE, kernel1);

        //morphologyEx(thickLine2, thickLine, MORPH_CLOSE, kernel5);

        imshow("test", testImg);
        imshow("reference", refImg);
        //imshow("img", thickLine);
        //imshow("img2", thickLine2);
        if((char) waitKey(1) == 'q') break;
    }

    return 0;
}
