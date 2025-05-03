#ifndef TEST_SSIM_HXX
#define TEST_SSIM_HXX

/*
 * 1.0      | Perfect matching
 * > 0.9    | Very high matching
 * 0.7–0.9  | Medium matching
 * < 0.5    | Low matching
 */

#include <opencv2/opencv.hpp>
#include <iostream>

cv::Scalar computeMSSIM(const cv::Mat& i1, const cv::Mat& i2, const cv::Mat& mask)
{
    const double C1 = 6.5025, C2 = 58.5225;
    int d = CV_32F;

    cv::Mat I1, I2;
    i1.convertTo(I1, d);
    i2.convertTo(I2, d);

    cv::Mat mu1, mu2;
    cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1_2 = mu1.mul(mu1);
    cv::Mat mu2_2 = mu2.mul(mu2);
    cv::Mat mu1_mu2 = mu1.mul(mu2);

    cv::Mat sigma1_2, sigma2_2, sigma12;
    cv::GaussianBlur(I1.mul(I1), sigma1_2, cv::Size(11, 11), 1.5);
    sigma1_2 -= mu1_2;
    cv::GaussianBlur(I2.mul(I2), sigma2_2, cv::Size(11, 11), 1.5);
    sigma2_2 -= mu2_2;
    cv::GaussianBlur(I1.mul(I2), sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1_mu2;

    cv::Mat t1, t2, t3;
    t1 = 2 * mu1_mu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);

    t1 = mu1_2 + mu2_2 + C1;
    t2 = sigma1_2 + sigma2_2 + C2;
    t1 = t1.mul(t2);

    cv::Mat ssim_map;
    cv::divide(t3, t1, ssim_map);

    if (mask.empty())
        return cv::mean(ssim_map);

    // Apply mask
    std::vector<cv::Mat> channels(3);
    cv::split(ssim_map, channels);

    cv::Scalar meanSSIM;
    for (int i = 0; i < 3; ++i)
        meanSSIM[i] = cv::mean(channels[i], mask)[0];

    return meanSSIM;
}

#endif // TEST_SSIM_HXX
