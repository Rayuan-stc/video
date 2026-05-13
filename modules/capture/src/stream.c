#include <pthread.h>

#include "fh_venc_mpi.h"

#include "capture_internal.h"
#include "libdmc.h"
#include "libdmc_pes.h"

static int g_get_stream_stop = 0;
static int g_get_stream_running = 0;

static void *capture_get_stream_proc(void *arg)
{
    FH_SINT32 ret, i;
    FH_SINT32 end_flag;
    FH_SINT32 subtype;
    FH_VENC_STREAM stream;
    FH_SINT32 *stop = (FH_SINT32 *)arg;

    while (*stop == 0) {
        WR_PROC_DEV(TRACE_PROC, "timing_GetStream_START");

        ret = FH_VENC_GetStream_Block(FH_STREAM_ALL & (~(FH_STREAM_JPEG)), &stream);
        WR_PROC_DEV(TRACE_PROC, "timing_EncBlkFinish_xxx");

        if (ret != 0) {
            printf("Error(%d - %x): FH_VENC_GetStream_Block(FH_STREAM_ALL & (~(FH_STREAM_JPEG))) "
                   "failed!\n",
                   ret, ret);
            continue;
        }

        if (stream.stmtype == FH_STREAM_H264) {
            subtype = stream.h264_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME
                                                                  : DMC_MEDIA_SUBTYPE_PFRAME;
            for (i = 0; i < stream.h264_stream.nalu_cnt; i++) {
                end_flag = (i == (stream.h264_stream.nalu_cnt - 1)) ? 1 : 0;
                dmc_input(stream.chan, DMC_MEDIA_TYPE_H264, subtype, stream.h264_stream.time_stamp,
                          stream.h264_stream.nalu[i].start, stream.h264_stream.nalu[i].length,
                          end_flag);
            }
        } else if (stream.stmtype == FH_STREAM_H265) {
            subtype = stream.h265_stream.frame_type == FH_FRAME_I ? DMC_MEDIA_SUBTYPE_IFRAME
                                                                  : DMC_MEDIA_SUBTYPE_PFRAME;
            for (i = 0; i < stream.h265_stream.nalu_cnt; i++) {
                end_flag = (i == (stream.h265_stream.nalu_cnt - 1)) ? 1 : 0;
                dmc_input(stream.chan, DMC_MEDIA_TYPE_H265, subtype, stream.h265_stream.time_stamp,
                          stream.h265_stream.nalu[i].start, stream.h265_stream.nalu[i].length,
                          end_flag);
            }
        } else if (stream.stmtype == FH_STREAM_MJPEG) {
            dmc_input(stream.chan, DMC_MEDIA_TYPE_MJPEG, 0, 0, stream.mjpeg_stream.start,
                      stream.mjpeg_stream.length, 1);
        }

        ret = FH_VENC_ReleaseStream(&stream);
        if (ret) {
            printf("Error(%d - %x): FH_VENC_ReleaseStream failed for chan(%d)!\n", ret, ret,
                   stream.chan);
        }
        WR_PROC_DEV(TRACE_PROC, "timing_GetStream_END");
    }

    *stop = 0;
    return NULL;
}

int capture_stream_start(char *dst_ip, unsigned int port)
{
    pthread_attr_t attr;
    pthread_t thread_stream;

    dmc_init();
    if (dst_ip != NULL && *dst_ip != 0) {
        dmc_pes_subscribe(1, dst_ip, port);
    }

    if (!g_get_stream_running) {
        g_get_stream_running = 1;
        g_get_stream_stop = 0;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_attr_setstacksize(&attr, 3 * 1024);
        pthread_create(&thread_stream, &attr, capture_get_stream_proc, &g_get_stream_stop);
    }

    return 0;
}

void capture_stream_stop(void)
{
    g_get_stream_stop = 1;
}
