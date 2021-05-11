﻿/**
 *Copyright (c) 2021 LZ.All Right Reserved
 *Author : LZ
 *Date: 2021.2.09
 *Email: 2117610943@qq.com
 */
#pragma once
#include "Scenes/GameScenes/Adventure/GameScene/LZAGSGameEndLayer.h"

class HGameEndLayer :public GSGameEndLayer
{
public:
	CREATE_FUNC(HGameEndLayer);
	virtual void successfullEntry();
	virtual void breakThrough();

CC_CONSTRUCTOR_ACCESS:
	HGameEndLayer();
	~HGameEndLayer();
	virtual bool init() override;

protected:
	virtual void rewardThing() override;
	virtual void rewardCoin(Button* button) override;
	virtual void showFailText() override;
};