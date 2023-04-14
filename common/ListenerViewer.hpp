/*****************************************************************************
*
* LIPS Corp.
*
* ListenerViewer
* - class FrameListener
* - class ListenerViewer
*
*****************************************************************************/
#ifndef _LIPS_LISTENER_VIEWER_HPP_
#define _LIPS_LISTENER_VIEWER_HPP_

#include <OpenNI.h>
#include "Viewer.h"

#include <iostream>
#include <queue>
#include <mutex>
#include <memory>
#include <string>

using namespace openni;

class FrameListener : public VideoStream::NewFrameListener
{
    public:
        void onNewFrame(VideoStream &stream)
        {
            VideoFrameRef frame;
            if ( stream.isValid() )
            {
                if ( stream.readFrame( &frame ) != STATUS_OK )
                {
                    std::cout << "Failed to read new frame: " << OpenNI::getExtendedError() << std::endl;
                    return;
                }
                if ( frame.isValid() )
                {
                    int bpp = 2;
                    VideoMode m = frame.getVideoMode();
                    switch(m.getPixelFormat())
                    {
                        case PIXEL_FORMAT_RGB888:
                        case PIXEL_FORMAT_JPEG:
                            bpp = 3;
                            break;
                        case PIXEL_FORMAT_GRAY8:
                            bpp = 1;
                            break;
                        case PIXEL_FORMAT_DEPTH_1_MM:
                        case PIXEL_FORMAT_DEPTH_100_UM:
                        case PIXEL_FORMAT_GRAY16:
                        case PIXEL_FORMAT_YUV422:
                        case PIXEL_FORMAT_YUYV:
                        default:
                            bpp = 2;
                            break;
                    }

                    //Push into frame queue
                    std::lock_guard<std::mutex> mlock( mtxQ );
                    frmQ.push( frame );
                }
            }
        }

        void readFrame( VideoFrameRef &frm )
        {
            std::lock_guard<std::mutex> mlock( mtxQ );
            if ( !frmQ.empty() )
            {
                frm = frmQ.front();
                frmQ.pop();
            }
        }

    private:
        std::mutex mtxQ;
        std::queue<VideoFrameRef> frmQ;
};

class DeviceListenerBase :
    public OpenNI::DeviceConnectedListener,
    public OpenNI::DeviceDisconnectedListener,
    public OpenNI::DeviceStateChangedListener
{
    friend class ListenerViewer;

    public:
        DeviceListenerBase(std::string uri) :
            device(nullptr),
            deviceURI(uri),
            execution( DeviceStreamExecution::NONE )
        {}

        virtual ~DeviceListenerBase() {}

        virtual void onDeviceConnected( const DeviceInfo *info ) = 0;

        virtual void onDeviceDisconnected( const DeviceInfo *info ) = 0;

        virtual void onDeviceStateChanged( const DeviceInfo *info, DeviceState state ) = 0;

        enum class DeviceStreamExecution : int
        {
            NONE,
            OPEN_AND_START,
            STOP_AND_CLOSE
        };

        bool readDepthFrame( VideoFrameRef &depth );
        bool readColorFrame( VideoFrameRef &color );
        bool readIrFrame( VideoFrameRef &ir );

        void startOrStop();

        void stopAndClose();

    protected:
        std::mutex mtx;
        std::string deviceURI;
        std::shared_ptr<Device> device;
        std::shared_ptr<VideoStream> streamDepth, streamIR, streamColor;
        std::shared_ptr<FrameListener> listenerDepth, listenerIR, listenerColor;
        DeviceStreamExecution execution;

    private:
        void createDeviceStream();

        void removeDeviceStream();
};

class ListenerViewer : public SampleViewer
{
    public:
        ListenerViewer(const char* strSampleName, DeviceListenerBase* listener, DisplayModes mode);
        ~ListenerViewer();

        openni::Status init()
        {
            return SampleViewer::init();
        }

        openni::Status run()	//Does not return
        {
            return SampleViewer::run();
        }

        void exit() //Must call this (to destroy the Window) when exit program
        {
            return SampleViewer::exit();
        }

        void display()
        {
            SampleViewer::display();
        }

        openni::Status openni_capturing();

    private:
        void _update_device_streams();
        DeviceListenerBase *m_listener;
};

#endif // _LIPS_LISTENER_VIEWER_HPP_