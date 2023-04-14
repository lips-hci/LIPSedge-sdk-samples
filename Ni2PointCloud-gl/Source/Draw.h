#ifndef __DRAW_H__
#define __DRAW_H__

#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

typedef enum
{
    SHADING_MODE_POINT,
    SHADING_MODE_MESH
} ShadingMode;

struct viewerState
{
    double yaw;
    double pitch;
    double lastX;
    double lastY;
    float offset;
    float lookatX;
    float lookatY;
    bool mouseLeft;
    bool mouseRight;
};

void viewerInit();
void viewerDraw( GLFWwindow* window, bool bVisualizedRGBFrame );

void toolbarInit( GLFWwindow* window );
void toolbarDraw();
void toolbarClose();

extern struct viewerState viewerStat;
extern ShadingMode shadingMode;
extern float g_toolbarWidth, g_toolbarHeight;

#endif //__DRAW_H__