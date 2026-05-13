#include <fcntl.h>
#include <linux/rtc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include "rtc_time.h"

int rtc_time_read(struct tm *tm_now)
{
    int fd = open("/dev/rtc", O_RDONLY);
    if (fd < 0) {
        perror("open error");
        return -1;
    }

    struct rtc_time rtc_now;
    int ret = ioctl(fd, RTC_RD_TIME, &rtc_now);
    close(fd);
    if (ret == 0 && rtc_now.tm_year >= 100) {
        memset(tm_now, 0, sizeof(*tm_now));
        tm_now->tm_year = rtc_now.tm_year;
        tm_now->tm_mon = rtc_now.tm_mon;
        tm_now->tm_mday = rtc_now.tm_mday;
        tm_now->tm_hour = rtc_now.tm_hour;
        tm_now->tm_min = rtc_now.tm_min;
        tm_now->tm_sec = rtc_now.tm_sec;
        return 0;
    }

    return -1;
}
