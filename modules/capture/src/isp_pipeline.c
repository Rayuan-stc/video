#include <stdlib.h>

#include "fh_system_mpi.h"
#include "vicap/fh_vicap_mpi.h"

#include "capture_internal.h"
#include "sensor.h"

static struct isp_sensor_if sensor_func;

int capture_isp_start(void)
{
    int ret;
    int vimod = 0; // 0:online 1:offline
    int vomod = 1; // 1:to vpu 2:to ddr

    ISP_MEM_INIT stMemInit = {0};
    ISP_VI_ATTR_S vi_attr = {0};

    ISP_PARAM_CONFIG stIsp_para_cfg;
    unsigned int param_size;
    char *isp_param_buff;
    Sensor_Init_t initConf = {0};

    FH_VICAP_DEV_ATTR_S stViDev = {0};
    FH_VICAP_VI_ATTR_S stViAttr = {0};

    FILE *param_file;

    isp_sensor_reset();

    stMemInit.enOfflineWorkMode = vimod;
    stMemInit.enIspOutMode = vomod;
    stMemInit.enIspOutFmt = 1; // 422 8bit
    stMemInit.stPicConf.u32Width = ISP_W;
    stMemInit.stPicConf.u32Height = ISP_H;
    ret = API_ISP_MemInit(0, &stMemInit);
    CHECK_RET(ret != 0, ret);

    vi_attr.u16InputHeight = ISP_H;
    vi_attr.u16InputWidth = ISP_W;
    vi_attr.u16PicHeight = ISP_H;
    vi_attr.u16PicWidth = ISP_W;
    vi_attr.u16FrameRate = ISP_F;
    vi_attr.enBayerType = BAYER_GBRG;
    ret = API_ISP_SetViAttr(0, &vi_attr);
    CHECK_RET(ret != 0, ret);

    sensor_func.init = sensor_init_imx415;
    sensor_func.set_sns_fmt = sensor_set_fmt_imx415;
    sensor_func.set_sns_reg = sensor_write_reg;
    sensor_func.get_sns_reg = sensor_read_reg;
    sensor_func.set_exposure_ratio = sensor_set_exposure_ratio_imx415;
    sensor_func.get_exposure_ratio = sensor_get_exposure_ratio_imx415;
    sensor_func.get_sensor_attribute = sensor_get_attribute_imx415;
    sensor_func.set_flipmirror = sensor_set_mirror_flip_imx415;
    sensor_func.get_sns_ae_default = GetAEDefault;
    sensor_func.get_sns_ae_info = GetAEInfo;
    sensor_func.set_sns_gain = SetGain;
    sensor_func.set_sns_intt = SetIntt;
    ret = API_ISP_SensorRegCb(0, 0, &sensor_func);
    CHECK_RET(ret != 0, ret);

    ret = API_ISP_SensorInit(0, &initConf);
    CHECK_RET(ret != 0, ret);

    ret = API_ISP_Init(0);
    CHECK_RET(ret != 0, ret);

    stViDev.enWorkMode = vimod;
    stViDev.stSize.u16Width = ISP_W;
    stViDev.stSize.u16Height = ISP_H;
    ret = FH_VICAP_InitViDev(0, &stViDev);
    CHECK_RET(ret != 0, ret);

    stViAttr.enWorkMode = vimod;
    stViAttr.stInSize.u16Width = ISP_W0;
    stViAttr.stInSize.u16Height = ISP_H0;
    stViAttr.stCropSize.bCutEnable = 1;
    stViAttr.stCropSize.stRect.u16Width = ISP_W;
    stViAttr.stCropSize.stRect.u16Height = ISP_H;
    ret = FH_VICAP_SetViAttr(0, &stViAttr);
    CHECK_RET(ret != 0, ret);

    if (vimod == 1) {
        FH_BIND_INFO src, dst;
        src.obj_id = FH_OBJ_VICAP;
        src.dev_id = 0;
        src.chn_id = 0;
        dst.obj_id = FH_OBJ_ISP;
        dst.dev_id = 0;
        dst.chn_id = 0;
        FH_SYS_Bind(src, dst);
    }

    ret = API_ISP_GetBinAddr(0, &stIsp_para_cfg);
    param_size = stIsp_para_cfg.u32BinSize;
    CHECK_RET(ret != 0, ret);

    isp_param_buff = (char *)malloc(param_size);
    param_file = fopen(SENSOR_PARAM, "rb");
    if (NULL == param_file) {
        free(isp_param_buff);
        printf("open file failed!\n");
        return -1;
    }

    if (param_size != fread(isp_param_buff, 1, param_size, param_file)) {
        free(isp_param_buff);
        fclose(param_file);
        printf("open file failed!\n");
        return -1;
    }

    ret = API_ISP_LoadIspParam(0, isp_param_buff);
    CHECK_RET(ret != 0, ret);
    free(isp_param_buff);
    fclose(param_file);

    ret = isp_server_run();
    CHECK_RET(ret != 0, ret);

    return 0;
}

int capture_apply_default_isp_params(void)
{
    int ret;

    ret = isp_set_param(ISP_AE, 1);
    CHECK_RET(ret != 0, ret);

    ret = isp_set_param(ISP_AWB, 1);
    CHECK_RET(ret != 0, ret);

    // 饱和度
    ret = isp_set_param(ISP_COLOR, 128);
    CHECK_RET(ret != 0, ret);

    // 亮度
    ret = isp_set_param(ISP_BRIGHT, 125);
    CHECK_RET(ret != 0, ret);

    // 自动降噪
    ret = isp_set_param(ISP_NR, 1);
    CHECK_RET(ret != 0, ret);

    ret = isp_set_param(ISP_MF, 3);
    CHECK_RET(ret != 0, ret);

    return 0;
}
