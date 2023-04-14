/*****************************************************************************
*
* LIPS Corp.
* modify this sample viewer example to draw side-by-sdie frame by OpenGL
* and extends hot keys to swith different cameras by number keys
* hot keys:
*      1: Depth and Image (default)
*      2: Depth
*      3: Image
*      4: Image with Depth overlay
*      5: Depth and IR
*    H/h: show/close help menu
*    M/m: mirror on/off
*  Esc/q: exit program
*****************************************************************************/
#ifndef _LIPS_SAMPLE_VIEWER_GL_H_
#define _LIPS_SAMPLE_VIEWER_GL_H_

#include <atomic>
#include <thread>
#include <mutex>
#include <OpenNI.h>

#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define MAX_DEPTH 16000 //16 meters: customer can adjust max depth range over 10m
#define MAX_AMPLITUDE 4095 //12bits amplitude as IR pixel data
#define WAIT_TIMEOUT_MS 20000

#define DEFAULT_WIDTH   640
#define DEFAULT_HEIGHT  480

enum DisplayModes
{
    DISPLAY_MODE_DEPTH_IMAGE,//key 1 (default)
    DISPLAY_MODE_DEPTH,      //key 2
    DISPLAY_MODE_IMAGE,      //key 3
    DISPLAY_MODE_OVERLAY,    //key 4
    DISPLAY_MODE_DEPTH_IR    //key 5
};

class SampleViewer
{
    public:
        SampleViewer( const char* strSampleName, openni::Device* device, DisplayModes mode,
                      openni::VideoStream*, openni::VideoStream*, openni::VideoStream* );
        SampleViewer( const char* strSampleName, openni::Device* device, DisplayModes mode );
        SampleViewer( const char* strSampleName, openni::Device* device );
        virtual ~SampleViewer();

        virtual openni::Status init();
        virtual openni::Status run();   //Does not return
        virtual void exit();

    protected:
        virtual void display();
        virtual void onKey( int key, int scancode, int action, int mods );
        virtual openni::Status openni_capturing();

        openni::Status initGLFW();

        std::mutex m_read_mtx;
        openni::VideoFrameRef m_colorFrame;
        openni::VideoFrameRef m_depthFrame;
        openni::VideoFrameRef m_irFrame;

        std::mutex m_device_mtx;
        openni::Device* m_device;

        openni::VideoStream* m_colorStream;
        openni::VideoStream* m_depthStream;
        openni::VideoStream* m_irStream;

        static SampleViewer* ms_self;
        DisplayModes m_eViewState;
        std::atomic<bool> m_capturing;

        SampleViewer( const SampleViewer& );
        SampleViewer& operator=( SampleViewer& );

        int m_windowWidth, m_windowHeight;
        int m_colorWidth, m_colorHeight;
        int m_depthWidth, m_depthHeight;
        int m_irWidth, m_irHeight;

    private:
        static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods );
        static void windowSize_callback( GLFWwindow* window, int width, int height );

        void run_func();
        void upload( const int index, int width, int height );
        void updateTexScaleSize();

        void frametoTex();
        void drawTex( const int index, float transX, float transY, float transZ );

        float m_pDepthHist[MAX_DEPTH];
        char m_strSampleName[ONI_MAX_STR];
        openni::RGB888Pixel* m_pTexMap[3]; //[0]=color [1]=depth [2]=ir
        unsigned int m_gl_handle[3]; //[0]=color [1]=depth [2]=ir
        bool m_bShowHelpMenu;
        std::thread capture_loop;

        //for glfw
        GLFWwindow* m_window;

        float m_fScale;

        //for help screen
        struct nk_context* m_nk;
        struct nk_font_atlas* m_atlas;
};


#endif // _LIPS_SAMPLE_VIEWER_GL_H_
