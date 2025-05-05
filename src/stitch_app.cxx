#include <iostream>

#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "stitch_app.hxx"
#include "image_stitching.hxx"
#include "logging.hxx"

StitchApp::StitchApp()
    : m_bQuiet(false)
    , m_bRecurseSearching(false)
    , m_keypointsCount(0)
    , m_distanceRatio(0)
    , m_ransacValue(0)
    , m_inputPath("")
    , m_outputPath("")
    , m_logfilePath("")
    , m_stitcher(nullptr)
    , m_logfileStream(nullptr)
{
}

StitchApp::~StitchApp()
{
    if (nullptr != m_stitcher)
    {
        delete m_stitcher;
    }
    closeLogfile();
}

void StitchApp::createArgList(po::options_description& desc)
{
    desc.add_options()
        ("help,h", "Produce help message")
        ("quiet,q", "Quiet (No output)")
        ("logfile,l", po::value<std::string>()->default_value(""),
         "Redirect output to the logfile")
        ("input,i", po::value<std::string>()->required(),
         "Source image files directory path")
        ("output,o", po::value<std::string>()->required(),
         "Output image files directory path")
        ("keypoints,k",po::value<int>()->default_value(10000),"Keypoints count")
        ("ratio", po::value<float>()->default_value(0.75),
         "Distance filter ratio")
        ("RANSAC,R", po::value<float>()->default_value(1.5), "RANSAC value")
        ("recurse,r", "Search for images recursively");
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
    if (vm.count("quiet"))
    {
        m_bQuiet = true;
    }
    if (vm.count("recurse"))
    {
        m_bRecurseSearching = true;
    }
    po::notify(vm);
    m_inputPath = vm["input"].as<std::string>();
    m_outputPath = vm["output"].as<std::string>();
    m_logfilePath = vm["logfile"].as<std::string>();
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
        Logging::LogError("Failed to parse arguments: %s", e.what());
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
        Logging::LogError("Images list is empty");
        exit(-1);
    }
    if (inputFiles.size() == 2)
    {
        stitch2Images(inputFiles[0], inputFiles[1]);
        return;
    }
    stitchImages(inputFiles);
}

void StitchApp::initLogging()
{
    if ( !m_logfilePath.empty() )
    {
        if ( fs::exists(m_logfilePath) )
        {
            if ( !fs::is_directory(m_logfilePath) )
            {
                Logging::LogWarn("The logfile aleady exists (overwriting)");
            }
            else
            {
                Logging::LogError("In your logfile path was located directory");
                exit(-2);
            }
        }
        m_logfileStream = new std::ofstream(m_logfilePath);
        if ( m_logfileStream->is_open() )
        {
            Logging::SetOutputStream(m_logfileStream);
        }
    }
}

void StitchApp::checkForOutputDir()
{
    if ( fs::exists(m_outputPath) )
    {
        if ( !fs::is_directory(m_outputPath) )
        {
            Logging::LogError("Output path is not a directory: %s",
                    m_outputPath.c_str());
            exit(-3);
        }
    }
    else
    {
        Logging::LogError("Output path does not exists: %s",
                m_outputPath.c_str());
        exit(-4);
    }
}

void StitchApp::loadFiles(ImageNames& inputFiles)
{
    if ( 0 != loadSourceFiles(inputFiles) )
    {
        Logging::LogError("Invalid input path: %s", m_inputPath.c_str());
        exit(-5);
    }
    sortFilenames(inputFiles);
}

void StitchApp::closeLogfile()
{
    if ( nullptr != m_logfileStream )
    {
        if ( m_logfileStream->is_open() )
        {
            m_logfileStream->close();
        }
        delete m_logfileStream;
        Logging::UnsetOutputStream();
    }
}

int StitchApp::Exec()
{
    int ec = 0;
    if ( m_bQuiet )
    {
        Logging::DisableLogging();
    }
    else
    {
        initLogging();
    }
    m_stitcher = new Stitcher(m_distanceRatio, m_keypointsCount, m_ransacValue);
    checkForOutputDir();
    ImageNames inputFiles;
    loadFiles(inputFiles);
    stitch(inputFiles);
    return 0;
}
