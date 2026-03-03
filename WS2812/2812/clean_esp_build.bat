@echo off
REM 清理 ESP-IDF 编译文件
echo 正在删除 build 文件夹...
if exist build (
    rmdir /S /Q build
    echo build 文件夹已删除.
) else (
    echo build 文件夹不存在.
)
pause