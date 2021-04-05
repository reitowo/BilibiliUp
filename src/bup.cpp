//
// Created by cnsch on 2021/3/25.
//

#include "bup/bup.h"

#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <cpp-base64/base64.h>
#include <rhash.h>
#include <k1ee/k1ee.h>
#include <random>
#include <cstdio>

#define TOSTR(X) #X
#define UGC_BUILD 1063
#define UGC_VERSION "2.3.0." TOSTR(UGC_BUILD)

static std::string signString(const std::string &str) {
    auto command = "bsign.exe \"" + str + "\" > output.temp";
    system(command.c_str());
    auto ret = k1ee::read_all_texts("./output.temp");
    std::filesystem::remove("./output.temp");
    return ret;
}

bup::BUpload::BUpload(uint64_t mid, const std::string &access_token) : mid(mid), access_key(access_token) {
}

std::shared_ptr<bup::Video> bup::BUpload::uploadVideo(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
        return nullptr;

    size_t file_size = std::filesystem::file_size(path);
    auto file_name = path.filename().string();

    std::cout << "上传视频 " << path.string() << std::endl;

    //member.bilibili.com/preupload
    auto resp = cpr::Get(
            cpr::Url("https://member.bilibili.com/preupload"),
            cpr::Parameters{
                    {"access_key", access_key},
                    {"mid",        std::to_string(mid)},
                    {"profile",    "ugcfr/pc3"}
            },
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"}
            }
            CPR_FIDDLER_PROXY
    );

    auto preupload_ret = nlohmann::json::parse(resp.text);
    std::string complete_url = preupload_ret["complete"];
    std::string server_filename = preupload_ret["filename"];
    std::string upload_url = preupload_ret["url"];

    std::cout << "Endpoint: " << upload_url << std::endl;

    // 上传
    int chunk = 0;
    int chunk_size = 2097152;
    int chunks = file_size / chunk_size + 1;
    size_t upload_ptr = 0;
    size_t remain_size = file_size;

    rhash_library_init();
    rhash ctx = rhash_init(RHASH_MD5);
    rhash full = rhash_init(RHASH_MD5);

    while (remain_size > 0) {
        size_t this_chunk_size = remain_size > chunk_size ? chunk_size : remain_size;
        remain_size -= this_chunk_size;

        std::ifstream fin(path, std::ios::binary);
        fin.seekg(upload_ptr);
        auto buf = std::make_unique<char[]>(this_chunk_size);
        fin.read(buf.get(), this_chunk_size);

        char chunk_md5[33] = {0};
        rhash_reset(ctx);
        rhash_update(ctx, buf.get(), this_chunk_size);
        rhash_final(ctx, nullptr);
        rhash_print(chunk_md5, ctx, RHASH_MD5, RHPR_DEFAULT);

        rhash_update(full, buf.get(), this_chunk_size);

        auto file_multipart = cpr::Multipart{
                {"version", UGC_VERSION},
                {"filesize", std::to_string(this_chunk_size)},
                {"chunk",    std::to_string(chunk)},
                {"chunks",   std::to_string(chunks)},
                {"md5",      chunk_md5},
                {
                 "file",     cpr::Buffer{buf.get(), buf.get() + this_chunk_size, file_name.c_str()},
                        "application/octet-stream"
                }
        };

        resp = cpr::Post(
                cpr::Url(upload_url),
                file_multipart,
                cpr::Header{
                        {"Accept-Encoding", "gzip, deflate"},
                        {"User-Agent",      ""},
                        {"Expect",          ""}
                }
                CPR_FIDDLER_PROXY
        );

        chunk++;
        upload_ptr += this_chunk_size;

        std::cout << "Upload (" << chunk << "/" << chunks << ") : [" << resp.status_code << "] " << resp.text <<
                  std::endl;
    }

    char file_md5[33] = {0};
    rhash_final(full, nullptr);
    rhash_print(file_md5, full, RHASH_MD5, RHPR_DEFAULT);

    rhash_free(ctx);
    rhash_free(full);

    // 完成
    resp = cpr::Post(
            cpr::Url(complete_url),
            cpr::Payload{
                    {"chunks",   std::to_string(chunks)},
                    {"filesize", std::to_string(file_size)},
                    {"md5",      file_md5},
                    {"version", UGC_VERSION},
                    {"name",     file_name}
            },
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"}
            }
            CPR_FIDDLER_PROXY
    );

    std::cout << resp.text << std::endl;

    auto final_ret = nlohmann::json::parse(resp.text);
    int ok = final_ret["OK"];

    if (ok == 1) {
        auto ret = std::make_shared<Video>();
        ret->filename = server_filename;
        ret->title = file_name.substr(0, file_name.find_last_of('.'));
        return ret;
    }

    return nullptr;
}

std::shared_ptr<bup::Cover> bup::BUpload::uploadCover(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
        return nullptr;

    std::cout << "上传封面 " << path.string() << std::endl;

    auto data = k1ee::read_all_bytes(path);

    cpr::Parameters param{{"access_key", access_key}};
    param.AddParameter(cpr::Parameter{"sign", signString(param.content)}, cpr::CurlHolder());

    auto resp = cpr::Post(
            cpr::Url("https://member.bilibili.com/x/vu/client/cover/up"),
            param,
            cpr::Multipart{
                    {"file", cpr::File(path.string())}
            },
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"}
            }
            CPR_FIDDLER_PROXY
    );

    std::cout << resp.text << std::endl;

    auto cover_ret = nlohmann::json::parse(resp.text);

    int code = cover_ret["code"];
    std::string url = cover_ret["data"]["url"];

    std::cout << "Url: " << url << std::endl;

    if (code == 0) {
        auto ret = std::make_shared<Cover>();
        ret->url = url;
        return ret;
    }

    return nullptr;
}

