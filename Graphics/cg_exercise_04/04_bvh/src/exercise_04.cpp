#include <cglib/rt/bvh.h>

#include <cglib/rt/triangle_soup.h>

#include <cglib/core/image.h>

/*
 * Create a 1 dimensional normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with size kernel_size elements that
 *                 has to be filled with the kernel values
 *
 */
void Image::create_gaussian_kernel_1d(
        float sigma,
        int kernel_size,
        float* kernel)
{
    cg_assert(kernel_size % 2 == 1);

    // TODO: calculate filter values as described on the exercise sheet.
    // Make sure your kernel is normalized
    int x;
    float sum = 0;

    for (int i = 0; i < kernel_size; ++i) {
        x = i - ((kernel_size - 1) / 2);
        kernel[i] = 1.f / (2 * M_PI * sigma * sigma) * exp(-1.f * (x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }

    for (int i = 0; i < kernel_size; i++) {
        kernel[i] /= sum;
    }
}

/*
 * Create a 2 dimensional quadratic and normalized gauss kernel
 *
 * Parameters:
 *  - sigma:       the parameter of the gaussian
 *  - kernel_size: the size of the kernel (has to be odd)
 *  - kernel:      an array with kernel_size*kernel_size elements that
 *                 has to be filled with the kernel values
 */
void Image::create_gaussian_kernel_2d(
        float sigma,
        int kernel_size,
        float* kernel)
{
    cg_assert(kernel_size%2==1);

    // TODO: calculate filter values as described on the exercise sheet.
    // Make sure your kernel is normalized
    int x, y;
    float sum = 0;

    for (int j = 0; j < kernel_size; ++j) {
        for (int i = 0; i < kernel_size; ++i) {
            x = i - ((kernel_size - 1) / 2);
            y = j - ((kernel_size - 1) / 2);
            kernel[i + j*kernel_size] = 1.f / (2 * M_PI * sigma * sigma) * exp(-1.f * (x * x + y * y) / (2 * sigma * sigma));
            sum += kernel[i + j*kernel_size];
        }
    }

    for (int i = 0; i < kernel_size * kernel_size; i++) {
        kernel[i] /= sum;
    }
}

/*
 * Convolve an image with a 2d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 2d-kernel
 *  - kernel:      the 2d-kernel with kernel_size*kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating
 *                 textures correctly
 */
std::shared_ptr<Image> Image::filter(int kernel_size, float* kernel, WrapMode wrap_mode) const
{
    cg_assert (kernel_size % 2 == 1 && "kernel size should be odd.");
    cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");

    std::shared_ptr<Image> dst_image = std::make_shared<Image>(m_width, m_height);
    glm::vec4 pixel;
    int half_kernel_size = kernel_size / 2;
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            pixel = glm::vec4(0.f, 0.f, 0.f, 0.f);

            for (int y_k = 0; y_k < kernel_size; ++y_k)
            {
                for (int x_k = 0; x_k < kernel_size; ++x_k)
                {
                    pixel += getPixel(y + y_k - half_kernel_size,
                                      x + x_k - half_kernel_size, wrap_mode
                                      ) * kernel[y_k * kernel_size + x_k];
                }
            }
            pixel[3] = 1.f;
            dst_image->setPixel(x, y, pixel);
        }
    }

    return dst_image;
}

/*
 * Convolve an image with a separable 1d filter kernel
 *
 * Parameters:
 *  - kernel_size: the size of the 1d kernel
 *  - kernel:      the 1d-kernel with kernel_size elements
 *  - wrap_mode:   needs to be known to handle repeating
 *                 textures correctly
 */
