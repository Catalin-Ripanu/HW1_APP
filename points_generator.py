import numpy as np
from random import randint

if __name__ == "__main__":
    points_no = 200_000
    points = np.random.rand(points_no, 2)
    clusters_no = 1_000
    print(clusters_no, points_no)

    for point in points:
        print(randint(1, 100) * point[0], randint(1, 100) * point[1])