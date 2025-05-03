#ifndef __IMAGE_PROCESSING_HXX__
#define __IMAGE_PROCESSING_HXX__

#include <vector>

namespace cv {
    class Mat;
    class DMatch;
    class KeyPoint;
    template <typename T> class Ptr;
}; // cv

typedef std::vector<cv::DMatch> DMatchVec;
typedef std::vector<cv::Point2f> Point2fVec;
typedef std::vector<cv::KeyPoint> KeyPoints;

class ImageProcessing
{
public:
    ImageProcessing();

public:
    void MakeGray(const cv::Mat& img, cv::Mat& res) const;
    void FindMatches(const cv::Mat& img1, const cv::Mat& img2,
            DMatchVec& matches, KeyPoints& img1Kpts, KeyPoints& img2Kpts,
            float ratio = 0.75, int keypointsCount = 10000) const;
    void TransformHomography(const KeyPoints& kp1, const KeyPoints& kp2,
            const DMatchVec& matches, cv::Mat& homography,
            float ransac = 1.5) const;
    void TransformCorners(const cv::Mat& img1, const cv::Mat& img2,
            const cv::Mat& homography, Point2fVec& allCorners) const;
    void WarpImages(const cv::Mat& img1, const cv::Mat& img2,
            const cv::Mat& homography, const Point2fVec& allCorners,
            cv::Mat& res) const;

private:
    void computeCorners(const cv::Mat& img, Point2fVec& corners) const;

private:
    cv::Ptr<cv::CLAHE> m_clahe;
};

#endif // __IMAGE_PROCESSING_HXX__
