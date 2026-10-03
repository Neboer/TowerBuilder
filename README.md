# TowerBuilder

基于官方规则的 Pyrga 双人热座叠塔棋。两名玩家在同一窗口轮流操作，全部棋子和界面由程序绘制，无需额外美术资源。

## 构建与运行

需要支持 C++17 的编译器、CMake 3.16+ 和 raylib 5.0+。当前 macOS 环境已实际构建、运行验证；其他平台尚未实机验证。

```sh
brew install cmake raylib
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/TowerBuilder
```

若旧 `build` 缓存来自其他目录，请改用新的构建目录，或用 CMake 3.24+ 的 `cmake --fresh -S . -B build` 重新配置。

## macOS 发布包

`v0.1` 提供 `TowerBuilder-v0.1-macos26plus-arm64.zip`：仅支持 Apple Silicon（arm64）和 macOS 26.0+。解压后打开 `TowerBuilder.app`；包内包含 raylib，无需安装 Homebrew。

应用只做本地 ad-hoc 签名，未做 Developer ID 签名或 Apple 公证。首次打开可能被 Gatekeeper 拦截；确认下载来自本仓库后，可按系统提示在“系统设置 → 隐私与安全性”中允许打开。不要全局关闭 Gatekeeper。

发布包由已有二进制生成，不重新编译，也不修改原始 `build/TowerBuilder`：

```sh
python3 scripts/package_macos.py --version v0.1
```

脚本检查 arm64 和 macOS 26.0 部署目标，内置 raylib 及其许可证，修正库路径、本地签名、验证并运行完整对局冒烟测试，输出 ZIP 和 SHA-256 校验文件至 `dist/`。

## 操作

- 点击选择形状，三角形另选朝向，再点击绿色高亮的合法格落子。
- 每格从底层 1 到顶层 3 显示棋子；玩家颜色和 P1/P2 标记表示归属。
- `1` / `2` / `3` 选择形状，方向键选择三角朝向；`Tab` / `Shift+Tab` 切换焦点，`Enter` / 空格操作。
- `H` 查看规则；`R` 新局。中途重开须确认，`Esc` 或 `N` 取消，`Y` 确认。每次新局交替先手。
- 结束后棋盘锁定，点击 Play again 再战；不提供悔棋、AI 或联网模式。

规则依据：[官方英文规则](https://argyxgames.com/pyrga/PYRGA_rules_EN.pdf)。先手第一手若下方块，自己的第二手不能再下方块；受限位置完全无法行动时只能另落空格。先控制三座完整塔者立即获胜，否则终局依次比较受控完整塔、两层塔、单层塔，全部相同为和局。

## 验证

```sh
ctest --test-dir build --output-on-failure
./build/TowerBuilder --smoke-test
```

CTest 包含规则行为回归和 24 场完整合法对局的冒烟测试；冒烟入口无需打开图形窗口。