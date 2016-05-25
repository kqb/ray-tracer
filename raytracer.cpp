/***********************************************************
     Starter code for Assignment 3

     This code was originally written by Jack Wang for
		    CSC418, SPRING 2005

		Implementations of functions in raytracer.h, 
		and the main function which specifies the 
		scene to be rendered.	

***********************************************************/


#include "raytracer.h"
#include "bmp_io.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <cstdlib>

const double RGB = 255;
bool glossy_reflection = false;
bool anti_alias = false;
unsigned long int texW[1];
long int texH[1];
char* bitmap[] = {"Pic/stones.bmp"};
unsigned char* texture_red[1];
unsigned char* texture_green[1];
unsigned char* texture_blue[1];



Raytracer::Raytracer() :
_lightSource(NULL) {
    _root = new SceneDagNode();
}

Raytracer::~Raytracer() {
    delete _root;
}

SceneDagNode* Raytracer::addObject(SceneDagNode* parent, SceneObject* obj,
        Material* mat) {
    SceneDagNode* node = new SceneDagNode(obj, mat);
    node->parent = parent;
    node->next = NULL;
    node->child = NULL;

    // Add the object to the parent's child list, this means
    // whatever transformation applied to the parent will also
    // be applied to the child.
    if (parent->child == NULL) {
        parent->child = node;
    } else {
        parent = parent->child;
        while (parent->next != NULL) {
            parent = parent->next;
        }
        parent->next = node;
    }

    return node;
    ;
}

LightListNode* Raytracer::addLightSource(LightSource* light) {
    LightListNode* tmp = _lightSource;
    _lightSource = new LightListNode(light, tmp);
    return _lightSource;
}

void Raytracer::rotate(SceneDagNode* node, char axis, double angle) {
    Matrix4x4 rotation;
    double toRadian = 2 * M_PI / 360.0;
    int i;

    for (i = 0; i < 2; i++) {
        switch (axis) {
            case 'x':
                rotation[0][0] = 1;
                rotation[1][1] = cos(angle * toRadian);
                rotation[1][2] = -sin(angle * toRadian);
                rotation[2][1] = sin(angle * toRadian);
                rotation[2][2] = cos(angle * toRadian);
                rotation[3][3] = 1;
                break;
            case 'y':
                rotation[0][0] = cos(angle * toRadian);
                rotation[0][2] = sin(angle * toRadian);
                rotation[1][1] = 1;
                rotation[2][0] = -sin(angle * toRadian);
                rotation[2][2] = cos(angle * toRadian);
                rotation[3][3] = 1;
                break;
            case 'z':
                rotation[0][0] = cos(angle * toRadian);
                rotation[0][1] = -sin(angle * toRadian);
                rotation[1][0] = sin(angle * toRadian);
                rotation[1][1] = cos(angle * toRadian);
                rotation[2][2] = 1;
                rotation[3][3] = 1;
                break;
        }
        if (i == 0) {
            node->trans = node->trans * rotation;
            angle = -angle;
        } else {
            node->invtrans = rotation * node->invtrans;
        }
    }
}

void Raytracer::translate(SceneDagNode* node, Vector3D trans) {
    Matrix4x4 translation;

    translation[0][3] = trans[0];
    translation[1][3] = trans[1];
    translation[2][3] = trans[2];
    node->trans = node->trans * translation;
    translation[0][3] = -trans[0];
    translation[1][3] = -trans[1];
    translation[2][3] = -trans[2];
    node->invtrans = translation * node->invtrans;
}

void Raytracer::scale(SceneDagNode* node, Point3D origin, double factor[3]) {
    Matrix4x4 scale;

    scale[0][0] = factor[0];
    scale[0][3] = origin[0] - factor[0] * origin[0];
    scale[1][1] = factor[1];
    scale[1][3] = origin[1] - factor[1] * origin[1];
    scale[2][2] = factor[2];
    scale[2][3] = origin[2] - factor[2] * origin[2];
    node->trans = node->trans * scale;
    scale[0][0] = 1 / factor[0];
    scale[0][3] = origin[0] - 1 / factor[0] * origin[0];
    scale[1][1] = 1 / factor[1];
    scale[1][3] = origin[1] - 1 / factor[1] * origin[1];
    scale[2][2] = 1 / factor[2];
    scale[2][3] = origin[2] - 1 / factor[2] * origin[2];
    node->invtrans = scale * node->invtrans;
}

