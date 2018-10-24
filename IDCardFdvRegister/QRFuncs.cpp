#include "stdafx.h"

#include <iostream>
#include <zbar.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc.hpp"

#include "QRFuncs.h"

using namespace std;
using namespace cv;
using namespace zbar;

//zbar�ӿ�
string ZbarDecoder(Mat img)
{
	string result;
	ImageScanner scanner;
	const void *raw = (&img)->data;
	// configure the reader
	scanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	// wrap image data
	Image image(img.cols, img.rows, "Y800", raw, img.cols * img.rows);
	// scan the image for barcodes
	int n = scanner.scan(image);
	// extract results
	result = image.symbol_begin()->get_data();
	image.set_data(NULL, 0);
	return result;
}

//�Զ�ֵͼ�����ʶ�����ʧ����������ж���ʶ��
string GetQRInBinImg(Mat binImg)
{
	string result = ZbarDecoder(binImg);
	if (result.empty())
	{
		Mat openImg;
		Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
		morphologyEx(binImg, openImg, MORPH_OPEN, element);
		result = ZbarDecoder(openImg);
	}
	return result;
}

//main function
string GetQR(Mat img)
{
	Mat binImg;
	//��otsu��ֵ����Ļ����ϣ�����������ֵ������ʶ��ģ��ͼ��
	int thre = (int)threshold(img, binImg, 0, 255, cv::THRESH_OTSU);
	string result;
	while (result.empty() && thre<255)
	{
		threshold(img, binImg, thre, 255, cv::THRESH_BINARY);
		result = GetQRInBinImg(binImg);
		thre += 20;//��ֵ������Ϊ20������Խ��ʶ����Խ�ͣ��ٶ�Խ��
	}
	return result;
}