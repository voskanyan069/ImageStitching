#include <opencv2/opencv.hpp>

#include "image_processing.hxx"

ImageProcessing::ImageProcessing()
    : m_clahe(cv::createCLAHE())
{
}

void ImageProcessing::MakeGray(const cv::Mat& img, cv::Mat& res) const
{
    cv::cvtColor(img, res, cv::COLOR_BGR2GRAY);
    m_clahe->setClipLimit(2.0);
    m_clahe->setTilesGridSize(cv::Size(8, 8));
    m_clahe->apply(res, res);
}

void ImageProcessing::FindMatches(const cv::Mat& img1, const cv::Mat& img2,
        DMatchVec& matches, KeyPoints& img1Keypoints, KeyPoints& img2Keypoints,
        float ratio, int keypointsCount) const
{
    cv::Mat img1Desc;
    cv::Mat img2Desc;
    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<DMatchVec> initialMatches;
    cv::Ptr<cv::SIFT> detector = cv::SIFT::create(keypointsCount);
    detector->detectAndCompute(img1, cv::noArray(), img1Keypoints, img1Desc);
    detector->detectAndCompute(img2, cv::noArray(), img2Keypoints, img2Desc);
    if (img1Desc.type() != CV_32F)
    {
        img1Desc.convertTo(img1Desc, CV_32F);
        img2Desc.convertTo(img2Desc, CV_32F);
    }
    matcher.knnMatch(img1Desc, img2Desc, initialMatches, 2);
    for (const auto& m : initialMatches)
    {
        if (m[0].distance < ratio * m[1].distance)
        {
            matches.push_back(m[0]);
        }
    }
}

void ImageProcessing::TransformHomography(const KeyPoints& img1Keypoints,
        const KeyPoints& img2Keypoints, const DMatchVec& matches,
        cv::Mat& homography, float ransacVal) const
{
    Point2fVec pts1;
    Point2fVec pts2;
    for (const auto& m : matches)
    {
        pts1.push_back(img1Keypoints[m.queryIdx].pt);
        pts2.push_back(img2Keypoints[m.trainIdx].pt);
    }
    cv::Mat inliers;
    homography = cv::findHomography(pts2, pts1, inliers, cv::RANSAC, ransacVal);
    if (homography.empty())
    {
        std::cerr << "Homography estimation failed" << std::endl;
    }
}

void ImageProcessing::computeCorners(const cv::Mat& img,
        Point2fVec& corners) const
{
    Point2fVec tmpCorners = {
        cv::Point2f(0, 0), cv::Point2f((float)(img.cols), 0),
        cv::Point2f((float)(img.cols), (float)(img.rows)),
        cv::Point2f(0, (float)(img.rows))
    };
    corners.insert(corners.end(), tmpCorners.begin(), tmpCorners.end());
}

void ImageProcessing::TransformCorners(const cv::Mat& img1, const cv::Mat& img2,
        const cv::Mat& homography, Point2fVec& allCorners) const
{
    Point2fVec corners1;
    Point2fVec corners2;
    Point2fVec warpedCorners2;
    computeCorners(img1, corners1);
    computeCorners(img2, corners2);
    cv::perspectiveTransform(corners2, warpedCorners2, homography);
    allCorners.insert(allCorners.end(), warpedCorners2.begin(),
            warpedCorners2.end());
    allCorners.insert(allCorners.end(), corners1.begin(), corners1.end());
}

void ImageProcessing::WarpImages(const cv::Mat& img1, const cv::Mat& img2,
        const cv::Mat& homography, const Point2fVec& allCorners,
        cv::Mat& res) const
{
    float minX = FLT_MAX;
    float minY = FLT_MAX;
    float maxX = -FLT_MAX;
    float maxY = -FLT_MAX;
    for (const auto& pt : allCorners)
    {
        minX = std::min(minX, pt.x);
        minY = std::min(minY, pt.y);
        maxX = std::max(maxX, pt.x);
        maxY = std::max(maxY, pt.y);
    }
    int offsetX = (minX < 0) ? - static_cast<int>(std::floor(minX)) : 0;
    int offsetY = (minY < 0) ? - static_cast<int>(std::floor(minY)) : 0;
    int width = static_cast<int>(std::ceil(maxX - minX));
    int height = static_cast<int>(std::ceil(maxY - minY));
    cv::Mat stitched(height, width, img1.type(), cv::Scalar::all(0));
    cv::Mat roi1 = stitched(cv::Rect(offsetX, offsetY, img1.cols, img1.rows));
    img1.copyTo(roi1);
    cv::Mat homographyOffset = homography.clone();
    homographyOffset.convertTo(homographyOffset, CV_64F);
    homographyOffset.at<double>(0, 2) += offsetX;
    homographyOffset.at<double>(1, 2) += offsetY;
    cv::Mat warpedImg2;
    cv::warpPerspective(img2, stitched, homographyOffset, stitched.size(),
            cv::INTER_LINEAR, cv::BORDER_TRANSPARENT);
    stitched.copyTo(res);
}
