import numpy as np
import matplotlib.pyplot as plt

X, Y = np.loadtxt('imaging.txt').T[:,0:10]
Xp, Yp = np.loadtxt('imaging.txt').T

C = np.polyfit(X,Y,1,full=False)
f = np.poly1d(C)
print(f)

#plt.plot(X,Y,'o')
plt.plot(Xp,f(Xp))
plt.plot(Xp,Yp,'o-')

plt.xlabel("Focal power (dpt)")
plt.ylabel("Piezo position (um)")
plt.savefig('tunablelenscalibration.png',dpi=150)
plt.show()