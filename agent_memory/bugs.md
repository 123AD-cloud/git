# 问题与风险

## 当前问题

- 用户要求灯效模式一的灯珠颜色全部改成白色，已按最小范围修改 `LedEffect1()` 的点亮颜色逻辑。

## 已知风险

- 当前修改未在实机观察实际视觉效果，需要在设备上确认模式一 LED_DATA_3/LED_DATA_2 都符合预期白色显示。
- 当前环境未发现完整交叉编译工具链，无法做固件编译验证。
- `app_src/new/pwm_led.c` 含非 UTF-8 字节，Codex 审查面板基于临时 patch 的撤销可能不可靠；此文件改动应优先通过 Git commit + `git revert` 管理撤销。
- 当前工作区存在无关改动 `Untitled Project.si4project/Untitled Project.sip_xm` 和未跟踪文件 `how 5c42036171fba1329b4f2fe95581bf0769864183`，本任务未处理。

## 失败尝试

- `rg --files` 在当前环境被系统拒绝运行，改用 PowerShell 原生命令查找文件。
- `apply_patch` 无法读取 `app_src/new/pwm_led.c`，原因是文件包含非 UTF-8 字节。
- 为避免改变文件整体编码，改用二进制方式只替换 `LedEffect1()` 范围内两处 ASCII 颜色赋值文本。

## 待确认

- 如果用户所说“一灯珠”特指某个单独灯珠/峰值点，而不是模式一中所有被点亮灯珠，需要进一步调整范围。

## 解决记录

- 已将 `LedEffect1()` 中 LED_DATA_3 和 LED_DATA_2 的固定蓝色显示改成统一白色显示。
- 对 `pwm_led.c` 的撤销不要依赖 Codex 审查面板；提交后使用 `git revert <commit>`，或未提交时使用 `git restore -- app_src/new/pwm_led.c` 做整文件撤销。