std::shared_ptr<Image> Image::filter_separable(int kernel_size, float* kernel, WrapMode wrap_mode) const
{
    cg_assert (kernel_size % 2 == 1 && "kernel size should be odd.");
    cg_assert (kernel_size > 0 && "kernel size should be greater than 0.");

    std::shared_ptr<Image> dst_image = std::make_shared<Image>(m_width, m_height);
    std::shared_ptr<Image> mid_image = std::make_shared<Image>(m_width, m_height);
    // TODO: realize the 2d convolution with two
    // convolutions of the image with a 1d-kernel.
    // convolve the image horizontally and then convolve
    // the result vertically (or vise-versa).
    //
    // use the methods getPixel(x, y, wrap_mode) and
    // setPixel(x, y, value) to get and set pixels of an image
    glm::vec4 pixel;

    int half_kernel_size = kernel_size / 2;

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            pixel = glm::vec4(0.f, 0.f, 0.f, 0.f);

            for (int x_k = 0; x_k < kernel_size; x_k++)
            {
                pixel += getPixel(x + x_k - half_kernel_size, y, wrap_mode) * kernel[x_k];
            }

            pixel[3] = 1.f;
            mid_image->setPixel(x, y, pixel);
        }
    }

    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            pixel = glm::vec4(0.f, 0.f, 0.f, 0.f);

            for (int y_k = 0; y_k < kernel_size; y_k++)
            {
                pixel += mid_image->getPixel(x, y + y_k - half_kernel_size, wrap_mode) * kernel[y_k];
            }

            pixel[3] = 1.f;
            dst_image->setPixel(x, y, pixel);
        }
    }

    return dst_image;
}

/**
 * Reorder triangle indices in the vector triangle_indices
 * in the range [first_triangle_idx, first_triangle_idx+num_triangles-1]
 * so that the range is split in two sets where all triangles in the first set
 * are "less than equal" than the median, and all triangles in the second set
 * are "greater than equal" the median.
 *
 * Ordering ("less than") is defined by the ordering of triangle
 * bounding box centers along the given axis.
 *
 * Triangle indices within a set need not be sorted.
 *
 * The resulting sets must have an equal number of elements if num_triangles
 * is even. Otherwise, one of the sets must have one more element.
 *
 * For example, 8 triangles must be split 4-4. 7 Triangles must be split
 * 4-3 or 3-4.
 *
 * Parameters:
 *  - first_triangle_idx: The index of the first triangle in the given range.
 *  - num_triangles:      The number of triangles in the range.
 *  - axis:               The sort axis. 0 is X, 1 is Y, 2 is Z.
 *
 * Return value:
 *  - The number of triangles in the first set.
 */
int BVH::reorder_triangles_median(
        int first_triangle_idx,
        int num_triangles,
        int axis)
{
    cg_assert(first_triangle_idx < static_cast<int>(triangle_indices.size()));
    cg_assert(first_triangle_idx >= 0);
    cg_assert(num_triangles <= static_cast<int>(triangle_indices.size() - first_triangle_idx));
    cg_assert(num_triangles > 1);
    cg_assert(axis >= 0);
    cg_assert(axis < 3);

    // Implement reordering.
    // Sorting in ascending order of AABB-s of the triangles along the axis
    //using lambda function for compare [capture-list](params){body}
    std::sort(
                first_triangle_idx + triangle_indices.begin(),
                first_triangle_idx + num_triangles + triangle_indices.begin(),
                [&axis, this](int i, int j)
    {

        auto max1 = std::max(std::max(triangle_soup.vertices[(i * 3)][axis],
                             triangle_soup.vertices[(i * 3) + 1][axis]),
                triangle_soup.vertices[(i * 3) + 2][axis]);

        auto min1 = std::min(std::min(triangle_soup.vertices[(i * 3)][axis],
                             triangle_soup.vertices[(i * 3) + 1][axis]),
                triangle_soup.vertices[(i * 3) + 2][axis]);

        auto max2 = std::max(std::max(triangle_soup.vertices[(j * 3)][axis],
                             triangle_soup.vertices[(j * 3) + 1][axis]),
                triangle_soup.vertices[(j * 3) + 2][axis]);

        auto min2 = std::min(std::min(triangle_soup.vertices[(j * 3)][axis],
                             triangle_soup.vertices[(j * 3) + 1][axis]),
                triangle_soup.vertices[(j * 3) + 2][axis]);

        return max1 + min1 < max2 + min2;

    });

    return num_triangles / 2;
}

