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
// Undeprecate CRT functions
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "Viewer.h"

#define NK_IMPLEMENTATION
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_VARARGS
#include <nuklear.h>

#define NK_GLFW_GL2_IMPLEMENTATION
#include <nuklear_glfw_gl2.h>

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE                  0x812F
#endif
#ifndef GL_GENERATE_MIPMAP_SGIS
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#endif

#include "OniSampleUtilities.h"

#define GL_WIN_SIZE_X   1280
#define GL_WIN_SIZE_Y   780
#define TEXTURE_SIZE    512

#define DEFAULT_DISPLAY_MODE DISPLAY_MODE_DEPTH_IMAGE

SampleViewer* SampleViewer::ms_self = NULL;

void SampleViewer::key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    SampleViewer::ms_self->onKey( key, scancode, action, mods );
}

void SampleViewer::windowSize_callback( GLFWwindow* window, int width, int height )
{
    SampleViewer::ms_self->updateTexScaleSize();
}

SampleViewer::SampleViewer( const char* strSampleName, openni::Device* device ) :
    m_device( device ),
    m_eViewState( DEFAULT_DISPLAY_MODE ), m_bShowHelpMenu( true ),
    m_depthStream( NULL ), m_colorStream( NULL ), m_irStream( NULL )
{
    ms_self = this;
    strncpy( m_strSampleName, strSampleName, ONI_MAX_STR );
}

SampleViewer::SampleViewer( const char* strSampleName, openni::Device* device, DisplayModes mode ) :
    m_device( device ),
    m_eViewState( mode ), m_bShowHelpMenu( true ),
    m_depthStream( NULL ), m_colorStream( NULL ), m_irStream( NULL )
{
    ms_self = this;
    strncpy( m_strSampleName, strSampleName, ONI_MAX_STR );
}

SampleViewer::SampleViewer( const char* strSampleName, openni::Device* device, DisplayModes mode,
                            openni::VideoStream* depth, openni::VideoStream* color, openni::VideoStream* ir ) :
    m_device( device ),
    m_eViewState( mode ), m_bShowHelpMenu( true ),
    m_depthStream( depth ), m_colorStream( color ), m_irStream( ir )
{
    ms_self = this;
    strncpy( m_strSampleName, strSampleName, ONI_MAX_STR );

    assert( m_depthStream );
    assert( m_colorStream );
    assert( m_irStream );
}

SampleViewer::~SampleViewer()
{
    for ( auto t : m_pTexMap )
    {
        delete t;
    }

    ms_self = NULL;
}

