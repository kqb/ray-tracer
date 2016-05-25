/***********************************************************
 Starter code for Assignment 3

 This code was originally written by Jack Wang for
 CSC418, SPRING 2005

 implements scene_object.h

 ***********************************************************/

#include <cmath>
#include <iostream>
#include "scene_object.h"

//bool UnitSquare::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
//		const Matrix4x4& modelToWorld) {
//	// TODO: implement intersection code for UnitSquare, which is
//	// defined on the xy-plane, with vertices (0.5, 0.5, 0),
//	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
//	// (0, 0, 1).
//	//
//	// Your goal here is to fill ray.intersection with correct values
//	// should an intersection occur.  This includes intersection.point,
//	// intersection.normal, intersection.none, intersection.t_value.
//	//
//	// HINT: Remember to first transform the ray into object space
//	// to simplify the intersection test.

bool UnitSquare::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
        const Matrix4x4& modelToWorld) {
    // TODO: implement intersection code for UnitSquare, which is
    // defined on the xy-plane, with vertices (0.5, 0.5, 0),
    // (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
    // (0, 0, 1).
    //
    // Your goal here is to fill ray.intersection with correct values
    // should an intersection occur.  This includes intersection.point,
    // intersection.normal, intersection.none, intersection.t_value.
    //
    // HINT: Remember to first transform the ray into object space
    // to simplify the intersection test.

//WTM_dir model_ray_dir
// WTM_ori model_ray_ori
    Vector3D model_ray_dir = worldToModel * ray.dir;
    Point3D model_ray_ori = worldToModel * ray.origin;
    double t = -model_ray_ori[2] / model_ray_dir[2];
    Point3D Possible_intersection = model_ray_ori + t *model_ray_dir;


    if (t < 0) {
        return false;
    }
    Point3D point = modelToWorld * Possible_intersection;
    if ((ray.intersection.none || t < ray.intersection.t_value) && Possible_intersection[0] < 1 && Possible_intersection[1] < 1 && Possible_intersection[0]>-1 && Possible_intersection[1]>-1) {
        Vector3D normal(0, 0, 1);
        ray.intersection.point = point;
        ray.intersection.t_value = t;
        ray.intersection.normal = transNorm(worldToModel, normal);
        ray.intersection.normal.normalize();
        ray.intersection.none = false;
        return true;
    }

    return false;
}

bool UnitSphere::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
        const Matrix4x4& modelToWorld) {
    // TODO: implement intersection code for UnitSphere, which is centred
    // on the origin.
    //
    // Your goal here is to fill ray.intersection with correct values
    // should an intersection occur.  This includes intersection.point,
    // intersection.normal, intersection.none, intersection.t_value.
    //
    // HINT: Remember to first transform the ray into object space
    // to simplify the intersection test.
    Vector3D model_ray_dir = worldToModel * ray.dir;
    Point3D model_ray_ori = worldToModel * ray.origin;
    Vector3D origin_ray = model_ray_ori - Point3D(0, 0, 0);
    double t;

    //help to find the t-value
    double a = model_ray_dir.dot(model_ray_dir);
    double b = 2 * origin_ray.dot(model_ray_dir);
    double c = origin_ray.dot(origin_ray) - 1;

    if ((b * b - 4 * a * c) < 0) {
        return false;
    }
    else {
        double t1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
        double t2 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

		if (t2>t1)
			t = t1;
		else
			t = t2;

        if (t <= 0) {
            return false;
        } else {
			if ((ray.intersection.none|| t < ray.intersection.t_value)){
				Point3D point = model_ray_ori + t * model_ray_dir;
				Vector3D normal = point - Point3D(0, 0, 0);
				ray.intersection.t_value = t;
				ray.intersection.point = modelToWorld * point;
				ray.intersection.normal = transNorm(worldToModel, normal);
				ray.intersection.normal.normalize();
				ray.intersection.Normal_inter = point;
				ray.intersection.none = false;
				return true;
			}
        }
    }
}

