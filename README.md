

Part B - Advance Ray Tracer:

Part B was implmented based on PartA with the following addition features:

1) Handling non-trivial compound objects containing quadratic surfaces:
   scene_object.cpp - dunctions () and () were added to handle cylinder and clone shapes.
 
2) Glossy reflections:
   raytracer.cpp - added case under shadeRay() function to handle glossy reflections case (glossy_reflection variable must be set to true).

3) Texture-mapping:
   raytracer.cpp - modified computeShading() to called mapShading() which handles case of mapping texture to sphere.   surfaces.

4) Adding environment mapping:
   raytracer.cpp - case added under shadeRay() to handle reflections with environmental mapping algorithm.

5) Refraction:
   raytracer.cpp - case added under shadeRay() to calculate light refractions.

References:
Peter-Shirley-Michael-Ashikhmin-Steve-Marschner-Fundamentals-of-Computer-Graphics-A-K-Peters-2009
http://mrl.nyu.edu/~dzorin/rend05/lecture2.pdf
http://www.dgp.toronto.edu/~egarfink/webspace/Notes/BasicRayTracing.pdf
http://raytracer.wikidot.com/glossy-reflection
# ray-tracer
