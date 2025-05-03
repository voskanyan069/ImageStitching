#ifndef __STITCH_APP_HXX__
#define __STITCH_APP_HXX__

#include <string>
#include <vector>

namespace boost { namespace filesystem {

}}; // boost::filesystem
namespace boost { namespace program_options {
    class options_description;
}}; // boost::program_options

namespace cv {
    class Mat;
}; // cv

class Stitcher;

namespace fs = boost::filesystem;
namespace po = boost::program_options;

typedef std::vector<std::string> ImageNames;

class StitchApp
{
public:
    StitchApp();
    ~StitchApp();

    StitchApp(StitchApp&&) = delete;
    StitchApp(const StitchApp&) = delete;

public:
    int ParseArgs(int argc, char** argv);
    int Exec();

private:
    void createArgList(po::options_description& desc);
    bool storeArguments(int argc, char** argv, po::options_description&);
    int loadSourceFiles(ImageNames& files);
    void saveFile(const std::string& filename, cv::Mat& image);
    void stitch(ImageNames& inputFiles);
    void stitchImages(ImageNames& inputFiles);
    void stitch2Images(const std::string& src1, const std::string& src2);

private:
    bool m_bLogging;
    bool m_bRecurseSearching;
    int m_keypointsCount;
    float m_distanceRatio;
    float m_ransacValue;
    std::string m_inputPath;
    std::string m_outputPath;
    Stitcher* m_stitcher;
};

#endif // __STITCH_APP_HXX__
