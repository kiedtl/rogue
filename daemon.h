#ifndef DAEMON_H
#define DAEMON_H

struct delayed_action {
    int d_type;
    int (*d_func)();
    int d_arg;
    int d_time;
};

#endif
