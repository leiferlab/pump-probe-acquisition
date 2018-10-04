In addition to 
- the last paragraph of PATH-tutorial1seelastparagraph.pdf
- and PATH-tutorial2.pdf

remember to copy the OpenCV .dll files in the folders where your wrapper DLL are. If you are using Visual Studio, you will probably have
two versions of your DLL, one in the x64\Debug folder and the other in x64\Release. You should put opencv_world341d.dll (or equivalent,
the final d after the version number means that it's the one for debugging) in x64\Debug, and opencv_world341.dll in x64\Release.
If you don't do this, LabView will not be able to load your DLL because it cannot find the .dll files on which your DLL depends.