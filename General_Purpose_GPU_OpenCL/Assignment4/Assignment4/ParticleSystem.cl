#define GID get_global_id(0)
#define LID get_local_id(0)

float4 cross3(float4 a, float4 b){
	float4 c;
	c.x = a.y * b.z - b.y * a.z;
	c.y = a.z * b.x - b.z * a.x;
	c.z = a.x * b.y - b.x * a.y;
	c.w = 0.f;
	return c;
}

float dot3(float4 a, float4 b){
	return a.x*b.x + a.y*b.y + a.z*b.z;
}


#define EPSILON 0.001f

// This function expects two points defining a ray (x0 and x1)
// and three vertices stored in v1, v2, and v3 (the last component is not used)
// it returns true if an intersection is found and sets the isectT and isectN
// with the intersection ray parameter and the normal at the intersection point.
bool LineTriangleIntersection(	float4 x0, float4 x1,
								float4 v1, float4 v2, float4 v3,
								float *isectT, float4 *isectN){

	float4 dir = x1 - x0;
	dir.w = 0.f;

	float4 e1 = v2 - v1;
	float4 e2 = v3 - v1;
	e1.w = 0.f;
	e2.w = 0.f;

	float4 s1 = cross3(dir, e2);
	float divisor = dot3(s1, e1);
	if (divisor == 0.f)
		return false;
	float invDivisor = 1.f / divisor;

	// Compute first barycentric coordinate
	float4 d = x0 - v1;
	float b1 = dot3(d, s1) * invDivisor;
	if (b1 < -EPSILON || b1 > 1.f + EPSILON)
		return false;

	// Compute second barycentric coordinate
	float4 s2 = cross3(d, e1);
	float b2 = dot3(dir, s2) * invDivisor;
	if (b2 < -EPSILON || b1 + b2 > 1.f + EPSILON)
		return false;

	// Compute _t_ to intersection point
	float t = dot3(e2, s2) * invDivisor;
	if (t < -EPSILON || t > 1.f + EPSILON)
		return false;

	// Store the closest found intersection so far
	*isectT = t;
	*isectN = cross3(e1, e2);
	*isectN = normalize(*isectN);
	return true;

}


