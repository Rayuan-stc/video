#include "fh_venc_mpi.h"

#include "capture_internal.h"

int capture_venc_config(int chan, int enc_w, int enc_h)
{
    FH_VENC_CHN_CAP cfg_vencmem;

    cfg_vencmem.support_type = FH_NORMAL_H264 | FH_NORMAL_H265;
    cfg_vencmem.max_size.u32Width = ISP_W;
    cfg_vencmem.max_size.u32Height = ISP_H;

    int ret = FH_VENC_CreateChn(chan, &cfg_vencmem);
    if (ret != 0) {
        return ret;
    }

    FH_VENC_CHN_CONFIG cfg_param;

    cfg_param.chn_attr.enc_type = FH_NORMAL_H264;
    cfg_param.chn_attr.h264_attr.profile = H264_PROFILE_MAIN;
    cfg_param.chn_attr.h264_attr.i_frame_intterval = 50;
    cfg_param.chn_attr.h264_attr.size.u32Width = enc_w;
    cfg_param.chn_attr.h264_attr.size.u32Height = enc_h;

    cfg_param.rc_attr.rc_type = FH_RC_H264_VBR;
    cfg_param.rc_attr.h264_vbr.bitrate = 2 * 1024 * 1024;
    cfg_param.rc_attr.h264_vbr.init_qp = 35;
    cfg_param.rc_attr.h264_vbr.ImaxQP = 42;
    cfg_param.rc_attr.h264_vbr.IminQP = 28;
    cfg_param.rc_attr.h264_vbr.PmaxQP = 42;
    cfg_param.rc_attr.h264_vbr.PminQP = 28;
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_count = 25;
    cfg_param.rc_attr.h264_vbr.FrameRate.frame_time = 1;
    cfg_param.rc_attr.h264_vbr.maxrate_percent = 200;
    cfg_param.rc_attr.h264_vbr.IFrmMaxBits = 0;
    cfg_param.rc_attr.h264_vbr.IP_QPDelta = 3;
    cfg_param.rc_attr.h264_vbr.I_BitProp = 5;
    cfg_param.rc_attr.h264_vbr.P_BitProp = 1;
    cfg_param.rc_attr.h264_vbr.fluctuate_level = 0;

    return FH_VENC_SetChnAttr(chan, &cfg_param);
}
