#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include "OpenNI.h"
#include "Draw.h"
#include "Device.h"

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

struct viewerState viewerStat;
ShadingMode shadingMode;

// for toolbar
struct nk_context* nk;
struct nk_font_atlas* atlas;
float g_toolbarWidth, g_toolbarHeight;
std::vector<std::string> strResolution;

void drawAxes( float axisSize = 1.f, float axisWidth = 4.f )
{
    // Triangles For X axis
    glBegin( GL_TRIANGLES );
    glColor3f( 1.0f, 0.0f, 0.0f );
    glVertex3f( axisSize * 1.1f, 0.f, 0.f );
    glVertex3f( axisSize, -axisSize * 0.05f, 0.f );
    glVertex3f( axisSize, axisSize * 0.05f, 0.f );
    glVertex3f( axisSize * 1.1f, 0.f, 0.f );
    glVertex3f( axisSize, 0.f, -axisSize * 0.05f );
    glVertex3f( axisSize, 0.f, axisSize * 0.05f );
    glEnd();

    // Triangles For Y axis
    glBegin( GL_TRIANGLES );
    glColor3f( 0.f, 1.f, 0.f );
    glVertex3f( 0.f, axisSize * 1.1f, 0.0f );
    glVertex3f( 0.f, axisSize, 0.05f * axisSize );
    glVertex3f( 0.f, axisSize, -0.05f * axisSize );
    glVertex3f( 0.f, axisSize * 1.1f, 0.0f );
    glVertex3f( 0.05f * axisSize, axisSize, 0.f );
    glVertex3f( -0.05f * axisSize, axisSize, 0.f );
    glEnd();

    // Triangles For Z axis
    glBegin( GL_TRIANGLES );
    glColor3f( 0.0f, 0.0f, 1.0f );
    glVertex3f( 0.0f, 0.0f, 1.1f * axisSize );
    glVertex3f( 0.0f, 0.05f * axisSize, 1.0f * axisSize );
    glVertex3f( 0.0f, -0.05f * axisSize, 1.0f * axisSize );
    glVertex3f( 0.0f, 0.0f, 1.1f * axisSize );
    glVertex3f( 0.05f * axisSize, 0.f, 1.0f * axisSize );
    glVertex3f( -0.05f * axisSize, 0.f, 1.0f * axisSize );
    glEnd();

    glLineWidth( axisWidth );

    // Drawing Axis
    glBegin( GL_LINES );
    // X axis - Red
    glColor3f( 1.0f, 0.0f, 0.0f );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( axisSize, 0.0f, 0.0f );

    // Y axis - Green
    glColor3f( 0.0f, 1.0f, 0.0f );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( 0.0f, axisSize, 0.0f );

    // Z axis - Blue
    glColor3f( 0.0f, 0.0f, 1.0f );
    glVertex3f( 0.0f, 0.0f, 0.0f );
    glVertex3f( 0.0f, 0.0f, axisSize );
    glEnd();
}