openni::Status SampleViewer::init()
{
    openni::VideoMode colorVideoMode;
    openni::VideoMode depthVideoMode;
    openni::VideoMode irVideoMode;

    m_colorWidth = DEFAULT_WIDTH;
    m_colorHeight = DEFAULT_HEIGHT;
    m_depthWidth = DEFAULT_WIDTH;
    m_depthHeight = DEFAULT_HEIGHT;
    m_irWidth = DEFAULT_WIDTH;
    m_irHeight = DEFAULT_HEIGHT;

    if ( m_depthStream && m_depthStream->isValid() && m_colorStream && m_colorStream->isValid() )
    {
        depthVideoMode = m_depthStream->getVideoMode();
        colorVideoMode = m_colorStream->getVideoMode();

        m_depthWidth = depthVideoMode.getResolutionX();
        m_depthHeight = depthVideoMode.getResolutionY();
        m_colorWidth = colorVideoMode.getResolutionX();
        m_colorHeight = colorVideoMode.getResolutionY();

        if ( m_depthWidth != m_colorWidth || m_depthHeight != m_colorHeight )
        {
            printf( "Warning - expects color and depth to be in same resolution: D: %dx%d, C: %dx%d\n",
                    m_depthWidth, m_depthHeight,
                    m_colorWidth, m_colorHeight );
            //return openni::STATUS_ERROR;
        }
    }
    else if ( m_depthStream && m_depthStream->isValid() )
    {
        depthVideoMode = m_depthStream->getVideoMode();
        m_depthWidth = depthVideoMode.getResolutionX();
        m_depthHeight = depthVideoMode.getResolutionY();
    }
    else if ( m_colorStream && m_colorStream->isValid() )
    {
        colorVideoMode = m_colorStream->getVideoMode();
        m_colorWidth = colorVideoMode.getResolutionX();
        m_colorHeight = colorVideoMode.getResolutionY();
    }
    else if ( !m_device )
    {
        printf( "Info- waiting for device connection...(please plug in your camera)\n" );
    }
    else
    {
        printf( "Warning - expects at least one of the streams to be valid...\n" );
        //return openni::STATUS_ERROR;
    }

    if ( m_irStream && m_irStream->isValid() )
    {
        irVideoMode = m_irStream->getVideoMode();
        m_irWidth = irVideoMode.getResolutionX();
        m_irHeight = irVideoMode.getResolutionY();
    }

    // Texture map init
    m_pTexMap[0] = new openni::RGB888Pixel[m_colorWidth * m_colorHeight];
    m_pTexMap[1] = new openni::RGB888Pixel[m_depthWidth * m_depthHeight];
    m_pTexMap[2] = new openni::RGB888Pixel[m_irWidth * m_irHeight];

    // Init both panel with black background
    memset( m_pTexMap[0], 0x0, m_colorWidth * m_colorHeight * sizeof( openni::RGB888Pixel ) );
    memset( m_pTexMap[1], 0x0, m_depthWidth * m_depthHeight * sizeof( openni::RGB888Pixel ) );
    memset( m_pTexMap[2], 0x0, m_irWidth * m_irHeight * sizeof( openni::RGB888Pixel ) );

    m_gl_handle[0] = 0;
    m_gl_handle[1] = 0;
    m_gl_handle[2] = 0;

    m_capturing = false;

    return initGLFW();
}

