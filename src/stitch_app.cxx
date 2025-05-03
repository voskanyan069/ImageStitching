#include <iostream>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "stitch_app.hxx"
#include "image_stitching.hxx"

StitchApp::StitchApp()
    : m_bLogging(false)
    , m_bRecurseSearching(false)
    , m_keypointsCount(0)
    , m_distanceRatio(0)
    , m_ransacValue(0)
    , m_inputPath("")
    , m_outputPath("")
    , m_stitcher(nullptr)
{
}

StitchApp::~StitchApp()
{
    if (nullptr != m_stitcher)
    {
        delete m_stitcher;
    }
}

void StitchApp::createArgList(po::options_description& desc)
{
    desc.add_options()
        ("help,h", "Produce help message")
        ("input,i", po::value<std::string>()->required(),
         "Source image files directory path")
        ("output,o", po::value<std::string>()->required(),
         "Output image files directory path")
        ("keypoints,k",po::value<int>()->default_value(10000),"Keypoints count")
        ("ratio", po::value<float>()->default_value(0.75),
         "Distance filter ratio")
        ("RANSAC,R", po::value<float>()->default_value(1.5), "RANSAC value")
        ("recurse,r", "Search for images recursively");
        ("logging,l", "Enable logging");
}

bool StitchApp::storeArguments(int argc, char** argv,
        po::options_description& desc)
{
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return false;
    }
    if (vm.count("logging"))
    {
        m_bLogging = true;
    }
    if (vm.count("recurse"))
    {
        m_bRecurseSearching = true;
    }
    po::notify(vm);
    m_inputPath = vm["input"].as<std::string>();
    m_outputPath = vm["output"].as<std::string>();
    m_keypointsCount = vm["keypoints"].as<int>();
    m_distanceRatio = vm["ratio"].as<float>();
    m_ransacValue = vm["RANSAC"].as<float>();
    return true;
}

int StitchApp::ParseArgs(int argc, char** argv)
{
    po::options_description desc("Allowed options");
    createArgList(desc);
    try
    {
        if ( storeArguments(argc, argv, desc) )
        {
            return 0;
        }
        return 1;
    }
    catch ( const po::error& e )
    {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return -1;
    }
}

int StitchApp::loadSourceFiles(ImageNames& files)
{
    fs::path path(m_inputPath);
    if (!fs::exists(path) || !fs::is_directory(path))
    {
        return -1;
    }
    if (m_bRecurseSearching)
    {
        for (const auto& entry : fs::recursive_directory_iterator(path))
        {
            if (fs::is_regular_file(entry.status()))
            {
                files.push_back(entry.path().string());
            }
        }
    }
    else
    {
        for (const auto& entry : fs::directory_iterator(path))
        {
            if (fs::is_regular_file(entry.status()))
            {
                files.push_back(entry.path().string());
            }
        }
    }
    return 0;
}

void sortFilenames(ImageNames& files)
{
    std::sort(files.begin(), files.end(), [](const std::string& a,
                const std::string& b)
    {
        fs::path path_a(a);
        fs::path path_b(b);
        return path_a.string() < path_b.string();
    });
}

void StitchApp::stitch2Images(const std::string& img1, const std::string& img2)
{
    cv::Mat* resultFile = new cv::Mat();
    m_stitcher->Stitch2Images(img1, img2, resultFile);
    m_stitcher->SaveFile(m_outputPath, "result_1.jpg", resultFile);
    if (nullptr != resultFile)
    {
        delete resultFile;
    }
}

void StitchApp::stitchImages(ImageNames& inputFiles)
{
    cv::Mat* resultFile = new cv::Mat();
    if (inputFiles.size() < 3)
    {
        m_stitcher->Stitch2Images(inputFiles[0], inputFiles[1], resultFile);
        m_stitcher->SaveFile(m_outputPath, "result_1.jpg", resultFile);
        if (nullptr != resultFile)
        {
            delete resultFile;
        }
        return;
    }
    m_stitcher->Stitch2Images(inputFiles[0], inputFiles[1], resultFile);
    m_stitcher->SaveFile(m_outputPath, "result_1.jpg", resultFile);
    for (int i = 2; i < inputFiles.size(); ++i)
    {
        m_stitcher->StitchToLastResult(inputFiles[i], resultFile);
        std::string resName = "result_" + std::to_string(i) + ".jpg";
        m_stitcher->SaveFile(m_outputPath, resName, resultFile);
    }
    if (nullptr != resultFile)
    {
        delete resultFile;
    }
}

void StitchApp::stitch(ImageNames& inputFiles)
{
    if (inputFiles.size() < 2)
    {
        std::cerr << "Images list is empty" << std::endl;
        exit(-1);
    }
    if (inputFiles.size() == 2)
    {
        stitch2Images(inputFiles[0], inputFiles[1]);
        return;
    }
    stitchImages(inputFiles);
}

int StitchApp::Exec()
{
    m_stitcher = new Stitcher(m_distanceRatio, m_keypointsCount, m_ransacValue);
    ImageNames inputFiles;
    if ( 0 != loadSourceFiles(inputFiles) )
    {
        std::cout << "Invalid directory path: " + m_inputPath << std::endl;
    }
    sortFilenames(inputFiles);
    stitch(inputFiles);
    return 0;
}
