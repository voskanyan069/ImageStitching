#include <iostream>

#include <opencv2/opencv.hpp>

#include "stitch_app.hxx"

int main(int argc, char** argv)
{
    StitchApp app;
    int pec = app.ParseArgs(argc, argv);
    if ( 0 != pec )
    {
        return pec;
    }
    int ec = app.Exec();
    return ec;
}
