# HazelQuick3D
H3D provides python bindings for animating and visualizing 3D data much more efficiently than maptlotlib. 

Internally, H3D provides python bindings built on C++ and OpenGL code. 

## Goal 
I have found many times that visualizing data in maptlotlib can be prohibitvely slow, especially in 3D. The point of H3D is to make it easier to do visualization in these cases. The library is very bare bones at the moment and I plan to add to it as needed.  

# Building
Clone with submodules:

```git clone --recurse-submodules -j8 https://github.com/meeree/HazelQuick3D.git```

Build with cmake:
```
mkdir build
cd build
cmake ..
make -j8
cp animate.py . # Copy an example into build folder. 
```

# Functionality and importing
Here are the functions you can currently run:
```
from hazelviz import animate, draw_triangles, draw_lines, draw_points
```

For an example, run ```animate.py``` in the build folder. It should animate a point going up and down in 3D space. More examples to come. 

To use the code in your library, copy ```hazelviz.so``` from the build folder to where you your python code is, so you can import it. 
