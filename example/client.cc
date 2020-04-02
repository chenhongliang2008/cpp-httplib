//
//  client.cc
//
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
//

#include <httplib.h>
#include <iostream>
#include <vld.h>

#define CA_CERT_FILE "./ca-bundle.crt"
//#define TEST_FILE "D:\\openSource\\test\\app.txt"
#define TEST_FILE "D:\\openSource\\test\\log"

using namespace std;

void ReadLocalFile(size_t offset, size_t length, httplib::DataSink &sink)
{
	char buff[1024] = { '\0' };
	int len = 0;
	static FILE *fd = NULL;
	static size_t sum = 0;
	if (NULL == fd)
	{
		fd = fopen(TEST_FILE, "rb+");
	}
	if (NULL != fd)
	{
		len = fread(buff, 1, sizeof(buff), fd);
		if (len > 0)
		{
			if (sink.is_writable())
			{
				sink.write(buff, len);
			}
			else
			{
				fclose(fd);
				fd = NULL;
			}
			if (feof(fd))
			{
				fclose(fd);
				fd = NULL;
			}
			sum += len;
		}
		else
		{
			cout << "fread error:" << ferror(fd) << endl;
			fclose(fd);
			fd = NULL;
		}
	}
}

int main(void) {
#if 0
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
  httplib::SSLClient cli("localhost", 8080);
  // httplib::SSLClient cli("google.com");
  // httplib::SSLClient cli("www.youtube.com");
  cli.set_ca_cert_path(CA_CERT_FILE);
  cli.enable_server_certificate_verification(true);
#else
  httplib::Client cli("localhost", 8080);
#endif

  auto res = cli.Get("/hi");
  if (res) {
    cout << res->status << endl;
    cout << res->get_header_value("Content-Type") << endl;
    cout << res->body << endl;
  } else {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    auto result = cli.get_openssl_verify_result();
    if (result) {
      cout << "verify error: " << X509_verify_cert_error_string(result) << endl;
    }
#endif
  }
#else
	httplib::Client cli("10.1.11.201", 8021);
	//cli.set_compress(true);
	/*1.上报文本信息
	auto res = cli.Post("/upload/document", "123456AAA", "text/plain");
	*/
	/*2.上报文件 */
	//获取文件大小
	struct stat st;
	if (::stat(TEST_FILE, &st) != 0)
	{
		return -1;
	}
	

	httplib::ContentProvider_ content_provider = [](std::string path, httplib::DataSink &sink)->int {
		char buff[1024] = { '\0' };
		int len = 0;
		static FILE *fd = NULL;
		static size_t sum = 0;
		if (NULL == fd)
		{
			fd = fopen(path.c_str(), "rb+");
		}
		if (NULL != fd)
		{
			len = fread(buff, 1, sizeof(buff), fd);
			if (len > 0)
			{
				sum += len;
				sink.write(buff, len);
				if (feof(fd))
				{
					fclose(fd);
					fd = NULL;
					return sum;
				}
				return sum;
			}
			else
			{
				cout << "fread error:" << ferror(fd) << endl;
				fclose(fd);
				fd = NULL;
				return -1;
			}
		}
		else
		{
			return -1;
		}
	};
	//auto res = cli.Post("/upload/document", content_length, content_provider,"text/plain");
	httplib::MultipartFormDataItemsEx itemsEx;
	httplib::MultipartFormDataEx dataEx;
	dataEx.boundary = httplib::detail::make_multipart_data_boundary();
	dataEx.content_provider = content_provider;
	dataEx.name = "file";
	dataEx.filename = "log";
	dataEx.content_type = "application/octet-stream";
	dataEx.content_length = st.st_size;
	dataEx.file_path = TEST_FILE;
	itemsEx.emplace_back(dataEx);
	auto res = cli.Post("/upload/document", httplib::Headers() ,itemsEx);
	if (res) {
		cout << res->status << endl;
		cout << res->get_header_value("Content-Type") << endl;
		cout << res->body << endl;
	}
	else {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
		auto result = cli.get_openssl_verify_result();
		if (result) {
			cout << "verify error: " << X509_verify_cert_error_string(result) << endl;
		}
#endif
	}
#endif

  return 0;
}
