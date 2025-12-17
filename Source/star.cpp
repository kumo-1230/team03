#include "star.h"

void Star::Initialize(int type)
{
	model = std::move(models[type]);
	nowType = type;
	//position = { 0,0,0 };
	radius = 150;
}

Star::Star(int type)
{
	Initialize(type);
}

Star::~Star()
{
}
