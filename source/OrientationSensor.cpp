#include "OrientationSensor.h"

#include <tchar.h>



#if 1
static ISensor* pOrientationSensor = NULL;

int OrientationSensor_Initialize(){
	// SensorManager オブジェクトの COM インターフェイスを作成する
		ISensorManager* pSensorManager = NULL;
		HRESULT hr = ::CoCreateInstance(__uuidof(SensorManager), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager));
		
		if (FAILED(hr))
		{
			//TCHAR line[] = _T("Unable to CoCreateInstance() the SensorManager.");
			//TextOut(hdc, 0, txt_y, line, _tcslen(line)); txt_y += txt_height;
			return 0;
		}
		
		
		// コンピューターの 3軸傾斜計センサー値を取得する
		{
			ISensorCollection* pSensorCollection = NULL;
			hr = pSensorManager->GetSensorsByType(SENSOR_TYPE_AGGREGATED_DEVICE_ORIENTATION, &pSensorCollection);
			if (FAILED(hr))
			{
				pSensorManager->Release();
				return 0;
			}
			
			ULONG count;
			pSensorCollection->GetCount(&count);
			if(count > 0){
				

				ISensor* pSensor = NULL;
				pSensorCollection->GetAt(0, &pSensor);
				
				// Check the current sensor state.
				SensorState state = SENSOR_STATE_NOT_AVAILABLE;
				hr = pSensor->GetState(&state);
				if(SUCCEEDED(hr)){
			

					if(state == SENSOR_STATE_ACCESS_DENIED){
					   hr = pSensorManager->RequestPermissions(0, pSensorCollection, TRUE);

				   }
				}




				ISensorDataReport* pReport = NULL;
				hr = pSensor->GetData(&pReport);
				if (FAILED(hr)){
					pSensor->Release();
					pSensorManager->Release();
				}

				{					
					PROPVARIANT var = {0};
					PROPVARIANT var_y = {0};
					PROPVARIANT var_z = {0};
					hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_ROTATION_MATRIX, &var);
					
					if (FAILED(hr)){
						pReport->Release();
						pSensor->Release();
					}else if(hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)){
						pReport->Release();
						pSensor->Release();
					}
				}
				pReport->Release();
				
				pOrientationSensor = pSensor;
			}

		}

		return 1;
}

int OrientationSensor_GetRotationMatrix(float* pmatrix){

	if(pOrientationSensor == NULL){
		return 0;
	}

	ISensor* pSensor = pOrientationSensor;

	ISensorDataReport* pReport = NULL;
	HRESULT hr = pSensor->GetData(&pReport);
	if (SUCCEEDED(hr)){
		

		PROPVARIANT var = {0};
		
		hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_ROTATION_MATRIX, &var);
				
		if (SUCCEEDED(hr)){
			if(var.vt == (VT_VECTOR | VT_UI1)){
				float* matrix = (float*)(var.caub.pElems);
				for(int k = 0; k < 9; k++){
					pmatrix[k] = matrix[k];
				}
			}else{
				return 0;
			}
		}else if(hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)){
			
			pReport->Release();
			return 0;

		}
	}
	pReport->Release();
			
	return 1;

}


int OrientationSensor_Terminate(){
	if(pOrientationSensor){
		pOrientationSensor->Release();
		return 0;
	}
	return 1;
}

#else

static ISensor* pInclinometerSensor = NULL;
				
int OrientationSensor_Initialize(){
	// SensorManager オブジェクトの COM インターフェイスを作成する
		ISensorManager* pSensorManager = NULL;
		HRESULT hr = ::CoCreateInstance(__uuidof(SensorManager), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSensorManager));
		
		if (FAILED(hr))
		{
			//TCHAR line[] = _T("Unable to CoCreateInstance() the SensorManager.");
			//TextOut(hdc, 0, txt_y, line, _tcslen(line)); txt_y += txt_height;
			return 0;
		}
		
		
		// コンピューターの 3軸傾斜計センサー値を取得する
		{
			ISensorCollection* pSensorCollection = NULL;
			hr = pSensorManager->GetSensorsByType(SENSOR_TYPE_INCLINOMETER_3D, &pSensorCollection);
			if (FAILED(hr))
			{
				pSensorManager->Release();
				return 0;
			}
			
			ULONG count;
			pSensorCollection->GetCount(&count);
			if(count > 0){
				

				ISensor* pSensor = NULL;
				pSensorCollection->GetAt(0, &pSensor);
				
				// Check the current sensor state.
				SensorState state = SENSOR_STATE_NOT_AVAILABLE;
				hr = pSensor->GetState(&state);
				if(SUCCEEDED(hr)){
			

					if(state == SENSOR_STATE_ACCESS_DENIED){
					   hr = pSensorManager->RequestPermissions(0, pSensorCollection, TRUE);

				   }
				}




				ISensorDataReport* pReport = NULL;
				hr = pSensor->GetData(&pReport);
				if (FAILED(hr)){
					pSensor->Release();
					pSensorManager->Release();
				}

				{					
					PROPVARIANT var_x = {0};
					PROPVARIANT var_y = {0};
					PROPVARIANT var_z = {0};
					//hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &var);
					hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &var_x);
					hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &var_y);
					hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &var_z);
					//hr = pSensor->GetProperty(SENSOR_DATA_TYPE_ACCELERATION_X_G, &var);
				
					if (FAILED(hr)){
						pReport->Release();
						pSensor->Release();
					}else if(hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)){
						pReport->Release();
						pSensor->Release();
					}
				}
				pReport->Release();
				
				pInclinometerSensor = pSensor;
			}

		}

		return 1;
}

int OrientationSensor_GetYawPitchRoll(float* pYaw, float* pPitch, float* pRoll){

	if(pInclinometerSensor == NULL){
		return 0;
	}

	ISensor* pSensor = pInclinometerSensor;

	ISensorDataReport* pReport = NULL;
	HRESULT hr = pSensor->GetData(&pReport);
	if (SUCCEEDED(hr)){
		

		PROPVARIANT var_x = {0};
		PROPVARIANT var_y = {0};
		PROPVARIANT var_z = {0};
		//hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &var);
		hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_X_DEGREES, &var_x);
		hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Y_DEGREES, &var_y);
		hr = pReport->GetSensorValue(SENSOR_DATA_TYPE_TILT_Z_DEGREES, &var_z);
		//hr = pSensor->GetProperty(SENSOR_DATA_TYPE_ACCELERATION_X_G, &var);
				
		if (SUCCEEDED(hr)){
			
			*pYaw = var_z.fltVal;
			*pPitch = var_x.fltVal;
			*pRoll = var_y.fltVal;

		}else if(hr == HRESULT_FROM_WIN32(ERROR_NOT_FOUND)){
			
			pReport->Release();
			return 0;

		}
	}
	pReport->Release();
			
	return 1;

}


int OrientationSensor_Terminate(){
	if(pInclinometerSensor){
		pInclinometerSensor->Release();
		return 0;
	}
	return 1;
}
#endif