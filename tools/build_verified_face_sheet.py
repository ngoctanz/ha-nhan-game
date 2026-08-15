import csv
import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps


ROOT = Path(__file__).resolve().parents[1]
FACE_ROOT = ROOT / "assets/faces"
SOURCE = FACE_ROOT / "priority_faces.csv"
OUTPUT = FACE_ROOT / "_catalog/verified-faces.png"
FONT_PATH = Path("C:/Windows/Fonts/arial.ttf")

COLS = 5
CELL_W, CELL_H = 310, 245
PADDING = 12
FACE_AREA = (186, 148)


def font(size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(str(FONT_PATH), size)


with SOURCE.open("r", encoding="utf-8-sig", newline="") as source:
    entries = list(csv.DictReader(source))

rows = math.ceil(len(entries) / COLS)
sheet = Image.new("RGB", (COLS * CELL_W, rows * CELL_H), "#20242b")
draw = ImageDraw.Draw(sheet)

name_font = font(17)
legacy_font = font(14)
meaning_font = font(13)

for index, entry in enumerate(entries):
    col, row = index % COLS, index // COLS
    x, y = col * CELL_W, row * CELL_H
    card = (x + 5, y + 5, x + CELL_W - 5, y + CELL_H - 5)
    draw.rounded_rectangle(card, radius=14, fill="#303640", outline="#4b5664", width=2)

    face_path = FACE_ROOT / "expressions" / f"{entry['name']}.png"
    face = Image.open(face_path).convert("RGBA")
    face.thumbnail(FACE_AREA, Image.Resampling.LANCZOS)
    face_x = x + (CELL_W - face.width) // 2
    face_y = y + PADDING + (FACE_AREA[1] - face.height) // 2
    sheet.paste(face, (face_x, face_y), face)

    text_y = y + 164
    name = entry["name"].removeprefix("verified_")
    draw.text((x + PADDING, text_y), name, font=name_font, fill="#f5d78e")
    draw.text((x + PADDING, text_y + 23), f"cũ: {entry['legacy_name']}",
              font=legacy_font, fill="#aeb8c5")
    meaning = entry["meaning_vi"]
    if len(meaning) > 39:
        meaning = meaning[:37].rstrip() + "…"
    draw.text((x + PADDING, text_y + 44), meaning,
              font=meaning_font, fill="#e8edf2")

OUTPUT.parent.mkdir(parents=True, exist_ok=True)
sheet.save(OUTPUT, optimize=True)
print(f"Created {OUTPUT} with {len(entries)} verified faces")
