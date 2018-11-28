#ifndef __AI_FDR_WRAP__
#define __AI_FDR_WRAP__

#include <opencv2/core/core.hpp>

using namespace std;

#ifdef _WIN32
	#ifdef AI_FDR_LIBRARY_EXPORTS
		#define AI_FDR_LIBRARY_API __declspec(dllexport)
	#else
		#define AI_FDR_LIBRARY_API __declspec(dllimport)
	#endif
#else
	#define AI_FDR_LIBRARY_API
#endif


class face_recog_model
{
public:
	virtual int ai_fdr_edge_descriptor(
		const IplImage& inimgmat,
		const std::vector<int> &infacerect,
		std::string &outfacefeat
	) = 0;


	virtual int ai_fdr_edge_descriptors(
		const IplImage& inimgmat,
		std::vector<std::vector<int> > &outfacerects,
		std::vector<std::string> &outfacefeats,
		const int upscale = 0,
		bool btopface = false
	) = 0;


	virtual int dectect_faces(
		const IplImage& inimgmat,
		std::vector<std::vector<int> > &outfacerects,
		const int upscale = 0,
		bool btop = false
	) = 0;


	virtual bool livecheck(
		const IplImage& inimgmat1,
		const IplImage& inimgmat2,
		std::vector<int> &outfacerect,
		std::string &outfacefeat,
		const int upscale = 0
	) = 0;

	virtual int ai_fdr_similarity(const std::string stredge1, const std::string stredge2, double& outsim)=0;
        virtual ~face_recog_model(){;};
};


class AI_FDR_LIBRARY_API fdr_model_wrap
{
public:
	fdr_model_wrap(const std::string& model_filename);

	int ai_fdr_edge_descriptors(
		const IplImage& inimgmat,
		std::vector<std::vector<int> > &outfacerects,
		std::vector<std::string> &outfacefeats,
		const int upscale = 0,
		bool btopface = false
	);
	
	int ai_fdr_edge_descriptor(
		const IplImage& inimgmat,
		const std::vector<int>& infacerect,
		std::string &outfacefeat
	);

	int dectect_faces(
		const IplImage& inimgmat,
		std::vector<std::vector<int> > &outfacerects,
		const int upscale = 0,
		bool btopface = false
	);

	virtual bool livecheck(
		const IplImage& inimgmat1,
		const IplImage& inimgmat2,
		std::vector<int> &outfacerect,
		std::string &outfacefeat,
		const int upscale = 0
		);

	int ai_fdr_similarity(const std::string stredge1, const std::string stredge2, double& outsim);

	~fdr_model_wrap();

private:
	face_recog_model* m_frm;
};

#endif //__AI_FDR_WRAP__
