from PIL import Image 
from glob import glob

root = "../data/earth_textures/"
pattern = "3_*"
destination_folder = "../data/earth_textures/"
size = 256

images = glob(root + pattern)


for image in images:
    im = Image.open(image)
    im = im.resize((size, size), Image.ANTIALIAS)
    im.save(destination_folder + "thumb_" + image.split("/")[-1:][0], "PNG")

