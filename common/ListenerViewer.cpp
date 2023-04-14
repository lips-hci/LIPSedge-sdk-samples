/*****************************************************************************
*
* LIPS Corp.
*
* ListenerViewer
* - class FrameListener
* - class ListenerViewer
*
*****************************************************************************/
#include "ListenerViewer.hpp"

using namespace std;
using namespace openni;

bool DeviceListenerBase::readDepthFrame( VideoFrameRef &depth )
{
    if (listenerDepth)
    {
        listenerDepth->readFrame(depth);
        return true;
    }
    return false;
}

bool DeviceListenerBase::readColorFrame( VideoFrameRef &color )
{
    if (listenerColor)
    {
        listenerColor->readFrame(color);
        return true;
    }
    return false;
}

bool DeviceListenerBase::readIrFrame( VideoFrameRef &ir )
{
    if (listenerIR)
    {
        listenerIR->readFrame(ir);
        return true;
    }
    return false;
}

void DeviceListenerBase::startOrStop()
{
    unique_lock<mutex> ulock( mtx );
    switch ( execution )
    {
        case DeviceStreamExecution::OPEN_AND_START:
            execution = DeviceStreamExecution::NONE;
            ulock.unlock();
            // Wait for device to get ready.
            this_thread::sleep_for( chrono::seconds( 1 ) );
            createDeviceStream();
            break;
        case DeviceStreamExecution::STOP_AND_CLOSE:
            execution = DeviceStreamExecution::NONE;
            ulock.unlock();
            this_thread::sleep_for( chrono::milliseconds( 300 ) );
            removeDeviceStream();
            break;
        case DeviceStreamExecution::NONE:
        default:
            break;
    }
}

inline void DeviceListenerBase::stopAndClose()
{
    removeDeviceStream();
}

void DeviceListenerBase::createDeviceStream()
{
    // Open new device.
    if ( device && device->isValid() )
    {
        device->close();
    }
    device.reset( new Device );
    if ( device->open( deviceURI.c_str() ) != STATUS_OK )
    {
        cout << "Cannot open device: " << OpenNI::getExtendedError() << endl;
        device.reset();
        return;
    }
    cout << "Device opened: " << deviceURI << endl;

	//TIPS: start Color stream prior to Depth stream can safe camera launch time
    // Create color stream on device.
    streamColor.reset( new VideoStream );
    if ( streamColor->create( *device, SENSOR_COLOR ) != STATUS_OK )
    {
        cout << "Failed to create color stream on device: " << OpenNI::getExtendedError() << endl;
        streamColor.reset();
    }
    else
    {
        listenerColor.reset( new FrameListener );
        if ( streamColor->addNewFrameListener( listenerColor.get() ) != STATUS_OK )
        {
            cout << "Failed to add color frame listener: " << OpenNI::getExtendedError() << endl;
            listenerColor.reset();
        }
        else
        {
            if ( streamColor->start() != STATUS_OK )
            {
                cout << "Failed to start color stream: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                cout << "Color stream started." << endl;
            }
        }
    }

    // Create depth stream on device.
    streamDepth.reset( new VideoStream );
    if ( streamDepth->create( *device, SENSOR_DEPTH ) != STATUS_OK )
    {
        cout << "Failed to create depth stream on device: " << OpenNI::getExtendedError() << endl;
        streamDepth.reset();
    }
    else
    {
        listenerDepth.reset( new FrameListener );
        if ( streamDepth->addNewFrameListener( listenerDepth.get() ) != STATUS_OK )
        {
            cout << "Failed to add depth frame listener: " << OpenNI::getExtendedError() << endl;
            listenerDepth.reset();
        }
        else
        {
            if ( streamDepth->start() != STATUS_OK )
            {
                cout << "Failed to start depth stream: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                cout << "Depth stream started." << endl;
            }
        }
    }

    // Create IR stream on device.
    streamIR.reset( new VideoStream );
    if ( streamIR->create( *device, SENSOR_IR ) != STATUS_OK )
    {
        cout << "Failed to create IR stream on device: " << OpenNI::getExtendedError() << endl;
        streamIR.reset();
    }
    else
    {
        listenerIR.reset( new FrameListener );
        if ( streamIR->addNewFrameListener( listenerIR.get() ) != STATUS_OK )
        {
            cout << "Failed to add IR frame listener: " << OpenNI::getExtendedError() << endl;
            listenerIR.reset();
        }
        else
        {
            if ( streamIR->start() != STATUS_OK )
            {
                cout << "Failed to start IR stream: " << OpenNI::getExtendedError() << endl;
            }
            else
            {
                cout << "IR stream started." << endl;
            }
        }
    }
}

void DeviceListenerBase::removeDeviceStream()
{
    // Stop IR stream.
    if ( streamIR )
    {
        streamIR->stop();
        if ( listenerIR )
        {
            streamIR->removeNewFrameListener( listenerIR.get() );
            listenerIR.reset();
        }
        streamIR->destroy();
        streamIR.reset();
        cout << "IR stream stopped." << endl;
    }

    // Stop depth stream.
    if ( streamDepth )
    {
        streamDepth->stop();
        if ( listenerDepth )
        {
            streamDepth->removeNewFrameListener( listenerDepth.get() );
            listenerDepth.reset();
        }
        streamDepth->destroy();
        streamDepth.reset();
        cout << "Depth stream stopped." << endl;
    }

    // Stop color stream.
    if ( streamColor )
    {
        streamColor->stop();
        if ( listenerColor )
        {
            streamColor->removeNewFrameListener( listenerColor.get() );
            listenerColor.reset();
        }
        streamColor->destroy();
        streamColor.reset();
        cout << "Color stream stopped." << endl;
    }

    // Close device.
    if ( device )
    {
        device->close();
        device.reset();
        cout << "Device closed." << endl;
    }
}

ListenerViewer::ListenerViewer(const char* strSampleName, DeviceListenerBase* listener, DisplayModes mode) :
    SampleViewer( strSampleName, listener->device.get(), mode ),
    m_listener( listener )
{
}

ListenerViewer::~ListenerViewer()
{
}

inline void ListenerViewer::_update_device_streams()
{
    std::lock_guard<std::mutex> m_lock(m_device_mtx);
    m_device = m_listener->device.get();
    m_depthStream = m_listener->streamDepth.get();
    m_colorStream = m_listener->streamColor.get();
    m_irStream = m_listener->streamIR.get();
}

openni::Status ListenerViewer::openni_capturing()
{
    if (!m_listener)
    {
        cout << "Error, cannot find DeviceListener, end capturing loop." << endl;
        return openni::STATUS_ERROR;
    }

	while(m_capturing)
    {
        std::lock_guard<std::mutex> m_lock(m_read_mtx);

        // Start/Stop streaming after device connected/disconnected.
        m_listener->startOrStop();

        // Remember to update streams if they are just created
        _update_device_streams();

        m_listener->readDepthFrame( m_depthFrame );
        m_listener->readColorFrame( m_colorFrame );
        m_listener->readIrFrame( m_irFrame );
    }

    m_listener->stopAndClose();

    return openni::STATUS_OK;
}