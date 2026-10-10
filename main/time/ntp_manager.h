#pragma once
#include <time.h>
#include <stdbool.h>
typedef enum { NET_STARTING, NET_SETUP, NET_CONNECTING, NET_NTP, NET_WIFI_FAILED,
               NET_NTP_FAILED, NET_SYNCED } network_phase_t;
typedef struct {
    bool connected, setup;
    time_t last_sync;
    char ap[33], password[17];
    network_phase_t phase;
    unsigned reason;
} network_status_t;
const char *ntp_status_message(network_status_t status);
void ntp_manager_start(void);
network_status_t ntp_status(void);
void ntp_request_setup(void);
