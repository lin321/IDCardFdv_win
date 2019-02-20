// MTLibNetwork.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MTLibNetwork.h"

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>
#include <openssl/sha.h>
using namespace web;
using namespace web::http;
using namespace web::http::client;


// ---- sha256摘要哈希 ---- //    
void sha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr)
{
	// 调用sha256哈希    
	unsigned char mdStr[33] = { 0 };
	SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);

	// 哈希后的字符串    
	encodedStr = std::string((const char *)mdStr);
	// 哈希后的十六进制串 32字节    
	char buf[65] = { 0 };
	char tmp[3] = { 0 };
	for (int i = 0; i < 32; i++)
	{
		sprintf_s(tmp, "%02X", mdStr[i]);
		strcat_s(buf, tmp);
	}
	buf[64] = '\0'; 
	encodedHexStr = std::string(buf);
}

// pre define
int __stdcall MTLibCallVerifyMain(json::value& verify_json, std::string shastr,
	std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout);
int __stdcall CallVerifySub(utility::string_t& url, web::json::value& postParameters,
				CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout);
/////////////////////

int __stdcall MTLibCallVerify(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::string idcardPhoto, std::vector < std::string> &verifyPhotos, int verifyPhotoNum,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout)
{
	json::value verify_json = json::value::object();

	std::string shastr_part = "";

	utility::string_t b64str = utility::conversions::to_string_t(idcardPhoto);
	verify_json[U("idcard_face_info")] = json::value::string(b64str);
	shastr_part += utility::conversions::to_utf8string(b64str);

	verify_json[U("verify_face_infos")] = json::value::array();
	for (int i = 0; i < verifyPhotoNum; i++) {
		utility::string_t b64str = utility::conversions::to_string_t(verifyPhotos[i]);
		verify_json[U("verify_face_infos")][i] = json::value::string(b64str);
		shastr_part += utility::conversions::to_utf8string(b64str);
	}

	return MTLibCallVerifyMain(verify_json, shastr_part,
		url, appId, apiKey, secretKey, uuid,
		macId, registeredNo,
		idcardId, idcardIssuedate,
		callverifyCB, userdata, timeout);
}

int __stdcall MTLibCallVerify_Image(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::vector<unsigned char> &idcardPhoto, std::vector<unsigned char> verifyPhotos[], int verifyPhotoNum,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout)
{
	json::value verify_json = json::value::object();

	std::string shastr_part = "";

	utility::string_t b64str = U("data:image/bmp;base64,") + utility::conversions::to_base64(idcardPhoto);
	verify_json[U("idcard_photo")] = json::value::string(b64str);
	shastr_part += utility::conversions::to_utf8string(b64str);

	verify_json[U("verify_photos")] = json::value::array();
	for (int i = 0; i < verifyPhotoNum; i++) {
		utility::string_t b64str = U("data:image/png;base64,") + utility::conversions::to_base64(verifyPhotos[i]);
		verify_json[U("verify_photos")][i] = json::value::string(b64str);
		shastr_part += utility::conversions::to_utf8string(b64str);
	}

	return MTLibCallVerifyMain(verify_json, shastr_part,
		url, appId, apiKey, secretKey, uuid,
		macId, registeredNo,
		idcardId, idcardIssuedate,
		callverifyCB, userdata, timeout);
}

int __stdcall MTLibUploadImage(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::string serial_no,
	std::vector<unsigned char> &idcardPhoto, std::vector<unsigned char> verifyPhotos[], int verifyPhotoNum,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout)
{
	json::value verify_json = json::value::object();

	std::string shastr_part = "";

	verify_json[U("Serial_No")] = json::value::string(utility::conversions::to_string_t(serial_no));
	shastr_part += serial_no;
	
	utility::string_t b64str;
	if (idcardPhoto.size() > 0)
		b64str = U("data:image/bmp;base64,") + utility::conversions::to_base64(idcardPhoto);
	else
		b64str = U("");
	verify_json[U("idcard_photo")] = json::value::string(b64str);
	shastr_part += utility::conversions::to_utf8string(b64str);

	verify_json[U("verify_photos")] = json::value::array();
	for (int i = 0; i < verifyPhotoNum; i++) {
		utility::string_t b64str;
		if(verifyPhotos[i].size() > 0)
			b64str = U("data:image/jpg;base64,") + utility::conversions::to_base64(verifyPhotos[i]);
		else
			b64str = U("");
		verify_json[U("verify_photos")][i] = json::value::string(b64str);
		shastr_part += utility::conversions::to_utf8string(b64str);
	}

	return MTLibCallVerifyMain(verify_json, shastr_part,
		url, appId, apiKey, secretKey, uuid,
		macId, registeredNo,
		idcardId, idcardIssuedate,
		callverifyCB, userdata, timeout);
}

