#!/bin/bash
# build.sh — 简单加速版

# 进入构建目录
cd build || { echo "进入 build 目录失败"; exit 1; }

# 运行 cmake 配置
cmake -G "MinGW Makefiles" .. 

# 并行编译（使用4个核心，根据你的电脑调整）
mingw32-make -j4

cd ..