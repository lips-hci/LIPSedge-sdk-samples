#ifndef __DEVICE_H__
#define __DEVICE_H__

struct pointData
{
    unsigned short depth;
    float worldX, worldY, worldZ;
    float r, g, b;
    float colorMapR, colorMapG, colorMapB;
    unsigned char ucharR, ucharG, ucharB;
};

void initParam();
void updatePointsDataSize();

bool openDevice();
void closeDevice();
bool streamCreate();
bool streamStart();
void streamStop();
bool readFrame();
void niComputeCloud();
void colorMapInit();
bool setDepthVideoMode( int index );
void setColorVideoMode();

extern int g_depthWidth, g_depthHeight;
extern bool g_bDepthUnit1mm;
extern struct pointData *g_pointsData;
extern int g_pointsDataSize;
extern bool g_bIsColorValid;

extern openni::VideoMode* g_depthVideoModes;
extern openni::VideoMode* g_colorVideoModes;
extern int g_depthVideoModeSize;
extern int g_colorVideoModeSize;
extern int g_selectedDepthRes;
extern int g_preSelectedDepthRes;

#endif //__DEVICE_H__