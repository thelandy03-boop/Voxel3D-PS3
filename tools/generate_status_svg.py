#!/usr/bin/env python3
"""
Voxel3D-PS3 Status SVG & Metrics Generator
Dynamically parses docs/ROADMAP.md checkboxes to generate docs/status.svg.

How to update:
Any developer or AI simply edits docs/ROADMAP.md:
  * [x] -> Completed / Done (green)
  * [~] -> Started / In progress (yellow)
  * [ ] -> Pending / To do (dark grey)
Then run:
  python3 tools/generate_status_svg.py
"""

import math
import re
from pathlib import Path

ROADMAP_PATH = Path("docs/ROADMAP.md")
SVG_PATH = Path("docs/status.svg")
README_PATH = Path("README.md")

WIDTH = 1060.0
HEIGHT = 520.0
MARGIN = 16.0
CHART_TOP = 84.0

DONE_COLOR = "#2ea043"
PARTIAL_COLOR = "#b08a1e"
TODO_COLOR = "#3a3d44"
BG_COLOR = "#161b22"
BORDER_COLOR = "#10141a"
GRID_STROKE = "#1f7a33"


class Rect:
    def __init__(self, x: float, y: float, w: float, h: float):
        self.x = x
        self.y = y
        self.w = w
        self.h = h


class Group:
    def __init__(self, name: str, items: list[str]):
        self.name = name
        self.items = items

    def count(self, state: str) -> int:
        return self.items.count(state)


def total_items(groups: list[Group]) -> int:
    return sum(len(g.items) for g in groups)


def counted_items(groups: list[Group], state: str) -> int:
    return sum(g.count(state) for g in groups)


def percent(groups: list[Group]) -> float:
    tot = total_items(groups)
    if tot == 0:
        return 0.0
    return 100.0 * (counted_items(groups, "done") + 0.5 * counted_items(groups, "partial")) / tot


def split_area(area: Rect, sizes: list[tuple[int, int]], out: list[tuple[int, Rect]]):
    if not sizes:
        return
    if len(sizes) == 1:
        out.append((sizes[0][0], area))
        return

    all_sum = sum(s[1] for s in sizes)
    left = 0
    cut = 1
    for i, s in enumerate(sizes[:-1]):
        left += s[1]
        cut = i + 1
        if left * 2 >= all_sum:
            break

    share = left / max(1, all_sum)
    if area.w >= area.h:
        w = area.w * share
        a = Rect(area.x, area.y, w, area.h)
        b = Rect(area.x + w, area.y, area.w - w, area.h)
    else:
        h = area.h * share
        a = Rect(area.x, area.y, area.w, h)
        b = Rect(area.x, area.y + h, area.w, area.h - h)

    split_area(a, sizes[:cut], out)
    split_area(b, sizes[cut:], out)


def escape_xml(s: str) -> str:
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def fitted_name(name: str, width: float) -> str:
    room = max(1, int(width / 6.8))
    if len(name) <= room:
        return name
    return name[: max(1, room - 1)].rstrip() + "…"


def draw_group(svg: list[str], area: Rect, group: Group):
    order = {"done": 0, "partial": 1, "todo": 2}
    sorted_items = sorted(group.items, key=lambda s: order.get(s, 2))
    n = len(sorted_items)
    if n == 0:
        return

    cols = max(1, min(n, math.ceil(math.sqrt(n * area.w / area.h))))
    rows = math.ceil(n / cols)
    h = area.h / rows

    for r in range(rows):
        cells = sorted_items[r * cols : min(n, (r + 1) * cols)]
        if not cells:
            continue
        w = area.w / len(cells)
        for c, state in enumerate(cells):
            fill = DONE_COLOR if state == "done" else (PARTIAL_COLOR if state == "partial" else TODO_COLOR)
            cx = area.x + c * w
            cy = area.y + r * h
            svg.append(
                f'<rect x="{cx:.1f}" y="{cy:.1f}" width="{w:.1f}" height="{h:.1f}" '
                f'fill="{fill}" stroke="{GRID_STROKE}" stroke-opacity=".55" stroke-width=".6"/>'
            )

    svg.append(
        f'<rect x="{area.x:.1f}" y="{area.y:.1f}" width="{area.w:.1f}" height="{area.h:.1f}" '
        f'fill="none" stroke="{BORDER_COLOR}" stroke-width="2"/>'
    )
    label = escape_xml(fitted_name(group.name, area.w - 10.0))
    svg.append(
        f'<text x="{area.x + 5.0:.1f}" y="{area.y + 14.0:.1f}" font-size="12" font-weight="600" '
        f'fill="#fff" stroke="#000" stroke-width="2.5" paint-order="stroke">{label}</text>'
    )


