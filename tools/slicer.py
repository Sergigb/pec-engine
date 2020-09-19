from PIL import Image 
import numpy as np

path = "../data/earth_textures/2k/"
top_level_images = ["0_0_0.png", "1_0_0.png", "2_0_0.png", "3_0_0.png", "4_0_0.png", "5_0_0.png"]

destination_folder = "../data/earth_textures/"

level = 2

for top_level_image in top_level_images:
    side = int(top_level_image.split("_")[0])
    n = (2**level / 2)

    image = np.array(Image.open(path + top_level_image))
    size = image.shape[0]
    subdivision_size = size / n

    for i in range(n):
        for j in range(n):
            patch = image[(subdivision_size / n) * i : (subdivision_size / n) * (i + 1),
                          (subdivision_size / n) * j : (subdivision_size / n) * (j + 1), :]
            filename = str(level) + "_" + str(side) + "_" + str(i) + "_" + str(j) + ".png"

            patch = Image.fromarray(patch)
            patch.save(destination_folder + filename)
