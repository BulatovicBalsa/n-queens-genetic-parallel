import numpy as np
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm


def draw(queens, n):
    dx, dy = 0.015, 0.05
    x = np.arange(-n / 2, n / 2, dx)
    y = np.arange(-n / 2, n / 2, dy)
    np.meshgrid(x, y)
    res = np.add.outer(range(n), range(n)) % 2
    plt.imshow(res, cmap="binary_r")

    for i in range(len(queens)):
        c1 = plt.Circle((i, queens[i]), 0.25, color="red")
        plt.gca().add_patch(c1)

    plt.xticks([])
    plt.yticks([])
    plt.show()
