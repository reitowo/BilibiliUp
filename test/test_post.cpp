#include <cassert>
#include <bup/bup.h>
#include <k1ee/encoding.h>
#include <Windows.h>
#include <k1ee/file.h>

int main()
{
	SetConsoleCP(65001);
	SetConsoleOutputCP(65001);
	
	bup::BUpload uploader(0, "your access token");
	auto cover = uploader.uploadCover(R"(D:\Work\LivestreamExtract\final\slice_cover.jpg)");
	
	auto video1 = uploader.uploadVideo(
		R"(D:\Work\LivestreamExtract\final\output_slice\1616743747978\6622-6620 (1616744500378).flv)");

	bup::Upload upload;
	upload.title = "[BUpload]分P测试上传";
	upload.description = "测试";
	upload.tag = "测试";
	upload.source = "直播流";
	upload.cover = cover;
	upload.videos.push_back(video1);

	auto result = uploader.upload(upload);
	
	auto video2 = uploader.uploadVideo(
		R"(D:\Work\LivestreamExtract\final\output_slice\1616743747978\6623-6622 (1616744388474).flv)");
	upload.videos.push_back(video2);
	
	result = uploader.edit(result.av, upload);
}
