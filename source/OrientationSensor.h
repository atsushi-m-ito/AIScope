#ifndef OrientationSensor_H
#define OrientationSensor_H

#include <InitGuid.h>
#include <SensorsApi.h>
#include <Sensors.h>

int OrientationSensor_Initialize();

int OrientationSensor_GetYawPitchRoll(float* pYaw, float* pPitch, float* pRoll);

int OrientationSensor_GetRotationMatrix(float* pmatrix);
int OrientationSensor_Terminate();

#endif //!OrientationSensor_H