void SampleViewer::frametoTex()
{
    if ( !m_colorFrame.isValid() && !m_depthFrame.isValid() && !m_irFrame.isValid() )
    {
        return;
    }

    memset( m_pTexMap[0], 0, m_colorWidth * m_colorHeight * sizeof( openni::RGB888Pixel ) );
    memset( m_pTexMap[1], 0, m_depthWidth * m_depthHeight * sizeof( openni::RGB888Pixel ) );
    memset( m_pTexMap[2], 0, m_irWidth * m_irHeight * sizeof( openni::RGB888Pixel ) );

    std::lock_guard<std::mutex> m_lock( m_read_mtx );

    if ( m_colorFrame.isValid() )
    {
        if ( m_colorFrame.getWidth() != m_colorWidth || m_colorFrame.getHeight() != m_colorHeight )
        {
            m_colorWidth = m_colorFrame.getWidth();
            m_colorHeight = m_colorFrame.getHeight();

            delete[] m_pTexMap[0];
            m_pTexMap[0] = new openni::RGB888Pixel[m_colorWidth * m_colorHeight];
            memset( m_pTexMap[0], 0x0, m_colorWidth * m_colorHeight * sizeof( openni::RGB888Pixel ) );
        }
    }

    if ( m_depthFrame.isValid() )
    {
        if ( m_depthFrame.getWidth() != m_depthWidth || m_depthFrame.getHeight() != m_depthHeight )
        {
            m_depthWidth = m_depthFrame.getWidth();
            m_depthHeight = m_depthFrame.getHeight();

            delete[] m_pTexMap[1];
            m_pTexMap[1] = new openni::RGB888Pixel[m_depthWidth * m_depthHeight];
            memset( m_pTexMap[1], 0x0, m_depthWidth * m_depthHeight * sizeof( openni::RGB888Pixel ) );
        }
    }

    if ( m_irFrame.isValid() )
    {
        if ( m_irFrame.getWidth() != m_irWidth || m_irFrame.getHeight() != m_irHeight )
        {
            m_irWidth = m_irFrame.getWidth();
            m_irHeight = m_irFrame.getHeight();

            delete[] m_pTexMap[2];
            m_pTexMap[2] = new openni::RGB888Pixel[m_irWidth * m_irHeight];
            memset( m_pTexMap[2], 0x0, m_irWidth * m_irHeight * sizeof( openni::RGB888Pixel ) );
        }
    }

    // check if we need to draw image frame to texture
    if ( ( m_eViewState == DISPLAY_MODE_IMAGE ||
            m_eViewState == DISPLAY_MODE_DEPTH_IMAGE ) && m_colorFrame.isValid() )
    {
        //m_read_mtx.lock();
        const openni::RGB888Pixel* pImage = ( const openni::RGB888Pixel* )m_colorFrame.getData();
        //BUG: isValid() is True, but getData() gives us NULL buffer ?!
        //WORKAROUND: we have to double-check the buffer pointer
        if (pImage != NULL)
            memcpy( m_pTexMap[0], pImage, m_colorWidth * m_colorHeight * sizeof( openni::RGB888Pixel ) );
        //m_read_mtx.unlock();
        upload( 0, m_colorWidth, m_colorHeight );
    }

    // check if we need to draw depth frame to texture
    if ( ( m_eViewState == DISPLAY_MODE_DEPTH ||
            m_eViewState == DISPLAY_MODE_DEPTH_IR ||
            m_eViewState == DISPLAY_MODE_DEPTH_IMAGE ) && m_depthFrame.isValid() )
    {
        //m_read_mtx.lock();
        calculateHistogram( m_pDepthHist, MAX_DEPTH, m_depthFrame );
        const openni::DepthPixel* pDepth = ( const openni::DepthPixel* )m_depthFrame.getData();
        int i = 0;
        for ( int y = 0; y < m_depthHeight; y++ )
        {
            //BUG: isValid() is True, but getData() gives us NULL buffer ?!
            //WORKAROUND: we have to double-check the buffer pointer
            if (pDepth == NULL)
                break; //keep TexMap as old frame

            for ( int x = 0; x < m_depthWidth; x++ )
            {
                if ( pDepth[i] != 0 )
                {
                    int nHistValue = m_pDepthHist[pDepth[i]];
                    m_pTexMap[1][i].r = nHistValue;
                    m_pTexMap[1][i].g = nHistValue;
                    m_pTexMap[1][i].b = 0;
                }
                i++;
            }
        }
        //m_read_mtx.unlock();
        upload( 1, m_depthWidth, m_depthHeight );
    }

    // check if we need to draw IR frame to texture
    if ( m_eViewState == DISPLAY_MODE_DEPTH_IR && m_irFrame.isValid() )
    {
        //m_read_mtx.lock();
        const openni::Grayscale16Pixel* pIr = ( const openni::Grayscale16Pixel* )m_irFrame.getData();
        int i = 0;
        for ( int y = 0; y < m_irHeight; y++ )
        {
            //BUG: isValid() is True, but getData() gives us NULL buffer ?!
            //WORKAROUND: we have to double-check the buffer pointer
            if (pIr == NULL)
                break; //keep TexMap as old frame

            for ( int x = 0; x < m_irWidth; x++ )
            {
                //let r=g=b=grayscale
                int nNormalIr = 255.0 * ( ( float )pIr[i] / MAX_AMPLITUDE );
                m_pTexMap[2][i].r = nNormalIr;
                m_pTexMap[2][i].g = nNormalIr;
                m_pTexMap[2][i].b = nNormalIr;
                i++;
            }
        }
        //m_read_mtx.unlock();
        upload( 2, m_irWidth, m_irHeight );
    }

    // check if we need to draw image and depth frame to texture
    if ( m_eViewState == DISPLAY_MODE_OVERLAY && m_colorFrame.isValid() && m_depthFrame.isValid() )
    {
        //m_read_mtx.lock();
        const openni::RGB888Pixel* pImage = ( const openni::RGB888Pixel* )m_colorFrame.getData();
        //BUG: isValid() is True, but getData() gives us NULL buffer ?!
        //WORKAROUND: we have to double-check the buffer pointer
        if (pImage != NULL)
            memcpy( m_pTexMap[0], pImage, m_colorWidth * m_colorHeight * sizeof( openni::RGB888Pixel ) );

        calculateHistogram( m_pDepthHist, MAX_DEPTH, m_depthFrame );
        const openni::DepthPixel* pDepth = ( const openni::DepthPixel* )m_depthFrame.getData();

        int i = 0;
        float alpha = 1.0;
        for ( int y = 0; y < m_depthHeight; y++ )
        {
            //BUG: isValid() is True, but getData() gives us NULL buffer ?!
            //WORKAROUND: we have to double-check the buffer pointer
            if (pDepth == NULL)
                break; //keep TexMap as old frame

            for ( int x = 0; x < m_depthWidth; x++ )
            {
                if ( pDepth[i] != 0 && x < m_colorWidth && y < m_colorHeight )
                {
                    int nHistValue = m_pDepthHist[pDepth[i]];
                    m_pTexMap[0][i].r = nHistValue * alpha + m_pTexMap[0][i].r * ( 1.0 - alpha );
                    m_pTexMap[0][i].g = nHistValue * alpha + m_pTexMap[0][i].g * ( 1.0 - alpha );
                    m_pTexMap[0][i].b = m_pTexMap[0][i].b * ( 1.0 - alpha );
                }
                i++;
            }
        }
        //m_read_mtx.unlock();
        upload( 0, m_colorWidth, m_colorHeight );
        return;
    }
}

