# UE 5.7 compatibility / UE 5.7 兼容适配

本分支基于 TDGameStudio/UnrealAngelscriptPlugin 的
`974e2811c76b5d80b0f2dc6f4c7a28955765bf80`，保留上游历史与许可证。
用于在 UE 5.7.1 安装版中使用已开始采用 UE 5.8 API 的新版插件。

## 修改内容

- 按引擎版本适配反射 Property 构造、引擎初始化委托和 UEnum 设置。
- 适配多播委托、JSON 键类型、碰撞查询、AssetBundle 重载和 CoreRedirect 比较。
- 明确不可复制的测试对象，兼容 UE 5.7 的 TOptional 检查。
- UE 5.7 已自动编译 UHT 生成的绑定文件，不再额外生成包装编译单元，避免启动时重复注册。

## 验证范围

2026-09-18 在一个外部 UE 5.7.1 C++ 工程中验证：编辑器 Development 构建成功；
4 项脚本集成测试通过（GameplayTag、C++ 调用脚本事件、脚本动作扩展、GAS 属性和输出参数）；
PIE 启动及运行中的脚本热重载通过。源码与该次验证使用的插件逐文件核对一致。

UE 5.8 调用分支保留，但没有在 UE 5.8 上复测。尚未验证 Shipping 打包与打包后脚本重载。
此 fork 只包含通用插件适配，不包含外部项目的资产、接口绑定或测试源码。
GameplayTags 和 GAS 扩展仍使用上游独立插件。

## English

This fork adds installed UE 5.7.1 compatibility to upstream commit
`974e2811c76b5d80b0f2dc6f4c7a28955765bf80`, preserving upstream history and licenses.
The changes cover reflection property construction, initialization delegates, enum and collision APIs,
overload resolution, redirect comparison, and non-copyable test state. On UE 5.7, UHT-generated
binding files are already compiled automatically; additional wrappers would register each bind twice.

Validated in an external UE 5.7.1 consumer: Development Editor build, four script integration tests,
PIE startup, and script hot reload during PIE. The adapted source files match that tested deployment.
UE 5.8 paths are retained but were not tested here. Shipping builds and packaged script reload remain
unverified. Install the plugin as `Plugins/Angelscript`; optional GameplayTags and GAS plugins remain
separate upstream dependencies.
