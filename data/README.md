# VectorFlow 项目目录结构

## 源代码目录
- `*.cpp`, `*.h` - C++ 源代码和头文件
- `CMakeLists.txt` - CMake 构建配置
- `icons.qrc` - Qt 资源文件配置

## 数据目录 (data/)

### SVG 测试文件 (data/svg-tests/)
- `test_*.svg` - 各种 SVG 功能测试文件
- `strz.min.svg` - 压缩的 SVG 文件
- `test_instructions.txt` - 测试说明文档

### 图标资源 (data/icons/)
- `*-tool.svg` - 各种工具的图标文件
- `*-tool-new.svg` - 新版本工具图标

### 临时文件 (data/temp/)
- 临时文件和测试代码

## 构建目录
- `build/` - CMake 生成的构建文件
- `external/` - 外部依赖库

## 文档目录
- `*.md` - 项目文档和分析文件

## Git 配置
- `.gitignore` - Git 忽略文件配置
- `.vscode/` - VS Code 编辑器配置