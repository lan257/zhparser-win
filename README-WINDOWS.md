```markdown

\# zhparser Windows (MSVC) build instructions



前提

\- Visual Studio "Desktop development with C++" 已安装

\- 已把 scws 源 clone 到本目录的 scws 子目录

\- 已把 amutu/zhparser 源 clone 到 zhparser 子目录

\- 已知 pg\_config.exe 路径（例如 `D:/pg17/bin/pg\_config.exe`）



快速构建（在 PowerShell 或 cmd）：



1\. 克隆源（如果还没做）：

&nbsp;  git clone --branch 1.2.3 --depth 1 https://github.com/hightman/scws.git scws

&nbsp;  git clone https://github.com/amutu/zhparser.git zhparser



2\. 从项目根目录调用 cmake（示例）：

&nbsp;  cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DPG\_CONFIG="D:/pg17/bin/pg\_config.exe"



&nbsp;  说明：

&nbsp;  - 你可以在 VS 里使用 "Open -> Folder" 打开 root，或者在命令行用上述 cmake 命令生成 VS 解决方案。

&nbsp;  - 如果使用不同的 VS 生成器，把 -G 替换为你的 VS 生成器名，例如 "Visual Studio 16 2019"。



3\. 用 Visual Studio 打开 build/zhparser-windows.sln 或直接用 cmake 编译：

&nbsp;  cmake --build build --config Release



4\. 安装 / 部署（需要管理员权限，或以 Postgres 服务可访问的权限）：

&nbsp;  - 把生成的 zhparser.dll 和 scws.dll 复制到 PostgreSQL 的 lib 目录（例：D:/pg17/lib）。

&nbsp;  - 把 zhparser 的扩展 SQL 文件（zhparser/zhparser--2.3.sql 等）复制到 PostgreSQL 的 share/extension 目录（例：D:/pg17/share/extension）。

&nbsp;  - 重启 PostgreSQL 服务。



5\. 在 psql 中创建扩展：

&nbsp;  psql -U postgres -d yourdb -c "CREATE EXTENSION zhparser;"



6\. 测试：

&nbsp;  SELECT \* FROM ts\_parse('zhparser','测试 2010年保障房建设在全国范围内获全面启动');



调试建议

\- 如果 CREATE EXTENSION 时出现 "could not load library" 或类似错误：

&nbsp; - 用 Dependency Walker / Dependencies 工具检查 zhparser.dll 的依赖，确认 scws.dll 与其他依赖是否在 D:/pg17/lib 或 PATH 上。

&nbsp; - 检查生成的 dll 是否使用与 PostgreSQL 相同的运行时（/MD）。在 VS 项目属性 -> C/C++ -> Code Generation -> Runtime Library 查看并统一设置。

&nbsp; - 若链接失败提示缺少 postgres.lib，请在 D:/pg17/lib 查找可用 .lib；如果存在，将其路径或者文件名添加到 zhparser 的 target\_link\_libraries 中。



如果你希望，我可以：

\- 进一步检测并根据你的 D:/pg17/lib 列表自动修改 CMake，以指定正确的 server import lib（postgres.lib）。

\- 根据你构建后遇到的错误（链接或运行时），给出具体修复补丁或修改建议。

```