def draw_chart(svg: list[str], area: Rect, title: str, groups: list[Group]):
    done = counted_items(groups, "done")
    tot = total_items(groups)
    pct = percent(groups)
    line = f"{title}: {pct:.1f}% ({done}/{tot})"

    svg.append(
        f'<text x="{area.x:.1f}" y="{area.y - 10.0:.1f}" font-size="14" font-weight="500" '
        f'fill="#e6edf3">{escape_xml(line)}</text>'
    )

    sizes = [(i, len(g.items)) for i, g in enumerate(groups) if len(g.items) > 0]
    sizes.sort(key=lambda s: s[1], reverse=True)

    places = []
    split_area(area, sizes, places)
    for i, place in places:
        draw_group(svg, place, groups[i])


def draw_badge(svg: list[str], x: float, label: str, val: float) -> float:
    lw = len(label) * 7.2 + 18.0
    vw = 66.0
    color = DONE_COLOR if val >= 75.0 else (PARTIAL_COLOR if val >= 30.0 else "#c0392b")

    svg.append(f'<rect x="{x:.1f}" y="16" width="{lw:.1f}" height="24" fill="#555b63"/>')
    svg.append(f'<rect x="{x + lw:.1f}" y="16" width="{vw:.1f}" height="24" fill="{color}"/>')
    svg.append(
        f'<text x="{x + 9.0:.1f}" y="32.5" font-size="12" font-weight="600" fill="#fff">{escape_xml(label)}</text>'
    )
    svg.append(
        f'<text x="{x + lw + 9.0:.1f}" y="32.5" font-size="12" font-weight="600" fill="#fff">{val:.1f}%</text>'
    )
    return x + lw + vw + 14.0


def parse_roadmap() -> tuple[list[Group], list[Group]]:
    """
    Parses docs/ROADMAP.md directly.
    Returns (subsystem_groups, phase_groups) based on real checkboxes.
    """
    if not ROADMAP_PATH.exists():
        raise FileNotFoundError(f"Cannot find {ROADMAP_PATH}")

    content = ROADMAP_PATH.read_text(encoding="utf-8")
    lines = content.splitlines()

    subsystem_groups: list[Group] = []
    phase_groups: list[Group] = []

    current_section = None  # 1 for Subsystems, 2 for Phases
    current_group_name = None
    current_items: list[str] = []

    def flush_group():
        nonlocal current_group_name, current_items
        if current_group_name and current_items:
            g = Group(current_group_name, list(current_items))
            if current_section == 1:
                subsystem_groups.append(g)
            elif current_section == 2:
                phase_groups.append(g)
            current_items = []
            current_group_name = None

    for line in lines:
        stripped = line.strip()

        # Major section detection
        if stripped.startswith("## 1. Módulos"):
            flush_group()
            current_section = 1
            continue
        elif stripped.startswith("## 2. Hitos"):
            flush_group()
            current_section = 2
            continue
        elif stripped.startswith("## "):
            flush_group()
            current_section = None
            continue

        # Subgroup detection
        if stripped.startswith("### ") and current_section in (1, 2):
            flush_group()
            title = stripped[4:].strip()
            # Clean title
            title = re.sub(r"\(.*?\)", "", title).strip()
            title = title.split(":")[0].strip()
            # Simplify name for SVG label
            title = title.replace("Formato de Malla ", "")
            title = title.replace("Plugin ", "")
            title = title.replace("Pipeline Gráfico ", "")
            title = title.replace("Pipeline Offline de ", "")
            title = title.replace("Scaffolding del Plugin ", "")
            title = title.replace("Sistema Data-Driven y ", "")
            current_group_name = title.strip()
            continue

        # Check for task checkboxes
        if current_group_name and current_section:
            if stripped.startswith(("* [x]", "- [x]")):
                current_items.append("done")
            elif stripped.startswith(("* [~]", "- [~]")):
                current_items.append("partial")
            elif stripped.startswith(("* [ ]", "- [ ]")):
                current_items.append("todo")

    flush_group()

    return subsystem_groups, phase_groups


