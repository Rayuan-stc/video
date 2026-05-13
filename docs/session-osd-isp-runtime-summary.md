# OSD、ISP 色彩与重复运行问题处理记录

本文汇总当前 session 以及 `019e167f-c3fb-70b1-93a1-ad858cd90b29` 相关 session 中定位过的问题、尝试方案和当前代码改动。

## Session 来源

- `019e167f-c3fb-70b1-93a1-ad858cd90b29`
  - 文件：`/home/fh8862/.codex/sessions/2026/05/11/rollout-2026-05-11T18-05-32-019e167f-c3fb-70b1-93a1-ad858cd90b29.jsonl`
  - 实际内容很短，只包含环境上下文、`hello`、以及读取 session 的请求。
  - 真正包含前序上下文的是相邻 session：`019e1676-bd0e-78f2-b446-1a58f4c065ca`。

- 当前 session
  - 主要围绕 `tests/videowatch.png`、`tests/videowatch_now.png`、`tests/run.log` 中暴露的问题继续修改。

## 问题列表

### 1. OSD 位置不对

现象：

- `tests/videowatch_now.png` 中 OSD 显示在画面中部。
- 需求是移动到左上角。

处理：

- 修改 `modules/capture/src/osd.c` 中各 `FHT_OSD_TextLine_t` 的坐标。
- 第一行：`x=32, y=32`
- 第二行：`x=32, y=96` 起始区域，其中年份被单独拆到另一个 layer。
- 第三行：`x=32, y=160`

### 2. OSD 内容需要三行

需求：

- 第一行：`camera channel1 - 0`
- 第二行：时间
- 第三行：`码流先锋队`

处理：

- 第一行直接使用 `sprintf(text_line_cfg[0].textInfo, "camera channel1 - %d", 0)`。
- 第三行使用 GB2312 字节串：

```c
FH_CHAR slogan_data[] = {
    0xc2, 0xeb, 0xc1, 0xf7, 0xcf, 0xc8, 0xb7, 0xe6, 0xb6, 0xd3, 0,
};
```

- 时间曾尝试使用 SDK 内置 `FHTEN_OSD_TimeFmt0`，但年份反色仍异常，最终改为拆分：
  - 年份 `FHT_OSD_YEAR4` 单独绘制。
  - 月日时分秒作为剩余时间串绘制。

### 3. 年份 `1970` 自动反色异常

现象：

- OSD 中只有年份字段 `1970` 始终显示为黑色。
- 其它字符可以正常按背景反色。

排查：

- 蓝色 mask 坐标：`x=32, y=224, w=360, h=80`
- 时间行坐标：`x=32, y=96`
- 二者没有重叠，所以不是 mask 压在年份后面导致。
- 将时间改成 SDK 内置 `timeOsdEnable = 1` / `FHTEN_OSD_TimeFmt0` 后，年份问题仍存在，说明 SDK 对年份 tag 的反色处理本身有差异。

当前处理：

- `osd_cfg.nOsdLayerNum` 从 `1` 改为 `2`。
- 第 0 个 layer 开启自动反色：

```c
pOsdLayerInfo[0].edgePixel = 0;
pOsdLayerInfo[0].osdInvertEnable = FH_OSD_INVERT_BY_CHAR;
pOsdLayerInfo[0].osdInvertThreshold.high_level = 180;
pOsdLayerInfo[0].osdInvertThreshold.low_level = 160;
```

- 第 1 个 layer 复制第 0 个 layer 配置，但关闭反色：

```c
pOsdLayerInfo[1] = pOsdLayerInfo[0];
pOsdLayerInfo[1].osdInvertEnable = FH_OSD_INVERT_DISABLE;
pOsdLayerInfo[1].layerId = 1;
```

- 年份单独画到第 1 个 layer：

```c
FH_CHAR year_tag_data[] = {
    0xe4, 0x01,
    0,
};
```

- 月日时分秒留在第 0 个自动反色 layer：

```c
FH_CHAR time_tail_tag_data[] = {
    '-',  0xe4, 0x03,
    '-',  0xe4, 0x04,
    0x20,
    0xe4, 0x07,
    ':',  0xe4, 0x09,
    ':',  0xe4, 0x0a,
    0,
};
```

### 4. OSD 从勾边改为自动反色

现象：

- 原来 OSD 通过 `edgePixel = 1` 做黑色勾边。
- 需求改为自动反色。

处理：

- 去掉勾边：`edgePixel = 0`
- 启用逐字符反色：`osdInvertEnable = FH_OSD_INVERT_BY_CHAR`
- 阈值恢复 SDK 示例中的原始值：`high_level = 180`，`low_level = 160`

### 5. 色块颜色与位置不对

现象：

- 早期用 `FH_VPSS_SetMask` 时，色块在左上角，且期望蓝色却显示成黄色/白色。
- 这是因为 VPU mask 的 `color` 不是按期望 RGB 解释。

处理：

- 将色块从 VPU mask 改到 OSD mask。
- 初始化 OSD 时增加：

