/*****************************************************************************
*
* Copyright (C) 2021 LIPS Corp.
*
* Ni2RawViewer-gl
*
* This sample opens a .raw file with argv and draw it in OpenGL window.
*
* Viewer hot keys:
*                   H/h: show/close help menu
*               Esc/Q/q: exit program
*                 Space: reset window size
*     up/left arrow key: move to previous frame
*  down/right arrow key: move to next frame
*
*****************************************************************************/

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

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

enum videoStream
{
    COLOR = 1,
    DEPTH = 2,
    IR = 4
};

enum moveToNewFrameAction
{
    MoveToPreviousFrame,
    MoveToNextFrame
};

int rawFileType;
int rawFileImgWidth, rawFileImgHeight;
std::string strFilePath, strFilename;
bool isBackslash;

unsigned int textureID;
int imgWidth, imgHeight;
unsigned char *img;
bool bShowHelpMenu;

int getRawFileType( std::string filename )
{
    if ( std::string::npos != filename.find( "Color_" ) )
    {
        return COLOR;
    }
    if ( std::string::npos != filename.find( "Depth_" ) )
    {
        return DEPTH;
    }
    if ( std::string::npos != filename.find( "IR_" ) )
    {
        return IR;
    }
    return 0;
}

int getFrameID()
{
    int index1 = strFilename.find( "_" );
    int index2 = strFilename.find( "_width_" );
    if ( index1 != std::string::npos && index2 != std::string::npos )
    {
        std::string strFrameID = strFilename.substr( index1 + 1, index2 - index1 - 1 );
        return std::stoi( strFrameID );
    }
    return -1;
}

bool getRawFileImgResolution( std::string filename )
{
    int index1 = strFilename.find( "_width_" );
    int index2 = strFilename.find( "_height_" );
    int index3 = strFilename.find( ".raw" );
    if ( index1 != std::string::npos && index2 != std::string::npos && index3 != std::string::npos )
    {
        std::string strWidth = strFilename.substr( index1 + 7, index2 - index1 - 7 );
        rawFileImgWidth = std::stoi( strWidth );

        std::string strHeight = strFilename.substr( index2 + 8, index3 - index2 - 8 );
        rawFileImgHeight = std::stoi( strHeight );
        return true;
    }
    return false;
}

