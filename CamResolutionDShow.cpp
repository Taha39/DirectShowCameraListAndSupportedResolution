// CamResolutionDShow.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>

#include <dshow.h>
#include <locale>
//#include <vector>
//#include <map>
#include <algorithm>
#include "CamResolutionDShow.h"
#pragma comment(lib, "strmiids.lib")

using namespace std;

typedef vector<std::pair<int, int>> vec_camera_resolution_;
typedef map<string, vec_camera_resolution_> map_camera_detail_;


void setcolor(unsigned int color)
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, color | FOREGROUND_INTENSITY);
}

void _FreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// pUnk should not be used.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}


/*HRESULT*/vec_camera_resolution_ CamCaps(IBaseFilter *pBaseFilter)
{
	HRESULT hr = 0;
	vector<IPin*> pins;
	IEnumPins *EnumPins;
	pBaseFilter->EnumPins(&EnumPins);
	pins.clear();
	for (;;)
	{
		IPin *pin;
		hr = EnumPins->Next(1, &pin, NULL);
		if (hr != S_OK) { break; }
		pins.push_back(pin);
		pin->Release();
	}
	EnumPins->Release();

	//printf("Device pins number: %d\n", pins.size());
	vec_camera_resolution_ vec_resolution;

	PIN_INFO pInfo;
	for (int i = 0; i < pins.size(); i++)
	{
		pins[i]->QueryPinInfo(&pInfo);

		setcolor(RED);

		/*if (pInfo.dir == 0)
		{
			wprintf(L"Pin name: %s (Ввод)\n", pInfo.achName);
		}

		if (pInfo.dir == 1)
		{
			wprintf(L"Pin name: %s (Выход)\n", pInfo.achName);
		}*/

		IEnumMediaTypes *emt = NULL;
		pins[i]->EnumMediaTypes(&emt);

		AM_MEDIA_TYPE *pmt;

		vector<SIZE> modes;
		setcolor(GRAY);
		//wprintf(L"Avialable resolutions.\n", pInfo.achName);
		
		for (;;)
		{
			hr = emt->Next(1, &pmt, NULL);
			if (hr != S_OK) { break; }

			if ((pmt->formattype == FORMAT_VideoInfo) &&
				//(pmt->subtype == MEDIASUBTYPE_RGB24) &&
				(pmt->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
				(pmt->pbFormat != NULL))
			{
				VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)pmt->pbFormat;
				SIZE s;
				// Get frame size
				s.cy = pVIH->bmiHeader.biHeight;
				s.cx = pVIH->bmiHeader.biWidth;
				vec_resolution.push_back(make_pair(s.cx, s.cy));
				// Битрейт
				unsigned int bitrate = pVIH->dwBitRate;
				modes.push_back(s);
				// Bits per pixel
				unsigned int bitcount = pVIH->bmiHeader.biBitCount;
				REFERENCE_TIME t = pVIH->AvgTimePerFrame; // blocks (100ns) per frame
				int FPS = floor(10000000.0 / static_cast<double>(t));
				//printf("Size: x=%d\ty=%d\tFPS: %d\t bitrate: %ld\tbit/pixel:%ld\n", s.cx, s.cy, FPS, bitrate, bitcount);
			}
			_FreeMediaType(*pmt);
		}
		//----------------------------------------------------
		// 
		// 
		// 
		//----------------------------------------------------
		modes.clear();
		emt->Release();
	}

	pins.clear();

	return vec_resolution;
}

/*
* Do something with the filter. In this sample we just test the pan/tilt properties.
*/
vec_camera_resolution_ process_filter(IBaseFilter *pBaseFilter)
{
	return CamCaps(pBaseFilter);
}


/*
* Enumerate all video devices
*
* See also:
*
* Using the System Device Enumerator:
*     http://msdn2.microsoft.com/en-us/library/ms787871.aspx
*/
map<string, vector<std::pair<int, int>>> enum_devices()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	HRESULT hr;
	//setcolor(GRAY);
	printf("Enumeraring videoinput devices ...\n");

	// Create the System Device Enumerator.
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		fprintf(stderr, "Error. Can't create enumerator.\n");
		return map_camera_detail_{};
	}

	map_camera_detail_ map_resolution;
	// Obtain a class enumerator for the video input device category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK)
	{
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
		{
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
				(void **)&pPropBag);
			if (SUCCEEDED(hr))
			{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				wstring camName;
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					setcolor(GREEN);
					camName = varName.bstrVal;
					//wprintf(L"------------------> %s <------------------\n", varName.bstrVal);
				}
				VariantClear(&varName);

				// To create an instance of the filter, do the following:
				IBaseFilter *pFilter;
				hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
					(void**)&pFilter);


				string name(camName.begin(), camName.end());
				auto resolution = process_filter(pFilter);
				vector<std::pair<int, int>>::iterator itr_unq;
				sort(resolution.begin(), resolution.end());
				itr_unq = std::unique(resolution.begin(), resolution.begin() + resolution.size());
				resolution.resize(std::distance(resolution.begin(), itr_unq));

				map_resolution.insert({ name, resolution });

				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();

	CoUninitialize();
	return map_resolution;
}


int wmain(int argc, wchar_t* argv[])
{
	//setlocale(LC_ALL, "Russian");
	//int result;

	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	auto cam_details = enum_devices();
	auto itr = cam_details.begin();
	for (; itr != cam_details.end(); itr++) {
		setcolor(GREEN);
		std::cout << "\n Cam Name: " << itr->first << " Resolution supportd: \n";
		auto resolution = itr->second;
		for (int i = 0; i < resolution.size(); i++) {
			setcolor(BLUE);
			std::cout << "\t \t " << resolution[i].first << " x " << resolution[i].second << std::endl;
		}
	}

	//CoUninitialize();
	getchar();
	return 0;
}