bool UnitCylinder::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
        const Matrix4x4& modelToWorld) {
    Vector3D Obj_dir = worldToModel * ray.dir;
    Point3D Obj_ori = worldToModel * ray.origin;


    //find intersection line
    double a = Obj_dir[0] * Obj_dir[0] + Obj_dir[2] * Obj_dir[2];
    double b = 2 * (Obj_ori[0] * Obj_dir[0] + Obj_ori[2] * Obj_dir[2]);
    double c = Obj_ori[0] * Obj_ori[0] + Obj_ori[2] * Obj_ori[2] - 2;

    if ((b * b - 4 * a * c) > 0) {
        double t1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
        double t2 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

        if ((Obj_ori + t1 * Obj_dir)[1] > 1 || (Obj_ori + t1 * Obj_dir)[1] < -1) {
            t1 = -1;
        }
        if ((Obj_ori + t2 * Obj_dir)[1] > 1 || (Obj_ori + t2 * Obj_dir)[1] < -1) {
            t2 = -1;
        }

        //add point t3 for check intersection y = -1
        double t3 = -(Obj_ori[1]-1) / Obj_dir[1];
        double t3_0 = (Obj_ori + t3 * Obj_dir)[0]*(Obj_ori + t3 * Obj_dir)[0];
        double t3_2 = (Obj_ori + t3 * Obj_dir)[2]*(Obj_ori + t3 * Obj_dir)[2];
        if ((t3_0 + t3_2) > 2) {
            t3 = -1;
        }
        //check for intersection y = 1
        double t4 = -(1 + Obj_ori[1]) / Obj_dir[1];
        double t4_0 = (Obj_ori + t4 * Obj_dir)[0]*(Obj_ori + t4 * Obj_dir)[0];
        double t4_2 = (Obj_ori + t4 * Obj_dir)[2]*(Obj_ori + t4 * Obj_dir)[2];
        if (t4_0 + t4_2 > 2) {
            t4 = -1;
        }

        //find min value
        double min = 1000;
        double t_n[4] = {t1, t2, t3, t4};
        for (int i = 0; i < 4; i++) {
            if (t_n[i] > 0 && t_n[i] < min) {
                min = t_n[i];
            }
        }
        if ((!ray.intersection.none && min > ray.intersection.t_value) ||
                min < 0 || min == 1000) {
            return false;
        }
        Point3D point = Obj_ori + min * Obj_dir;
        Vector3D normal;
        if (min != t3) {
            normal = Vector3D(point[0], 0, point[2]);
        }else{
			normal = Vector3D(-point[0], 0, -point[2]);
		}

        ray.intersection.normal = transNorm(worldToModel, normal);
        ray.intersection.normal.normalize();
        ray.intersection.point = modelToWorld * point;
        ray.intersection.t_value = min;
        ray.intersection.none = false;
        return true;

    }
    return false;
}

bool UnitCone::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
        const Matrix4x4& modelToWorld) {
    Vector3D Obj_dir = worldToModel * ray.dir;
    Point3D Obj_ori = worldToModel * ray.origin;


    //find intersection line
    double a = Obj_dir[0] * Obj_dir[0] + Obj_dir[2] * Obj_dir[2] - Obj_dir[1] * Obj_dir[1];
    double b = 2 * (Obj_ori[0] * Obj_dir[0]) + 2 * (Obj_ori[2] * Obj_dir[2]) - 2 * (Obj_ori[1] * Obj_dir[1]);
    double c = Obj_ori[0] * Obj_ori[0] + Obj_ori[2] * Obj_ori[2] - Obj_ori[1] * Obj_ori[1];

    if ((b * b - 4 * a * c) > 0) {
        double t1 = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
        double t2 = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);

        if ((Obj_ori + t1 * Obj_dir)[1] > 1 || (Obj_ori + t1 * Obj_dir)[1] < -1) {
            t1 = -1;
        }
        if ((Obj_ori + t2 * Obj_dir)[1] > 1 || (Obj_ori + t2 * Obj_dir)[1] < -1) {
            t2 = -1;
        }

        //add point t3 for check intersection y = -1
        double t3 = -(Obj_ori[1]-1) / Obj_dir[1];
        double t3_0 = (Obj_ori + t3 * Obj_dir)[0]*(Obj_ori + t3 * Obj_dir)[0];
        double t3_2 = (Obj_ori + t3 * Obj_dir)[2]*(Obj_ori + t3 * Obj_dir)[2]+1;
        if ((t3_0 + t3_2) > 2) {
            t3 = -1;
        }
        //check for intersection y = 1
        double t4 = -(1 + Obj_ori[1]) / Obj_dir[1];
        double t4_0 = (Obj_ori + t4 * Obj_dir)[0]*(Obj_ori + t4 * Obj_dir)[0];
        double t4_2 = (Obj_ori + t4 * Obj_dir)[2]*(Obj_ori + t4 * Obj_dir)[2]+1;
        if (t4_0 + t4_2 > 2) {
            t4 = -1;
        }

        //find min value
        double min = 1000;
        double t_n[4] = {t1, t2, t3, t4};
        for (int i = 0; i < 4; i++) {
            if (t_n[i] > 0 && t_n[i] < min) {
                min = t_n[i];
            }
        }
        if ((!ray.intersection.none && min > ray.intersection.t_value) ||
                min < 0 || min == 1000) {
            return false;
        }
        Point3D point = Obj_ori + min * Obj_dir;
        Vector3D normal;
        if (min != t3) {
            normal = Vector3D(point[0], 0, point[2]);
        }else{
			normal = Vector3D(-point[0], 0, -point[2]);
		}

        ray.intersection.normal = transNorm(worldToModel, normal);
        ray.intersection.normal.normalize();
        ray.intersection.point = modelToWorld * point;
        ray.intersection.t_value = min;
        ray.intersection.none = false;
        return true;

    }
    return false;
}
