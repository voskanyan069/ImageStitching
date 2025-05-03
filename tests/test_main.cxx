#include <iostream>
#include <opencv2/opencv.hpp>

#include "psnr.hxx"
#include "ssim.hxx"

cv::Mat warpImg(cv::Mat img)
{
}

int main(int argc, char** argv)
{
    if ( argc < 4 )
    {
        std::cout << "Usage: " << argv[0] << " img1 img2 stitched" << std::endl;
        return -1;
    }
    cv::Mat img1 = cv::imread(argv[1]);
    cv::Mat stitchedResult = cv::imread(argv[2]); // your stitched result

    int cropWidth = std::min(img1.cols, stitchedResult.cols);
    int cropHeight = std::min(img1.rows, stitchedResult.rows);
    cv::Mat stitchedCropped = stitchedResult(cv::Rect(0, 0, cropWidth, cropHeight));
    cv::Mat img1Cropped = img1(cv::Rect(0, 0, cropWidth, cropHeight));

    // Step 2: Create mask of img1 (where it is not black)
    cv::Mat mask1;
    cv::cvtColor(img1Cropped, mask1, cv::COLOR_BGR2GRAY);
    cv::threshold(mask1, mask1, 1, 255, cv::THRESH_BINARY);

    // Step 3: Compute PSNR and SSIM using the mask
    double psnrOverlap = computePSNR(img1Cropped, stitchedCropped, mask1);
    cv::Scalar ssimOverlap = computeMSSIM(img1Cropped, stitchedCropped, mask1);

    // Step 4: Print
    std::cout << "PSNR (Overlap Area) = " << psnrOverlap << std::endl;
    std::cout << "SSIM (Overlap Area) = " << ssimOverlap << std::endl;

    return 0;
}
