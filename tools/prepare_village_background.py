from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/environment/bamboo_village_meme_source.png"
OUTPUT = ROOT / "assets/environment/bamboo_village_meme.png"
TARGET_SIZE = (2400, 720)

# Source is 1983x793. This crop removes excess sky and the foreground grass,
# placing the main street near the game's GroundY=622 without aspect distortion.
CROP = (0, 135, 1983, 730)


image = Image.open(SOURCE).convert("RGB")
if image.size != (1983, 793):
    raise RuntimeError(f"Unexpected source size: {image.size}")

prepared = image.crop(CROP).resize(TARGET_SIZE, Image.Resampling.LANCZOS)
prepared.save(OUTPUT, optimize=True)
print(f"Created {OUTPUT} at {prepared.size}")