void drawPointCloud( int width, int height, struct pointData* pointsData, ShadingMode shadingMode, bool bVisualizedRGBFrame )
{
    if ( shadingMode == SHADING_MODE_POINT )
    {
        glBegin( GL_POINTS );
        for ( int i = 0; i < ( width * height ); i++ )
        {
            if ( pointsData[i].depth != 0 )
            {
                if ( bVisualizedRGBFrame )
                {
                    glColor3f( pointsData[i].r, pointsData[i].g, pointsData[i].b );
                    glVertex3f( pointsData[i].worldX, pointsData[i].worldY, pointsData[i].worldZ );
                }
                else
                {
                    glColor3f( pointsData[i].colorMapR, pointsData[i].colorMapG, pointsData[i].colorMapB );
                    glVertex3f( pointsData[i].worldX, pointsData[i].worldY, pointsData[i].worldZ );
                }
            }
        }
        glEnd();
    }
    else if ( shadingMode == SHADING_MODE_MESH )
    {
        float dis = 50.0f;
        if ( !g_bDepthUnit1mm )
        {
            dis *= 10.0f;
        }
        int p1, p2, p3;
        glBegin( GL_TRIANGLES );
        for ( int y = 0, i = 0; y < ( height - 1 ); y++ )
        {
            for ( int x = 0; x < ( width - 1 ); x++ )
            {
                p1 = width * y + x;
                p2 = width * ( y + 1 ) + ( x + 1 );
                p3 = width * y + ( x + 1 );
                if ( pointsData[p1].depth > 0 && pointsData[p2].depth > 0 && pointsData[p3].depth > 0 )
                {
                    if ( abs( pointsData[p1].depth - pointsData[p2].depth ) > dis
                            || abs( pointsData[p1].depth - pointsData[p3].depth ) > dis
                            || abs( pointsData[p2].depth - pointsData[p3].depth ) > dis
                       )
                    {
                        continue;
                    }
                    if ( bVisualizedRGBFrame )
                    {
                        glColor3f( pointsData[p1].r, pointsData[p1].g, pointsData[p1].b );
                        glVertex3f( pointsData[p1].worldX, pointsData[p1].worldY, pointsData[p1].worldZ );

                        glColor3f( pointsData[p2].r, pointsData[p2].g, pointsData[p2].b );
                        glVertex3f( pointsData[p2].worldX, pointsData[p2].worldY, pointsData[p2].worldZ );

                        glColor3f( pointsData[p3].r, pointsData[p3].g, pointsData[p3].b );
                        glVertex3f( pointsData[p3].worldX, pointsData[p3].worldY, pointsData[p3].worldZ );
                    }
                    else
                    {
                        glColor3f( pointsData[p1].colorMapR, pointsData[p1].colorMapG, pointsData[p1].colorMapB );
                        glVertex3f( pointsData[p1].worldX, pointsData[p1].worldY, pointsData[p1].worldZ );

                        glColor3f( pointsData[p2].colorMapR, pointsData[p2].colorMapG, pointsData[p2].colorMapB );
                        glVertex3f( pointsData[p2].worldX, pointsData[p2].worldY, pointsData[p2].worldZ );

                        glColor3f( pointsData[p3].colorMapR, pointsData[p3].colorMapG, pointsData[p3].colorMapB );
                        glVertex3f( pointsData[p3].worldX, pointsData[p3].worldY, pointsData[p3].worldZ );
                    }
                }

                p1 = width * y + x;
                p2 = width * ( y + 1 ) + ( x + 1 );
                p3 = width * ( y + 1 ) + x;
                if ( pointsData[p1].depth > 0 && pointsData[p2].depth > 0 && pointsData[p3].depth > 0 )
                {
                    if ( abs( pointsData[p1].depth - pointsData[p2].depth ) > dis
                            || abs( pointsData[p1].depth - pointsData[p3].depth ) > dis
                            || abs( pointsData[p2].depth - pointsData[p3].depth ) > dis
                       )
                    {
                        continue;
                    }
                    if ( bVisualizedRGBFrame )
                    {
                        glColor3f( pointsData[p1].r, pointsData[p1].g, pointsData[p1].b );
                        glVertex3f( pointsData[p1].worldX, pointsData[p1].worldY, pointsData[p1].worldZ );

                        glColor3f( pointsData[p2].r, pointsData[p2].g, pointsData[p2].b );
                        glVertex3f( pointsData[p2].worldX, pointsData[p2].worldY, pointsData[p2].worldZ );

                        glColor3f( pointsData[p3].r, pointsData[p3].g, pointsData[p3].b );
                        glVertex3f( pointsData[p3].worldX, pointsData[p3].worldY, pointsData[p3].worldZ );
                    }
                    else
                    {
                        glColor3f( pointsData[p1].colorMapR, pointsData[p1].colorMapG, pointsData[p1].colorMapB );
                        glVertex3f( pointsData[p1].worldX, pointsData[p1].worldY, pointsData[p1].worldZ );

                        glColor3f( pointsData[p2].colorMapR, pointsData[p2].colorMapG, pointsData[p2].colorMapB );
                        glVertex3f( pointsData[p2].worldX, pointsData[p2].worldY, pointsData[p2].worldZ );

                        glColor3f( pointsData[p3].colorMapR, pointsData[p3].colorMapG, pointsData[p3].colorMapB );
                        glVertex3f( pointsData[p3].worldX, pointsData[p3].worldY, pointsData[p3].worldZ );
                    }
                }
            }
        }
        glEnd();
    }
}

void viewerInit()
{
    viewerStat.yaw = 0.0;
    viewerStat.pitch = 0.0;
    viewerStat.lastX = 0.0;
    viewerStat.lastY = 0.0;
    viewerStat.offset = 0.0;
    viewerStat.lookatX = 0.0;
    viewerStat.lookatY = 0.0;
    viewerStat.mouseLeft = false;
    viewerStat.mouseRight = false;
    shadingMode = SHADING_MODE_POINT;
}