def update_readme_milestones(phase_groups: list[Group]):
    """Keeps the README.md table and general progress in sync with real numbers."""
    if not README_PATH.exists():
        return

    content = README_PATH.read_text(encoding="utf-8")

    # Calculate overall roadmap percentage
    overall_pct = percent(phase_groups)

    # Update badge URL in README.md
    content = re.sub(
        r"https://geps\.dev/progress/\d+",
        f"https://geps.dev/progress/{int(round(overall_pct))}",
        content,
    )

    # Format each phase row
    # Example: | **Fase 0** | Reversing de `EBOOT.BIN` ... | `■■■■■■■■□□` | **80%** | 🟡 En curso |
    phase_meta = {
        "Fase 0": ("Reversing de `EBOOT.BIN` & Mapeo de Offsets", phase_groups[0] if len(phase_groups) > 0 else None),
        "Fase 1": ("Pipeline Offline (`.vox` / `.bbmodel` ➔ `.vx3`)", phase_groups[1] if len(phase_groups) > 1 else None),
        "Fase 2": ("Scaffolding SPRX & Trampolines PPU (PPC ABI)", phase_groups[2] if len(phase_groups) > 2 else None),
        "Fase 3": ("Renderizado RSX de Bajo Nivel (`libgcm` Draw Calls)", phase_groups[3] if len(phase_groups) > 3 else None),
        "Fase 4": ("Cinemática ADS & Retroceso Amortiguado (LERP)", phase_groups[4] if len(phase_groups) > 4 else None),
        "Fase 5": ("Sistema Data-Driven (`config.json`) & Release v1.0", phase_groups[5] if len(phase_groups) > 5 else None),
    }

    new_rows = []
    for phase_key, (desc, group) in phase_meta.items():
        if group:
            p = percent([group])
            # 10 blocks
            blocks_filled = int(round(p / 10.0))
            blocks = "■" * blocks_filled + "□" * (10 - blocks_filled)
            if p >= 100.0:
                status_pill = "🟢 Completado"
            elif p > 0.0:
                status_pill = "🟡 En curso"
            else:
                status_pill = "⚪ Pendiente"
            new_rows.append(f"| **{phase_key}** | {desc} | `{blocks}` | **{p:.1f}%** | {status_pill} |")

    table_block = "\n".join(new_rows)
    pattern = r"(\| \*\*Fase 0\*\* \|[\s\S]*?\| \*\*Fase 5\*\* \|[^\n]*\n)"
    content = re.sub(pattern, table_block + "\n", content)

    README_PATH.write_text(content, encoding="utf-8")
    print(f"Updated {README_PATH} milestone table with real metrics ({overall_pct:.1f}% total).")


def generate_status():
    spec_groups, phase_groups = parse_roadmap()

    spec_pct = percent(spec_groups)
    roadmap_pct = percent(phase_groups)

    print(f"Parsed REAL metrics from {ROADMAP_PATH}:")
    print(f" - Specifications: {spec_pct:.1f}% ({counted_items(spec_groups, 'done')}/{total_items(spec_groups)})")
    print(f" - Roadmap phases: {roadmap_pct:.1f}% ({counted_items(phase_groups, 'done')}/{total_items(phase_groups)})")

    svg_parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {WIDTH} {HEIGHT}" width="{WIDTH}" height="{HEIGHT}" '
        f'font-family="-apple-system,BlinkMacSystemFont,Segoe UI,Helvetica,Arial,sans-serif">',
        f'<rect width="{WIDTH}" height="{HEIGHT}" fill="{BG_COLOR}"/>',
    ]

    # Header Badges
    x = MARGIN
    x = draw_badge(svg_parts, x, "subsystems", spec_pct)
    draw_badge(svg_parts, x, "roadmap", roadmap_pct)

    # Two Charts Layout (Side by side)
    chart_w = (WIDTH - 3.0 * MARGIN) / 2.0
    chart_h = HEIGHT - CHART_TOP - 42.0

    chart1_area = Rect(MARGIN, CHART_TOP, chart_w, chart_h)
    chart2_area = Rect(MARGIN * 2.0 + chart_w, CHART_TOP, chart_w, chart_h)

    draw_chart(svg_parts, chart1_area, "System Architecture", spec_groups)
    draw_chart(svg_parts, chart2_area, "Implementation Roadmap", phase_groups)

    # Legend at bottom
    leg_y = HEIGHT - 20.0
    leg_x = MARGIN
    for col, name in [(DONE_COLOR, "done"), (PARTIAL_COLOR, "started"), (TODO_COLOR, "to do")]:
        svg_parts.append(f'<rect x="{leg_x}" y="{leg_y - 10}" width="12" height="12" fill="{col}"/>')
        svg_parts.append(
            f'<text x="{leg_x + 18}" y="{leg_y}" font-size="12" fill="#8b949e">{name}</text>'
        )
        leg_x += 80.0

    svg_parts.append("</svg>\n")

    content = "\n".join(svg_parts)
    SVG_PATH.write_text(content, encoding="utf-8")
    print(f"Generated {SVG_PATH} successfully!")

    update_readme_milestones(phase_groups)


if __name__ == "__main__":
    generate_status()
