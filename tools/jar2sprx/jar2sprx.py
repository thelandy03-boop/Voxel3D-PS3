#!/usr/bin/env python3
"""
jar2sprx - Transpilador de Mods de Minecraft Java Edition (.jar) a C++ / PS3 SPRX
Voxel3D-PS3 Project

Este script toma un archivo .jar de Forge (ej. 1.6.4 / 1.7.10), extrae sus recursos
(texturas, modelos, metadatos) y transpila las clases de ítems/bloques a código C++ nativo
listo para ser ejecutado en Minecraft PlayStation 3 Edition.
"""

import argparse
import json
import os
import re
import sys
import zipfile
from pathlib import Path


class Jar2SprxConverter:
    def __init__(self, jar_path: Path, output_dir: Path, base_item_id: int = 5000):
        self.jar_path = Path(jar_path)
        self.output_dir = Path(output_dir)
        self.base_item_id = base_item_id
        self.mod_info = {
            "modid": "unknown_mod",
            "name": "Unknown Mod",
            "version": "1.0.0",
            "description": ""
        }
        self.items = []
        self.blocks = []
        self.textures = []

    def extract_mod_metadata(self, z: zipfile.ZipFile):
        """Intenta leer mcmod.info para extraer metadatos del mod."""
        if "mcmod.info" in z.namelist():
            try:
                raw_info = z.read("mcmod.info").decode("utf-8", errors="ignore")
                # Limpiar posibles caracteres invalidos o trailing commas
                raw_info = re.sub(r',\s*]', ']', raw_info)
                raw_info = re.sub(r',\s*}', '}', raw_info)
                data = json.loads(raw_info)
                if isinstance(data, list) and len(data) > 0:
                    data = data[0]
                if isinstance(data, dict):
                    self.mod_info["modid"] = data.get("modid", self.mod_info["modid"])
                    self.mod_info["name"] = data.get("name", self.mod_info["name"])
                    self.mod_info["version"] = data.get("version", self.mod_info["version"])
                    self.mod_info["description"] = data.get("description", "")
            except Exception as e:
                print(f"[!] Aviso al leer mcmod.info: {e}")
        else:
            # Fallback al nombre del archivo
            stem = self.jar_path.stem
            self.mod_info["modid"] = re.sub(r'[^a-zA-Z0-9_]', '_', stem.lower())
            self.mod_info["name"] = stem.replace("_", " ").replace("-", " ").title()

        print(f"[+] Mod Detectado: {self.mod_info['name']} (ID: {self.mod_info['modid']}) v{self.mod_info['version']}")

    def scan_assets(self, z: zipfile.ZipFile):
        """Escanea texturas de ítems y bloques dentro del JAR."""
        modid = self.mod_info["modid"]
        item_texture_pattern = re.compile(r"assets/([^/]+)/textures/items/(.+)\.png$", re.IGNORECASE)
        block_texture_pattern = re.compile(r"assets/([^/]+)/textures/blocks/(.+)\.png$", re.IGNORECASE)

        for name in z.namelist():
            item_match = item_texture_pattern.match(name)
            if item_match:
                asset_modid, texture_name = item_match.groups()
                self.items.append({
                    "name": texture_name.replace("_", " ").title(),
                    "internal_name": f"{asset_modid}:{texture_name}",
                    "texture_path": name,
                    "type": "item"
                })

            block_match = block_texture_pattern.match(name)
            if block_match:
                asset_modid, texture_name = block_match.groups()
                self.blocks.append({
                    "name": texture_name.replace("_", " ").title(),
                    "internal_name": f"{asset_modid}:{texture_name}",
                    "texture_path": name,
                    "type": "block"
                })

        print(f"[+] Recursos encontrados: {len(self.items)} ítems, {len(self.blocks)} bloques.")

    def generate_cpp_code(self) -> str:
        """Genera el código C++ nativo para registrar estos elementos en el ModLoader de PS3."""
        modid = self.mod_info["modid"]
        safe_modid = re.sub(r'[^a-zA-Z0-9_]', '_', modid)

        lines = [
            f"/* Auto-generado por jar2sprx a partir de {self.jar_path.name} */",
            f"/* Mod: {self.mod_info['name']} v{self.mod_info['version']} */",
            "",
            "#include <stdint.h>",
            "#include <string>",
            "#include <vector>",
            "#include \"forge/ForgeShim.h\"",
            "",
            f"class Mod_{safe_modid} : public ForgeMod {{",
            "public:",
            f"    Mod_{safe_modid}() : ForgeMod(\"{modid}\", \"{self.mod_info['name']}\", \"{self.mod_info['version']}\") {{}}",
            "",
            "    void onPreInit() override {",
            f"        // Inicializacion y registro de {len(self.items)} items",
        ]

        current_id = self.base_item_id
        for it in self.items:
            clean_name = re.sub(r'[^a-zA-Z0-9_]', '', it['name'])
            lines.append(f"        // Item: {it['name']} ({it['internal_name']})")
            lines.append(f"        GameRegistry::registerItem(new Item({current_id}), \"{it['internal_name']}\");")
            current_id += 1

        lines.extend([
            "    }",
            "",
            "    void onInit() override {",
            "        // Eventos post-registro e inicializacion grafica",
            "    }",
            "};",
            "",
            f"// Instancia global exportada para modloader.sprx",
            f"REGISTER_FORGE_MOD(Mod_{safe_modid});",
            ""
        ])

        return "\n".join(lines)

    def generate_voxel3d_config(self) -> dict:
        """Genera la configuracion JSON para mapear modelos 3D con Voxel3D-PS3."""
        items_config = []
        current_id = self.base_item_id

        for it in self.items:
            tex_stem = Path(it["texture_path"]).stem
            model_file = f"{tex_stem}.vx3"
            items_config.append({
                "name": it["name"],
                "item_id": current_id,
                "metadata": 0,
                "model_path": f"/dev_hdd0/game/BLES01976/USRDIR/voxel3d/models/{model_file}",
                "transform": {
                    "scale": [0.0625, 0.0625, 0.0625],
                    "rest": {
                        "position": [0.25, -0.20, -0.45],
                        "rotation": [0.0, 45.0, 0.0]
                    },
                    "ads": {
                        "position": [0.0, -0.15, -0.35],
                        "rotation": [0.0, 0.0, 0.0],
                        "fov_multiplier": 0.75
                    }
                }
            })
            current_id += 1

        return {
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "version": 1,
            "mod_id": self.mod_info["modid"],
            "name": self.mod_info["name"],
            "items": items_config
        }

    def process(self):
        if not self.jar_path.exists():
            print(f"[!] Error: El archivo JAR '{self.jar_path}' no existe.")
            return False

        self.output_dir.mkdir(parents=True, exist_ok=True)
        assets_out = self.output_dir / "assets"
        assets_out.mkdir(parents=True, exist_ok=True)

        print(f"[+] Leyendo archivo JAR: {self.jar_path}")
        with zipfile.ZipFile(self.jar_path, "r") as z:
            self.extract_mod_metadata(z)
            self.scan_assets(z)

            # Extraer texturas encontradas
            for it in self.items:
                tex_data = z.read(it["texture_path"])
                dest_file = assets_out / Path(it["texture_path"]).name
                dest_file.write_bytes(tex_data)

            for bk in self.blocks:
                tex_data = z.read(bk["texture_path"])
                dest_file = assets_out / Path(bk["texture_path"]).name
                dest_file.write_bytes(tex_data)

        # Generar codigo C++
        cpp_code = self.generate_cpp_code()
        cpp_file = self.output_dir / f"{self.mod_info['modid']}_mod.cpp"
        cpp_file.write_text(cpp_code, encoding="utf-8")
        print(f"[✓] Codigo C++ transpilado: {cpp_file}")

        # Generar JSON de configuracion
        config_data = self.generate_voxel3d_config()
        config_file = self.output_dir / "config.json"
        config_file.write_text(json.dumps(config_data, indent=2), encoding="utf-8")
        print(f"[✓] Configuracion Voxel3D generada: {config_file}")

        print(f"\n[★] Transpilacion exitosa! Salida en: {self.output_dir}")
        return True


def main():
    parser = argparse.ArgumentParser(description="jar2sprx - Transpilador de Mods de Minecraft Java a C++ / PS3 SPRX")
    parser.add_argument("--jar", required=True, type=Path, help="Ruta al archivo .jar del mod")
    parser.add_argument("--out", type=Path, default=Path("output_mod"), help="Directorio de salida para los archivos generados")
    parser.add_argument("--base-id", type=int, default=5000, help="ID base de inicio para nuevos items (por defecto: 5000)")
    args = parser.parse_args()

    converter = Jar2SprxConverter(args.jar, args.out, args.base_id)
    converter.process()


if __name__ == "__main__":
    main()
