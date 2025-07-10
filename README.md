# NDK Binder Example

这是一个使用Android NDK Binder的示例项目，展示了如何在原生代码中调用Binder named service。

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置文件
├── CMakePresets.json       # CMake预设配置
├── build.sh               # 构建脚本
├── src/                   # 源代码目录
│   └── main.cpp          # 主程序源文件
└── README.md             # 项目说明文档
```

## 环境要求

- Android NDK
- CMake 3.18或更高版本
- Ninja构建系统

## 构建说明

1. 确保已正确设置`ANDROID_NDK_HOME`环境变量：
   ```bash
   export ANDROID_NDK_HOME=/path/to/your/android-ndk
   ```

2. 为构建脚本添加执行权限：
   ```bash
   chmod +x build.sh
   ```

3. 运行构建脚本：
   ```bash
   # 构建arm64-v8a版本（默认）
   ./build.sh
   
   # 或构建armeabi-v7a版本
   ./build.sh android-arm
   ```

4. 推送并运行
   ```bash
   source push_and_run.sh 
   ```


## 使用说明

1. 在`main.cpp`中修改`service_name`为你要连接的实际服务名称
2. 在TODO部分实现具体的服务调用逻辑
3. 编译并部署到Android设备上运行

## 注意事项

- 确保目标Android设备/模拟器已启动你要连接的Binder服务
- 程序需要适当的权限才能连接到系统服务
- 建议在实际使用时添加错误处理和日志记录
