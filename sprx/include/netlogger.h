#ifndef NETLOGGER_H
#define NETLOGGER_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Niveles de severidad para telemetría
typedef enum {
    NETLOG_LEVEL_INFO  = 0,
    NETLOG_LEVEL_WARN  = 1,
    NETLOG_LEVEL_ERROR = 2,
    NETLOG_LEVEL_DEBUG = 3
} NetLogLevel;

/**
 * Inicializa el socket UDP de telemetría apuntando a la IP y puerto de la máquina host (PC).
 */
void netlog_init(const char* host_ip, uint16_t port);

/**
 * Cierra el socket UDP y libera recursos de red.
 */
void netlog_close(void);

/**
 * Envío de formato crudo por UDP.
 */
void netlog_printf(const char* fmt, ...);

/**
 * Funciones de conveniencia con prefijo de nivel y marca temporal.
 */
void netlog_info(const char* fmt, ...);
void netlog_warn(const char* fmt, ...);
void netlog_error(const char* fmt, ...);
void netlog_debug(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif // NETLOGGER_H
