#include <pthread.h>
#include <string.h>
#include <time.h>

#include "FHAdv_OSD_mpi.h"

#include "capture_internal.h"
#include "font_array.h"
#include "rtc_time.h"

#define OSD_LAYER_ID 0
#define OSD_TIME_LINE_ID 1

static pthread_mutex_t g_osd_time_lock = PTHREAD_MUTEX_INITIALIZER;
static int g_osd_time_stop = 0;
static int g_osd_time_running = 0;
static pthread_t g_osd_time_thread;
static FHT_OSD_TextLine_t g_osd_time_line;
static FH_CHAR g_osd_time_text[64];

static int osd_time_should_stop(void)
{
    pthread_mutex_lock(&g_osd_time_lock);
    int stop = g_osd_time_stop;
    pthread_mutex_unlock(&g_osd_time_lock);

    return stop;
}

static int osd_read_current_time(struct tm *tm_now)
{
    if (rtc_time_read(tm_now) == 0) {
        return 0;
    }

    time_t now = time(NULL);
    if (now == (time_t)-1 || localtime_r(&now, tm_now) == NULL) {
        return -1;
    }

    return 0;
}

static int osd_refresh_time_line(void)
{
    struct tm tm_now;

    if (osd_read_current_time(&tm_now) != 0) {
        return -1;
    }

    snprintf(g_osd_time_text, sizeof(g_osd_time_text), "%04d-%02d-%02d %02d:%02d:%02d",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_hour,
             tm_now.tm_min, tm_now.tm_sec);

    int ret = FHAdv_Osd_SetTextLine(0, 0, OSD_LAYER_ID, &g_osd_time_line);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_SetTextLine time failed with %d\n", ret);
        return ret;
    }

    return 0;
}

static void *osd_time_proc(void *arg)
{
    (void)arg;

    while (!osd_time_should_stop()) {
        osd_refresh_time_line();
        for (int i = 0; i < 10 && !osd_time_should_stop(); i++) {
            usleep(100000);
        }
    }

    return NULL;
}

static int osd_start_time_thread(void)
{
    pthread_attr_t attr;

    pthread_mutex_lock(&g_osd_time_lock);
    if (g_osd_time_running) {
        pthread_mutex_unlock(&g_osd_time_lock);
        return 0;
    }

    g_osd_time_stop = 0;
    g_osd_time_running = 1;
    pthread_mutex_unlock(&g_osd_time_lock);

    pthread_attr_init(&attr);

    int ret = pthread_create(&g_osd_time_thread, &attr, osd_time_proc, NULL);
    pthread_attr_destroy(&attr);
    if (ret != 0) {
        pthread_mutex_lock(&g_osd_time_lock);
        g_osd_time_stop = 1;
        g_osd_time_running = 0;
        pthread_mutex_unlock(&g_osd_time_lock);
        printf("Error: create OSD time thread failed!\n");
        return -1;
    }

    return 0;
}

