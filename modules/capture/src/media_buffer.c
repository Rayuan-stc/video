#include <string.h>

#include "fh_vb_mpi.h"
#include "fh_vb_mpipara.h"

#include "capture_internal.h"

void capture_media_driver_config(void)
{
    VB_CONF_S stVbConf;
    FH_SINT32 ret;

    FH_VB_Exit();

    memset(&stVbConf, 0, sizeof(VB_CONF_S));
    stVbConf.u32MaxPoolCnt = 4;
    stVbConf.astCommPool[0].u32BlkSize = 3840 * 2160 * 3;
    stVbConf.astCommPool[0].u32BlkCnt = 4;
    stVbConf.astCommPool[1].u32BlkSize = 1920 * 1080 * 3;
    stVbConf.astCommPool[1].u32BlkCnt = 4;
    stVbConf.astCommPool[2].u32BlkSize = 1280 * 720 * 3;
    stVbConf.astCommPool[2].u32BlkCnt = 4;
    stVbConf.astCommPool[3].u32BlkSize = 768 * 448 * 3;
    stVbConf.astCommPool[3].u32BlkCnt = 4;

    ret = FH_VB_SetConf(&stVbConf);
    if (ret) {
        printf("[FH_VB_SetConf] failed with:%x\n", ret);
    }

    ret = FH_VB_Init();
    if (ret) {
        printf("[FH_VB_Init] failed with:%x\n", ret);
    }

    WR_PROC_DEV(ENC_PROC, "allchnstm_0_20000000_40");
    WR_PROC_DEV(ENC_PROC, "stm_20000000_40");
    WR_PROC_DEV(JPEG_PROC, "frmsize_1_3000000_3000000");
    WR_PROC_DEV(JPEG_PROC, "jpgstm_12000000_2");
    WR_PROC_DEV(JPEG_PROC, "mjpgstm_12000000_2");
}
