/*****************************************************************************
*
* Copyright (C) 2022 LIPS Corp.
*
* This simple viewer example draws depth/IR/color frames using OpenCV highgui
*
******************************************************************************/

// STL Headers
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

// OpenNI2 Headers
#include <OpenNI.h>

// OpenCV Headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace openni;
using namespace cv;

enum showOp
{
    DEPTH = 1,
    COLOR = 2,
    IR = 4
};

const int MAX_DEPTH = 10000; //10000mm = 10m

int Xres = 0;
int Yres = 0;
unsigned short Zres = 0;

void onMouse( int Event, int x, int y, int flags, void* param )
{
    if ( Event == EVENT_MOUSEMOVE )
    {
        Xres = x;
        Yres = y;
    }
}

int getUserInput()
{
    int option = ( DEPTH + COLOR ); //default Depth & RGB
    std::cout << endl;
    std::cout << "1) Depth only" << endl;
    std::cout << "2) Color only" << endl;
    std::cout << "3) IR only" << endl;
    std::cout << "4) Depth and Color" << endl;
    std::cout << "5) Depth and IR" << endl;
    std::cout << "6) Color and IR" << endl;
    std::cout << "7) All" << endl;
    std::cout << "0) Exit" << endl;
    std::cout << "Please input your choice : ";
    std::cin >> option;
    switch ( option )
    {
        case 1:
            return DEPTH;
        case 2:
            return COLOR;
        case 3:
            return IR;
        case 4:
            return ( DEPTH + COLOR );
        case 5:
            return ( DEPTH + IR );
        case 6:
            return ( COLOR + IR );
        case 7:
            return ( DEPTH + COLOR + IR );
        case 0:
            return -1;
        default:
            return getUserInput();
    };
}

double calcFPS( int64 startTick, uint64 count )
{
    if ( count < 150 )
    {
        return 0.0f;
    }
    else
    {
        double execute_time = ( double )( cv::getTickCount() - startTick ) / cv::getTickFrequency();
        return ( count / execute_time );
    }
}

void printVideoMode ( const VideoMode &mode )
{
    int xRes = mode.getResolutionX();
    int yRes = mode.getResolutionY();
    int fps = mode.getFps();

    std::cout << "Resolution ( " << xRes << " x " << yRes << " ), FPS = " << fps << endl;
}

//Refer to OpenNI2 samples common OniSampleUtilities
void calculateHistogram( float* pHistogram, int histogramSize, const openni::VideoFrameRef& frame )
{
    const openni::DepthPixel* pDepth = ( const openni::DepthPixel* )frame.getData();

    // Calculate the accumulative histogram (the yellow display...)
    memset( pHistogram, 0, histogramSize * sizeof( float ) );
    int restOfRow = frame.getStrideInBytes() / sizeof( openni::DepthPixel ) - frame.getWidth();
    int height = frame.getHeight();
    int width = frame.getWidth();

    unsigned int nNumberOfPoints = 0;
    for ( int y = 0; y < height; ++y )
    {
        for ( int x = 0; x < width; ++x, ++pDepth )
        {
            if ( *pDepth != 0 )
            {
                pHistogram[*pDepth]++;
                nNumberOfPoints++;
            }
        }
        pDepth += restOfRow;
    }
    for ( int nIndex = 1; nIndex < histogramSize; nIndex++ )
    {
        pHistogram[nIndex] += pHistogram[nIndex - 1];
    }
    if ( nNumberOfPoints )
    {
        for ( int nIndex = 1; nIndex < histogramSize; nIndex++ )
        {
            pHistogram[nIndex] = ( 256 * ( 1.0f - ( pHistogram[nIndex] / nNumberOfPoints ) ) );
        }
    }
}

void printHelp()
{
    cout << endl;
    cout << "=========== SimpleViewer-cv ============" << endl;
    cout << " q/Q : Quit program" << endl;
    cout << " c/C : Capture frame and save it to file" << endl;
    cout << " m/M : Mirroring the image" << endl;
    cout << " i/I : More pixel information" << endl;
    cout << " f/F : Depth-Color frame sync on/off" << endl;
    cout << "========================================" << endl;
    cout << endl;
}

