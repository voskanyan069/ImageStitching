#ifndef __IMAGE_STITCHING_HXX__
#define __IMAGE_STITCHING_HXX__

#include <string>

namespace cv {
    class Mat;
}; // cv

class ImageProcessing;

class Stitcher
{
public:
    Stitcher(float distanceRatio, int keypointsCount, float ransacValue);
    ~Stitcher() = default;

public:
    void Stitch2Images(cv::Mat* img1, cv::Mat* img2, cv::Mat* result);
    void Stitch2Images(const std::string& img1, const std::string& img2,
            cv::Mat* result);
    void StitchToLastResult(cv::Mat* newImg, cv::Mat* result);
    void StitchToLastResult(const std::string& newImg, cv::Mat* result);
    void SaveFile(const std::string& dir, const std::string& file,
            cv::Mat* result);

private:
    void stitch(const ImageProcessing& proc, const cv::Mat& img1,
            const cv::Mat& img2, const cv::Mat& gray1, const cv::Mat& gray2,
            cv::Mat& result);

private:
    int m_keypointsCount;
    float m_distanceRatio;
    float m_ransacValue;
    cv::Mat* m_lastStitched;
};

#endif // __IMAGE_STITCHING_HXX__
