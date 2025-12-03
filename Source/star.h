#pragma once

#include "System/ModelRenderer.h"
#include "stage.h"

class Player;

class Star : public Stage
{
private:
	std::shared_ptr<Model> models[3] = {
	std::make_shared<Model>("Data/Model/Rock/rock_10.mdl"),
	std::make_shared<Model>("Data/Model/Rock/rock_15.mdl"),
	std::make_shared<Model>("Data/Model/Rock/rock_20.mdl")
	};
private:
 	void Initialize(int type);
public:
	DirectX::XMFLOAT3 getPosition() { return position; }
	void setAngle(DirectX::XMFLOAT3 ang) { angle = ang; }
public:
	Star(int type);
	~Star() override;

};