#include <string.h>

#include "fh_system_mpi.h"
#include "fh_vpu_mpi.h"

#include "capture_internal.h"

int capture_vpss_start(void)
{
    int ret;

    FH_VPU_SET_GRP_INFO grp_info;
    grp_info.vi_max_size.u32Width = ISP_W;
    grp_info.vi_max_size.u32Height = ISP_H;
    grp_info.ycmean_en = 1;
    grp_info.ycmean_ds = 16;

    ret = FH_VPSS_CreateGrp(GROUP_ID, &grp_info);
    CHECK_RET(ret != 0, ret);

    FH_VPU_SIZE vi_pic;
    vi_pic.vi_size.u32Width = ISP_W;
    vi_pic.vi_size.u32Height = ISP_H;
    vi_pic.crop_area.crop_en = 0;
    vi_pic.crop_area.vpu_crop_area.u32X = 0;
    vi_pic.crop_area.vpu_crop_area.u32Y = 0;
    vi_pic.crop_area.vpu_crop_area.u32Width = 0;
    vi_pic.crop_area.vpu_crop_area.u32Height = 0;

    ret = FH_VPSS_SetViAttr(GROUP_ID, &vi_pic);
    CHECK_RET(ret != 0, ret);

    ret = FH_VPSS_Enable(GROUP_ID, VPU_MODE_ISP);
    CHECK_RET(ret != 0, ret);

    FH_VPU_CHN_INFO chn_info = {0};
    chn_info.bgm_enable = 1;
    chn_info.cpy_enable = 1;
    chn_info.sad_enable = 1;
    chn_info.bgm_ds = 8;
    chn_info.chn_max_size.u32Width = ISP_W;
    chn_info.chn_max_size.u32Height = ISP_H;
    chn_info.out_mode = VPU_VOMODE_SCAN;
    chn_info.support_mode = 1 << chn_info.out_mode;
    chn_info.bufnum = 3;
    chn_info.max_stride = 0;
    ret = FH_VPSS_CreateChn(GROUP_ID, 0, &chn_info);
    CHECK_RET(ret != 0, ret);

    FH_VPU_CHN_CONFIG chn_attr;
    chn_attr.vpu_chn_size.u32Width = 1920;
    chn_attr.vpu_chn_size.u32Height = 1080;
    chn_attr.crop_area.crop_en = 0;
    chn_attr.crop_area.vpu_crop_area.u32X = 0;
    chn_attr.crop_area.vpu_crop_area.u32Y = 0;
    chn_attr.crop_area.vpu_crop_area.u32Width = 0;
    chn_attr.crop_area.vpu_crop_area.u32Height = 0;
    chn_attr.offset = 0;
    chn_attr.depth = 1;
    chn_attr.stride = 0;
    ret = FH_VPSS_SetChnAttr(GROUP_ID, 0, &chn_attr);
    CHECK_RET(ret != 0, ret);

    ret = FH_VPSS_SetVOMode(GROUP_ID, 0, VPU_VOMODE_SCAN);
    CHECK_RET(ret != 0, ret);

    ret = FH_VPSS_OpenChn(GROUP_ID, 0);
    CHECK_RET(ret != 0, ret);

    return 0;
}

int capture_bind_encoder_pipeline(void)
{
    int ret;
    FH_BIND_INFO src, dst;

    src.obj_id = FH_OBJ_ISP;
    src.dev_id = 0;
    src.chn_id = 0;
    dst.obj_id = FH_OBJ_VPU_VI;
    dst.dev_id = 0;
    dst.chn_id = 0;
    ret = FH_SYS_Bind(src, dst);
    CHECK_RET(ret != 0, ret);

    src.obj_id = FH_OBJ_VPU_VO;
    src.dev_id = GROUP_ID;
    src.chn_id = 0;

    dst.obj_id = FH_OBJ_ENC;
    dst.dev_id = 0;
    dst.chn_id = 0;

    ret = FH_SYS_Bind(src, dst);
    CHECK_RET(ret != 0, ret);

    return 0;
}
