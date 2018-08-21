#ifndef __ZZLIBRARY_H__
#define __ZZLIBRARY_H__

#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef VOID (__stdcall *MoveDetectCallback)(HWND hWnd, ULONG_PTR userdata);
typedef VOID (__stdcall *IdCardCallback)(ULONG_PTR userdata);

typedef VOID (__stdcall *FaceImageCallback)(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata);
// result为-2表示未获取到人脸，为-1表示获取人脸特征失败，为-3表示不是活体，为0表示成功
typedef VOID (__stdcall *FaceResultCallback)(HWND hWnd, LONG result, BSTR feature, ULONG_PTR userdata);
// result为-2表示未获取到人脸，为-1表示匹配特征失败，为-3表示不是活体，为0表示成功
typedef VOID (__stdcall *FaceDetectCallback)(HWND hWnd, LONG result, ULONG_PTR userdata);
// result为1表示无图像，为2表示无人脸，为0表示不是活体，为3表示结果
typedef VOID (__stdcall *FaceDetectExCallback)(HWND hWnd, LONG result, ULONG score, ULONG_PTR userdata);

typedef VOID (__stdcall *FingerImageCallback)(BSTR imgBase64, ULONG_PTR userdata);
// result为-2表示超时，为-1表示获取特征值失败，为0表示成功
typedef VOID (__stdcall *FingerResultCallback)(LONG result, BSTR feature, ULONG_PTR userdata);
// result为-2表示超时，为-1表示匹配特征失败，为0表示成功
typedef VOID (__stdcall *FingerDetectCallback)(LONG result, ULONG_PTR userdata);
// result为1提示按下手指，为2提示拿开手指，为3表示结果
typedef VOID (__stdcall *FingerDetectExCallback)(LONG result, ULONG score1, ULONG score2, ULONG_PTR userdata);

BOOL __stdcall ZZReleaseString(BSTR str);
BOOL __stdcall ZZSetJpegQuality(ULONG nQuality);

LONG __stdcall ZZGetDevCount(VOID);
BOOL __stdcall ZZOpenDevice(HWND hWnd, WORD wPurpose);
BOOL __stdcall ZZOpenDeviceEx(HWND hWnd, ULONG nIndex);
BOOL __stdcall ZZOpenChildDevice(HWND hWnd, WORD wPurpose);
BOOL __stdcall ZZOpenChildDeviceEx(HWND hWnd, ULONG nIndex);
BOOL __stdcall ZZOpenHideDevice(HWND hWnd, WORD wPurpose);
BOOL __stdcall ZZOpenHideDeviceEx(HWND hWnd, ULONG nIndex);
BOOL __stdcall ZZCloseDevice(HWND hWnd);
BOOL __stdcall ZZCloseChildDevice(HWND hWnd);
BOOL __stdcall ZZCloseHideDevice(HWND hWnd);
BOOL __stdcall ZZCloseAllDevice(VOID);

BSTR __stdcall ZZGetDeviceName(HWND hWnd);
BSTR __stdcall ZZGetChildDeviceName(HWND hWnd);
BSTR __stdcall ZZGetHideDeviceName(HWND hWnd);
BOOL __stdcall ZZShowFilter(HWND hWnd);
BOOL __stdcall ZZShowChildFilter(HWND hWnd);
BOOL __stdcall ZZShowHideFilter(HWND hWnd);
BOOL __stdcall ZZShowPin(HWND hWnd);
BOOL __stdcall ZZShowChildPin(HWND hWnd);
BOOL __stdcall ZZShowHidePin(HWND hWnd);
BSTR __stdcall ZZGetVidPid(HWND hWnd);
BSTR __stdcall ZZGetChildVidPid(HWND hWnd);
BSTR __stdcall ZZGetHideVidPid(HWND hWnd);

BOOL __stdcall ZZIsSupportYUY2(HWND hWnd);
BOOL __stdcall ZZIsChildSupportYUY2(HWND hWnd);
BOOL __stdcall ZZIsHideSupportYUY2(HWND hWnd);
BOOL __stdcall ZZIsSupportMJPG(HWND hWnd);
BOOL __stdcall ZZIsChildSupportMJPG(HWND hWnd);
BOOL __stdcall ZZIsHideSupportMJPG(HWND hWnd);
LONG __stdcall ZZGetResolutionCount(HWND hWnd, LONG flag);
LONG __stdcall ZZGetChildResolutionCount(HWND hWnd, LONG flag);
LONG __stdcall ZZGetHideResolutionCount(HWND hWnd, LONG flag);
LONG __stdcall ZZGetResolutionWidth(HWND hWnd, LONG flag, LONG idx);
LONG __stdcall ZZGetChildResolutionWidth(HWND hWnd, LONG flag, LONG idx);
LONG __stdcall ZZGetHideResolutionWidth(HWND hWnd, LONG flag, LONG idx);
LONG __stdcall ZZGetResolutionHeight(HWND hWnd, LONG flag, LONG idx);
LONG __stdcall ZZGetChildResolutionHeight(HWND hWnd, LONG flag, LONG idx);
LONG __stdcall ZZGetHideResolutionHeight(HWND hWnd, LONG flag, LONG idx);
BOOL __stdcall ZZSetResolution(HWND hWnd, LONG flag, LONG width, LONG height);
BOOL __stdcall ZZSetChildResolution(HWND hWnd, LONG flag, LONG width, LONG height);
BOOL __stdcall ZZSetHideResolution(HWND hWnd, LONG flag, LONG width, LONG height);
LONG __stdcall ZZGetFormat(HWND hWnd);
LONG __stdcall ZZGetChildFormat(HWND hWnd);
LONG __stdcall ZZGetHideFormat(HWND hWnd);
LONG __stdcall ZZGetWidth(HWND hWnd);
LONG __stdcall ZZGetChildWidth(HWND hWnd);
LONG __stdcall ZZGetHideWidth(HWND hWnd);
LONG __stdcall ZZGetHeight(HWND hWnd);
LONG __stdcall ZZGetChildHeight(HWND hWnd);
LONG __stdcall ZZGetHideHeight(HWND hWnd);