/*
 * Build a BVH recursively using the object median split heuristic.
 *
 * This method must first fully initialize the current node, and then
 * potentially split it.
 *
 * A node must not be split if it contains MAX_TRIANGLES_IN_LEAF triangles or
 * less. No leaf node may be empty. All nodes must have either two or no
 * children.
 *
 * Use reorder_triangles_median to perform the split in triangle_indices.
 * Split along X for depth 0. Then, proceed in the order Y, Z, X, Y, Z, X, Y, ..
 *
 * Parameters:
 *  - node_idx:           The index of the node to be split.
 *  - first_triangle_idx: An index into the array triangle_indices. It points
 *                        to the first triangle contained in the current node.
 *  - num_triangles:      The number of triangles contained in the current node.
 */
void BVH::
build_bvh(int node_idx, int first_triangle_idx, int num_triangles, int depth)
{
    cg_assert(num_triangles > 0);
    cg_assert(node_idx >= 0);
    cg_assert(node_idx < static_cast<int>(nodes.size()));
    cg_assert(depth >= 0);

    Node& node = nodes[node_idx];

    // Implement recursive build.

    node.triangle_idx = first_triangle_idx;
    node.num_triangles = num_triangles;
    node.aabb.min = glm::vec3(FLT_MAX);
    node.aabb.max = glm::vec3(-FLT_MAX);

    //node with more triangles than MAX_TRIANGLES_IN_LEAF constant

    if (num_triangles > BVH::MAX_TRIANGLES_IN_LEAF)
    {
        int triangles = reorder_triangles_median(first_triangle_idx, num_triangles, (depth % 3));

        // building the left node
        nodes.push_back(Node());
        node.left = nodes.size() - 1;
        build_bvh(node.left, first_triangle_idx, triangles, depth + 1);

        // building the right node
        nodes.push_back(Node());
        node.right = nodes.size() - 1;
        build_bvh(node.right, first_triangle_idx + triangles, num_triangles - triangles, depth + 1);

        //settings up new min/max
        node.aabb.min = glm::vec3(
                    std::min(nodes[node.left].aabb.min[0], nodes[node.right].aabb.min[0]),
                std::min(nodes[node.left].aabb.min[1], nodes[node.right].aabb.min[1]),
                std::min(nodes[node.left].aabb.min[2], nodes[node.right].aabb.min[2]));

        node.aabb.max = glm::vec3(
                    std::max(nodes[node.left].aabb.max[0], nodes[node.right].aabb.max[0]),
                std::max(nodes[node.left].aabb.max[1], nodes[node.right].aabb.max[1]),
                std::max(nodes[node.left].aabb.max[2], nodes[node.right].aabb.max[2]));
        return;
    }

    else
    {
        node.left = -1;
        node.right = -1;

        for (int axis = 0; axis < 3; ++axis)
            for (int t = first_triangle_idx; t < first_triangle_idx + num_triangles; t++)
                for (int v = 0; v < 3; v++)
                {
                    if (node.aabb.max[axis] < triangle_soup.vertices[v + triangle_indices[t] * 3][axis])
                        node.aabb.max[axis] = triangle_soup.vertices[v + triangle_indices[t] * 3][axis];

                    if (node.aabb.min[axis] > triangle_soup.vertices[v + triangle_indices[t] * 3][axis])
                        node.aabb.min[axis] = triangle_soup.vertices[v + triangle_indices[t] * 3][axis];
                }
    }
}

