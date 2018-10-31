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
	std::queue<IplImage*> photoQueue;
	std::queue<std::string> serialNoQueue;
private:
	std::vector<uchar> idcardPhoto;
	std::vector<uchar> verifyPhotos[1];
public:
	bool isQueueEmpty();
	bool insert(std::string serial_no, IplImage* frame, IplImage* photo);
	bool upload(std::string url, std::string appId, std::string apiKey, std::string secretKey,
		std::string macId, std::string registeredNo,
		std::string idcardId, std::string idcardIssuedate,
		int timeout);
};
