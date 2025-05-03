#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

#include "image_stitching.hxx"
#include "image_processing.hxx"

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
    std::cout<< "Image1:Size: " << img1.cols << "x" << img1.rows << std::endl;
    std::cout<< "Image2:Size: " << img2.cols << "x" << img2.rows << std::endl;
    proc.FindMatches(gray1, gray2, matches, img1Kpts, img2Kpts,
            m_distanceRatio, m_keypointsCount);
    std::cout << "KeyPoints1:Size: " << img1Kpts.size() << std::endl;
    std::cout << "KeyPoints1:Size: " << img2Kpts.size() << std::endl;
    std::cout << "Matches:Size: " << matches.size() << std::endl;
    proc.TransformHomography(img1Kpts, img2Kpts, matches,
            homography, m_ransacValue);
    proc.TransformCorners(img1, img2, homography, allCorners);
    proc.WarpImages(img1, img2, homography, allCorners, result);
    std::cout << "Result:Size: " << result.cols << "x" <<
        result.rows << std::endl;
}

void Stitcher::Stitch2Images(cv::Mat* img1, cv::Mat* img2, cv::Mat* result)
{
    if (nullptr == img1 || nullptr == img2)
    {
        std::cerr << "Image file is empty" << std::endl;
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
    std::cout << "Stitch To Last: " << newImg << std::endl;
}

void Stitcher::SaveFile(const std::string& dir, const std::string& file,
        cv::Mat* result)
{
    if (nullptr == result)
    {
        std::cerr << "Result file is empty" << std::endl;
        return;
    }
    fs::path outputPath(dir);
    fs::path path = outputPath / file;
    m_lastStitched = result;
    std::cout << "Saving result file to: " << path.string() << std::endl;
    cv::imwrite(path.string(), *(result));
    std::cout << "Result file is saved: " << path.string() << std::endl;
}
