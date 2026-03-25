from PIL import Image

img = Image.open("frame.ppm")

img = img.transpose(Image.FLIP_TOP_BOTTOM)

img.save("frame.png")

print("PNG gerado com sucesso!")