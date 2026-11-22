/**
 * Voxel3D-PS3: Cliente de Telemetría UDP en Tiempo Real (NetLogger)
 * Emite paquetes UDP hacia la estación de trabajo (PC) sin bloquear el hilo principal.
 */

#include "netlogger.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static int s_log_socket = -1;
static struct sockaddr_in s_host_addr;
static int s_initialized = 0;

void netlog_init(const char* host_ip, uint16_t port) {
    if (s_log_socket >= 0) {
        netlog_close();
    }

    s_log_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_log_socket < 0) {
        return;
    }

    memset(&s_host_addr, 0, sizeof(s_host_addr));
    s_host_addr.sin_family = AF_INET;
    s_host_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host_ip, &s_host_addr.sin_addr) <= 0) {
        close(s_log_socket);
        s_log_socket = -1;
        return;
    }

    s_initialized = 1;
    netlog_info("Voxel3D-PS3 NetLogger conectado a %s:%u", host_ip, port);
}

void netlog_close(void) {
    if (s_log_socket >= 0) {
        close(s_log_socket);
        s_log_socket = -1;
    }
    s_initialized = 0;
}

void netlog_printf(const char* fmt, ...) {
    if (s_log_socket < 0 || !s_initialized) {
        return;
    }

    char buffer[512];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len > 0) {
        // Enviar por UDP sin bloqueo
        sendto(s_log_socket, buffer, (size_t)len, 0,
               (struct sockaddr*)&s_host_addr, sizeof(s_host_addr));
    }
}

static void log_with_prefix(const char* prefix, const char* fmt, va_list args) {
    if (s_log_socket < 0 || !s_initialized) {
        return;
    }

    char message[400];
    vsnprintf(message, sizeof(message), fmt, args);

    char final_buf[512];
    int len = snprintf(final_buf, sizeof(final_buf), "[Voxel3D] %s %s\n", prefix, message);

    if (len > 0) {
        sendto(s_log_socket, final_buf, (size_t)len, 0,
               (struct sockaddr*)&s_host_addr, sizeof(s_host_addr));
    }
}

void netlog_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_prefix("[INFO]", fmt, args);
    va_end(args);
}

void netlog_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_prefix("[WARN]", fmt, args);
    va_end(args);
}

void netlog_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_prefix("[ERROR]", fmt, args);
    va_end(args);
}

void netlog_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_with_prefix("[DEBUG]", fmt, args);
    va_end(args);
}
