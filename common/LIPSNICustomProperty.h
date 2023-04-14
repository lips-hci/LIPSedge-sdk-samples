#ifndef LIPS_NI_CUSTOME_PROPERTY
#define LIPS_NI_CUSTOME_PROPERTY

#include <cstdint>

/*
 * This header is to define LIPS customized properties for OpenNI/OpenNI2
 *
 * To create new property for your purpose, you need to define
 * Name (in string) and index number (in integer)
 *
 * NOTE: some index number are already used by NI2 and you cannot overwrite it
 *
 * Update: 2021/11/10
 *
 * API Version: MAJOR.MINOR.PATCH
 */
#define API_VER_MAJOR 1
#define API_VER_MINOR 4
#define API_VER_PATCH 0

#define radian2deg(r) ((float)r*57.2957795)

// Define property in integer format for NI2
enum
{
    // Pixel format
    ONI_PIXEL_FORMAT_DEPTH_IR_COMPRESSED     = 150,

    // LIPS Stream properties
    // Camera intrinsics
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_X      = 200,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_Y      = 201,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_X   = 202,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_Y   = 203,
    // IMU
    LIPS_STREAM_PROPERTY_IMUDATA             = 204, //imudata
    // Camera extrinsics
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_R      = 205,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_L      = 206,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_X_R = 207,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_Y_R = 208,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_X_L = 209,
    LIPS_STREAM_PROPERTY_PRINCIPAL_POINT_Y_L = 210,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_X_R    = 211,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_Y_R    = 212,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_X_L    = 213,
    LIPS_STREAM_PROPERTY_FOCAL_LENGTH_Y_L    = 214,
    LIPS_STREAM_PROPERTY_ORIGINAL_RES_X      = 215,
    LIPS_STREAM_PROPERTY_ORIGINAL_RES_Y      = 216,

    // Other
    LIPS_STREAM_PROPERTY_IMU_EN              = 217, //reserved for string "imu_en"
    LIPS_DEPTH_IMU_ACCEL_OFFSET              = 218,
    LIPS_DEPTH_IMU_GYRO_OFFSET               = 219,
    LIPS_STREAM_PROPERTY_EXTRINSIC_TO_DEPTH  = 220,
    LIPS_STREAM_PROPERTY_EXTRINSIC_TO_COLOR  = 221,
    LIPS_STREAM_PROPERTY_EXTRINSIC_TO_IR     = 224,
    LIPS_STREAM_PROPERTY_RADIAL_DISTORTION   = 222,
    LIPS_STREAM_PROPERTY_TANGENTIAL_DISTORTION = 223,

    // LIPS Device properties
    LIPS_DEVICE_NAME                         = 300,
    LIPS_DEVICE_SENSOR_INFO_IR               = 301,
    LIPS_DEVICE_SENSOR_INFO_COLOR            = 302,
    LIPS_DEVICE_SENSOR_INFO_DEPTH            = 303,
    // Refer to LIPS Config settings
    LIPS_DEVICE_CONFIG_LENS_MODE             = 304, //lens mode, e.g. 0/1/2/3..
    LIPS_DEVICE_CONFIG_LENS_MODE_EXT         = 305, //lens mode extendtion
    LIPS_DEVICE_PROPERTY_IMUDATA             = 306,
    LIPS_DEVICE_PROPERTY_LASER_ENABLE        = 307, //1=laser on, 0=laser off

    // Sensor properties
    LIPS_DEPTH_SENSOR_READ_REGISTER          = 400, //lipsDepthSensorReadRegister
    LIPS_DEPTH_SENSOR_WRITE_REGISTER         = 401, //lipsDepthSensorWriteRegister
    LIPS_DEPTH_SENSOR_TEMPERATURE            = 402,
    LIPS_DEPTH_SENSOR_LOW_POWER_EN           = 403, //1=low-power mode, 0=normal mode

    // Refer to LIPS Config settings
    LIPS_CONFIG_LENS_MODE                    = 500, //[Deprecated] replaced by Device property

    LIPS_DEVICE_FACE_RECOGNITION             = 600,
    LIPS_DEVICE_FACE_REGISTRATION            = 601,
    LIPS_DEVICE_FACE_DELETE_DATABASE         = 602,
    LIPS_DEVICE_FACE_NUMBER_REGISTERED       = 603,
};

// Define properties in string format for NI
static const char LIPS_STREAM_PROPERTY_IMUDATA_STR[] = "imudata";
static const char LIPS_DEPTH_SENSOR_READ_REGISTER_STR[] = "lipsDepthSensorReadRegister";
static const char LIPS_DEPTH_SENSOR_WRITE_REGISTER_STR[] = "lipsDepthSensorWriteRegister";
static const char LIPS_CONFIG_LENS_MODE_STR[] = "lens_mode";
static const char LIPS_DEPTH_SENSOR_TEMPERATURE_STR[] = "sensor_temp";

// Device properties
static const char LIPS_DEVICE_CONFIG_LENS_MODE_STR[] = "lips_device_lens_mode";
static const char LIPS_DEVICE_CONFIG_LENS_MODE_EXT_STR[] = "lips_device_lens_mode_ext";

//IMU offset
static const char LIPS_DEPTH_IMU_ACCEL_OFFSET_STR[] = "imu_accel_offset";
static const char LIPS_DEPTH_IMU_GYRO_OFFSET_STR[] = "imu_gyro_offset";

// Pre-defined depth operation mode
static const unsigned int LIPS_CONFIG_NEAR_MODE   = 0;
static const unsigned int LIPS_CONFIG_NORMAL_MODE = 1;

typedef enum
{
    STANDBY_MODE     = 0, //same as Low-Power Mode
    NORMAL_OPERATION = 1, //Active mode
    DYN_POWER_DOWN   = 2  //OPT8320 only
} DeviceFuncMode ;

struct LIPSSensorRegRWCmd
{
    uint8_t  dev;
    uint16_t addr;
    uint8_t  MSB;
    uint8_t  LSB;
    uint32_t data;
};

struct CameraExtrinsicMatrix
{
    float rotation[3][3];
    float translation[3];
};

struct RadialDistortionCoeffs
{
    double k1, k2, k3, k4, k5, k6;
};

struct TangentialDistortionCoeffs
{
    double p1, p2;
};

#endif //LIPS_NI_CUSTOME_PROPERTY