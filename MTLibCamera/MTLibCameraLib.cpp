
#include "stdafx.h"

#include "MTLibCameraLib.h"


HWND preview_hWnd = NULL;
MTLibFaceImageCallBack MTLibFaceImageCB = NULL;
MTLibFaceResultCallBack MTLibFaceResultCB = NULL;

static VOID __stdcall FaceImageCB(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata)
{
	if (NULL != MTLibFaceImageCB) {
		MTLibFaceImageCB(hWnd, imgBase64, userdata);
	}

	ZZReleaseString(imgBase64);
}

static VOID __stdcall FaceResultCB(HWND hWnd, LONG result, BSTR feature, ULONG_PTR userdata)
{
	if (NULL != MTLibFaceResultCB) {
		MTLibFaceResultCB(hWnd, result, userdata);
	}

	ZZReleaseString(feature);
}



int __stdcall MTLibLoadCamera()
{
	ZZInitFaceMgr();

	return 0;
}

int __stdcall MTLibOpenCamera(HWND hWnd,
	MTLibFaceImageCallBack fiCB, ULONG_PTR fi_userdata,
	MTLibFaceResultCallBack frCB, ULONG_PTR fr_userdata)
{
	preview_hWnd = hWnd;
	MTLibFaceImageCB = fiCB;
	MTLibFaceResultCB = frCB;

	ZZOpenDevice(preview_hWnd, 2);
	ZZOpenHideDevice(preview_hWnd, 4);
	ZZOpenVideo(preview_hWnd);
	ZZOpenHideVideo(preview_hWnd);
	ZZGetFaceFeature(preview_hWnd, FaceImageCB, fi_userdata, 2, FaceResultCB, fr_userdata);

	return 0;
}

int __stdcall MTLibCloseCamera()
{
	ZZStopGetFace(preview_hWnd);
	ZZCloseHideVideo(preview_hWnd);
	ZZCloseVideo(preview_hWnd);
	ZZCloseHideDevice(preview_hWnd);
	ZZCloseDevice(preview_hWnd);

	preview_hWnd = NULL;
	MTLibFaceImageCB = NULL;
	MTLibFaceResultCB = NULL;
	return 0;
}

int __stdcall MTLibUnloadCamera()
{
	ZZDeinitFaceMgr();

	return 0;
}