/*
* Intersect the BVH recursively, returning the nearest intersection if
* there is one.
*
* Caution: BVH nodes may overlap.
*
* Parameters:
*  - ray:                  The ray to intersect the BVH with.
*  - idx:                  The node to be intersected.
*  - nearest_intersection: The distance to the intersection point, if an
*                          intersection was found. Must not be changed
*                          otherwise.
*  - isect:                The intersection, if one was found. Must not be
*                          changed otherwise.
*
* Return value:
*  true if an intersection was found, false otherwise.
*/
bool BVH::
intersect_recursive(const Ray &ray, int idx, float *nearest_intersection, Intersection* isect) const
{
    cg_assert(nearest_intersection);
    cg_assert(isect);
    cg_assert(idx >= 0);
    cg_assert(idx < static_cast<int>(nodes.size()));

    const Node &n = nodes[idx];

    // This is a leaf node. Intersect all triangles.
    if(n.left < 0)
    {
        glm::vec3 bary(0.f);
        bool hit = false;
        for(int i = 0; i < n.num_triangles; ++i)
        {
            int x = triangle_indices[n.triangle_idx + i];
            float dist;
            glm::vec3 b;
            if(intersect_triangle(ray.origin, ray.direction,
                                  triangle_soup.vertices[x * 3 + 0],
                                  triangle_soup.vertices[x * 3 + 1],
                                  triangle_soup.vertices[x * 3 + 2],
                                  b, dist))
            {
                hit = true;
                if(dist <= *nearest_intersection)
                {
                    *nearest_intersection = dist;
                    bary = b;
                    cg_assert(x >= 0);
                    if(isect)
                        triangle_soup.fill_intersection(isect, x, *nearest_intersection, bary);
                }
            }
        }

        return hit;
    }
    // This is an inner node. Recurse into child nodes.
    else
    {
        bool left = true;
        bool right = true;

        Intersection isect_l = Intersection();
        Intersection isect_r = Intersection();

        // TODO: Implement recursive traversal here.
        for (int axis = 0; axis < 3; ++axis)
        {
            bool in_box = ray.origin[axis] < nodes[n.left].aabb.min[axis] ||
                    ray.origin[axis] > nodes[n.left].aabb.max[axis];
            bool out_of_eps = std::abs(ray.direction[axis]) < FLT_EPSILON;

            if (in_box && out_of_eps)
            {
                left = false;
                break;
            }
        }
        for (int axis = 0; axis < 3; ++axis)
        {
            bool in_box = ray.origin[axis] > nodes[n.right].aabb.min[axis] ||
                    ray.origin[axis] < nodes[n.right].aabb.max[axis];
            bool out_of_eps = std::abs(ray.direction[axis]) < FLT_EPSILON;

            if (in_box && out_of_eps)
            {
                right = false;
                break;
            }
        }

        if (!left && !right)
            return false;

        //calculating t1,t2 for left
        double near = -FLT_MAX;
        double far = FLT_MAX;
        double t1 = 0.0;
        double t2 = 0.0;

        if (left)
        {
            for (int axis = 0; axis < 3; ++axis)
            {
                t1 = (nodes[n.left].aabb.min[axis] - ray.origin[axis]) / ray.direction[axis];
                t2 = (nodes[n.left].aabb.max[axis] - ray.origin[axis]) / ray.direction[axis];

                if (t1 > t2)
                {
                    double tmp = t2;
                    t2 = t1;
                    t1 = tmp;
                }
                if (t1 > near) near = t1;
                if (t2 < far) far = t2;
            }
        }
        //when there is no intersection
        if (near > far || far < 0) left = false;

        //calculating t1,t2 for right
        near = -FLT_MAX;
        far = FLT_MAX;
        if (right) {
            for (int axis = 0; axis < 3; ++axis)
            {
                t1 = (nodes[n.right].aabb.min[axis] - ray.origin[axis]) / ray.direction[axis];
                t2 = (nodes[n.right].aabb.max[axis] - ray.origin[axis]) / ray.direction[axis];

                if (t1 > t2)
                {
                    float tmp = t2;
                    t2 = t1;
                    t1 = tmp;
                }

                if (t1 > near)
                    near = t1;

                if (t2 < far)
                    far = t2;
            }
        }
        //when there is no intersection
        if (near > far || far < 0)
            right = false;

        //no intersection
        if (!left && !right)
            return false;

        //intersection
        float nearest_l = FLT_MAX;
        float nearest_r = FLT_MAX;

        if (left)
        {
            left = intersect_recursive(ray, n.left, &nearest_l, &isect_l);
        }

        if (right)
        {
            right = intersect_recursive(ray, n.right, &nearest_r, &isect_r);
        }

        if (left && right)
        {
            if (nearest_l > nearest_r)
            {
                *nearest_intersection = nearest_r;
                *isect = isect_r;
            }
            else
            {
                *nearest_intersection = nearest_l;
                *isect = isect_l;
            }
            return true;
        }
        else if (left)
        {
            *nearest_intersection = nearest_l;
            *isect = isect_l;
            return true;
        }
        else if (right)
        {
            *nearest_intersection = nearest_r;
            *isect = isect_r;
            return true;
        }
    }

    return false;
}
