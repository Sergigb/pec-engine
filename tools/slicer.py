from PIL import Image 
import numpy as np

path = "../data/earth_textures/2k/"
top_level_images = ["0_0_0.png", "1_0_0.png", "2_0_0.png", "3_0_0.png", "4_0_0.png", "5_0_0.png"]

destination_folder = "../data/earth_textures/"

level = 3

for top_level_image in top_level_images:
    side = int(top_level_image.split("_")[0])
    n = (2**level / 2)

    image = np.array(Image.open(path + top_level_image))
    size = image.shape[0]
    subdivision_size = size / n
    print(subdivision_size)

    for i in range(n):
        for j in range(n):
            patch = image[(subdivision_size) * i : (subdivision_size) * (i + 1),
                          (subdivision_size) * j : (subdivision_size) * (j + 1), :]
            filename = str(level) + "_" + str(side) + "_" + str(i) + "_" + str(j) + ".png"
            print(patch.shape)

            patch = Image.fromarray(patch)
            patch.save(destination_folder + filename)
