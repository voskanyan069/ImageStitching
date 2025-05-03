#ifndef TEST_PSNR_HXX
#define TEST_PSNR_HXX

/*
 * PSNR VALUE
 * > 40 dB  | Perfect
 * 30–40 dB | Good
 * 20–30 dB | Normal
 * < 20 dB  | Bad
 */

#include <opencv2/opencv.hpp>
#include <iostream>

double computePSNR(const cv::Mat& I1, const cv::Mat& I2, const cv::Mat& mask)
{
    cv::Mat s1;
    cv::absdiff(I1, I2, s1);
    s1.convertTo(s1, CV_32F);
    s1 = s1.mul(s1);

    if (mask.empty())
        mask = cv::Mat::ones(I1.size(), CV_8U);

    double sse = cv::sum(s1.mul(mask))[0] + cv::sum(s1.mul(mask))[1] + cv::sum(s1.mul(mask))[2];
    int totalPixels = cv::countNonZero(mask) * I1.channels();

    if (sse <= 1e-10 || totalPixels == 0)
        return 100;

    double mse = sse / totalPixels;
    double psnr = 10.0 * log10((255 * 255) / mse);
    return psnr;
}

#endif // TEST_PSNR_HXX
