from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/characters/creatures/chicken/chicken-atlas.png"
OUTPUT = SOURCE.parent
NAMES = ("idle", "suspicious", "run", "startled")
CANVAS_SIZE = (420, 360)
SUBJECT_MAX = (390, 330)
BASELINE_Y = 342


def alpha_box(image: Image.Image) -> tuple[int, int, int, int]:
    box = image.getchannel("A").getbbox()
    if box is None:
        raise RuntimeError("Chicken pose has no visible pixels")
    return box


atlas = Image.open(SOURCE).convert("RGBA")
cell_width = atlas.width // 2
cell_height = atlas.height // 2
cells = [
    atlas.crop((0, 0, cell_width, cell_height)),
    atlas.crop((cell_width, 0, atlas.width, cell_height)),
    atlas.crop((0, cell_height, cell_width, atlas.height)),
    atlas.crop((cell_width, cell_height, atlas.width, atlas.height)),
]

trimmed = [cell.crop(alpha_box(cell)) for cell in cells]
scale = min(
    SUBJECT_MAX[0] / max(image.width for image in trimmed),
    SUBJECT_MAX[1] / max(image.height for image in trimmed),
)

for name, image in zip(NAMES, trimmed):
    size = (round(image.width * scale), round(image.height * scale))
    resized = image.resize(size, Image.Resampling.LANCZOS)
    canvas = Image.new("RGBA", CANVAS_SIZE, (0, 0, 0, 0))
    x = (CANVAS_SIZE[0] - resized.width) // 2
    y = BASELINE_Y - resized.height
    canvas.alpha_composite(resized, (x, y))
    canvas.save(OUTPUT / f"{name}.png", optimize=True)

print("Created:", ", ".join(f"{name}.png" for name in NAMES))
