#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "MidOpenGL.h"

namespace PV
{
	namespace Shapes
	{
		class Sphere
		{
		public:
			Sphere(float radius, float lats, float longs);
			unsigned int arrayHandle;
			/**
			* Stores the total size of the sphere.
			*/
			unsigned int size;
		private:
			/**
			 * The radius of the sphere.
			 */
			float radius;
			/**
			 * The buffer to store the sphere's elements.
			 */
			unsigned int elementBuffer;
			/**
			 * The buffer to store the sphere's normals.
			 */
			unsigned int normalBuffer;
			/**
			 * The buffer to store the sphere's UV texture coordinates.
			 */
			unsigned int uvBuffer;
			/**
			 * The buffer to store the sphere's vertex colors.
			 */
			unsigned int colorBuffer;
			/**
			 * The buffer to store the sphere's vertices.
			 */
			unsigned int vertexBuffer;
		};
	};
};

#endif