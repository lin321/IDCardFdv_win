#pragma once

#include <queue>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"


class CImgUploadMgt {
public:
	CImgUploadMgt();
	~CImgUploadMgt();
public:
	std::queue<IplImage*> frameQueue;
	std::queue<IplImage*> frameHideQueue;
	std::queue<IplImage*> photoQueue;
	std::queue<std::string> serialNoQueue;
	std::queue<std::string> idcardIdQueue;
	std::queue<std::string> idcardIssuedateQueue;
private:
	std::vector<uchar> idcardPhoto;
	std::vector<uchar> verifyPhotos[2];
public:
	bool isQueueEmpty();
	bool insert(std::string serial_no, IplImage* frame, IplImage* frameHide, IplImage* photo,
		std::string idcardId, std::string idcardIssuedate);
	bool upload(std::string url, std::string appId, std::string apiKey, std::string secretKey,
		std::string macId, std::string registeredNo,
		int timeout);
};
