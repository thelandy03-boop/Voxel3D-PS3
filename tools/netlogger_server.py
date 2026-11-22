#!/usr/bin/env python3
"""
Voxel3D-PS3: Servidor de Telemetría NetLogger (UDP)
Recibe y formatea en tiempo real los logs emitidos por la consola PlayStation 3
o RPCS3 sin impactar el framerate del juego.
"""

import argparse
import datetime
import socket
import sys

# ANSI Colors
RESET = "\033[0m"
BOLD = "\033[1m"
RED = "\033[91m"
GREEN = "\033[92m"
YELLOW = "\033[93m"
BLUE = "\033[94m"
MAGENTA = "\033[95m"
CYAN = "\033[96m"
GRAY = "\033[90m"


def colorize_log(line: str) -> str:
    """Añade colores según el nivel de log en la línea."""
    if "[ERROR]" in line:
        return f"{RED}{line}{RESET}"
    elif "[WARN]" in line:
        return f"{YELLOW}{line}{RESET}"
    elif "[INFO]" in line:
        return f"{GREEN}{line}{RESET}"
    elif "[DEBUG]" in line:
        return f"{CYAN}{line}{RESET}"
    return line


def main():
    parser = argparse.ArgumentParser(
        description="Voxel3D-PS3 UDP Telemetry Listener",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("--ip", default="0.0.0.0", help="IP interface to bind")
    parser.add_argument("--port", "-p", type=int, default=3000, help="UDP listening port")
    parser.add_argument("--save-log", "-s", type=str, default=None, help="Save log stream to file")

    args = parser.parse_args()

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.bind((args.ip, args.port))
    except Exception as e:
        print(f"{RED}[!] Error al enlazar socket UDP en {args.ip}:{args.port} -> {e}{RESET}")
        sys.exit(1)

    print("=" * 60)
    print(f"{BOLD}{CYAN}  Voxel3D-PS3 Telemetry Server (NetLogger){RESET}")
    print(f"  Escuchando en: {BOLD}{args.ip}:{args.port}{RESET} (UDP)")
    if args.save_log:
        print(f"  Guardando logs en: {BOLD}{args.save_log}{RESET}")
    print("  Presiona Ctrl+C para detener.")
    print("=" * 60 + "\n")

    log_file = None
    if args.save_log:
        log_file = open(args.save_log, "a", encoding="utf-8")

    try:
        while True:
            data, addr = sock.recvfrom(2048)
            now = datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]
            text = data.decode("utf-8", errors="replace").rstrip()

            # Consola en vivo
            client_tag = f"{GRAY}[{now} | {addr[0]}:{addr[1]}]{RESET}"
            colored_text = colorize_log(text)
            print(f"{client_tag} {colored_text}", flush=True)

            if log_file:
                log_file.write(f"[{now} | {addr[0]}:{addr[1]}] {text}\n")
                log_file.flush()

    except KeyboardInterrupt:
        print(f"\n{YELLOW}[*] Servidor NetLogger detenido por el usuario.{RESET}", flush=True)
    finally:
        sock.close()
        if log_file:
            log_file.close()


if __name__ == "__main__":
    main()
