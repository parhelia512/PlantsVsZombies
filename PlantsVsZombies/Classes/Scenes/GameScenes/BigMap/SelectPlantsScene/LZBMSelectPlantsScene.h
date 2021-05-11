﻿/**
 *Copyright (c) 2021 LZ.All Right Reserved
 *Author : LZ
 *Date: 2021.2.14
 *Email: 2117610943@qq.com
 */
#pragma once
#include "Scenes/GameScenes/Adventure/SelectPlantsScene/LZASelectPlantsScene.h"

class BMSelectPlantsScene :public SelectPlantsScene
{
public:
	CREATE_FUNC(BMSelectPlantsScene);

CC_CONSTRUCTOR_ACCESS:
	BMSelectPlantsScene();
	~BMSelectPlantsScene();
	virtual bool init() override;

protected:
	virtual void replaceScene() override;
	virtual void createBackgroundLayer() override;
	virtual void controlShowRequirement() override;
	virtual void selectPlantsCallBack() override;
	virtual void eventUpdate(SPSEventType eventType) override;
};