```c
graph_ctrl |= FHT_OSD_GRAPH_CTRL_MASK_AFTER_VP;
```

- 在 `modules/capture/src/osd.c` 中实现 `capture_apply_default_mask()`，调用 `FHAdv_Osd_SetMask()`：

```c
mask_cfg.area.fTopLeftX = 32;
mask_cfg.area.fTopLeftY = 224;
mask_cfg.area.fWidth = 360;
mask_cfg.area.fHeigh = 80;
mask_cfg.osdColor.fAlpha = 255;
mask_cfg.osdColor.fRed = 0;
mask_cfg.osdColor.fGreen = 0;
mask_cfg.osdColor.fBlue = 255;
```

- 删除 `modules/capture/src/vpu.c` 中旧的 `capture_apply_default_mask()`，避免继续使用 `FH_VPSS_SetMask()`。

当前工作树中 `vpu.c` 没有未提交改动，因为旧实现已经在后续状态中被移除或回退到当前最终状态之外；当前最终有效实现位于 `osd.c`。

### 6. 画面颜色失真、偏紫

现象：

- `tests/videowatch.png` 中画面有明显偏色。
- 用户明确要求 `ret = isp_set_param(ISP_MF, 3);` 必须保留，否则画面方向会倒。

尝试过的方案：

- 曾尝试将 `ISP_MF` 改为 `0`。该方案会修掉一部分 Bayer/方向带来的偏色，但画面方向不符合实际需求，因此回退。
- 曾尝试将 `API_ISP_SetMirrorAndflip()` 改为 `API_ISP_SetMirrorAndflipEx()` 并传入 Bayer pattern。实际效果仍不满足，且可能进一步改变 ISP/Sensor 的 Bayer 关系。

当前处理：

- 保持 `ISP_MF = 3`。
- 撤回 `API_ISP_SetMirrorAndflipEx()`，恢复使用：

```c
ret |= API_ISP_SetMirrorAndflip(0, mirror, flip);
```

说明：

- 当前代码层面没有继续强行改 Bayer。
- 颜色失真仍可能与 `/home/imx415_mipi_attr.hex`、AWB/CCM 参数、sensor 实际安装方向和 ISP 参数包有关。
- 如果当前画面仍偏色，下一步应针对 ISP 参数包或 AWB/色彩矩阵继续查，而不是再通过关闭 `MF=3` 规避。

### 7. 第二次运行 `videocatch` 失败

日志：

文件：`tests/run.log`

第二次运行出现：

```text
ERROR: Pkg load err !
wait pic_start outtime, id,0
API_ISP_Run error with ret = 0xa0084100
FH_VENC_GetStream_Block(...) failed
```

原因判断：

- 原 `capture_legacy_stop()` 只调用 `capture_stream_stop()`。
- 取流线程是 detached，退出时不能等待线程结束。
- VENC、VPSS、OSD、ISP、SYS 都没有释放。
- 第一次退出后，开发板上的媒体资源残留，第二次启动重新加载 ISP package 或等待编码帧时失败。

当前处理：

- `modules/capture/src/stream.c`
  - 保存 `pthread_t g_stream_thread`
  - 取流线程从 detached 改为 joinable
  - `capture_stream_stop()` 中设置停止标志并 `pthread_join()`

- `modules/capture/src/capture.c`
  - `capture_legacy_stop()` 中补齐清理流程：

```c
FH_VENC_StopRecvPic(0);
capture_stream_stop();
FH_SYS_UnBindbySrc(src);
FH_SYS_UnBindbyDst(dst);
FHAdv_Osd_UnInit(0);
FH_VENC_DestroyChn(0);
FH_VPSS_CloseChn(GROUP_ID, 0);
FH_VPSS_DestroyChn(GROUP_ID, 0);
FH_VPSS_Disable(GROUP_ID);
FH_VPSS_DestroyGrp(GROUP_ID);
API_ISP_Exit(0);
API_ISP_Close();
FH_SYS_Exit();
```

## 当前源码改动文件

当前工作树中有改动的文件：

- `modules/capture/src/capture.c`
- `modules/capture/src/stream.c`
- `modules/capture/src/isp.c`
- `modules/capture/src/osd.c`

## 当前验证

已执行：

```bash
make
```

结果：

- 编译通过。
- 生成：`build/videocatch`

## 遗留风险与建议

- 年份反色问题当前通过单独 layer 绕开，而不是修 SDK 内部年份 tag 的反色逻辑。
- 画面颜色失真尚未从根因完全闭环。当前只撤回了会影响 Bayer 的尝试，并保留 `MF=3`。若画面仍偏色，应继续检查：
  - `/home/imx415_mipi_attr.hex` 是否匹配当前 sensor 和镜像方向。
  - AWB/CCM/色彩矩阵配置。
  - sensor 实际安装方向与 `BAYER_GBRG` 是否一致。
  - `API_ISP_SetMirrorAndflip()` 是否只改方向，不改 Bayer，如果实际 SDK 内部行为不同，需要用开发板实测确认。
- 第二次运行问题已补清理路径，但还需要在开发板连续运行两次验证。
