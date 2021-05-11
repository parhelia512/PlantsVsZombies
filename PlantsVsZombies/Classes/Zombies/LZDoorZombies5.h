﻿/**
 *Copyright (c) 2021 LZ.All Right Reserved
 *Author : LZ
 *Date: 2021.4.21
 *Email: 2117610943@qq.com
 */

#pragma once
#include "LZDoorZombies.h"

class DoorZombies5 :public DoorZombies
{
public:
	static DoorZombies5* create(Node* node = nullptr);
	virtual void createZombie() override;
	virtual void createPreviewZombie() override;

CC_CONSTRUCTOR_ACCESS:
	DoorZombies5(Node* node = nullptr);

protected:
	virtual void zombieInjuredEventUpdate() override;
};