int capture_osd_start(void)
{
    int ret;
    int graph_ctrl = 0;

    graph_ctrl |= FHT_OSD_GRAPH_CTRL_TOSD_AFTER_VP;
    graph_ctrl |= FHT_OSD_GRAPH_CTRL_MASK_AFTER_VP;

    ret = FHAdv_Osd_Init(0, FHT_OSD_DEBUG_LEVEL_ERROR, graph_ctrl, 0, 0);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_Init failed with %x\n", ret);
        return ret;
    }

    FHT_OSD_FontLib_t font_lib;

    font_lib.pLibData = asc16;
    font_lib.libSize = sizeof(asc16);
    ret = FHAdv_Osd_LoadFontLib(FHEN_FONT_TYPE_ASC, &font_lib);
    if (ret != 0) {
        printf("Error: Load ASC font lib failed, ret=%d\n", ret);
        return ret;
    }

    font_lib.pLibData = gb2312;
    font_lib.libSize = sizeof(gb2312);
    ret = FHAdv_Osd_LoadFontLib(FHEN_FONT_TYPE_CHINESE, &font_lib);
    if (ret != 0) {
        printf("Error: Load CHINESE font lib failed, ret=%d\n", ret);
        return ret;
    }

    FHT_OSD_CONFIG_t osd_cfg;
    FHT_OSD_Layer_Config_t pOsdLayerInfo[4];
    FHT_OSD_TextLine_t text_line_cfg[4];
    FH_CHAR text_data[4][128];

    memset(&osd_cfg, 0, sizeof(osd_cfg));
    memset(&pOsdLayerInfo[0], 0, 4 * sizeof(FHT_OSD_Layer_Config_t));
    memset(&text_line_cfg[0], 0, 4 * sizeof(FHT_OSD_TextLine_t));
    memset(&text_data, 0, sizeof(text_data));

    osd_cfg.osdRotate = 0;
    osd_cfg.pOsdLayerInfo = &pOsdLayerInfo[0];
    osd_cfg.nOsdLayerNum = 1;

    pOsdLayerInfo[0].layerStartX = 0;
    pOsdLayerInfo[0].layerStartY = 0;
    pOsdLayerInfo[0].osdSize = 64;

    pOsdLayerInfo[0].normalColor.fAlpha = 255;
    pOsdLayerInfo[0].normalColor.fRed = 255;
    pOsdLayerInfo[0].normalColor.fGreen = 255;
    pOsdLayerInfo[0].normalColor.fBlue = 255;

    pOsdLayerInfo[0].invertColor.fAlpha = 255;
    pOsdLayerInfo[0].invertColor.fRed = 0;
    pOsdLayerInfo[0].invertColor.fGreen = 0;
    pOsdLayerInfo[0].invertColor.fBlue = 0;

    pOsdLayerInfo[0].edgeColor.fAlpha = 255;
    pOsdLayerInfo[0].edgeColor.fRed = 0;
    pOsdLayerInfo[0].edgeColor.fGreen = 0;
    pOsdLayerInfo[0].edgeColor.fBlue = 0;

    pOsdLayerInfo[0].bkgColor.fAlpha = 0;
    pOsdLayerInfo[0].edgePixel = 0;
    pOsdLayerInfo[0].osdInvertEnable = FH_OSD_INVERT_BY_CHAR;
    pOsdLayerInfo[0].osdInvertThreshold.high_level = 180;
    pOsdLayerInfo[0].osdInvertThreshold.low_level = 160;
    pOsdLayerInfo[0].layerFlag = FH_OSD_LAYER_USE_TWO_BUF;
    pOsdLayerInfo[0].layerId = OSD_LAYER_ID;

    ret = FHAdv_Osd_Ex_SetText(0, 0, &osd_cfg);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_Ex_SetText failed with %d\n", ret);
        return ret;
    }

    text_line_cfg[0].textInfo = text_data[0];
    text_line_cfg[1].textInfo = text_data[1];
    text_line_cfg[2].textInfo = text_data[2];
    FH_CHAR slogan_data[] = {
        0xc2, 0xeb, 0xc1, 0xf7, 0xcf, 0xc8, 0xb7, 0xe6, 0xb6, 0xd3, 0,
    };

    sprintf(text_line_cfg[0].textInfo, "camera%d", 0);
    text_line_cfg[0].textEnable = 1;
    text_line_cfg[0].timeOsdEnable = 0;
    text_line_cfg[0].textLineWidth = 1024;
    text_line_cfg[0].linePositionX = 32;
    text_line_cfg[0].linePositionY = 32;
    text_line_cfg[0].lineId = 0;
    text_line_cfg[0].enable = 1;

    ret = FHAdv_Osd_SetTextLine(0, 0, pOsdLayerInfo[0].layerId, &text_line_cfg[0]);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_Ex_SetText failed with %d\n", ret);
        return ret;
    }

    memset(&g_osd_time_line, 0, sizeof(g_osd_time_line));
    g_osd_time_line.textInfo = g_osd_time_text;
    g_osd_time_line.textEnable = 1;
    g_osd_time_line.timeOsdEnable = 0;
    g_osd_time_line.textLineWidth = 1024;
    g_osd_time_line.linePositionX = 32;
    g_osd_time_line.linePositionY = 96;
    g_osd_time_line.lineId = OSD_TIME_LINE_ID;
    g_osd_time_line.enable = 1;

    ret = osd_refresh_time_line();
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_Ex_SetText failed with %d\n", ret);
        return ret;
    }

    ret = osd_start_time_thread();
    if (ret != 0) {
        return ret;
    }

    strcat(text_line_cfg[2].textInfo, slogan_data);
    text_line_cfg[2].textEnable = 1;
    text_line_cfg[2].timeOsdEnable = 0;
    text_line_cfg[2].textLineWidth = 1024;
    text_line_cfg[2].linePositionX = 32;
    text_line_cfg[2].linePositionY = 160;
    text_line_cfg[2].lineId = 2;
    text_line_cfg[2].enable = 1;

    ret = FHAdv_Osd_SetTextLine(0, 0, pOsdLayerInfo[0].layerId, &text_line_cfg[2]);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_Ex_SetText failed with %d\n", ret);
        return ret;
    }

    return 0;
}

void capture_osd_stop(void)
{
    pthread_t osd_time_thread;

    pthread_mutex_lock(&g_osd_time_lock);
    if (!g_osd_time_running) {
        pthread_mutex_unlock(&g_osd_time_lock);
        FHAdv_Osd_UnInit(0);
        return;
    }

    g_osd_time_stop = 1;
    osd_time_thread = g_osd_time_thread;
    pthread_mutex_unlock(&g_osd_time_lock);

    pthread_join(osd_time_thread, NULL);

    pthread_mutex_lock(&g_osd_time_lock);
    g_osd_time_running = 0;
    pthread_mutex_unlock(&g_osd_time_lock);

    FHAdv_Osd_UnInit(0);
}

int capture_apply_default_mask(void)
{
    int ret;
    FHT_OSD_Mask_t mask_cfg;

    memset(&mask_cfg, 0, sizeof(mask_cfg));

    mask_cfg.enable = 1;
    mask_cfg.maskId = 0;
    mask_cfg.rotate = 0;
    mask_cfg.area.fTopLeftX = 32;
    mask_cfg.area.fTopLeftY = 224;
    mask_cfg.area.fWidth = 360;
    mask_cfg.area.fHeigh = 80;
    mask_cfg.osdColor.fAlpha = 255;
    mask_cfg.osdColor.fRed = 0;
    mask_cfg.osdColor.fGreen = 0;
    mask_cfg.osdColor.fBlue = 255;

    ret = FHAdv_Osd_SetMask(0, &mask_cfg);
    if (ret != FH_SUCCESS) {
        printf("FHAdv_Osd_SetMask failed with %x\n", ret);
        return ret;
    }

    return 0;
}