bup::UploadResult bup::BUpload::upload(const Upload &info) {
    std::cout << "投稿" << std::endl;

    auto j = nlohmann::json::object();
    j["build"] = 1063;
    j["copyright"] = info.copyright ? 1 : 2;
    j["cover"] = info.cover->url;
    j["desc"] = info.description;
    j["no_reprint"] = 1; //禁止转载
    j["open_elec"] = info.openElectricity ? 1 : 0; //充电面板
    j["tag"] = info.tag;
    j["tid"] = info.typeId;
    j["title"] = info.title;
    j["source"] = info.source;
    j["up_close_danmu"] = info.closeDanmu;
    j["up_close_reply"] = info.closeReply;

    j["videos"] = nlohmann::json::array();
    for (auto v : info.videos) {
        std::cout << "视频：" << v->title << std::endl;
        auto vj = nlohmann::json::object();

        vj["desc"] = "";
        vj["filename"] = v->filename;
        vj["title"] = v->title;

        j["videos"].push_back(vj);
    }

    auto postJson = j.dump();

    cpr::Parameters param{{"access_key", access_key}};
    param.AddParameter(cpr::Parameter{"sign", signString(param.content)}, cpr::CurlHolder());

    auto resp = cpr::Post(
            cpr::Url("https://member.bilibili.com/x/vu/client/add"),
            param,
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"},
                    {"Content-Type",    "application/json"}
            },
            cpr::Body{postJson}
            CPR_FIDDLER_PROXY
    );

    std::cout << resp.text << std::endl;

    auto post_ret = nlohmann::json::parse(resp.text);

    UploadResult result;
    int code = post_ret["code"];

    result.succeed = code == 0;
    if (result.succeed) {
        result.av = post_ret["data"]["aid"];
        result.bv = post_ret["data"]["bvid"];
        result.error = post_ret["message"];
        std::cout << "投稿成功: " << result.bv << std::endl;
    }

    return result;
}

bup::UploadResult bup::BUpload::edit(uint64_t av, const Upload &info) {
    std::cout << "投稿" << std::endl;

    auto j = nlohmann::json::object();
    j["aid"] = av;
    j["build"] = 1063;
    j["copyright"] = info.copyright ? 1 : 2;
    j["cover"] = info.cover->url;
    j["desc"] = info.description;
    j["no_reprint"] = 1; //禁止转载
    j["open_elec"] = info.openElectricity ? 1 : 0; //充电面板
    j["tag"] = info.tag;
    j["tid"] = info.typeId;
    j["title"] = info.title;
    j["source"] = info.source;
    j["up_close_danmu"] = info.closeDanmu;
    j["up_close_reply"] = info.closeReply;

    j["videos"] = nlohmann::json::array();
    for (auto v : info.videos) {
        std::cout << "视频：" << v->title << std::endl;
        auto vj = nlohmann::json::object();

        vj["desc"] = "";
        vj["filename"] = v->filename;
        vj["title"] = v->title;

        j["videos"].push_back(vj);
    }

    auto postJson = j.dump();

    cpr::Parameters param{{"access_key", access_key}};
    param.AddParameter(cpr::Parameter{"sign", signString(param.content)}, cpr::CurlHolder());

    auto resp = cpr::Post(
            cpr::Url("https://member.bilibili.com/x/vu/client/edit"),
            param,
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"},
                    {"Content-Type",    "application/json"}
            },
            cpr::Body{postJson}
            CPR_FIDDLER_PROXY
    );

    std::cout << resp.text << std::endl;

    auto post_ret = nlohmann::json::parse(resp.text);

    UploadResult result;
    int code = post_ret["code"];

    result.succeed = code == 0;
    if (result.succeed) {
        result.av = post_ret["data"]["aid"];
        result.bv = post_ret["data"]["bvid"];
        result.error = post_ret["message"];
        std::cout << "修改稿件成功: " << result.bv << std::endl;
    }

    return result;
}

bool bup::BUpload::isPassedReview(uint64_t av) {

    cpr::Parameters param{
            {"access_key", access_key},
            {"aid",        std::to_string(av)},
            {"build",      std::to_string(UGC_BUILD)}
    };
    param.AddParameter(cpr::Parameter{"sign", signString(param.content)}, cpr::CurlHolder());

    auto resp = cpr::Get(
            cpr::Url("http://member.bilibili.com/x/client/archive/view"),
            param,
            cpr::Header{
                    {"Accept-Encoding", "gzip, deflate"},
                    {"User-Agent",      ""},
                    {"Connection",      "keep-alive"}
            }
            CPR_FIDDLER_PROXY
    );

    std::cout << resp.text << std::endl;

    auto view_ret = nlohmann::json::parse(resp.text);

    int code = view_ret["code"];
    if (code == 0) {
        int state = view_ret["data"]["archive"]["state"];

        for(auto video : view_ret["data"]["videos"]) {
            int fail_code = video["fail_code"];
            int xcode_state = video["xcode_state"];
            int status = video["status"];

            std::cout << video["cid"] << " " << status << " " << xcode_state << std::endl;
        }

        if (state != 0) {
            std::cout << "未开放浏览" << std::endl;
            return false;
        }
    }

    std::cout << "已开放浏览" << std::endl;
    return true;
}
