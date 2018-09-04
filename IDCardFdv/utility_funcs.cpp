// utility_funcs.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "utility_funcs.h"
//�ļ�ͷ�ṹ��
#pragma pack(push)
#pragma pack(2)  //���ֽڶ��룬����bmp_fileheader��ռ16Byte
//bmp�ļ�ͷ
struct bmp_fileheader
{
	unsigned short   bfType; //�ļ���ʶ
	unsigned long	 bfSize; //�ļ���С
	unsigned long	 bfReserved1; //����
	unsigned long    bfOffBits;//����ƫ��
};

//bmp��Ϣͷ
struct bmp_infoheader
{
	unsigned long    biSize;//��Ϣͷ����
	unsigned long    biWidth;//���
	unsigned long    biHeight;//�߶�
	unsigned short   biPlanes;//������=1
	unsigned short   biBitCount;//����λ��
	unsigned long    biCompression;//ѹ��˵��
	unsigned long    biSizeImage;//λͼ���ݵĴ�С
	unsigned long    biXPelsPerMeter;//ˮƽ�ֱ���
	unsigned long    biYPelsPerMeter;//��ֱ�ֱ���
	unsigned long    biClrUsed;//ʹ�õ���ɫ��
	unsigned long    biClrImportant;//��Ҫ����ɫ��
};

#pragma pack(pop)

using namespace cv;
IplImage *BMP2Ipl(unsigned char *src, int FileSize)
{
	bmp_fileheader *Fileheader = (bmp_fileheader *)src;
	//������
	assert(FileSize == Fileheader->bfSize);
	//�ж��Ƿ���λͼ
	if (Fileheader->bfType != 0x4d42)
		return NULL;
	//��ʼ������Ϣͷ
	bmp_infoheader *bmpheader = (bmp_infoheader *)(src + sizeof(bmp_fileheader));
	unsigned int width = bmpheader->biWidth;//���
	unsigned int height = bmpheader->biHeight;//�߶�
	unsigned int hSize = bmpheader->biSize;//��Ϣͷ����

	if (bmpheader->biBitCount < 24)
		return NULL;//֧����ɫ��32λ

	if (bmpheader->biCompression != 0)
		return NULL;//��֧��ѹ���㷨

					//���ݴ�С
	unsigned int dataByteSize = bmpheader->biSizeImage;//ʵ��λͼ���ݴ�С
	unsigned int rowByteSize = dataByteSize / height;//�����ÿ������

	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, bmpheader->biBitCount / 8);
	//����ʵ����������ɫ��
	img->origin = IPL_ORIGIN_BL;
	memcpy(img->imageData, src + Fileheader->bfOffBits, FileSize - Fileheader->bfOffBits);

	/*
	unsigned char *gc = src + FileSize - rowByteSize;
	char *p = img->imageData;
	for (int i = 0; i<height; ++i)
	{
		memcpy(p, gc, rowByteSize);
		p += rowByteSize;
		gc -= rowByteSize;
	}/**/

	
	return img;

}