int main( int argc, char* argv[] )
{
    int option = getUserInput();

    if ( -1 == option )
    {
        std::cout << "Exit program. Bye Bye!" << endl;
        return 0;
    }

    if ( STATUS_OK != OpenNI::initialize() )
    {
        std::cout << "After initialization: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    Device device;

    if ( STATUS_OK != device.open( ANY_DEVICE ) )
    {
        std::cout << "Cannot open device: " << OpenNI::getExtendedError() << endl;
        return 1;
    }

    VideoMode vmDepth;
    VideoMode vmColor;
    VideoMode vmIR;

    VideoStream vsDepth;
    VideoStream vsColor;
    VideoStream vsIR;

    VideoFrameRef depth_frame;
    VideoFrameRef color_frame;
    VideoFrameRef ir_frame;

    uint64 depth_frame_count = 0;
    uint64 color_frame_count = 0;
    uint64 ir_frame_count = 0;

    if ( option & DEPTH )
    {
        std::cout << "Depth stream is selected." << endl;

        const SensorInfo* sinfo = device.getSensorInfo( SENSOR_DEPTH );
        if ( sinfo == NULL )
        {
            std::cout << "This device cannot support depth sensor: " << OpenNI::getExtendedError() << endl;
        }
        else
        {
            if ( STATUS_OK != vsDepth.create( device, SENSOR_DEPTH ) )
            {
                std::cout << "Cannot create depth stream on device: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                if ( STATUS_OK != vsDepth.start() )
                {
                    std::cout << "Cannot start depth stream on device: " << OpenNI::getExtendedError() << endl;
                }
                if ( vsDepth.isValid() )
                {
                    std::cout << "Depth stream is starting ..." << endl;
                    vmDepth = vsDepth.getVideoMode();
                    printVideoMode( vmDepth );
                }
            }
        }
    }

    if ( option & COLOR )
    {
        std::cout << "Color stream is selected." << endl;

        const SensorInfo* sinfo = device.getSensorInfo( SENSOR_COLOR );
        if ( sinfo == NULL )
        {
            std::cout << "This device cannot support color sensor: " << OpenNI::getExtendedError() << endl;
        }
        else
        {
            if ( STATUS_OK != vsColor.create( device, SENSOR_COLOR ) )
            {
                std::cout << "Cannot create color stream on device: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                if ( STATUS_OK != vsColor.start() )
                {
                    std::cout << "Cannot start color stream on device: " << OpenNI::getExtendedError() << endl;
                }
                if ( vsColor.isValid() )
                {
                    std::cout << "Color stream is starting ..." << endl;
                    vmColor = vsColor.getVideoMode();
                    printVideoMode( vmColor );
                }
            }
        }
    }

    if ( option & IR )
    {
        std::cout << "IR stream is selected." << endl;

        const SensorInfo* sinfo = device.getSensorInfo( SENSOR_IR );
        if ( sinfo == NULL )
        {
            std::cout << "This device cannot support IR sensor: " << OpenNI::getExtendedError() << endl;
        }
        else
        {
            if ( STATUS_OK != vsIR.create( device, SENSOR_IR ) )
            {
                std::cout << "Cannot create IR stream on device: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                if ( STATUS_OK != vsIR.start() )
                {
                    std::cout << "Cannot start IR stream on device: " << OpenNI::getExtendedError() << endl;
                }
                if ( vsIR.isValid() )
                {
                    std::cout << "IR stream is starting ..." << endl;
                    vmIR = vsIR.getVideoMode();
                    printVideoMode( vmIR );
                }
            }
        }
    }

    bool quit = false;
    bool capture = false; //press once to capture one frame and save it to file
    bool showText = true; //default ON
    bool mirror = false;
    bool frameSync = false; //ONLY support Depth-Color sync

    float depthHist[MAX_DEPTH + 1];  //depth histogram map, +1 avoids array out-of-index error

    vector<int> quality;
    quality.push_back( IMWRITE_PNG_COMPRESSION );
    quality.push_back( 0 );
    int64 tStart = cv::getTickCount();

    if ( option & DEPTH )
    {
        namedWindow( "Depth view", WINDOW_NORMAL );
    }
    if ( option & COLOR )
    {
        namedWindow( "Color view", WINDOW_NORMAL );
    }
    if ( option & IR )
    {
        namedWindow( "IR view", WINDOW_NORMAL );
    }

    printHelp();

    while ( true )
    {
        if ( option & DEPTH )
        {
            setWindowProperty( "Depth view", WND_PROP_FULLSCREEN, 0 );
        }
        if ( option & COLOR )
        {
            setWindowProperty( "Color view", WND_PROP_FULLSCREEN, 0 );
        }
        if ( option & IR )
        {
            setWindowProperty( "IR view", WND_PROP_FULLSCREEN, 0 );
        }

        if ( option & DEPTH )
        {
            if ( STATUS_OK == vsDepth.readFrame( &depth_frame ) )
            {
                calculateHistogram( depthHist, MAX_DEPTH, depth_frame );

                Mat imgDepth( depth_frame.getHeight(), depth_frame.getWidth(), CV_16UC1, ( void* )depth_frame.getData() );
                Mat img8bitDepth = Mat( depth_frame.getHeight(), depth_frame.getWidth(), CV_8U, Scalar( 0 ) );
                //use histogram depth to generate texture Mat
                const openni::DepthPixel* pDepth = ( const openni::DepthPixel* )depth_frame.getData();
                uchar *pTexMap = img8bitDepth.data;
                for ( int y = 0; y < depth_frame.getHeight(); y++ )
                {
                    for ( int x = 0; x < depth_frame.getWidth(); x++, pDepth++, pTexMap++ )
                    {
                        if ( *pDepth != 0 )
                        {
                            int nHistValue = depthHist[*pDepth];
                            *pTexMap = nHistValue;
                        }
                    }
                }
                Mat img8bit3ChDepth;
                Mat img8bit3ChMask = Mat( depth_frame.getHeight(), depth_frame.getWidth(), CV_8UC3, Scalar( 0, 255, 255 ) );
                cvtColor( img8bitDepth, img8bit3ChDepth, COLOR_GRAY2BGR );
                cv::bitwise_and( img8bit3ChDepth, img8bit3ChMask, img8bit3ChDepth );

                if ( mirror )
                {
                    flip( img8bit3ChDepth, img8bit3ChDepth, 1 );
                }

                // Show FPS, X, Y, Depth value
                if ( showText )
                {
                    double fps = calcFPS( tStart, ++depth_frame_count );
                    stringstream streamFPS;
                    streamFPS << "FPS: " << fixed << setprecision( 2 ) << fps;
                    putText( img8bit3ChDepth, streamFPS.str(), Point( 5, 25 ), FONT_HERSHEY_DUPLEX, ( depth_frame.getWidth() > 320 ) ? 1.0 : 0.5, Scalar( 255, 255, 255 ) );

                    Zres = imgDepth.at<ushort>( Yres, Xres );
                    stringstream streamXY;
                    streamXY << "X: " << Xres << ", Y: " << Yres;
                    putText( img8bit3ChDepth, streamXY.str(), Point( 5, 50 ), FONT_HERSHEY_DUPLEX, ( depth_frame.getWidth() > 320 ) ? 1.0 : 0.5, Scalar( 255, 255, 255 ) );

                    stringstream streamZ;
                    streamZ << "Depth: " << Zres;
                    putText( img8bit3ChDepth, streamZ.str(), Point( 5, 75 ), FONT_HERSHEY_DUPLEX, ( depth_frame.getWidth() > 320 ) ? 1.0 : 0.5, Scalar( 255, 255, 255 ) );
                }

                resizeWindow( "Depth view", img8bit3ChDepth.size().width, img8bit3ChDepth.size().height );
                imshow( "Depth view", img8bit3ChDepth );

                if ( capture )
                {
                    imwrite( "depth_" + std::to_string( depth_frame.getFrameIndex() ) + ".png", img8bit3ChDepth, quality );
                }
            }
            setMouseCallback( "Depth view", onMouse, NULL );
        }

        if ( option & COLOR )
        {
            if ( STATUS_OK == vsColor.readFrame( &color_frame ) )
            {
                Mat imgColor( color_frame.getHeight(), color_frame.getWidth(), CV_8UC3, ( void* )color_frame.getData() );
                Mat imgBGRColor;
                cvtColor( imgColor, imgBGRColor, COLOR_RGB2BGR );
                if ( mirror )
                {
                    flip( imgBGRColor, imgBGRColor, 1 );
                }

                // Show FPS
                if ( showText )
                {
                    double fps = calcFPS( tStart, ++color_frame_count );
                    stringstream stream;
                    stream << "FPS: " << fixed << setprecision( 2 ) << fps;
                    putText( imgBGRColor, stream.str(), Point( 5, 25 ), FONT_HERSHEY_DUPLEX, ( color_frame.getWidth() > 320 ) ? 1.0 : 0.5, Scalar( 255, 255, 255 ) );
                }

                resizeWindow( "Color view", imgBGRColor.size().width, imgBGRColor.size().height );
                imshow( "Color view", imgBGRColor );

                if ( capture )
                {
                    imwrite( "color_" + std::to_string( color_frame.getFrameIndex() ) + ".png", imgBGRColor, quality );
                }
            }
        }

        if ( option & IR )
        {
            if ( STATUS_OK == vsIR.readFrame( &ir_frame ) )
            {
                Mat imgIR ( ir_frame.getHeight(), ir_frame.getWidth(), CV_16UC1, ( void* )ir_frame.getData() );
                Mat img8bitIR;
                imgIR.convertTo( img8bitIR, CV_8U, 255.0 / 4096 );
                if ( mirror )
                {
                    flip( img8bitIR, img8bitIR, 1 );
                }

                // Show FPS
                if ( showText )
                {
                    double fps = calcFPS( tStart, ++ir_frame_count );
                    stringstream stream;
                    stream << "FPS: " << fixed << setprecision( 2 ) << fps;
                    putText( img8bitIR, stream.str(), Point( 5, 25 ), FONT_HERSHEY_DUPLEX, ( ir_frame.getWidth() > 320 ) ? 1.0 : 0.5, Scalar( 255, 255, 255 ) );
                }

                resizeWindow( "IR view", img8bitIR.size().width, img8bitIR.size().height );
                imshow( "IR view", img8bitIR );

                if ( capture )
                {
                    imwrite( "ir_" + std::to_string( ir_frame.getFrameIndex() ) + ".png", img8bitIR, quality );
                }
            }
        }

        char keyInput = waitKey( 1 );
        if ( keyInput != -1 )
        {
            switch ( keyInput )
            {
                case 'Q': // Q = 81
                case 'q': // q = 113
                    //q for exit
                    quit = true;
                    cout << "[User input] quit the program" << endl;
                    break;
                case 'C': // C = 67
                case 'c': // c = 99
                    // depth
                    capture = true;
                    cout << "[User input] capture one frame" << endl;
                    break;
                case 'M':
                case 'm':
                    mirror = ( mirror ) ? false : true;
                    cout << "[User input] turn " << ( mirror ? "on" : "off" ) << " image mirroring" << endl;
                    break;
                case 'I': // I = 73
                case 'i': // i = 105
                    showText = ( showText ) ? false : true;
                    cout << "[User input] turn " << ( showText ? "on" : "off" ) << " more pixel information" << endl;
                    break;
                case 'F': // F = 70
                case 'f': // f = 102
                    if ( ( option & DEPTH ) && ( option & COLOR ) )
                    {
                        if ( vsDepth.isValid() && vsColor.isValid() )
                        {
                            frameSync = ( frameSync ) ? false : true;
                            if ( frameSync != device.getDepthColorSyncEnabled() )
                            {
                                device.setDepthColorSyncEnabled( frameSync );
                            }
                            cout << "[User input] turn " << ( frameSync ? "on" : "off" ) << " Depth-Color frame sync" << endl;
                        }
                    }
                    else
                    {
                        cout << "Only support Depth-Color frame sync!" << endl;
                    }
                    break;
                default:
                    break;
            }
        }
        else
        {
            capture = false;
        }

        if ( quit )
        {
            break;
        }
    }

    if ( option & DEPTH )
    {
        vsDepth.destroy();
    }
    if ( option & COLOR )
    {
        vsColor.destroy();
    }
    if ( option & IR )
    {
        vsIR.destroy();
    }

    device.close();
    OpenNI::shutdown();
    destroyAllWindows();

    return 0;
}