Matrix4x4 Raytracer::initInvViewMatrix(Point3D eye, Vector3D view,
        Vector3D up) {
    Matrix4x4 mat;
    Vector3D w;
    view.normalize();
    up = up - up.dot(view) * view;
    up.normalize();
    w = view.cross(up);

    mat[0][0] = w[0];
    mat[1][0] = w[1];
    mat[2][0] = w[2];
    mat[0][1] = up[0];
    mat[1][1] = up[1];
    mat[2][1] = up[2];
    mat[0][2] = -view[0];
    mat[1][2] = -view[1];
    mat[2][2] = -view[2];
    mat[0][3] = eye[0];
    mat[1][3] = eye[1];
    mat[2][3] = eye[2];

    return mat;
}

void Raytracer::traverseScene(SceneDagNode* node, Ray3D& ray) {
    SceneDagNode *childPtr;

    // Applies transformation of the current node to the global
    // transformation matrices.
    _modelToWorld = _modelToWorld * node->trans;
    _worldToModel = node->invtrans * _worldToModel;
    if (node->obj) {
        // Perform intersection.
        if (node->obj->intersect(ray, _worldToModel, _modelToWorld)) {
            ray.intersection.mat = node->mat;
        }
    }
    // Traverse the children.
    childPtr = node->child;
    while (childPtr != NULL) {
        traverseScene(childPtr, ray);
        childPtr = childPtr->next;
    }

    // Removes transformation of the current node from the global
    // transformation matrices.
    _worldToModel = node->trans * _worldToModel;
    _modelToWorld = _modelToWorld * node->invtrans;
}

void Raytracer::mapShading(Ray3D& ray, LightListNode* curLight) {
    int i;
    
    //Calculate light distance
    Vector3D light_distance = curLight->light->get_position() - ray.intersection.point;
   

    Ray3D shadow_ray;
    shadow_ray.dir = light_distance;
    shadow_ray.dir.normalize();
    shadow_ray.origin = ray.intersection.point+0.0001*shadow_ray.dir;
    traverseScene(_root, shadow_ray);

    // if no texture is available to map or the mapping is not enabled
    if (ray.intersection.mat->tex == -1) {
        curLight->light->shade(ray);
    } else {
		    // texture mapping case
			Vector3D x(1, 0, 0);
			Vector3D y(0, 1, 0);
			//get the intersection point in object space
			Vector3D Pt_obj = (Point3D(0, 0, 0) - ray.intersection.Normal_inter);
			Pt_obj.normalize();

			double theta = acos(Pt_obj.dot(x)/sin(acos(y.dot(Pt_obj))))/(2*M_PI);

			if ((y.cross(x)).dot(Pt_obj) < 0)
				i =  ((1 - theta) * texH[ray.intersection.mat->tex]) * texW[ray.intersection.mat->tex] + (acos(y.dot(Pt_obj)) / M_PI * texW[ray.intersection.mat->tex]);
			else
				i = (theta * texH[ray.intersection.mat->tex]) * texW[ray.intersection.mat->tex]	+ (acos(y.dot(Pt_obj)) / M_PI * texW[ray.intersection.mat->tex]);
			//locate the pixel position and set colours
			ray.col[0] = texture_red[ray.intersection.mat->tex][i] / RGB;
			ray.col[1] = texture_green[ray.intersection.mat->tex][i] / RGB;
			ray.col[2] = texture_blue[ray.intersection.mat->tex][i] / RGB;

    }
}

void Raytracer::computeShading(Ray3D& ray) {
    LightListNode* curLight = _lightSource;
    for (;;) {
        if (curLight == NULL)
            break;

        mapShading(ray, curLight);
        curLight = curLight->next;
    }
}

void Raytracer::initPixelBuffer() {
    int numbytes = _scrWidth * _scrHeight * sizeof (unsigned char);
    _rbuffer = new unsigned char[numbytes];
    _gbuffer = new unsigned char[numbytes];
    _bbuffer = new unsigned char[numbytes];
    for (int i = 0; i < _scrHeight; i++) {
        for (int j = 0; j < _scrWidth; j++) {
            _rbuffer[i * _scrWidth + j] = 0;
            _gbuffer[i * _scrWidth + j] = 0;
            _bbuffer[i * _scrWidth + j] = 1;
        }
    }
}

