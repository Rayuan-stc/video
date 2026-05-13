#ifndef CAPTURE_INTERNAL_H
#define CAPTURE_INTERNAL_H

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef CHECK_RET
#define CHECK_RET(state, error_code)                                                               \
    if (state) {                                                                                   \
        printf("[%s]:%d line [%s] return 0x%x ERROR\n", __FILE__, __LINE__, __func__, error_code); \
        return error_code;                                                                         \
    }
#endif

#define ISP_W0 3864
#define ISP_H0 2192
#define ISP_W 3840
#define ISP_H 2160
#define ISP_F 30

#define GROUP_ID 0

#define ISP_PROC "/proc/driver/isp"
#define VPU_PROC "/proc/driver/vpu"
#define BGM_PROC "/proc/driver/bgm"
#define ENC_PROC "/proc/driver/enc"
#define JPEG_PROC "/proc/driver/jpeg"
#define TRACE_PROC "/proc/driver/trace"

#define WR_PROC_DEV(device, cmd)                                                                   \
    do {                                                                                           \
        int _tmp_fd;                                                                               \
        _tmp_fd = open(device, O_RDWR, 0);                                                         \
        if (_tmp_fd >= 0) {                                                                        \
            write(_tmp_fd, cmd, sizeof(cmd));                                                      \
            close(_tmp_fd);                                                                        \
        }                                                                                          \
    } while (0)

void capture_media_driver_config(void);
int capture_isp_start(void);
int capture_vpss_start(void);
int capture_venc_config(int chan, int enc_w, int enc_h);
int capture_bind_encoder_pipeline(void);
int capture_stream_start(char *dst_ip, unsigned int port);
void capture_stream_stop(void);
int capture_osd_start(void);
void capture_osd_stop(void);
int capture_apply_default_isp_params(void);
int capture_apply_default_mask(void);

#endif /* CAPTURE_INTERNAL_H */
