
#include "utility_funcs.h"
#include <Dshow.h>
#include <algorithm>
#include <comdef.h>

int getDeviceIndex(std::string vid, std::string pid)
{
	std::string str_vid = "VID_" + vid;
	std::string str_pid = "PID_" + pid;
	transform(str_vid.begin(), str_vid.end(), str_vid.begin(), ::tolower);
	transform(str_pid.begin(), str_pid.end(), str_pid.begin(), ::tolower);
	int vidfind = -1;
	int pidfind = -1;
	int retIdx = -1;

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));


	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			//if (!silent)printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}


				// Find the DevicePath, description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				//hr = pPropBag->Read(L"Description", &varName, 0);

				//if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				hr = pPropBag->Read(L"DevicePath", &varName, 0);

				if (SUCCEEDED(hr)) {

					//hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					_bstr_t bstr_t(varName.bstrVal);
					std::string strVal(bstr_t);

					vidfind = strVal.find(str_vid);
					pidfind = strVal.find(str_pid);

					printf("SETUP: %i) %s \n", deviceCounter, strVal.c_str());
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				if (vidfind >= 0 && pidfind >= 0) {
					retIdx = deviceCounter;
					break;
				}

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		//printf("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	//return deviceCounter;
	return retIdx;
}