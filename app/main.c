#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "capture.h"

static int g_sig_stop = 0;

static void handle_sig(int signo)
{
    (void)signo;
    g_sig_stop = 1;
}

int main(int argc, char *argv[])
{
    char *dst_ip = argc > 1 ? argv[1] : NULL;
    unsigned int port = argc > 2 ? strtol(argv[2], NULL, 0) : 1234;

    signal(SIGINT, handle_sig);
    signal(SIGQUIT, handle_sig);
    signal(SIGKILL, handle_sig);
    signal(SIGTERM, handle_sig);

    int ret = capture_legacy_start(dst_ip, port);
    if (ret != 0) {
        return ret;
    }

    while (!g_sig_stop) {
        usleep(20000);
    }

    capture_legacy_stop();
    return 0;
}
