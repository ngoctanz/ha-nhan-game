from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "assets/environment/bamboo_village.png"
OUTPUT = ROOT / "assets/environment/bamboo_village_runtime.png"
SOURCE_SIZE = (2168, 725)
TARGET_SIZE = (2400, 720)
SIDE_EXTENSION = (TARGET_SIZE[0] - SOURCE_SIZE[0]) // 2


image = Image.open(SOURCE).convert("RGB")
if image.size != SOURCE_SIZE:
    raise RuntimeError(f"Unexpected source size: {image.size}; expected {SOURCE_SIZE}")

# Remove only the five vertically excessive pixels. The playable scene itself
# remains at native scale; narrow reflected edge strips complete WorldWidth
# without distorting houses, road, or character placement.
center = image.crop((0, 2, SOURCE_SIZE[0], 722))
left = center.crop((0, 0, SIDE_EXTENSION, TARGET_SIZE[1])).transpose(
    Image.Transpose.FLIP_LEFT_RIGHT
)
right = center.crop(
    (SOURCE_SIZE[0] - SIDE_EXTENSION, 0, SOURCE_SIZE[0], TARGET_SIZE[1])
).transpose(Image.Transpose.FLIP_LEFT_RIGHT)

prepared = Image.new("RGB", TARGET_SIZE)
prepared.paste(left, (0, 0))
prepared.paste(center, (SIDE_EXTENSION, 0))
prepared.paste(right, (SIDE_EXTENSION + SOURCE_SIZE[0], 0))
prepared.save(OUTPUT, optimize=True)
print(f"Created {OUTPUT} at {prepared.size} without stretching the source")
