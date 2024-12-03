To compile the program you will need Visual Studio (2022) for Windows, the files surfaceModeller.cpp, surfaceModeller.h, shaderUtils.cpp,
shaderUtils.h, subdivcurve.h, vertex_shader.glsl, and fragment_shader.glsl.
Because we are using VS you will also need the .sln and .vcxproj makefiles. 

IMPORTANT: To compile the project you may need the x64 folder which has freeglut.dll, this has been provided in the .zip submission.
On our end this folder in addition to the usual Dependencies folder with GLEW and freeglut is what was necessary to compile.

User inputs:
"L" key to toggle line drawing
"N" key to toggle normal vector drawing (in line drawing mode)
"P" key to toggle points
"E" key to export the current mesh to a file
"I" key to import mesh from a file
"left click + horizontal mouse movement" camera rotation around surface's y-axis
"right click + vertical mouse movement" camera elevation change (0-60 degrees)

Working:
Drawing Style, Camera Control, VAO/VBO Mesh Rendering, Mesh Exporting, Importing and Mesh Viewer

Not implemented:
Bonus shaders