BOOL __stdcall ZZSetChildPosition(HWND hWnd, LONG position);
BOOL __stdcall ZZSetChildRatio(HWND hWnd, LONG ratio);
BOOL __stdcall ZZOpenVideo(HWND hWnd);
BOOL __stdcall ZZOpenChildVideo(HWND hWnd);
BOOL __stdcall ZZOpenHideVideo(HWND hWnd);
BOOL __stdcall ZZCloseVideo(HWND hWnd);
BOOL __stdcall ZZCloseChildVideo(HWND hWnd);
BOOL __stdcall ZZCloseHideVideo(HWND hWnd);

BOOL __stdcall ZZRotateLeft(HWND hWnd);
BOOL __stdcall ZZRotateRight(HWND hWnd);
BOOL __stdcall ZZDeskew(HWND hWnd);
BOOL __stdcall ZZMultiDeskew(HWND hWnd);
BOOL __stdcall ZZUnDeskew(HWND hWnd);
BOOL __stdcall ZZEnableDate(HWND hWnd, COLORREF color, LONG position);
BOOL __stdcall ZZEnableDateEx(HWND hWnd, COLORREF color, LONG position, LONG ratio, LPCWSTR fontName, BYTE opacity);
BOOL __stdcall ZZDisableDate(HWND hWnd);
BOOL __stdcall ZZEnableWatermark(HWND hWnd, LPCWSTR text, COLORREF color, LONG position);
BOOL __stdcall ZZEnableWatermarkEx(HWND hWnd, LPCWSTR text, COLORREF color, LONG position, LONG ratio, LPCWSTR fontName, BYTE opacity);
BOOL __stdcall ZZDisableWatermark(HWND hWnd);
BOOL __stdcall ZZCapture(HWND hWnd, LPCWSTR fileName);
BSTR __stdcall ZZCaptureToBase64(HWND hWnd);
BOOL __stdcall ZZMultiCapture(HWND hWnd, LPCWSTR fileName);
BOOL __stdcall ZZMoveDetect(HWND hWnd, MoveDetectCallback fDetect, ULONG_PTR pDetect);
BOOL __stdcall ZZUnMoveDetect(HWND hWnd);
BOOL __stdcall ZZClearBkgndColor(HWND hWnd, BOOL bEnable);
BOOL __stdcall ZZSetDragState(HWND hWnd);
BOOL __stdcall ZZSetCustomState(HWND hWnd);
BOOL __stdcall ZZSetZoomIn(HWND hWnd);
BOOL __stdcall ZZSetZoomOut(HWND hWnd);
BOOL __stdcall ZZStartRecord(HWND hWnd, LPCWSTR fileName, LONG bitRate, LONG frameRate);
BOOL __stdcall ZZStartChildRecord(HWND hWnd, LPCWSTR fileName, LONG bitRate, LONG frameRate); // 该函数不推荐再被使用
BOOL __stdcall ZZStopRecord(HWND hWnd);
BOOL __stdcall ZZStopChildRecord(HWND hWnd); // 该函数不推荐再被使用
BOOL __stdcall ZZStartRecordEx(HWND hWnd, LPCWSTR fileName, LONG bitRate, LONG frameRate); // 该函数不推荐再被使用
BOOL __stdcall ZZStopRecordEx(HWND hWnd); // 该函数不推荐再被使用

BSTR __stdcall ZZGetTempFileName(LPCWSTR ext);
BOOL __stdcall ZZDeleteFile(LPCWSTR fileName);
BSTR __stdcall ZZGetBase64(LPCWSTR fileName);
BOOL __stdcall ZZSaveBase64(LPCWSTR fileName, BSTR base64);
BOOL __stdcall ZZUploadToHttpServer(LPCWSTR fileName, LPCWSTR url, LPCWSTR contentType, LPCWSTR remoteName);
BSTR __stdcall ZZGetLastHttpResponse(VOID);
BOOL __stdcall ZZUploadToFtpServer(LPCWSTR fileName, LPCWSTR url, LPCWSTR remoteName);

