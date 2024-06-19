import hazelviz as hv
import numpy as np

N = 100
cols = np.zeros((N * 3)).tolist()
pts = np.zeros((N, 3))
pts[:, 1] = np.linspace(-1, 1, N)
pts = pts.flatten().tolist()
cols = list(pts)

hv.animate(pts, cols, N // 10, .1, 0)
