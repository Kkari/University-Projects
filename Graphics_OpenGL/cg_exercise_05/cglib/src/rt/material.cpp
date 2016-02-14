#include <cglib/rt/material.h>
#include <cglib/rt/intersection.h>

void MaterialSample::
evaluate(
	Material const& material, 
	Intersection const& isect)
{
	k_d    = glm::vec3(material.k_d->evaluate(isect.uv, isect.dudv));
	k_s    = glm::vec3(material.k_s->evaluate(isect.uv, isect.dudv));
	k_r    = glm::vec3(material.k_r->evaluate(isect.uv, isect.dudv));
	k_t    = glm::vec3(material.k_t->evaluate(isect.uv, isect.dudv));
	normal = glm::vec3(material.normal->evaluate(isect.uv, isect.dudv));
	normal = glm::normalize(glm::vec3(2.f*normal[0]-1.f, normal[2], 2.f*normal[1]-1.f));

	k_a = 0.1f * k_d; // simple ambient term
	eta = material.eta;
	n = material.n;

	auto sum = k_s + k_d + k_r + k_t + k_a;
	for (int i = 0; i < 3; ++i)
	{
		if (sum[i] > 1.f) {
			k_s[i] /= sum[i];
			k_d[i] /= sum[i];
			k_r[i] /= sum[i];
			k_t[i] /= sum[i];
			k_a[i] /= sum[i];
		}
	}

}