void viewerDraw( GLFWwindow* window, bool bVisualizedRGBFrame )
{
    int windowWidth, windowHeight;
    glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

    glViewport( 0, 0, windowWidth, windowHeight );
    glClearColor( 0.0, 0.0, 0.0, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective( 60, ( float )windowWidth / windowHeight, 0.01f, 100000.0f );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    gluLookAt( viewerStat.lookatX, viewerStat.lookatY, 0, viewerStat.lookatX, viewerStat.lookatY, 1, 0, 1, 0 );

    glPointSize( windowWidth / 640.0 );
    glEnable( GL_DEPTH_TEST );

    glTranslatef( 0, 0, viewerStat.offset );
    glRotated( viewerStat.pitch, 1, 0, 0 );
    glRotated( viewerStat.yaw, 0, 1, 0 );
    glTranslatef( 0, 0, -0.5f );
    drawPointCloud( g_depthWidth, g_depthHeight, g_pointsData, shadingMode, g_bIsColorValid && bVisualizedRGBFrame );
    drawAxes( 100.0 );
}

void toolbarInit( GLFWwindow* window )
{
    gladLoadGL( glfwGetProcAddress );
    nk = nk_glfw3_init( window, NK_GLFW3_INSTALL_CALLBACKS );
    nk_glfw3_font_stash_begin( &atlas );
    nk_glfw3_font_stash_end();

    for ( int i = 0; i < g_depthVideoModeSize; i++ )
    {
        std::string str = std::to_string( g_depthVideoModes[i].getResolutionX() ) + " x "
                          + std::to_string( g_depthVideoModes[i].getResolutionY() ) + " @ "
                          + std::to_string( g_depthVideoModes[i].getFps() ) + " fps";
        if ( g_depthVideoModes[i].getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_1_MM )
        {
            str += " (1 mm)";
        }
        else if ( g_depthVideoModes[i].getPixelFormat() == openni::PIXEL_FORMAT_DEPTH_100_UM )
        {
            str += " (100 um)";
        }
        strResolution.push_back( str );
    }
}

void toolbarDraw()
{
    g_toolbarWidth = 410.0f;
    g_toolbarHeight = 400.0f;

    nk_glfw3_new_frame();

    if ( nk_begin( nk, "", nk_rect( 0.0f, 0.0f, g_toolbarWidth, g_toolbarHeight ), 0 ) )
    {
        nk_flags align_left = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
        nk_layout_row_dynamic( nk, 20, 1 );

        nk_label( nk, "Depth resolution:  ", align_left );
        if ( nk_combo_begin_label( nk, strResolution[g_selectedDepthRes].c_str(), nk_vec2( nk_widget_width( nk ), 200.0f ) ) )
        {
            nk_layout_row_dynamic( nk, 20, 1 );
            for ( int i = 0; i < g_depthVideoModeSize; i++ )
            {
                if ( nk_combo_item_label( nk, strResolution[i].c_str(), NK_TEXT_LEFT ) )
                {
                    g_selectedDepthRes = i;
                }
            }
            nk_combo_end( nk );
        }

        nk_label( nk, "", 0 ); // separator
        nk_label( nk, "Keyboard:", align_left );
        nk_label( nk, "C/c             visualized with RGB frame or color map", align_left );
        nk_label( nk, "E/e             export to .ply", align_left );
        nk_label( nk, "H/h             show/hide help screen", align_left );
        nk_label( nk, "P/p             start/pause", align_left );
        nk_label( nk, "S/s             Shading:point/mesh", align_left );
        nk_label( nk, "Esc/Q/q         exit", align_left );
        nk_label( nk, "Space           reset viewer", align_left );
        nk_label( nk, "Arrow keys      rotate around center", align_left );

        nk_label( nk, "", 0 ); // separator
        nk_label( nk, "Mouse left      rotate around center", align_left );
        nk_label( nk, "Mouse right     change field of view", align_left );
        nk_label( nk, "Mouse wheel     move forward/back", align_left );
    }

    nk_end( nk );
    nk_glfw3_render( NK_ANTI_ALIASING_ON );
}

void toolbarClose()
{
    nk_glfw3_shutdown();
}
