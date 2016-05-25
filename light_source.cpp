/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

void PointLight::setRenderMode(int mode) {
          render_mode = mode;
}

void PointLight::shade( Ray3D& ray ) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.  
	Vector3D normal = ray.intersection.normal;
    normal.normalize();
 
    Vector3D eye = -ray.dir;
    eye.normalize();
 
    Vector3D light = _pos - ray.intersection.point;
    light.normalize();
 
    Material *material = ray.intersection.mat;

    double diffuse = normal.dot(light);
    double specular = (2 * normal.dot(light) * normal - light).dot(eye);
 
    if(diffuse < 0){
        diffuse = 0;
    }
    if(specular < 0){
        specular = 0;
    }
    specular = pow(specular, material->specular_exp);
    Colour colour; 
    
    if (render_mode == 0){ // 0 -> signature
            colour = _col_diffuse * material->diffuse;
            ray.col = colour;
            ray.col.clamp();
	}else if (render_mode == 1){ // 1 -> diffuse & ambient
            colour = _col_ambient * material->ambient + _col_diffuse * material->diffuse * Colour(diffuse, diffuse, diffuse);
            ray.col = colour;
            ray.col.clamp();
	}else { // phong model
            colour = _col_ambient * material->ambient + _col_diffuse * material->diffuse * Colour(diffuse, diffuse, diffuse) + _col_specular * material->specular * Colour(specular, specular, specular);
            ray.col = colour;
            ray.col.clamp();
    }
}

