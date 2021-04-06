//
// Created by cnsch on 2021/3/25.
//

#ifndef BILIBILIUP_BUP_H
#define BILIBILIUP_BUP_H

#include <string>
#include <memory>
#include <vector>
#include <filesystem>
#include <cpr/cpr.h>

#ifdef USE_FIDDLER_PROXY
#define CPR_FIDDLER_PROXY\
	,\
	cpr::Proxies{{"https", "http://127.0.0.1:8888"}, {"http", "http://127.0.0.1:8888"}},\
	cpr::VerifySsl{false}
#else
#define CPR_FIDDLER_PROXY 
#endif

namespace bup
{
	struct Cover final
	{
		std::string url;
	};

	struct Video final
	{ 
		std::string filename;
		std::string title;
	};

	struct Upload final
	{
		//自制声明
		bool copyright = false;
		//开启充电面板 open_elec
		bool openElectricity = false;
		// 关闭弹幕
		bool closeDanmu = false;
		//关闭评论区
		bool closeReply = false;

		//标题
		std::string title;
		//转载来源
		std::string source;
		//简介
		std::string description;
		//动态
		std::string dynamic;
		//标签，","分割
		std::string tag;
		//类别 tid 17:单机游戏
		int typeId = 17;

		//封面
		std::shared_ptr<Cover> cover;
		//视频
		std::vector<std::shared_ptr<Video>> videos;
	};

	struct UploadResult final
	{
		bool succeed = false;
		std::string error;

		uint64_t av;
		std::string bv;
	};
	  
	class BUpload final
	{ 
		uint64_t mid;
		std::string access_key;  

	public:  
		BUpload() = delete;
		~BUpload() = default;
		BUpload(const BUpload&) = delete;
		BUpload& operator=(const BUpload&) = delete;
		BUpload(BUpload&&) = delete;
		BUpload& operator=(BUpload&&) = delete;

		// 使用Fiddler与Proxifier抓投稿工具获得
		BUpload(uint64_t mid, const std::string& access_token);

		std::shared_ptr<Video> uploadVideo(const std::filesystem::path& path);
		std::shared_ptr<Cover> uploadCover(const std::filesystem::path& path);
		UploadResult upload(const Upload& info);
		UploadResult edit(uint64_t av, const Upload& info);
		bool isPassedReview(uint64_t av);
	};
}

#endif //BILIBILIUP_BUP_H
