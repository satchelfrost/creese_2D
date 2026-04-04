#include <time.h>
#include <stdio.h>

#ifndef NANOS_PER_SEC
    #define NANOS_PER_SEC (1000*1000*1000)
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

static struct {
    double curr;
    double prev;
    double update;
    double draw;
    double frame;
    double target;
    size_t frame_count;
} time_keep = {0};

void begin_timer()
{
    time_keep.curr   = get_time();
    time_keep.update = time_keep.curr - time_keep.prev;
    time_keep.prev   = time_keep.curr;
}

void end_timer()
{
    time_keep.curr = get_time();
    time_keep.draw = time_keep.curr - time_keep.prev;
    time_keep.prev = time_keep.curr;
    time_keep.frame = time_keep.update + time_keep.draw;

    if (time_keep.frame < time_keep.target) {
        wait_time(time_keep.target - time_keep.frame);

        time_keep.curr = get_time();
        double wait = time_keep.curr - time_keep.prev;
        time_keep.prev = time_keep.curr;
        time_keep.frame += wait;
    }

    time_keep.frame_count++;
}

double get_frame_time()
{
    return time_keep.frame;
}

double get_time()
{
#if defined(__linux__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / (double)NANOS_PER_SEC;
#else
    uint64_t value;
    uint64_t frequency;
    QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
    QueryPerformanceCounter((LARGE_INTEGER*)&value);
    return (double)value / frequency;
#endif
}

int get_fps()
{
    double frame_time = get_frame_time();
    if (frame_time == 0) return 0;
    else return (int)roundf(1.0f / frame_time);
}

void log_fps()
{
    static int fps = -1;
    int curr_fps = get_fps();
    if (curr_fps != fps) {
        printf("FPS: %d (%fms)\n", curr_fps, get_frame_time() * 1000.0f);
        fps = curr_fps;
    }
}

void wait_time(double seconds)
{
    if (seconds <= 0) return;

    /* prepare for partial busy wait loop */
    double destination_time = get_time() + seconds;
    double sleep_secs = seconds - seconds * 0.05;

    /* for now wait time only supports linux */
#if defined(__linux__)
    struct timespec req = {0};
    time_t sec = sleep_secs;
    long nsec = (sleep_secs - sec) * 1000000000L;
    req.tv_sec = sec;
    req.tv_nsec = nsec;

    while (nanosleep(&req, &req) == -1) continue;
#endif

#if defined(_WIN32)
    Sleep((unsigned long)(sleep_secs * 1000.0));
#endif

    /* partial busy wait loop */
    while (get_time() < destination_time) {}
}