openni::Status SampleViewer::openni_capturing()
{
    openni::VideoStream* m_streams[3];
    m_streams[0] = m_colorStream;
    m_streams[1] = m_depthStream;
    m_streams[2] = m_irStream;

    int changedIndex;
    while ( m_capturing )
    {
        openni::Status rc = openni::OpenNI::waitForAnyStream( m_streams, 3, &changedIndex, WAIT_TIMEOUT_MS );
        if ( rc != openni::STATUS_OK )
        {
            printf( "Wait failed. Frame is not coming for %d ms.\n", WAIT_TIMEOUT_MS );
            //return;
        }

        std::lock_guard<std::mutex> m_lock( m_read_mtx );
        switch ( changedIndex )
        {
            case 0:
                if ( m_streams[0] && m_streams[0]->isValid() )
                {
                    m_streams[0]->readFrame( &m_colorFrame );
                }

                break;
            case 1:
                if ( m_streams[1] && m_streams[1]->isValid() )
                {
                    m_streams[1]->readFrame( &m_depthFrame );
                }
                break;
            case 2:
                if ( m_streams[2] && m_streams[2]->isValid() )
                {
                    m_streams[2]->readFrame( &m_irFrame );
                }
                break;
            default:
                printf( "Error in wait\n" );
                continue;
        }
    }

    return openni::STATUS_OK;
}

void SampleViewer::run_func()
{
    openni_capturing();
}

openni::Status SampleViewer::run()  //Does not return
{
    m_capturing = true;
    capture_loop = std::thread( &SampleViewer::run_func, this );
    while ( !glfwWindowShouldClose( m_window ) )
    {
        glfwGetFramebufferSize( m_window, &m_windowWidth, &m_windowHeight );

        frametoTex();
        updateTexScaleSize();
        display();

        if ( m_bShowHelpMenu )
        {
            nk_glfw3_new_frame();
            if ( nk_begin( m_nk, "", nk_rect( 0.0f, 0.0f, 280.0f, 230.0f ), 0 ) )
            {
                nk_flags align_left = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
                nk_layout_row_dynamic( m_nk, 20, 1 );
                nk_label( m_nk, "hot keys:", align_left );
                nk_label( m_nk, "1         Depth + Image", align_left );
                nk_label( m_nk, "2         Depth", align_left );
                nk_label( m_nk, "3         Image", align_left );
                nk_label( m_nk, "4         Overlay: Depth-to-Image", align_left );
                nk_label( m_nk, "5         Depth + IR", align_left );
                nk_label( m_nk, "H/h       Show/Close This Help Menu", align_left );
                nk_label( m_nk, "M/m       Mirror on/off", align_left );
                nk_label( m_nk, "Q/q/Esc   Exit Program", align_left );
            }
            nk_end( m_nk );
            nk_glfw3_render( NK_ANTI_ALIASING_ON );
        }

        glfwSwapBuffers( m_window );
        glfwPollEvents();
    }

    m_capturing = false; //in case of Window is closed by button, we have to stop thread capturing
    printf( "Waiting for window exit...\n" );
    capture_loop.join();

    return openni::STATUS_OK;
}