int __stdcall MTLibCallVerifyMain(json::value& verify_json, std::string shastr_part,
	std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout)
{
	std::string shastr = "";
	verify_json[U("appId")] = json::value::string(utility::conversions::to_string_t(appId));
	verify_json[U("apiKey")] = json::value::string(utility::conversions::to_string_t(apiKey));
	//verify_json[U("secretKey")] = json::value::string(utility::conversions::to_string_t(secretKey));
	shastr += appId;
	shastr += apiKey;
	shastr += secretKey;

	//std::string macId = "00:09:4c:53:78:2c"; // test
	verify_json[U("MacId")] = json::value::string(utility::conversions::to_string_t(macId));
	shastr += macId;

	verify_json[U("uuid")] = json::value::string(utility::conversions::to_string_t(uuid));
	shastr += uuid;

	//std::string registerno = "9081"; // test
	verify_json[U("RegisteredNo")] = json::value::string(utility::conversions::to_string_t(registeredNo));
	shastr += registeredNo;

	verify_json[U("idcard_id")] = json::value::string(utility::conversions::to_string_t(idcardId));
	verify_json[U("idcard_issuedate")] = json::value::string(utility::conversions::to_string_t(idcardIssuedate));
	shastr += idcardId;
	shastr += idcardIssuedate;

	shastr += shastr_part;

	std::string shaEncoded;
	std::string shaEncodedHex;
	sha256(shastr, shaEncoded, shaEncodedHex);
	verify_json[U("checksum")] = json::value::string(utility::conversions::to_string_t(shaEncodedHex));

	return CallVerifySub(utility::conversions::to_string_t(url), verify_json, callverifyCB, userdata, timeout);
}


int __stdcall CallVerifySub(utility::string_t& url, web::json::value& postParameters, 
							CallVerifyCallback callverifyCB, MTLIBPTR userdata,
							int timeout )
{
	std::string ret;

	http::uri uri = http::uri(url);
	http_client_config config;
	config.set_timeout(utility::seconds(timeout));
	config.set_validate_certificates(true);
	http_client client(uri, config);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	postRequest.set_body(postParameters);

	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);
		http_response resp = getTask.get();
		const json::value& jval = resp.extract_json().get();
		const web::json::object& jobj = jval.as_object();
		if (jval.has_field(U("Err_no"))) {
			int err_no = jobj.at(L"Err_no").as_integer();
			if (err_no != 0) {
				utility::string_t err_msg = jobj.at(L"Err_msg").as_string();
				ret = utility::conversions::to_utf8string(err_msg);
				//AfxMessageBox(err_msg.c_str());
				//printf("%ls\n", err_msg.c_str());
				assert(err_no != MTLIBNETWORK_NETWORK_ERROR);
				callverifyCB(err_no, ret, -1, "", userdata);
				return -1;
			}
			else {
				double sim = 0.0;
				if (jval.has_field(U("Similarity"))) {
					sim = jobj.at(L"Similarity").as_double();
					//CString csTemp;
					//csTemp.Format("%.2f", sim);
					//ret = csTemp.GetString();
				}
				ret = "";

				std::string serial_no = "";
				if (jval.has_field(U("Serial_No"))) {
					utility::string_t sno = jobj.at(L"Serial_No").as_string();
					serial_no = utility::conversions::to_utf8string(sno);
				}
				callverifyCB(err_no, ret, sim, serial_no, userdata);
			}
		}
	}
	catch (const std::exception &e) {
		(void)e;
		ret = "network error!";
		//AfxMessageBox(_T("network error!"));
		//printf("network error!\n");
		callverifyCB(MTLIBNETWORK_NETWORK_ERROR, ret, -1, "", userdata);
		return -1;
	}


	return 0;
}


