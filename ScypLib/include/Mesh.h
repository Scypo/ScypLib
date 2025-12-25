#pragma once
#include"ScypLib/Vec3.h"
#include"ScypLib/Vec2.h"
#include<vector>

namespace sl
{
	class Mesh
	{
	public:
		Mesh(unsigned int vao, unsigned int vbo, unsigned int ibo)
			: vao(vao), vbo(vbo), ibo(ibo) {}
		unsigned int vao;
		unsigned int vbo;
		unsigned int ibo;
	};
}