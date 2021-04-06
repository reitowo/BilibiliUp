# Bilibili Uploader

自动化B站投稿

### 安装

CMake工程，做插件使用可以用vcpkg registries功能进行安装，vcpkg不会用请看：https://www.anquanke.com/post/id/234093

在VS工程启用vcpkg，并且在工程目录添加vcpkg-configuration.json文件如下内容，然后编译工程即可自动进行解析安装

```json
{
    "registries": [
        {
            "kind": "git",
            "repository": "https://github.com/cnSchwarzer/vcpkg-ports.git",
            "packages": [ "bup" ]
        }
    ]
}
```
注意，需要手动把`vcpkg_installed\x64-windows\tools\bup`中的bsign.exe复制到程序目录

### 例子

```c++
#include <cassert>
#include <bup/bup.h> 
#include <Windows.h>

int main()
{
    //UTF8
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    
    bup::BUpload uploader(你的UID, 用Proxifier和抓包抓B站投稿客户端的AccessToken);
    auto cover = uploader.uploadCover(R"(Path to Cover)");
    
    auto video1 = uploader.uploadVideo(R"(Path to Video 1)");
    
    bup::Upload upload;
    upload.title = "[BUpload]分P测试上传";
    upload.description = "测试";
    upload.tag = "测试";
    upload.source = "直播流";
    upload.cover = cover;
    upload.videos.push_back(video1);
    
    auto result = uploader.upload(upload);
    
    auto video2 = uploader.uploadVideo(R"(Path to Video 2)");
    upload.videos.push_back(video2);
    
    result = uploader.edit(result.av, upload);
}

```

