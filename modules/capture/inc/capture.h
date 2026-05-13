#ifndef VIDEO_CAPTURE_H
#define VIDEO_CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif

int capture_legacy_start(char *dst_ip, unsigned int port);
void capture_legacy_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* VIDEO_CAPTURE_H */
