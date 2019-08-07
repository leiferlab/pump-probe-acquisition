import numpy as np
import matplotlib.pyplot as plt
import sys

filename = "piezoCalibrationRemoteFocusing.txt"


XY = np.loadtxt(filename).T
X = XY[0]-100
Y = XY[1]-100

C = np.polyfit(X,Y,2,full=False)
f = np.poly1d(C)
print(f)

plt.plot(X,Y,'o')
plt.plot(X,f(X))
plt.plot(X,-0.5*X-0.0013*X**2)
plt.show()

X = XY[1]-100
Y = XY[0]-100

C = np.polyfit(X,Y,2,full=False)
f = np.poly1d(C)
print(f)

plt.plot(X,Y,'o')
plt.plot(X,f(X))
plt.show()