//==========================================
int __stdcall callregisterSub(utility::string_t& url, web::json::value& postParameters, CallRegisterCallback callregisterCB, MTLIBPTR userdata, int timeout);
int __stdcall MTLibCallRegister(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string productsn, std::string macId,
	CallRegisterCallback callregisterCB, MTLIBPTR userdata, int timeout)
{
	json::value reg_json = json::value::object();

	std::string shastr = "";

	reg_json[U("appId")] = json::value::string(utility::conversions::to_string_t(appId));
	reg_json[U("apiKey")] = json::value::string(utility::conversions::to_string_t(apiKey));
	//reg_json[U("secretKey")] = json::value::string(utility::conversions::to_string_t(secretKey));
	shastr += appId;
	shastr += apiKey;
	shastr += secretKey;

	reg_json[U("MacId")] = json::value::string(utility::conversions::to_string_t(macId));
	shastr += macId;

	reg_json[U("uuid")] = json::value::string(utility::conversions::to_string_t(uuid));
	shastr += uuid;

	reg_json[U("productsn")] = json::value::string(utility::conversions::to_string_t(productsn));
	shastr += productsn;

	std::string shaEncoded;
	std::string shaEncodedHex;
	sha256(shastr, shaEncoded, shaEncodedHex);
	reg_json[U("checksum")] = json::value::string(utility::conversions::to_string_t(shaEncodedHex));
	return callregisterSub(utility::conversions::to_string_t(url), reg_json, callregisterCB, userdata,timeout);
}
int __stdcall callregisterSub(utility::string_t& url, web::json::value& postParameters, CallRegisterCallback callregisterCB, MTLIBPTR userdata, int timeout)
{
	std::string ret;

	http::uri uri = http::uri(url);
	http_client_config config;
	config.set_timeout(utility::seconds(timeout));
	config.set_validate_certificates(true);
	http_client client(uri, config);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	postRequest.set_body(postParameters);

	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);
		http_response resp = getTask.get();
		const json::value& jval = resp.extract_json().get();
		const web::json::object& jobj = jval.as_object();
		if (jval.has_field(U("Err_no"))) {
			int err_no = jobj.at(L"Err_no").as_integer();
			if (err_no != 0) {
				utility::string_t err_msg = jobj.at(L"Err_msg").as_string();
				ret = utility::conversions::to_utf8string(err_msg);
				//AfxMessageBox(ret.c_str());
				//printf("%ls\n", err_msg.c_str());
				assert(err_no != MTLIBNETWORK_NETWORK_ERROR);	
				std::string regno = "";
				callregisterCB(err_no, ret, regno, userdata);
				return -1;
			}
			else {
				utility::string_t RegisteredNo = jobj.at(L"RegisteredNo").as_string();
				std::string regno = utility::conversions::to_utf8string(RegisteredNo);
				ret = "";
				callregisterCB(err_no, ret, regno, userdata);
			}
		}
	}
	catch (const std::exception &e) {
		(void)e;
		ret = "network error!";
		//printf("network error!\n");
		std::string regno = "";
		callregisterCB(MTLIBNETWORK_NETWORK_ERROR, ret, regno, userdata);
		return -1;
	}

	return 0;
}


int __stdcall MTLibTestUrl(std::string url, TestUrlCallback testurlCB, MTLIBPTR userdata, int timeout)
{
	utility::string_t urlstr = utility::conversions::to_string_t(url);
	http::uri uri = http::uri(urlstr);
	http_client_config config;
	config.set_timeout(utility::seconds(timeout));
	config.set_validate_certificates(true);
	http_client client(uri, config);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);
		http_response resp = getTask.get();
		if (resp.status_code() == 200) {
			if(testurlCB)
				testurlCB(0, userdata);
		}
		else {
			if (testurlCB)
				testurlCB(-1, userdata);
			return -1;
		}
	}
	catch (const std::exception &e) {
		(void)e;
		//printf("Error exception:%s\n", e.what());
		if (testurlCB)
			testurlCB(MTLIBNETWORK_NETWORK_ERROR, userdata);
		return -1;
	}

	return 0;
}

int __stdcall MTLibTestUrlIgnoreResp(std::string url, TestUrlCallback testurlCB, MTLIBPTR userdata, int timeout)
{
	utility::string_t urlstr = utility::conversions::to_string_t(url);
	http::uri uri = http::uri(urlstr);
	http_client_config config;
	config.set_timeout(utility::seconds(timeout));
	config.set_validate_certificates(true);
	http_client client(uri, config);
	web::http::http_request postRequest;
	postRequest.set_method(methods::POST);
	try {
		Concurrency::task<web::http::http_response> getTask = client.request(postRequest);	
		http_response resp = getTask.get();
	}
	catch (const std::exception &e) {
		(void)e;
		//printf("Error exception:%s\n", e.what());
		if (testurlCB)
			testurlCB(MTLIBNETWORK_NETWORK_ERROR, userdata);
		return -1;
	}

	return 0;
}