void Raytracer::flushPixelBuffer(char *file_name) {
    bmp_write(file_name, _scrWidth, _scrHeight, _rbuffer, _gbuffer, _bbuffer);
    delete _rbuffer;
    delete _gbuffer;
    delete _bbuffer;
}

Colour Raytracer::shadeRay(Ray3D& ray) {
    Colour col(0.0, 0.0, 0.0);
    traverseScene(_root, ray);

    // Don't bother shading if the ray didn't hit anything.
    if (!ray.intersection.none) {
        computeShading(ray);

        col = ray.col;

        if (ray.intersection.mat->transparency > 0) {
			// reflection case
            if (ray.rlef_ind < 1) {
                Ray3D ref;
                ref.rlef_ind = ray.rlef_ind + 1;
                ref.origin = ray.intersection.point;
                ray.dir.normalize();
                ref.dir = ray.dir - 2 * ray.intersection.normal.dot(ray.dir) * ray.intersection.normal;
                ref.dir.normalize();
                ref.origin = ref.origin + 0.0001 * ref.dir;

                // glossy reflection case
                if(glossy_reflection){
                    Colour tempc(0, 0, 0);
                    int i = 0;
                    while (i < 100) {
                        Ray3D tempRay;
                        double theta = acos(pow((1 - (double) (rand() / RAND_MAX)), ray.intersection.mat->reflectivity));
                        double x = sin(2 * M_PI * (double) (rand() / RAND_MAX)) * cos(theta) / 15;
                        double y = sin(2 * M_PI * (double) (rand() / RAND_MAX)) * sin(theta) / 15;
                        tempRay.dir = x * ref.dir.cross(ray.intersection.normal) + y * ref.dir.cross(ref.dir.cross(ray.intersection.normal)) + ref.dir;
                        tempRay.dir.normalize();
                        tempRay.origin = ref.origin;
                        tempRay.ns = ref.ns;
                        tempc = tempc + (ray.intersection.mat->reflectivity) * shadeRay(tempRay);
                        i++;
                    }
                    col = col + tempc / 100;
                } else {
                	col = col + (ray.intersection.mat->reflectivity) * shadeRay(ref);
                }


                col.clamp();
            }

            // refraction case
            if (ray.rlef_ind < 2) {
                Vector3D Ray_obj = (ray.intersection.point - ray.origin);
                Ray_obj.normalize();
                Ray3D newRay;
                float f;
                float r;
                if (ray.intersection.normal.dot(Ray_obj) < 0) {
                    f= 0.66;
                    r = sqrtf(1 - f * f * (1 - (ray.intersection.normal.dot(-Ray_obj)) *(ray.intersection.normal.dot(-Ray_obj))));
                    if (r >= 0) {
                        newRay.rlef_ind = ray.rlef_ind + 1;
                        newRay.dir = (f * (ray.intersection.normal.dot(-Ray_obj)) - r) * ray.intersection.normal - f * (-Ray_obj);

                    }
                } else {
                    f = 1.5;
                    Vector3D Ray_obj = ray.dir;
                    Ray_obj.normalize();
                    r = sqrtf(1 - f * f * (1 - (-ray.intersection.normal.dot(-Ray_obj))
                            *(-ray.intersection.normal.dot(-Ray_obj))));
                    if (r >= 0) {
                        newRay.rlef_ind = ray.rlef_ind + 1;
                        newRay.dir = (f * (-ray.intersection.normal.dot(-Ray_obj)) - r) * (-ray.intersection.normal - f * (-Ray_obj));
                    }
                }
                if (r >= 0) {
                    newRay.dir.normalize();
                    newRay.origin = (ray.intersection.point + 0.0001 * Ray_obj);
                    Colour tempCol = shadeRay(newRay);
                    col = col + (ray.intersection.mat->transparency) * tempCol;
                    col.clamp();
                }
            }
        }
    }
    return col;
}

