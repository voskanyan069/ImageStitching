#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "image_stitching.hxx"
#include "image_processing.hxx"
#include "logging.hxx"

namespace fs = boost::filesystem;

Stitcher::Stitcher(float distanceRatio, int keypointsCount, float ransacValue)
    : m_lastStitched(nullptr)
    , m_distanceRatio(distanceRatio)
    , m_keypointsCount(keypointsCount)
    , m_ransacValue(ransacValue)
{
}

void Stitcher::stitch(const ImageProcessing& proc, const cv::Mat& img1,
        const cv::Mat& img2, const cv::Mat& gray1, const cv::Mat& gray2,
        cv::Mat& result)
{
    DMatchVec matches;
    KeyPoints img1Kpts;
    KeyPoints img2Kpts;
    cv::Mat homography;
    Point2fVec allCorners;
    Logging::LogInfo("Image1:Size: %dx%d", img1.cols, img1.rows);
    Logging::LogInfo("Image2:Size: %dx%d", img2.cols, img2.rows);
    proc.FindMatches(gray1, gray2, matches, img1Kpts, img2Kpts,
            m_distanceRatio, m_keypointsCount);
    Logging::LogInfo("KeyPoints1:Size: %d", img1Kpts.size());
    Logging::LogInfo("KeyPoints2:Size: %d", img2Kpts.size());
    Logging::LogInfo("Matches:Size: %d", matches.size());
    proc.TransformHomography(img1Kpts, img2Kpts, matches,
            homography, m_ransacValue);
    proc.TransformCorners(img1, img2, homography, allCorners);
    proc.WarpImages(img1, img2, homography, allCorners, result);
    Logging::LogInfo("Result:Size: %dx%d", result.cols, result.rows);
}

void Stitcher::Stitch2Images(cv::Mat* img1, cv::Mat* img2, cv::Mat* result)
{
    if (nullptr == img1 || nullptr == img2)
    {
        Logging::LogError("Image file is empty");
    }
    ImageProcessing proc;
    cv::Mat gray1;
    cv::Mat gray2;
    proc.MakeGray(*img1, gray1);
    proc.MakeGray(*img2, gray2);
    stitch(proc, *img1, *img2, gray1, gray2, *result);
}

void Stitcher::StitchToLastResult(cv::Mat* newImg, cv::Mat* result)
{
    Stitch2Images(m_lastStitched, newImg, result);
}

void Stitcher::Stitch2Images(const std::string& img1, const std::string& img2,
        cv::Mat* result)
{
    cv::Mat srcFile1 = cv::imread(img1);
    cv::Mat srcFile2 = cv::imread(img2);
    Stitch2Images(&srcFile1, &srcFile2, result);
}

void Stitcher::StitchToLastResult(const std::string& newImg, cv::Mat* result)
{
    cv::Mat img = cv::imread(newImg);
    StitchToLastResult(&img, result);
    Logging::LogInfo("Stitch To Last: %s", newImg.c_str());
}

void Stitcher::SaveFile(const std::string& dir, const std::string& file,
        cv::Mat* result)
{
    if (nullptr == result)
    {
        Logging::LogError("Result file is empty");
        return;
    }
    fs::path outputPath(dir);
    fs::path path = outputPath / file;
    m_lastStitched = result;
    Logging::LogInfo("Saving result file to: %s", path.string().c_str());
    cv::imwrite(path.string(), *(result));
    Logging::LogInfo("Result file is saved: %s", path.string().c_str());
}
