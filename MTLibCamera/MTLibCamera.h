// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� MTLIBCAMERA_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// MTLIBCAMERA_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef MTLIBCAMERA_EXPORTS
#define MTLIBCAMERA_API __declspec(dllexport)
#else
#define MTLIBCAMERA_API __declspec(dllimport)
#endif

// �����Ǵ� MTLibCamera.dll ������
class MTLIBCAMERA_API CMTLibCamera {
public:
	CMTLibCamera(void);
	// TODO:  �ڴ�������ķ�����
};

extern MTLIBCAMERA_API int nMTLibCamera;

MTLIBCAMERA_API int fnMTLibCamera(void);