void Raytracer::render(int width, int height, Point3D eye, Vector3D view,
        Vector3D up, double fov, char* fileName) {
    Matrix4x4 viewToWorld;
    _scrWidth = width;
    _scrHeight = height;
    double factor = (double(height)/2)/tan(fov*M_PI/360.0);

	bmp_read(bitmap[0], &texW[0],&texH[0], &texture_red[0], &texture_green[0],&texture_blue[0]);
    initPixelBuffer();
    viewToWorld = initInvViewMatrix(eye, view, up);

    for (int i = 0; i < _scrHeight; i++) {
        for (int j = 0; j < _scrWidth; j++) {
            // Sets up ray origin and direction in view space, 
            // image plane is at z = -1.
            Point3D origin(0, 0, 0);
            Point3D imagePlane;
			Colour col(0, 0, 0);

            //handle AA
			if (anti_alias){
				for (float count1 =i;count1<i+1.0f;count1+=0.5f){
					for (float count2 =j;count2<j+1.0f;count2+=0.5f){
						imagePlane[0] = (-double(width)/2+0.5+count2)/factor;
						imagePlane[1] = (-double(height)/2 + 0.5+count1)/factor;
						imagePlane[2] = -1;

						Point3D rayOriginWorld = viewToWorld * imagePlane;
						Vector3D rayDirWorld = rayOriginWorld - eye;
						rayDirWorld.normalize();

						Ray3D ray(rayOriginWorld, rayDirWorld);
						col = shadeRay(ray);

						_rbuffer[i*width+j] += int(col[0]*255*0.25f);
						_gbuffer[i*width+j] += int(col[1]*255*0.25f);
						_bbuffer[i*width+j] += int(col[2]*255*0.25f);
					}
				}
			}else {
				Ray3D ray;
				ray.origin = viewToWorld * origin;

				imagePlane[0] = (-double(width)/2+0.5 +j)/factor;
				imagePlane[1] = (-double(height)/2+0.5+i)/factor;
	            imagePlane[2] = -1;

				ray.dir = viewToWorld * Point3D(imagePlane[0], imagePlane[1], imagePlane[2])-ray.origin;
				ray.dir.normalize();
				col = shadeRay(ray);

				_rbuffer[i*width+j] = int(col[0]*255);
				_gbuffer[i*width+j] = int(col[1]*255);
				_bbuffer[i*width+j] = int(col[2]*255);
			}
        }
    }

    flushPixelBuffer(fileName);
}