BOOL __stdcall ZZOpenIdCard(IdCardCallback fIdCard, ULONG_PTR pIdCard);
BOOL __stdcall ZZOpenIdCardEx(BOOL bAutoRead, IdCardCallback fIdCard, ULONG_PTR pIdCard);
BOOL __stdcall ZZCloseIdCard(VOID);
BOOL __stdcall ZZReadIdCard(VOID);
BSTR __stdcall ZZGetIdCardResult(LONG flag);
BOOL __stdcall ZZGetIdCardImage(LPCWSTR fileName, LONG flag);
BSTR __stdcall ZZGetIdCardFingerFeature(WORD nFlag);
BSTR __stdcall ZZGetBarcodeResult(LPCWSTR fileName);
BSTR __stdcall ZZGetBarcodeResultEx(LPCWSTR fileName);
BSTR __stdcall ZZGetOcrResult(LPCWSTR fileName);
BSTR __stdcall ZZGetRecognitionResult(ULONG nType, LPCWSTR fileName);

BOOL __stdcall ZZInitRelay(VOID);
BOOL __stdcall ZZDeinitRelay(VOID);
BOOL __stdcall ZZOpenChannel(VOID);
BOOL __stdcall ZZCloseChannel(VOID);

BOOL __stdcall ZZInitFaceMgr(VOID);
BOOL __stdcall ZZDeinitFaceMgr(VOID);
BSTR __stdcall ZZGetFaceTemplFromFile(LPCWSTR fileName);
BOOL __stdcall ZZGetFaceTempl(HWND hWnd, FaceImageCallback fImage, ULONG_PTR pImage, LONG imgFormat, 
	FaceResultCallback fResult, ULONG_PTR pResult);
BOOL __stdcall ZZGetFaceFeature(HWND hWnd, FaceImageCallback fImage, ULONG_PTR pImage, LONG imgFormat, 
	FaceResultCallback fResult, ULONG_PTR pResult);
BOOL __stdcall ZZStopGetFace(HWND hWnd);
BOOL __stdcall ZZCompareFace(BSTR feature1, BSTR feature2, LONG threshold);
ULONG __stdcall ZZCompareFaceEx(BSTR feature1, BSTR feature2);
BOOL __stdcall ZZFaceDetect(HWND hWnd, BSTR feature, LONG threshold, FaceImageCallback fImage, ULONG_PTR pImage, LONG imgFormat, 
	FaceDetectCallback fDetect, ULONG_PTR pDetect);
BOOL __stdcall ZZFaceDetectEx(HWND hWnd, BSTR feature, FaceImageCallback fImage, ULONG_PTR pImage, LONG imgFormat, 
	FaceDetectExCallback fDetect, ULONG_PTR pDetect);
BOOL __stdcall ZZUnFaceDetect(HWND hWnd);

BOOL __stdcall ZZInitFingerMgr(VOID);
BOOL __stdcall ZZDeinitFingerMgr(VOID);
BSTR __stdcall ZZGetFingerTemplFromFile(LPCWSTR fileName);
BOOL __stdcall ZZSetGetFingerTemplTimes(ULONG nTimes);
BOOL __stdcall ZZGetFingerTempl(DWORD timeout, FingerImageCallback fImage, ULONG_PTR pImage, LONG imgFormat,
	FingerResultCallback fResult, ULONG_PTR pResult);
BOOL __stdcall ZZGetFingerFeature(DWORD timeout, FingerImageCallback fImage, ULONG_PTR pImage, LONG imgFormat,
	FingerResultCallback fResult, ULONG_PTR pResult);
BOOL __stdcall ZZStopGetFinger(VOID);
BOOL __stdcall ZZCompareFinger(BSTR feature1, BSTR feature2, LONG threshold);
ULONG __stdcall ZZCompareFingerEx(BSTR feature1, BSTR feature2);
BOOL __stdcall ZZFingerDetect(DWORD timeout, BSTR feature1, BSTR feature2, LONG threshold, FingerImageCallback fImage, ULONG_PTR pImage, LONG imgFormat,
	FingerDetectCallback fDetect, ULONG_PTR pDetect);
BOOL __stdcall ZZFingerDetectEx(BSTR feature1, BSTR feature2, FingerImageCallback fImage, ULONG_PTR pImage, LONG imgFormat,
	FingerDetectExCallback fDetect, ULONG_PTR pDetect);
BOOL __stdcall ZZUnFingerDetect(VOID);
BSTR __stdcall ZZGetFingerImageBase64Indirect(LONG imgFormat);
BSTR __stdcall ZZGetFingerFeatureIndirect(VOID);

BOOL __stdcall ZZAddFileToImageList(LPCWSTR strFileName);
BOOL __stdcall ZZClearImageList(VOID);
BOOL __stdcall ZZSaveFileFromImageList(LPCWSTR strFileName);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ZZLIBRARY_H__ */