bool readRawFile()
{
    int fileSize = 0;

    //check filename extension
    int index = strFilename.find( ".raw" );
    if ( index == std::string::npos )
    {
        std::cout << "[ERROR] Not support this filename extension." << std::endl;
        return false;
    }

    //open file
    std::string strOpenFilename;
    if ( isBackslash )
    {
        strOpenFilename = strFilePath + "\\" + strFilename;
    }
    else
    {
        strOpenFilename = strFilePath + "/" + strFilename;
    }

    FILE *file = fopen( strOpenFilename.c_str(), "r+" );
    if ( file == NULL )
    {
        std::cout << "[ERROR] Fail to open file." << std::endl;
        return false;
    }

    //get file size
    fseek( file, 0, SEEK_END );
    fileSize = ftell( file );

    //close file
    fclose( file );

    //check file size
    if ( fileSize == 0 )
    {
        std::cout << "[ERROR] File size is 0." << std::endl;
        return false;
    }

    //check resolution of .raw
    int rawFileImgSize = 0;
    if ( rawFileType == COLOR )
    {
        rawFileImgSize = rawFileImgWidth * rawFileImgHeight * 3;
    }
    else if ( rawFileType == DEPTH || rawFileType == IR )
    {
        rawFileImgSize = rawFileImgWidth * rawFileImgHeight * 2;
    }
    if ( rawFileImgSize != fileSize )
    {
        std::cout << "[ERROR] Fail to get the resolution." << std::endl;
        return false;
    }

    if ( rawFileType == COLOR )
    {
        unsigned char *rawFileData;
        rawFileData = new unsigned char[rawFileImgWidth * rawFileImgHeight * 3];

        //read file data
        file = fopen( strOpenFilename.c_str(), "r+" );
        fread( rawFileData, sizeof( unsigned char ), rawFileImgWidth * rawFileImgHeight * 3, file );
        fclose( file );

        //prepare array for OpenGL to draw
        if ( img != nullptr )
        {
            delete[] img;
            img = nullptr;
        }
        imgWidth = rawFileImgWidth;
        imgHeight = rawFileImgHeight;
        img = new unsigned char[rawFileImgWidth * rawFileImgHeight * 3];
        for ( int i = 0; i < ( rawFileImgWidth * rawFileImgHeight * 3 ); i++ )
        {
            img[i] = rawFileData[i];
        }
        delete[] rawFileData;
    }
    else if ( rawFileType == DEPTH || rawFileType == IR )
    {
        unsigned short int *rawFileData;
        rawFileData = new unsigned short int[rawFileImgWidth * rawFileImgHeight];

        //read file data
        file = fopen( strOpenFilename.c_str(), "r+" );
        fread( rawFileData, sizeof( unsigned short int ), rawFileImgWidth * rawFileImgHeight, file );
        fclose( file );

        //prepare array for OpenGL to draw
        if ( img != nullptr )
        {
            delete[] img;
            img = nullptr;
        }
        imgWidth = rawFileImgWidth;
        imgHeight = rawFileImgHeight;
        img = new unsigned char[rawFileImgWidth * rawFileImgHeight * 3];

        if ( rawFileType == DEPTH )
        {
            int maxValue = 100000;
            float depthHist[100000] = { 0.0f };
            int pointSize = 0;
            for ( int i = 0; i < ( rawFileImgWidth * rawFileImgHeight ); i++ )
            {
                if ( rawFileData[i] != 0 && rawFileData[i] < maxValue )
                {
                    depthHist[rawFileData[i]]++;
                    pointSize++;
                }
            }
            for ( int i = 1; i < maxValue; i++ )
            {
                depthHist[i] += depthHist[i - 1];
            }
            for ( int i = 1; i < maxValue; i++ )
            {
                if ( depthHist[i] != 0 && pointSize > 0 )
                {
                    depthHist[i] = ( pointSize - depthHist[i] ) / pointSize;
                }
            }
            for ( int i = 0; i < ( rawFileImgWidth * rawFileImgHeight ); i++ )
            {
                img[3 * i] = depthHist[rawFileData[i]] * 255;//R
                img[3 * i + 1] = img[3 * i];//G
                img[3 * i + 2] = 0;//B
            }
        }
        else if ( rawFileType == IR )
        {
            for ( int i = 0; i < ( rawFileImgWidth * rawFileImgHeight ); i++ )
            {
                img[3 * i] = rawFileData[i] * 255 / 4096.0; //R
                img[3 * i + 1] = img[3 * i];//G
                img[3 * i + 2] = img[3 * i];//B
            }
        }
        delete[] rawFileData;
    }

    glBindTexture( GL_TEXTURE_2D, textureID );
    glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img );
    return true;
}

