/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* Ni2PointCloud-gl
*
* This sample uses OpenNI2 API to calculate point cloud data and draw it in OpenGL window
*
*****************************************************************************/

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <fstream>

#include <OpenNI.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "Device.h"
#include "Draw.h"

bool bComputeCloud;
bool bQuit;
bool bShowToolbar;
bool bVisualizedRGBFrame;

void exportToPlyFile()
{
    // Get .ply filename
    time_t rawTime;
    struct tm *timeInfo;
    char filename[80];
    time( &rawTime );
    timeInfo = localtime( &rawTime );
    strftime( filename, 80, "%Y%m%d_%H%M%S.ply", timeInfo );

    // Count non-zero numbers
    int verticesSize = 0;
    for ( int i = 0; i < g_pointsDataSize; i++ )
    {
        if ( g_pointsData[i].depth != 0 )
        {
            verticesSize++;
        }
    }

    // Write .ply file
    std::ofstream out( filename );
    out << "ply\n";
    out << "format binary_little_endian 1.0\n";
    out << "element vertex " << verticesSize << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";
    if ( g_bIsColorValid && bVisualizedRGBFrame )
    {
        out << "property uchar red\n";
        out << "property uchar green\n";
        out << "property uchar blue\n";
    }
    out << "end_header\n";
    out.close();
    out.open( filename, std::ios_base::app | std::ios_base::binary );
    for ( int i = 0; i < g_pointsDataSize; i++ )
    {
        if ( g_pointsData[i].depth != 0 )
        {
            out.write( reinterpret_cast< const char * >( &( g_pointsData[i].worldX ) ), sizeof( float ) );
            out.write( reinterpret_cast< const char * >( &( g_pointsData[i].worldY ) ), sizeof( float ) );
            out.write( reinterpret_cast< const char * >( &( g_pointsData[i].worldZ ) ), sizeof( float ) );
            if ( g_bIsColorValid && bVisualizedRGBFrame )
            {
                out.write( reinterpret_cast< const char * >( &g_pointsData[i].ucharR ), sizeof( unsigned char ) );
                out.write( reinterpret_cast< const char * >( &g_pointsData[i].ucharG ), sizeof( unsigned char ) );
                out.write( reinterpret_cast< const char * >( &g_pointsData[i].ucharB ), sizeof( unsigned char ) );
            }
        }
    }
    out.close();
}

void cursor_position_callback( GLFWwindow* window, double xpos, double ypos )
{
    if ( bShowToolbar && xpos < g_toolbarWidth && ypos < g_toolbarHeight )
    {
        return;
    }

    if ( viewerStat.mouseLeft )
    {
        viewerStat.yaw -= ( xpos - viewerStat.lastX );
        viewerStat.pitch += ( ypos - viewerStat.lastY );
    }

    if ( viewerStat.mouseRight )
    {
        viewerStat.lookatX += ( xpos - viewerStat.lastX );
        viewerStat.lookatY += ( ypos - viewerStat.lastY );
    }

    viewerStat.lastX = xpos;
    viewerStat.lastY = ypos;
}

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( action == GLFW_PRESS )
    {
        if ( key == GLFW_KEY_SPACE )
        {
            viewerStat.yaw = 0.0;
            viewerStat.pitch = 0.0;
            viewerStat.offset = 0.0;
            viewerStat.lookatX = 0.0;
            viewerStat.lookatY = 0.0;
        }
        else if ( key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q )
        {
            bQuit = true;
        }
        else if ( key == GLFW_KEY_C )
        {
            bVisualizedRGBFrame ^= true;
        }
        else if ( key == GLFW_KEY_E )
        {
            if ( bComputeCloud )
            {
                bComputeCloud = false;
                exportToPlyFile();
                bComputeCloud = true;
            }
            else
            {
                exportToPlyFile();
            }
        }
        else if ( key == GLFW_KEY_H )
        {
            bShowToolbar ^= true;
        }
        else if ( key == GLFW_KEY_P )
        {
            bComputeCloud ^= true;
        }
        else if ( key == GLFW_KEY_S )
        {
            if ( shadingMode == SHADING_MODE_POINT )
            {
                shadingMode = SHADING_MODE_MESH;
            }
            else
            {
                shadingMode = SHADING_MODE_POINT;
            }
        }
    }

    if ( action == GLFW_PRESS || action == GLFW_REPEAT )
    {
        if ( key == GLFW_KEY_RIGHT )
        {
            viewerStat.yaw -= 1.0f;
        }
        else if ( key == GLFW_KEY_LEFT )
        {
            viewerStat.yaw += 1.0f;
        }
        else if ( key == GLFW_KEY_DOWN )
        {
            viewerStat.pitch += 1.0f;
        }
        else if ( key == GLFW_KEY_UP )
        {
            viewerStat.pitch -= 1.0f;
        }
    }
}

void mouse_button_callback( GLFWwindow* window, int button, int action, int mods )
{
    if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
    {
        viewerStat.mouseLeft = true;
    }
    else
    {
        viewerStat.mouseLeft = false;
    }

    if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
    {
        viewerStat.mouseRight = true;
    }
    else
    {
        viewerStat.mouseRight = false;
    }
}

void scroll_callback( GLFWwindow* window, double xoffset, double yoffset )
{
    viewerStat.offset -= static_cast<float>( yoffset * 100 );
}

int main( int argc, char *argv[] )
{
    GLFWwindow* window;

    bComputeCloud = true;
    bQuit = false;
    bShowToolbar = true;
    bVisualizedRGBFrame = true;

    initParam();

    if ( !openDevice() )
    {
        system( "pause" );
        return 0;
    }
    if ( !streamCreate() )
    {
        system( "pause" );
        return 0;
    }
    setDepthVideoMode( g_selectedDepthRes );
    setColorVideoMode();
    updatePointsDataSize();
    if ( !streamStart() )
    {
        system( "pause" );
        return 0;
    }

    // Initialize the library
    if ( !glfwInit() )
    {
        streamStop();
        closeDevice();
        return false;
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow( 1280, 720, "Ni2PointCloud", NULL, NULL );

    if ( !window )
    {
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent( window );

    toolbarInit( window );

    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetScrollCallback( window, scroll_callback );
    glfwSetCursorPosCallback( window, cursor_position_callback );
    glfwSetKeyCallback( window, key_callback );

    colorMapInit();

    // Loop until the user closes the window
    while ( !glfwWindowShouldClose( window ) )
    {
        // Poll for and process events
        glfwPollEvents();

        if ( bComputeCloud )
        {
            readFrame();
            niComputeCloud();
        }

        viewerDraw( window, bVisualizedRGBFrame );

        if ( bShowToolbar )
        {
            toolbarDraw();
            if ( g_preSelectedDepthRes != g_selectedDepthRes )
            {
                streamStop();
                if ( setDepthVideoMode( g_selectedDepthRes ) )
                {
                    setColorVideoMode();
                    g_preSelectedDepthRes = g_selectedDepthRes;
                }
                updatePointsDataSize();
                streamStart();

                bComputeCloud = true;
            }
        }

        if ( bQuit )
        {
            break;
        }

        // Swap front and back buffers
        glfwSwapBuffers( window );
    }
    streamStop();
    closeDevice();

    toolbarClose();
    glfwTerminate();
    return 0;
}