bool CheckCollisions(	float4 x0, float4 x1,
						__global float4 *gTriangleSoup, 
						__local float4* lTriangleCache,	// The cache should hold as many vertices as the number of threads (therefore the number of triangles is nThreads/3)
						uint nTriangles,
						float  *t,
						float4 *n){


	// ADD YOUR CODE HERE
	// Each vertex of a triangle is stored as a float4, the last component is not used.
	// gTriangleSoup contains vertices of triangles in the following layout:
	// --------------------------------------------------------------
	// | t0_v0 | t0_v1 | t0_v2 | t1_v0 | t1_v1 | t1_v2 | t2_v0 | ...
	// --------------------------------------------------------------

	// First check collisions loading the triangles from the global memory.
	// Iterate over all triangles, load the vertices and call the LineTriangleIntersection test 
	// for each triangle to find the closest intersection. 

	// Notice that each thread has to read vertices of all triangles.
	// As an optimization you should implement caching of the triangles in the local memory.
	// The cache should hold as many float4 vertices as the number of threads.
	// In other words, each thread loads (at most) *one vertex*.
	// Consequently, the number of triangles in the cache will be nThreads/4.
	// Notice that if there are many triangles (e.g. CubeMonkey.obj), not all
	// triangles can fit into the cache at once. 
	// Therefore, you have to load the triangles in chunks, until you process them all.

	// The caching implementation should roughly follow this scheme:
	//uint nProcessed = 0;  
	//while (nProcessed < nTriangles) {
	//	Load a 'k' triangles in to the cache
	//	Iterate over the triangles in the cache and test for the intersection
	//	nProcessed += k;  
	//}

	// Init seed parameter to an improbably far point
        float closest_isect_param = FLT_MAX;
    // Resulting normal at the intersection
    float4 closest_isect_normal;
    
    // The caching implementation
    // Running variable for the processed triangle count.
        uint num_processed_triangles = 0;
	

    // Here we iterate through the triangles in the suggessted manner
    // when we load the triangles we multiply nprocessed by 3 in order to
    // jump over the processed vertices. But we only advance it by lid_x / 3
    // in each iteration, so it will be about triangles and not vertices.
    // 
    // We don't need any further checks, because the number of threads is a
    // multiple of 3.
    while (num_processed_triangles < nTriangles)
	{
		// Load a 'k' triangles in to the cache
        // Every processed triangle means 3 processed points
                lTriangleCache[LID] = gTriangleSoup[num_processed_triangles * 3 + LID];
		barrier(CLK_LOCAL_MEM_FENCE);
		
        //Iterate over the triangles in the cache and test for the intersection
		for (int i = 0; i < get_local_size(0) / 3; ++i)
		{
            // Intersection parameter output
                        float temp_isect_param;
            // Intersection normal output
                        float4 temp_isect_normal;
			
            // Test the segment for inteserctions in this batch of triangles
            if (LineTriangleIntersection(x0, x1, lTriangleCache[3 * i], 
                                            lTriangleCache[3 * i + 1], 
                                            lTriangleCache[3 * i + 2], 
                                            &temp_isect_param, &temp_isect_normal))
			{
                // If we found an intersection closer than the closest one,
                // then we have to update the stuff.
                                if (temp_isect_param < closest_isect_param)
				{
                                        closest_isect_param = temp_isect_param;
                                        closest_isect_normal = temp_isect_normal;
				}
			}
		}

        // Advance by the number of triangles that our workgroup can handle.
                num_processed_triangles += get_local_size(0) / 3;
	}

    // See if there was an intersection
        if (closest_isect_param < FLT_MAX)
	{
                *t = closest_isect_param;
                *n = closest_isect_normal;
		return true;
	}

	return false;
}
								



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the integration kernel. Implement the missing functionality
//
// Input data:
// gAlive         - Field of flag indicating whether the particle with that index is alive (!= 0) or dead (0). You will have to modify this
// gForceField    - 3D texture with the  force field
// sampler        - 3D texture sampler for the force field (see usage below)
// nParticles     - Number of input particles
// nTriangles     - Number of triangles in the scene (for collision detection)
// lTriangleCache - Local memory cache to be used during collision detection for the triangles
// gTriangleSoup  - The triangles in the scene (layout see the description of CheckCollisions())
// gPosLife       - Position (xyz) and remaining lifetime (w) of a particle
// gVelMass       - Velocity vector (xyz) and the mass (w) of a particle
// dT             - The timestep for the integration (the has to be subtracted from the remaining lifetime of each particle)
//
// Output data:
// gAlive   - Updated alive flags
// gPosLife - Updated position and lifetime
// gVelMass - Updated position and mass
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Integrate(__global uint *gAlive,
						__read_only image3d_t gForceField, 
						sampler_t sampler,
						uint nParticles,
						uint nTriangles,
						__local float4 *lTriangleCache,
						__global float4 *gTriangleSoup,
						__global float4 *gPosLife, 
						__global float4 *gVelMass,
						float dT
						)  {

	
    // If there is no particle for the thread, return.
    if (GID >= nParticles)
		return;
	
    // If a particle is dead we should remove it
    /*if (gPosLife[GID].w <= 0)
	{
        //particle not alive
		gAlive[GID] = 0; 
		
        //no new particle created
        gAlive[GID + nParticles] = 0;
        return;
	}*/
		
    // A base gravitational field, so it will move more vividly along,
    // not only in a square
	float4 gAccel = (float4)(0.f, -9.81f, 0.f, 0.f);
    //    float4 gAccel = (float4)(0.f, 0.f, 0.f, 0.f);
	
    // Verlet Velocity Integration
	float4 x0 = gPosLife[GID];
	float4 v0 = gVelMass[GID];

	float mass = v0.w;
	float life = x0.w;

    // perform lookup in the 3D forcefield texture
	float4 lookUp = x0;
    // set it to 0 because of the mip level
	lookUp.w = 0.f;

    // get the forcefield values from the texture, and add our initial gravitation
        float4 F0 = read_imagef(gForceField, sampler, lookUp);


	// ADD YOUR CODE HERE (INSTEAD OF THE LINES BELOW) 
	// to finish the implementation of the Verlet Velocity Integration
	
    // Force = acc * mass newtonian equation
    float4 a0 = F0 / mass + gAccel;

    // Calculate x(t + dt)
	float4 x1 = x0 + v0 * dT + 0.5f * a0 * dT * dT;

    // Look up the force in this position
	lookUp = x1;
        float4 F1 = read_imagef(gForceField, sampler, lookUp);
	// a(t + dt)
    float4 a1 = F1 / mass + gAccel;
    // get v(t + dt) according to the equation in the docs
	float4 v1 = v0 + 0.5f * (a0 + a1) * dT;
	
	
	// Check for collisions and correct the position and velocity of the particle if it collides with a triangle
	// - Don't forget to offset the particles from the surface a little bit, otherwise they might get stuck in it.
	// - Dampen the velocity (e.g. by a factor of 0.7) to simulate dissipation of the energy.
    
    // parameter of the nearest collision
    float t;
    // normal of the same
	float4 n;
	
	if (CheckCollisions(x0, x1, gTriangleSoup, lTriangleCache, nTriangles, &t, &n))
	{
        // Move it back along the curve that it is travelling, and offset it by n * eps
		x1 = x0 + (x1 - x0) * t + n * EPSILON;
        // Dampen the velocity, and give it a new direction
		v1 = 0.7f * (v1 - 2 * (dot3(v1, n) * n));
	}

    // Set the mass for the new velMass vector
	v1.w = mass;
    // Assign it to be official
	gVelMass[GID] = v1;

	// Kill the particle if its life < 0.f by setting the corresponding flag in gAlive to 0.
	life -= 0.1f;//dT;
	x1.w = life;
	gPosLife[GID] = x1;

	if (life < 0)
    {
		gAlive[GID] = 0;
    }
	else
    {
		gAlive[GID] = 1;
    }

	// Independently of the status of the particle, possibly create a new one.
	// For instance, if the particle gets too fast (or too high, or passes through some region), it is split into two...
	// if it goes too far in the x direction, or it has a too big velocity in the y direction it splits into two
    if (x1.y > 0.9f || v1.y > 0.9f && gAlive[GID] > 0)
	{
		gAlive[GID + nParticles] = 1;
		gPosLife[GID + nParticles] = gPosLife[GID];
        // The resulting particle will have a short lifespan
		gPosLife[GID + nParticles].w = 0.05;

		// Change the impulse, so it diverges from the original particle
		gVelMass[GID + nParticles].x = gVelMass[GID].x * 2;
		gVelMass[GID + nParticles].y = -gVelMass[GID].y * 2;
		gVelMass[GID + nParticles].z = gVelMass[GID].z * 2;
		gVelMass[GID + nParticles].w = gVelMass[GID].w * 0.1;
		
        // scale down the original particle a little bit
        gVelMass[GID].x = gVelMass[GID].x * 0.2;
		gVelMass[GID].y = gVelMass[GID].y * 0.2;
		gVelMass[GID].z = gVelMass[GID].z * 0.2;
		gVelMass[GID].w = gVelMass[GID].w * 0.9;
	}		
	else
    {
        //this here is very important (initializing the whole array at first)
		gAlive[GID + nParticles] = 0;
    }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the tiny kernel that initializes the output array
__kernel void Clear(__global float4* gPosLife, __global float4* gVelMass) {
	gPosLife[GID] = 0.f;
	gVelMass[GID] = 0.f;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Reorganize(	__global uint* gAlive, __global uint* gRank,	
							__global float4* gPosLifeIn,  __global float4* gVelMassIn,
							__global float4* gPosLifeOut, __global float4* gVelMassOut) {
	
	////Re-order the particles according to the gRank obtained from the parallel prefix sum
	////ADD YOUR CODE HERE
	// If the particle is alive
    if (gAlive[GID])
	{
        // Find its position in the compressed array with exclusive sum
		int index = gRank[GID] - 1;

        // Update the data about the particle to be in the correct place
		gPosLifeOut[index] = gPosLifeIn[GID];
		gVelMassOut[index] = gVelMassIn[GID];
	}
}
