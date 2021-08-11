#include <cstdio>
#include <sysexits.h>

#include "motion.h"
using namespace modules::motion;

int main(int argc, const char *argv[]) {
    const char *output = argv[1];
    if (!argv[1]) {
        fprintf(stderr, "Usage: %s <output>\n", argv[0]);
        return EX_USAGE;
    }

    FILE *fd = fopen(output, "w");
    if (!fd) {
        fprintf(stderr, "%s: can't open output file %s: ", argv[0], output);
        perror(NULL);
        return EX_OSERR;
    }

    // common settings
    const int idlerSteps = 100;
    const int selectorSteps = 80;
    const int maxFeedRate = 1000;

    for (int accel = 2000; accel <= 50000; accel *= 2) {
        // first axis using nominal values
        motion.SetPosition(Idler, 0);
        motion.SetAcceleration(Idler, accel);
        motion.PlanMoveTo(Idler, idlerSteps, maxFeedRate);

        fprintf(fd, "[{\"steps\": %d, \"accel\": %d, \"maxrate\": %d}, ",
            idlerSteps, accel, maxFeedRate);

        // second axis finishes slightly sooner at triple acceleration to maximize the
        // aliasing effects
        int accel_3 = accel * 3;
        motion.SetPosition(Selector, 0);
        motion.SetAcceleration(Selector, accel_3);
        motion.PlanMoveTo(Selector, selectorSteps, maxFeedRate);

        fprintf(fd, "{\"steps\": %d, \"accel\": %d, \"maxrate\": %d}]\n",
            selectorSteps, accel_3, maxFeedRate);

        // step and output time, interval and positions
        unsigned long ts = 0;
        st_timer_t next;
        do {
            next = motion.Step();
            pos_t pos_idler = motion.CurPosition(Idler);
            pos_t pos_selector = motion.CurPosition(Selector);

            fprintf(fd, "%lu %u %d %d\n", ts, next, pos_idler, pos_selector);

            ts += next;
        } while (next);
        fprintf(fd, "\n\n");
    }

    return EX_OK;
}
