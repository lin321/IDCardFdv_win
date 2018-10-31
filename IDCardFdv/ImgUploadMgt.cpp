// ImgUploadMgt.cpp : 实现文件
//
#include "stdafx.h"
#include "ImgUploadMgt.h"
#include "MTLibNetwork.h"

using namespace std;

CImgUploadMgt::CImgUploadMgt()
{
	idcardPhoto = vector<uchar>(256 * 1024);	// 2.4.9 bug，必须足够大
	verifyPhotos[0] = vector<uchar>(10 * 1024 * 1024);
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
}

bool CImgUploadMgt::isQueueEmpty()
{
	//if (0 == frameQueue.size() && 0 == photoQueue.size())
	if(0 == serialNoQueue.size())
		return true;

	return false;
}

bool CImgUploadMgt::insert(std::string serial_no, IplImage* frame, IplImage* photo)
{
	if (frame) {
		frameQueue.push(frame);
	}
	if (photo) {
		photoQueue.push(photo);
	}
	serialNoQueue.push(serial_no);

	return true;
}

// 上传callback
static void __stdcall UploadCB(int err_no, std::string err_msg, double unuse, std::string serial_no, MTLIBPTR userdata)
{
	CImgUploadMgt* mgt = (CImgUploadMgt*)userdata;

	if (MTLIBNETWORK_NETWORK_ERROR == err_no) {
		// 超时或其他网络错误处理
		return;
	}

	mgt->frameQueue.pop();
	mgt->photoQueue.pop();
	mgt->serialNoQueue.pop();
}

bool CImgUploadMgt::upload(std::string url, std::string appId, std::string apiKey, std::string secretKey,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	int timeout)
{
	if (0 == frameQueue.size() || 0 == photoQueue.size())
		return false;

	string serial_no = serialNoQueue.front();
	IplImage* frame = frameQueue.front();
	IplImage* photo = photoQueue.front();
	cv::Mat matframe = cv::cvarrToMat(frame);
	cv::Mat matphoto = cv::cvarrToMat(photo);	

	vector<int> param = vector<int>(2);
	cv::imencode(".png", matphoto, idcardPhoto, param);
	cv::imencode(".png", matframe, verifyPhotos[0], param);

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
		serial_no,
		idcardId, idcardIssuedate, idcardPhoto, verifyPhotos, 1,
		UploadCB, (MTLIBPTR)this, timeout);

	return true;
}
