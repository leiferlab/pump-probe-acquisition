import numpy as np
import matplotlib.pyplot as plt
import sys

filename = sys.argv[1]


XY = np.loadtxt(filename)
X = XY[0]
Y = XY[1]

C = np.polyfit(X,Y,2,full=False)
f = np.poly1d(C)
print(f)

plt.plot(X,Y,'o')
plt.plot(X,f(X))
plt.show()

X = XY[1]
Y = XY[0]

C = np.polyfit(X,Y,2,full=False)
f = np.poly1d(C)
print(f)

plt.plot(X,Y,'o')
plt.plot(X,f(X))
plt.show()