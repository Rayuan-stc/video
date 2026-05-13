#include <stdio.h>

#include "fh_system_mpi.h"
#include "fh_venc_mpi.h"

#include "capture.h"
#include "capture_internal.h"

int capture_legacy_start(char *dst_ip, unsigned int port)
{
    int ret;

    printf("demo_main driver_config\n");
    capture_media_driver_config();

    ret = FH_SYS_Init();
    CHECK_RET(ret != 0, ret);

    printf("start_isp\n");
    ret = capture_isp_start();
    CHECK_RET(ret != 0, ret);
    printf("start_isp success\n");

    ret = capture_vpss_start();
    CHECK_RET(ret != 0, ret);

    ret = capture_venc_config(0, 1920, 1080);
    CHECK_RET(ret != 0, ret);

    ret = capture_bind_encoder_pipeline();
    CHECK_RET(ret != 0, ret);

    ret = FH_VENC_StartRecvPic(0);
    CHECK_RET(ret != 0, ret);

    ret = capture_stream_start(dst_ip, port);
    CHECK_RET(ret != 0, ret);

    ret = capture_osd_start();
    CHECK_RET(ret != 0, ret);

    ret = capture_apply_default_isp_params();
    CHECK_RET(ret != 0, ret);

    ret = capture_apply_default_mask();
    CHECK_RET(ret != 0, ret);

    return 0;
}

void capture_legacy_stop(void)
{
    capture_osd_stop();
    capture_stream_stop();
}