void SampleViewer::exit()
{
    glfwDestroyWindow( m_window );
    glfwTerminate();
}

void SampleViewer::drawTex( const int index, float transX, float transY, float transZ )
{
    int w, h;
    if ( index == 0 )
    {
        w = m_colorWidth;
        h = m_colorHeight;
    }
    else if ( index == 1 )
    {
        w = m_depthWidth;
        h = m_depthHeight;
    }
    else if ( index == 2 )
    {
        w = m_irWidth;
        h = m_irHeight;
    }
    else
    {
        return;
    }

    glPushMatrix();
    glTranslatef( transX, transY, transZ );
    glScalef( m_fScale, m_fScale, 0.0 );

    // Display the OpenGL texture map
    glBindTexture( GL_TEXTURE_2D, m_gl_handle[index] );
    glBegin( GL_QUADS );
    // bottom left
    glTexCoord2i( 0, 1 );
    glVertex2f( 0, 0 );
    // bottom right
    glTexCoord2i( 1, 1 );
    glVertex2f( w, 0 );
    // upper right
    glTexCoord2i( 1, 0 );
    glVertex2f( w, h );
    // upper left
    glTexCoord2i( 0, 0 );
    glVertex2f( 0, h );
    glEnd();

    glPopMatrix();
}

void SampleViewer::display()
{
    glViewport( 0, 0, m_windowWidth, m_windowHeight );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, m_windowWidth, 0, m_windowHeight, -1.0, 1.0 );

    //key 1, 2, 5
    if ( m_eViewState == DISPLAY_MODE_DEPTH_IMAGE || m_eViewState == DISPLAY_MODE_DEPTH || m_eViewState == DISPLAY_MODE_DEPTH_IR )
    {
        if ( m_depthFrame.isValid() )
        {
            drawTex( 1, 0, m_windowHeight - m_depthHeight * m_fScale, 0 );
        }

        if ( m_eViewState == DISPLAY_MODE_DEPTH_IMAGE && m_colorFrame.isValid() ) //key 1
        {
            drawTex( 0, m_windowWidth / 2.0, m_windowHeight - m_colorHeight * m_fScale, 0 );
        }
        else if ( m_eViewState == DISPLAY_MODE_DEPTH_IR && m_irFrame.isValid() ) //key 5
        {
            drawTex( 2, m_windowWidth / 2.0, m_windowHeight - m_irHeight * m_fScale, 0 );
        }
    }
    else if ( m_eViewState == DISPLAY_MODE_IMAGE && m_colorFrame.isValid() ) //key 3
    {
        drawTex( 0, 0, m_windowHeight - m_colorHeight * m_fScale, 0 );
    }
    else if ( m_eViewState == DISPLAY_MODE_OVERLAY && m_colorFrame.isValid() && m_depthFrame.isValid() ) //key 4
    {
        drawTex( 0, 0, m_windowHeight - m_colorHeight * m_fScale, 0 );
    }
}