int main(int argc, char* argv[]) {
    // Build your scene and setup your camera here, by calling
    // functions from Raytracer.  The code here sets up an example
    // scene and renders it from two different view points, DO NOT
    // change this if you're just implementing part one of the
    // assignment.
    Raytracer raytracer;
    int width = 640;
    int height = 480;
//	int width = 150;
//	int height = 100;
//	int width = 1920;
//	int height = 1080;

    if (argc == 3) {
        width = atoi(argv[1]);
        height = atoi(argv[2]);
    }

    // Camera parameters.

    Vector3D up(0, 1, 0);
    double fov = 60;

    // light source
    PointLight *lightSource = new PointLight(Point3D(0, 6, 0), Colour(0.95, 0.95, 0.95));
    raytracer.addLightSource(lightSource);

    // Shading materials
    Material gold( Colour(0.3, 0.3, 0.3), Colour(0.75164, 0.60648, 0.22648), Colour(0.628281, 0.555802, 0.366065), 51.2, -1, 0.2, 0.5);
	Material jade( Colour(0, 0, 0), Colour(0.54, 0.89, 0.63), Colour(0.316228, 0.316228, 0.316228), 12.8, -1, 0.4, 0.2 );
	Material silver( Colour(0.19225, 0.19225, 0.19225), Colour(0.50754, 0.50754, 0.50754), Colour(0.508273, 0.508273, 0.508273), 0.4, -1, 0.2, 0 );
	Material pearl( Colour(0.25, 0.20725, 0.20725), Colour(1, 0.829, 0.829), Colour(0.296648, 0.296648, 0.296648), 0.088, -1, 0.2, 0 );
	Material glass(Colour(0, 0, 0), Colour(0, 0, 0), Colour(1, 1, 1), 80.0, -1, 1, 1);
    Material blue(Colour(0.25, 0.25, 0.25), Colour(1, 0.5, 0.5), Colour(0.628281, 0.555802, 0.366065), 51.2, -1, 0.2, 0.2);
    Material red(Colour(0.25, 0.25, 0.25), Colour(1.0, 0.0, 0.0), Colour(0.6, 0.6, 0.6), 50, -1, 0.2, 0.2);
	Material texture_map1(Colour(0, 0, 0), Colour(0, 0, 0), Colour(0, 0, 0), 50, 0, 0, 0);

	// add objects
	SceneDagNode* sphere1 = raytracer.addObject(new UnitSphere(), &glass);
    SceneDagNode* sphere2 = raytracer.addObject(new UnitSphere(), &texture_map1);
	SceneDagNode* sphere3 = raytracer.addObject(new UnitSphere(), &glass);

    SceneDagNode* cylinder3 = raytracer.addObject(new UnitCylinder(), &blue);

    SceneDagNode* cone2 = raytracer.addObject(new UnitCone(), &pearl);
    SceneDagNode* cone4 = raytracer.addObject(new UnitCone(), &jade);
	SceneDagNode* cone5 = raytracer.addObject(new UnitCone(), &gold);
	SceneDagNode* cone6 = raytracer.addObject(new UnitCone(), &silver);

    SceneDagNode* plane1 = raytracer.addObject(new UnitSquare(), &red);
    SceneDagNode* plane2 = raytracer.addObject(new UnitSquare(), &jade);
    SceneDagNode* plane3 = raytracer.addObject(new UnitSquare(), &gold);
    SceneDagNode* plane4 = raytracer.addObject(new UnitSquare(), &jade);
    SceneDagNode* plane5 = raytracer.addObject(new UnitSquare(), &red);

    double planef[3] = {16,16,1};

    raytracer.translate(plane1, Vector3D(-8, 1, -5));
    raytracer.translate(plane2, Vector3D(0, 8, -5));
    raytracer.translate(plane4, Vector3D(0, -8, -5));
    raytracer.translate(plane3, Vector3D(0, 0, -12));
    raytracer.translate(plane5, Vector3D(8, 0, -5));

    raytracer.translate(sphere1, Vector3D(-2, 0, -1));
    raytracer.translate(sphere2, Vector3D(2, 1.5, -4));
    raytracer.translate(sphere3, Vector3D(2, 0, -1));
    raytracer.translate(cylinder3, Vector3D(-4, 0, -6));
    raytracer.translate(cone2, Vector3D(3, -3, -6));

    raytracer.rotate(plane1, 'y', 90);
    raytracer.rotate(plane2, 'x', 90);
    raytracer.rotate(plane4, 'x', 270);
    raytracer.rotate(plane5, 'y', 270);

    raytracer.scale(plane1, Point3D(0, 0, 0), planef);
    raytracer.scale(plane2, Point3D(0, 0, 0), planef);
    raytracer.scale(plane3, Point3D(0, 0, 0), planef);
    raytracer.scale(plane4, Point3D(0, 0, 0), planef);
    raytracer.scale(plane5, Point3D(0, 0, 0), planef);

    double conef[3] = {1.5,1.5,1.5};
	raytracer.translate(cone4, Vector3D(-1, -2 , -6));
	raytracer.translate(cone5, Vector3D(3, -3, -6));
	raytracer.translate(cone6, Vector3D(-1, -2, -6));
	raytracer.scale(cone6, Point3D(0, 0, 0), conef);
	raytracer.scale(cone4, Point3D(0, 0, 0), conef);

	raytracer.rotate(cone4, 'z', 90);
	raytracer.rotate(cone5, 'z', 45);
	raytracer.rotate(cone2, 'z', 135);


	// phong model
	lightSource->setRenderMode(2);

    Point3D eye2(0, 0, 3);
    Vector3D view2(0, 0, -5);
    raytracer.render(width, height, eye2, view2, up, fov, "front.bmp");

    Point3D eye(2, 5, 1);
    Vector3D view(-1, -5, -3);
    raytracer.render(width, height, eye, view, up, fov, "top1.bmp");


    Point3D eye3(0, 2, 1);
    Vector3D view3(-3, -5, -10);
    raytracer.render(width, height, eye3, view3, up, fov, "top2.bmp");


    return 0;
}
