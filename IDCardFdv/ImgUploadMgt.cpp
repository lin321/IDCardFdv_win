// ImgUploadMgt.cpp : 实现文件
//
#include "stdafx.h"
#include "ImgUploadMgt.h"
#include "MTLibNetwork.h"

using namespace std;

CCriticalSection g_UploadCS;

CImgUploadMgt::CImgUploadMgt()
{
	idcardPhoto = vector<uchar>(256 * 1024);	// 2.4.9 bug，必须足够大
	verifyPhotos[0] = vector<uchar>(10 * 1024 * 1024);
	verifyPhotos[1] = vector<uchar>(10 * 1024 * 1024);
}

CImgUploadMgt::~CImgUploadMgt()
{
	while (frameQueue.size()>0) {
		IplImage* img = frameQueue.front();
		if (img) {
			cvReleaseImage(&img);
		}
		frameQueue.pop();
	}

	while (frameHideQueue.size()>0) {
		IplImage* img = frameHideQueue.front();
		if (img) {
			cvReleaseImage(&img);
		}
		frameHideQueue.pop();
	}

	while (photoQueue.size()>0) {
		IplImage* img = photoQueue.front();
		if (img) {
			cvReleaseImage(&img);
		}
		photoQueue.pop();
	}

	while (serialNoQueue.size() > 0) {
		serialNoQueue.pop();
	}

	while (idcardIdQueue.size() > 0) {
		idcardIdQueue.pop();
	}

	while (idcardIssuedateQueue.size() > 0) {
		idcardIssuedateQueue.pop();
	}
}

bool CImgUploadMgt::isQueueEmpty()
{
	//if (0 == frameQueue.size() && 0 == photoQueue.size())
	if(0 == serialNoQueue.size())
		return true;

	return false;
}

bool CImgUploadMgt::insert(std::string serial_no, IplImage* frame, IplImage* frameHide, IplImage* photo,
	std::string idcardId, std::string idcardIssuedate)
{
	bool ret = false;

	g_UploadCS.Lock();
	if (frameQueue.size() > 1000)	// 内存最大缓存数量
		ret = false;
	else {
		if (frame) {
			frameQueue.push(frame);
		}
		if (frameHide) {
			frameHideQueue.push(frameHide);
		}
		if (photo) {
			photoQueue.push(photo);
		}
		serialNoQueue.push(serial_no);
		idcardIdQueue.push(idcardId);
		idcardIssuedateQueue.push(idcardIssuedate);
		ret = true;
	}
	g_UploadCS.Unlock();

	return ret;
}

// 上传callback
static void __stdcall UploadCB(int err_no, std::string err_msg, double unuse, std::string serial_no, MTLIBPTR userdata)
{
	CImgUploadMgt* mgt = (CImgUploadMgt*)userdata;

	if (MTLIBNETWORK_NETWORK_ERROR == err_no) {
		// 超时或其他网络错误处理
		return;
	}

	g_UploadCS.Lock();
	mgt->frameQueue.pop();
	mgt->frameHideQueue.pop();
	mgt->photoQueue.pop();
	mgt->serialNoQueue.pop();
	mgt->idcardIdQueue.pop();
	mgt->idcardIssuedateQueue.pop();
	g_UploadCS.Unlock();
}

bool CImgUploadMgt::upload(std::string url, std::string appId, std::string apiKey, std::string secretKey,
	std::string macId, std::string registeredNo,
	int timeout)
{
	if (0 == frameQueue.size() || 0 == frameHideQueue.size() || 0 == photoQueue.size())
		return false;

	g_UploadCS.Lock();
	string serial_no = serialNoQueue.front();
	IplImage* frame = frameQueue.front();
	IplImage* frameHide = frameHideQueue.front();
	IplImage* photo = photoQueue.front();
	string idcardId = idcardIdQueue.front();
	string idcardIssuedate = idcardIssuedateQueue.front();
	g_UploadCS.Unlock();

	cv::Mat matframe = cv::cvarrToMat(frame);
	cv::Mat matframeHide = cv::cvarrToMat(frameHide);
	cv::Mat matphoto = cv::cvarrToMat(photo);

	vector<int> param = vector<int>(2);
	cv::imencode(".png", matphoto, idcardPhoto, param);
	cv::imencode(".jpg", matframe, verifyPhotos[0], param);
	cv::imencode(".jpg", matframeHide, verifyPhotos[1], param);

	// uuid
	CString struuid(L"error");
	RPC_CSTR guidStr;
	GUID guid;
	if (UuidCreateSequential(&guid) == RPC_S_OK)
	{
		if (UuidToString(&guid, &guidStr) == RPC_S_OK)
		{
			struuid = (LPTSTR)guidStr;
			RpcStringFree(&guidStr);
		}
	}
	std::string uuid = std::string(struuid);

	MTLibUploadImage(url,
		appId, apiKey, secretKey, uuid,
		macId, registeredNo,
		idcardId, idcardIssuedate, 
		serial_no,idcardPhoto, verifyPhotos, 2,
		UploadCB, (MTLIBPTR)this, timeout);

	return true;
}
