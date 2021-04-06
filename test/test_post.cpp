#include <cassert>
#include <bup/bup.h>
#include <k1ee/encoding.h>
#include <Windows.h>
#include <k1ee/file.h>

int main()
{
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);

	std::string workingFolder = "D:/Work/LivestreamExtract/final";
	bup::BUpload uploader(std::atol(k1ee::read_all_texts( workingFolder + "/bili-uid.txt").c_str()),
		k1ee::read_all_texts( workingFolder + "/bili-accesstoken.txt"));
	
	auto cover = uploader.uploadCover(R"(D:\Work\LivestreamExtract\final\slice_cover.jpg)");
	
	auto video1 = uploader.uploadVideo(
		R"(D:\ObsOutput\2021-02-23 19-41-58.flv)");

	bup::Upload upload;
	upload.title = "[BUpload]分P测试上传";
	upload.description = "测试";
	upload.tag = "测试";
	upload.source = "直播流";
	upload.cover = cover;
	upload.videos.push_back(video1);

	auto result = uploader.upload(upload);
	
	auto video2 = uploader.uploadVideo(
		R"(D:\ObsOutput\2021-03-06 15-30-07.flv)");
	upload.videos.push_back(video2);
	
	result = uploader.edit(result.av, upload);
}
