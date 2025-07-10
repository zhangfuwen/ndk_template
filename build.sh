#!/bin/bash

# 检查ANDROID_NDK_HOME环境变量
if [ -z "$ANDROID_NDK_HOME" ]; then
    echo "错误: ANDROID_NDK_HOME 环境变量未设置"
    echo "请设置ANDROID_NDK_HOME指向你的Android NDK安装目录"
    exit 1
fi

# 检查NDK目录是否存在
if [ ! -d "$ANDROID_NDK_HOME" ]; then
    echo "错误: NDK目录不存在: $ANDROID_NDK_HOME"
    exit 1
fi

# 默认构建arm64版本
ABI=${1:-android-arm64}

# 创建并进入构建目录
BUILD_DIR="build/$ABI"
mkdir -p "$BUILD_DIR"

# 配置CMake项目
echo "配置CMake项目..."
cmake --preset "$ABI" \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake"

# 构建项目
echo "构建项目..."
cmake --build --preset "$ABI"

if [ $? -eq 0 ]; then
    echo "构建成功！"
    echo "输出文件位于: $BUILD_DIR"
else
    echo "构建失败！"
    exit 1
fi