bool moveToNewFrame( int action )
{
    std::string strNewFilename, strWidth, strHeight, strFrameID;
    int iFrameID = getFrameID();

    if ( action == MoveToPreviousFrame )
    {
        iFrameID--;
    }
    else if ( action == MoveToNextFrame )
    {
        iFrameID++;
    }

    if ( iFrameID < 0 )
    {
        return false;
    }

    strWidth = std::to_string( rawFileImgWidth );
    strHeight = std::to_string( rawFileImgHeight );
    strFrameID = std::to_string( iFrameID );
    if ( rawFileType == DEPTH )
    {
        strNewFilename = "Depth_" + strFrameID + "_width_" + strWidth + "_height_" + strHeight + ".raw";
    }
    else if ( rawFileType == COLOR )
    {
        strNewFilename = "Color_" + strFrameID + "_width_" + strWidth + "_height_" + strHeight + ".raw";
    }
    else if ( rawFileType == IR )
    {
        strNewFilename = "IR_" + strFrameID + "_width_" + strWidth + "_height_" + strHeight + ".raw";
    }

    std::string str = strFilename;
    strFilename = strNewFilename;
    if ( !readRawFile() )
    {
        strFilename = str;
        return false;
    }
    return true;
}

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    if ( action != GLFW_PRESS )
    {
        return;
    }

    if ( key == GLFW_KEY_H )
    {
        bShowHelpMenu = !bShowHelpMenu;
    }
    else if ( key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE )
    {
        glfwSetWindowShouldClose( window, GLFW_TRUE );
    }
    else if ( key == GLFW_KEY_SPACE )
    {
        glfwSetWindowSize( window, imgWidth, imgHeight );
    }
    else if ( key == GLFW_KEY_UP || key == GLFW_KEY_LEFT )
    {
        if ( !moveToNewFrame( MoveToPreviousFrame ) )
        {
            std::cout << "[ERROR] Fail to move to previous frame." << std::endl;
        }
    }
    else if ( key == GLFW_KEY_DOWN || key == GLFW_KEY_RIGHT )
    {
        if ( !moveToNewFrame( MoveToNextFrame ) )
        {
            std::cout << "[ERROR] Fail to move to next frame." << std::endl;
        }
    }
}

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        return 0;
    }

    if ( !glfwInit() )
    {
        return 0;
    }

    GLFWwindow* window;

    // for help screen
    struct nk_context* nk;
    struct nk_font_atlas* atlas;

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow( 320, 240, "Ni2RawViewer-gl", NULL, NULL );
    if ( !window )
    {
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent( window );

    //for help screen
    gladLoadGL( glfwGetProcAddress );
    nk = nk_glfw3_init( window, NK_GLFW3_INSTALL_CALLBACKS );
    nk_glfw3_font_stash_begin( &atlas );
    nk_glfw3_font_stash_end();

    glfwSetKeyCallback( window, key_callback );

    textureID = 0;
    glEnable( GL_TEXTURE_2D );
    glGenTextures( 1, &textureID );

    img = nullptr;
    imgWidth = 0;
    imgHeight = 0;
    isBackslash = false;
    bShowHelpMenu = true;

    //Get file information.
    std::string strInput = argv[1];
    int index = strInput.find_last_of( "\\" );
    if ( index != std::string::npos )
    {
        strFilePath = strInput.substr( 0, index );
        strFilename = strInput.substr( index + 1 );
        isBackslash = true;
    }
    else
    {
        index = strInput.find_last_of( "/" );
        if ( index != std::string::npos )
        {
            strFilePath = strInput.substr( 0, index );
            strFilename = strInput.substr( index + 1 );
        }
        else
        {
            strFilePath = ".";
            strFilename = strInput;
        }
    }

    //get .raw type.
    rawFileType = getRawFileType( strFilename );
    if ( rawFileType == 0 )
    {
        std::cout << "[ERROR] Not support this filename." << std::endl;
        return 0;
    }

    //get width and height of .raw
    if ( !getRawFileImgResolution( strFilename ) )
    {
        std::cout << "[ERROR] Fail to get the resolution from this filename." << std::endl;
        return 0;
    }

    if ( !readRawFile() )
    {
        return 0;
    }

    glfwSetWindowSize( window, imgWidth, imgHeight );

    // Loop until the user closes the window
    while ( !glfwWindowShouldClose( window ) )
    {
        if ( imgWidth <= 0 || imgHeight <= 0 )
        {
            break;
        }

        int windowWidth, windowHeight;
        glfwGetFramebufferSize( window, &windowWidth, &windowHeight );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
        glOrtho( 0, windowWidth, 0, windowHeight, -1.0, 1.0 );
        glViewport( 0, 0, windowWidth, windowHeight );

        glBindTexture( GL_TEXTURE_2D, textureID );
        glBegin( GL_QUADS );

        glTexCoord2f( 0.0, 1.0 );
        glVertex2f( 0, 0 );

        glTexCoord2f( 1.0, 1.0 );
        glVertex2f( windowWidth, 0 );

        glTexCoord2f( 1.0, 0.0 );
        glVertex2f( windowWidth, windowHeight );

        glTexCoord2f( 0.0, 0.0 );
        glVertex2f( 0, windowHeight );

        glEnd();

        if ( bShowHelpMenu )
        {
            nk_glfw3_new_frame();
            if ( nk_begin( nk, "", nk_rect( 0.0f, 0.0f, 360.0f, 230.0f ), 0 ) )
            {
                nk_flags align_left = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE;
                nk_layout_row_dynamic( nk, 20, 1 );
                std::string strShowFilename = "Filename: " + strFilename;
                std::string strShowImgSize = "Size: " + std::to_string( imgWidth ) + " x " + std::to_string( imgHeight );
                nk_label( nk, strShowFilename.c_str(), align_left );
                nk_label( nk, strShowImgSize.c_str(), align_left );
                nk_label( nk, "", 0 );
                nk_label( nk, "hot keys:", align_left );
                nk_label( nk, "H/h                    Show/Close This Help Menu", align_left );
                nk_label( nk, "Q/q/Esc                Exit Program", align_left );
                nk_label( nk, "Space                  Reset window size", align_left );
                nk_label( nk, "up/left arrow key      Move to previous frame", align_left );
                nk_label( nk, "down/right arrow key   Move to next frame", align_left );
            }
            nk_end( nk );
            nk_glfw3_render( NK_ANTI_ALIASING_ON );
        }

        // Swap front and back buffers
        glfwSwapBuffers( window );

        // Poll for and process events
        glfwPollEvents();
    }

    if ( img != nullptr )
    {
        delete[] img;
    }

    glfwTerminate();
    return 0;
}
