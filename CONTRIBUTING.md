# Guía de Contribución para Voxel3D-PS3

Agradecemos el interés en colaborar con **Voxel3D-PS3**. Este proyecto es una iniciativa de desarrollo de bajo nivel para PlayStation 3 con estándares estrictos de rigor técnico, estabilidad de hardware y honestidad en el seguimiento del avance.

---

## 1. Principio Fundamental: Cero Métricas Falsas

En este repositorio mantenemos una política estricta de transparencia técnica:

* **No marcar tareas no probadas:** Queda prohibido cambiar casillas a `* [x]` (completado) en [`docs/ROADMAP.md`](docs/ROADMAP.md) a menos que la característica cuente con código fuente funcional, compilable y verificado en emulador RPCS3 o consola física.
* **Separación de diseño e implementación:** La redacción de un documento técnico o estructura de cabecera no equivale a la implementación completa de la fase.
* **Actualización auditable:** Cualquier cambio en el porcentaje del proyecto debe estar respaldado por commits que contengan el código ejecutable correspondiente.

---

## 2. Estándares de Código C++ (Cell PPU)

El plugin `.sprx` se ejecuta en la unidad PPU del procesador Cell Broadband Engine. Todo código enviado debe apegarse a las restricciones del compilador `ppu-lv2-gcc`:

* **Dialecto:** C++03 / C++11 compatible con GCC 7.2.0 (target `ppu-lv2`).
* **Flags estrictos:**
  * `-fno-rtti`: No utilizar polimorfismo con RTTI en caliente (`typeid`, `dynamic_cast`).
  * `-fno-exceptions`: Manejo de errores mediante códigos de retorno o estructuras de estado; nunca usar bloques `try/catch`.
  * `-fno-strict-aliasing`: Respetar el aliasing de memoria en conversiones de punteros de hardware.
* **Gestión de Memoria en Runtime:**
  * **Prohibido:** Ejecutar `malloc`, `free`, `new` o `delete` dentro del bucle de renderizado por frame (`renderItemInFirstPerson`).
  * **Obligatorio:** Toda alocación en VRAM o memoria XDR debe ser estática y ejecutarse durante la inicialización del módulo (`voxel3d_prx_start`).

---

## 3. Flujo de Trabajo con Git

Para mantener un historial limpio y estructurado:

* **Convención de Commits (Conventional Commits):**
  * `feat:` Nueva característica funcional o módulo de código.
  * `fix:` Corrección de fallos o desalineaciones de memoria.
  * `docs:` Modificaciones o adiciones en documentación técnica.
  * `refactor:` Reorganización de código sin alterar comportamiento.
  * `tool:` Cambios en herramientas offline del pipeline.
* **Actualización del Gráfico de Estado:**
  Si un commit altera tareas en [`docs/ROADMAP.md`](docs/ROADMAP.md), se debe ejecutar el generador antes de enviar el commit:
  ```bash
  python3 tools/generate_status_svg.py
  ```

---

## 4. Proceso de Envío de Pull Requests

1. Crea una rama descriptiva a partir de `main` (`git checkout -b feature/nombre-modulo`).
2. Implementa los cambios asegurando que no se rompan las reglas de compilación del `Makefile`.
3. Valida la ejecución en RPCS3 verificando que la pila no sufra desalineaciones ni excepciones en el log.
4. Envía tu Pull Request describiendo detalladamente las pruebas realizadas y los resultados obtenidos.