void SampleViewer::upload( const int index, int width, int height )
{
    glBindTexture( GL_TEXTURE_2D, m_gl_handle[index] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTexMap[index] );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    glPixelStorei( GL_UNPACK_ROW_LENGTH, 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
}

void SampleViewer::updateTexScaleSize()
{
    glfwGetFramebufferSize( m_window, &m_windowWidth, &m_windowHeight );
    float depthWidthScale = m_windowWidth / 2.0f / ( float )m_depthWidth;
    float depthHeightScale = m_windowHeight / ( float )m_depthHeight;
    float colorWidthScale = m_windowWidth / 2.0f / ( float )m_colorWidth;
    float colorHeightScale = m_windowHeight / ( float )m_colorHeight;
    float min = 10.0f;
    if ( m_depthFrame.isValid() )
    {
        if ( min > depthWidthScale )
        {
            min = depthWidthScale;
        }
        if ( min > depthHeightScale )
        {
            min = depthHeightScale;
        }
    }
    if ( m_colorFrame.isValid() )
    {
        if ( min > colorWidthScale )
        {
            min = colorWidthScale;
        }
        if ( min > colorHeightScale )
        {
            min = colorHeightScale;
        }
    }
    if ( !m_depthFrame.isValid() && !m_colorFrame.isValid() )
    {
        min = 1.0;
    }
    m_fScale = min;
}

void SampleViewer::onKey( int key, int scancode, int action, int mods )
{
    //device access lock, m_device can be accessed by child class like ListenerViewer
    std::lock_guard<std::mutex> m_lock( m_device_mtx );
    if ( action != GLFW_PRESS  )
    {
        return;
    }

    if ( key == GLFW_KEY_H )
    {
        //Help Menu: press once to show menu, press again to close help menu
        m_bShowHelpMenu = !m_bShowHelpMenu;
    }
    else if ( key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE )
    {
        glfwSetWindowShouldClose( m_window, GLFW_TRUE );
    }

    if ( !m_device )
    {
        return;
    }
    if ( key == GLFW_KEY_1 && m_colorStream && m_colorStream->isValid() )
    {
        m_eViewState = DISPLAY_MODE_DEPTH_IMAGE; //side-by-side
        m_device->setImageRegistrationMode( openni::IMAGE_REGISTRATION_OFF );
    }
    else if ( key == GLFW_KEY_2 )
    {
        m_eViewState = DISPLAY_MODE_DEPTH;
        m_device->setImageRegistrationMode( openni::IMAGE_REGISTRATION_OFF );
    }
    else if ( key == GLFW_KEY_3 && m_colorStream && m_colorStream->isValid() )
    {
        m_eViewState = DISPLAY_MODE_IMAGE;
        m_device->setImageRegistrationMode( openni::IMAGE_REGISTRATION_OFF );
    }
    else if ( key == GLFW_KEY_4 && m_colorStream && m_colorStream->isValid() )
    {
        m_eViewState = DISPLAY_MODE_OVERLAY;
        m_device->setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );
    }
    else if ( key == GLFW_KEY_5 )
    {
        m_eViewState = DISPLAY_MODE_DEPTH_IR; //side-by-side
        m_device->setImageRegistrationMode( openni::IMAGE_REGISTRATION_OFF );
    }
    else if ( key == GLFW_KEY_M )
    {
        if ( m_colorStream && m_colorStream->isValid() )
        {
            m_colorStream->setMirroringEnabled( !m_colorStream->getMirroringEnabled() );
        }
        if ( m_depthStream && m_depthStream->isValid() )
        {
            m_depthStream->setMirroringEnabled( !m_depthStream->getMirroringEnabled() );
        }
        if ( m_irStream && m_irStream->isValid() )
        {
            m_irStream->setMirroringEnabled( !m_irStream->getMirroringEnabled() );
        }
    }
}

openni::Status SampleViewer::initGLFW()
{
    if ( !glfwInit() )
    {
        return openni::STATUS_ERROR;
    }

    m_windowWidth = GL_WIN_SIZE_X;
    m_windowHeight = GL_WIN_SIZE_Y;
    m_window = glfwCreateWindow( m_windowWidth, m_windowHeight, m_strSampleName, NULL, NULL );
    if ( !m_window )
    {
        glfwTerminate();
        return openni::STATUS_ERROR;
    }

    glfwMakeContextCurrent( m_window );

    //for help screen
    gladLoadGL( glfwGetProcAddress );
    m_nk = nk_glfw3_init( m_window, NK_GLFW3_INSTALL_CALLBACKS );
    nk_glfw3_font_stash_begin( &m_atlas );
    nk_glfw3_font_stash_end();

    glfwSetKeyCallback( m_window, key_callback );
    glfwSetWindowSizeCallback( m_window, windowSize_callback );

    glClearColor( 0.2, 0.2, 0.2, 1.0 );
    glDisable( GL_DEPTH_TEST );
    glEnable( GL_TEXTURE_2D );

    if ( !m_gl_handle[0] )
    {
        glGenTextures( 1, &m_gl_handle[0] );
    }
    if ( !m_gl_handle[1] )
    {
        glGenTextures( 1, &m_gl_handle[1] );
    }
    if ( !m_gl_handle[2] )
    {
        glGenTextures( 1, &m_gl_handle[2] );
    }
    return openni::STATUS_OK;
}
