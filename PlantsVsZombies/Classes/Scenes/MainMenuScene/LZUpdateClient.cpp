﻿/**
 *Copyright (c) 2020 LZ.All Right Reserved
 *Author : LZ
 *Date: 2020.4.2
 *Email: 2117610943@qq.com
 */

#include "LZUpdateClient.h"
#include "spine/spine-cocos2dx.h"
#include "Based/LZPlayMusic.h"
#include "Based/LZMouseEventControl.h"

#include <stdio.h>

using namespace spine;

UpdateClient::UpdateClient() :
  _dialog(nullptr)
, _remindText(nullptr)
, _progressText(nullptr)
, _explanText(nullptr)
, _loadBarBackground(nullptr)
, _loadingBar(nullptr)
, _historyText(nullptr)
, _isNewDowndload(true)
{
	_downloader.reset(new network::Downloader());
}

bool UpdateClient::init()
{
	if (!LayerColor::initWithColor(Color4B(0, 0, 0, 180)))return false;
	createShieldLayer(this);

	createDiglog();
	return true;
}

void UpdateClient::createDiglog()
{
	_dialog = Sprite::createWithSpriteFrameName("LevelObjiectivesBg.png");
	_dialog->setPosition(_director->getWinSize() / 2);
	_dialog->setScale(0.9f);
	this->addChild(_dialog);

	auto PauseAnimation = SkeletonAnimation::createWithData(_global->userInformation->getAnimationData().find("PauseAnimation")->second);
	PauseAnimation->setAnimation(0, "animation", true);
	PauseAnimation->setPosition(Vec2(530, 650));
	_dialog->addChild(PauseAnimation);

	showText();

	createButton(_global->userInformation->getGameText().find("下载器下载")->second->text, Vec2(165, 100), Update_Button::下载器下载);
	createButton(_global->userInformation->getGameText().find("直接下载")->second->text, Vec2(405, 100), Update_Button::直接下载);
	createButton(_global->userInformation->getGameText().find("网站下载")->second->text, Vec2(645, 100), Update_Button::网站下载);
	//createButton(_global->userInformation->getGameText().find("百度网盘下载")->second->text, Vec2(645, 100), Update_Button::百度网盘下载);
	//createButton(_global->userInformation->getGameText().find("腾讯微云下载")->second, Vec2(405, 100), Update_Button::腾讯微云下载);
	createButton(_global->userInformation->getGameText().find("关闭游戏")->second->text, Vec2(885, 100), Update_Button::退出游戏);
	createButton(_global->userInformation->getGameText().find("确定")->second->text, Vec2(520, 100), Update_Button::确定);

	/* 创建触摸监听 */
	createTouchtListener(_dialog);

	UserDefault::getInstance()->setStringForKey("EDITION", UserInformation::getNewEditionName());
}

void UpdateClient::createButton(const std::string& name, Vec2& vec2, Update_Button buttonType)
{
	/* 创建返回主菜单按钮 */
	auto button = ui::Button::create("ButtonNew2.png", "ButtonNew.png", "",cocos2d::ui::Widget::TextureResType::PLIST);
	auto label = Label::createWithTTF(name, GAME_FONT_NAME_1, _global->userInformation->getGameText().find("确定")->second->fontsize);
	label->enableShadow(Color4B(0, 0, 0, 200));//设置阴影
	label->setScale(2.0f);
	button->setTitleLabel(label);
	button->setTitleColor(Color3B::WHITE);
	button->setPosition(vec2);
	button->setScale(0.5f);
	button->setName(to_string(static_cast<int>(buttonType)));
	_dialog->addChild(button);

	if (buttonType == Update_Button::确定)
	{
		button->setVisible(false);
	}

	button->addTouchEventListener([=](Ref* sender, ui::Widget::TouchEventType type)
		{
			switch (type)
			{
			case ui::Widget::TouchEventType::BEGAN:
				PlayMusic::playMusic("gravebutton");
				break;
			case ui::Widget::TouchEventType::ENDED:
				switch (buttonType)
				{
				case Update_Button::百度网盘下载:
					Application::getInstance()->openURL(_global->userInformation->getGameText().find("百度网盘网址")->second->text);
					break;
				case Update_Button::腾讯微云下载:
					Application::getInstance()->openURL(_global->userInformation->getGameText().find("腾讯微云网址")->second->text);
					break;
				case Update_Button::下载器下载:
					ShellExecute(NULL, L"open", L"PVZUpdate.exe", NULL, NULL, SW_SHOWNORMAL);
					Director::getInstance()->end();
					break;
				case Update_Button::直接下载:
					_remindText->setString(_global->userInformation->getGameText().find("正在连接中，请稍等！")->second->text);
					button->setEnabled(false);
					((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::退出游戏))))->setEnabled(false);
					((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::下载器下载))))->setEnabled(false);
					if (_textScrollView) 
					{
						_textScrollView->removeFromParent();
						_textScrollView = nullptr;
					}
					downloadData();
					break;
				case Update_Button::网站下载:
					Application::getInstance()->openURL(_global->userInformation->getGameText().find("官方网址")->second->text);
					break;
				case Update_Button::退出游戏:
					Director::getInstance()->end();
					break;
				case Update_Button::确定:
					ShellExecute(NULL, L"open", stringToWstring(_fileName), NULL, NULL, SW_SHOWNORMAL);
					Director::getInstance()->end();
					break;
				}
			}
		});
}

void UpdateClient::showText()
{
	addScrollView();
	_historyText = Label::createWithTTF(_global->userInformation->getGameText().find("更新信息加载中！")->second->text, GAME_FONT_NAME_1, 
		_global->userInformation->getGameText().find("更新信息加载中！")->second->fontsize);
	_historyText->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	_historyText->setColor(Color3B::BLACK);
	_historyText->setMaxLineWidth(650); 
	_textScrollView->addChild(_historyText);
	_textScrollView->setInnerContainerSize(_historyText->getContentSize());
	_historyText->setPosition(Vec2(_dialog->getContentSize().width / 2.f - 150, _textScrollView->getInnerContainerSize().height - 150));
	downloadHistoryText();

	/* 标题 */
	_remindText = Label::createWithTTF(_global->userInformation->getGameText().find("检测到有新版本，请选择更新方式！")->second->text, GAME_FONT_NAME_1, 
		_global->userInformation->getGameText().find("检测到有新版本，请选择更新方式！")->second->fontsize);
	_remindText->setPosition(Vec2(_dialog->getContentSize().width / 2.f, _dialog->getContentSize().height / 2.f + 200));
	_remindText->setColor(Color3B::BLACK);
	_remindText->setMaxLineWidth(900);
	_remindText->setName("Update");
	_dialog->addChild(_remindText);

	/* 进度文字 */
	_progressText = Label::createWithTTF("", GAME_FONT_NAME_1, 25);
	_progressText->setMaxLineWidth(900);
	_progressText->setPosition(Vec2(_dialog->getContentSize().width / 2.f, _dialog->getContentSize().height / 2.f));
	_dialog->addChild(_progressText);

	/* 说明文字 */
    _explanText = Label::createWithTTF("", GAME_FONT_NAME_1, 30);
	_explanText->setPosition(Vec2(_dialog->getContentSize().width / 2.f, _dialog->getContentSize().height / 2.f + 100));
	_explanText->setColor(Color3B::BLACK);
	_explanText->setMaxLineWidth(900);
	_dialog->addChild(_explanText);
}

void UpdateClient::addScrollView()
{
	_textScrollView = ui::ScrollView::create();
	_textScrollView->setDirection(ui::ScrollView::Direction::VERTICAL);
	_textScrollView->setAnchorPoint(Vec2::ANCHOR_MIDDLE);
	_textScrollView->setContentSize(Size(720.0f, 320.0f));
	_textScrollView->setPosition(_dialog->getContentSize() / 2.0f);
	_textScrollView->setBounceEnabled(true);
	_textScrollView->setScrollBarPositionFromCorner(Vec2(20, 0));
	_textScrollView->setScrollBarWidth(10);
	_textScrollView->setScrollBarColor(Color3B::BLACK);
	_dialog->addChild(_textScrollView);
	addMouseEvent();
}

void UpdateClient::addMouseEvent()
{
	/* 鼠标滑动监听 */
	auto mouse = EventListenerMouse::create();
	mouse->onMouseScroll = [=](Event* event)
	{
		const auto mouseEvent = static_cast<EventMouse*>(event);
		const float movex = mouseEvent->getScrollY() * 5;

		MouseEventControl::mouseScrollControlListener(_textScrollView, movex, ui::ScrollView::Direction::VERTICAL);
	};
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mouse, _textScrollView);
}

void UpdateClient::downloadHistoryText()
{
	const string sURLList = _global->userInformation->getGameText().find("更新信息网址")->second->text;
	_downloader->createDownloadDataTask(sURLList);
	_downloader->onDataTaskSuccess = [this](const cocos2d::network::DownloadTask& task,
		std::vector<unsigned char>& data)
	{
		string historyNetWork;
		for (auto p : data)
		{
			historyNetWork += p;
		}

		TTFConfig ttfConfig(GAME_FONT_NAME_1, 25, GlyphCollection::DYNAMIC);
		_historyText->setTTFConfig(ttfConfig);
		_historyText->setString(historyNetWork);
		_textScrollView->setInnerContainerSize(_historyText->getContentSize());
		_historyText->setPosition(Vec2(350, _textScrollView->getInnerContainerSize().height));
	};
	_downloader->onTaskError = [this](const cocos2d::network::DownloadTask& task,
		int errorCode,
		int errorCodeInternal,
		const std::string& errorStr)
	{
		_historyText->setString(_global->userInformation->getGameText().find("更新信息加载失败！")->second->text);
		_textScrollView->setInnerContainerSize(_historyText->getContentSize());
	};
}

void UpdateClient::downloadData()
{
	if (!_loadBarBackground)
	{
		_loadBarBackground = Sprite::createWithSpriteFrameName("bgFile.png");
		_loadBarBackground->setPosition(Vec2(_dialog->getContentSize().width / 2.f, _dialog->getContentSize().height / 2.f - 100));
		_loadBarBackground->setScale(1.5f);
		_dialog->addChild(_loadBarBackground);
	}

	if (!_loadingBar)
	{
		_loadingBar = ui::LoadingBar::create();
		_loadingBar->loadTexture("progressFile.png",cocos2d::ui::Widget::TextureResType::PLIST);
		_loadingBar->setDirection(LoadingBar::Direction::LEFT); /* 设置加载方向 */
		_loadingBar->setPercent(0);
		_loadingBar->setScale(1.5f);
		_loadingBar->setPosition(Vec2(_dialog->getContentSize().width / 2.f, _dialog->getContentSize().height / 2.f - 100));
		_dialog->addChild(_loadingBar);
	}

	_explanText->setColor(Color3B::BLACK);
	_explanText->setString("");

	_fileName = _global->userInformation->getGameText().find("资源名称")->second->text + UserInformation::getNewEditionName(true) + ".exe";
	static string path = FileUtils::getInstance()->fullPathForFilename("PlantsVsZombies.exe");
	path = path.replace(path.rfind("PlantsVsZombies.exe"), sizeof("PlantsVsZombies.exe"), "") + _fileName;
	_downloader->createDownloadFileTask(_global->userInformation->getGameText().find("资源网址")->second->text, path, _fileName);
	
	downloadProgress();
	downloadSuccess();
	downloadError();
}

void UpdateClient::downloadProgress()
{
	_downloader->onTaskProgress = [=](const network::DownloadTask& task,
		int64_t bytesReceived,
		int64_t totalBytesReceived,
		int64_t totalBytesExpected)
	{
		if (totalBytesExpected <= 0)
		{
			_loadingBar->setPercent(100);
			_explanText->setString(_global->userInformation->getGameText().find("解释说明_快")->second->text);

			if (bytesReceived / 1024.f * 10 >= 1000)
			{
				_progressText->setString(StringUtils::format(
					_global->userInformation->getGameText().find("%.1fMB/s  [已下载 : %dKB]")->second->text.c_str(),
					float(bytesReceived / 1024.f / 1024.f) * 10, int(totalBytesReceived / 1024)));
			}
			else
			{
				_progressText->setString(StringUtils::format(
					_global->userInformation->getGameText().find("%.1fKB/s  [已下载 : %dKB]")->second->text.c_str(),
					float(bytesReceived / 1024.f) * 10, int(totalBytesReceived / 1024)));
			}
		}
		else
		{
			_explanText->setString(_global->userInformation->getGameText().find("解释说明_慢")->second->text);

			const float percent = float(totalBytesReceived * 100) / totalBytesExpected;
			_loadingBar->setPercent(percent);

			const int hour = (totalBytesExpected - totalBytesReceived) / (bytesReceived * 10) / 3600;
			const int min = ((totalBytesExpected - totalBytesReceived) / (bytesReceived * 10) - hour * 3600) / 60;
			const int second = (totalBytesExpected - totalBytesReceived) / (bytesReceived * 10) - hour * 3600 - min * 60;

			if (bytesReceived / 1024.f * 10 >= 1000)
			{
				_progressText->setString(StringUtils::format(
					_global->userInformation->getGameText().find("%.1fMB/s  %dKB/%dKB  %.2f%%  time:%02d:%02d:%02d")->second->text.c_str(),
					bytesReceived / 1024.f / 1024.f * 10, int(totalBytesReceived / 1024), int(totalBytesExpected / 1024), percent, hour, min, second));
			}
			else
			{
				_progressText->setString(StringUtils::format(
					_global->userInformation->getGameText().find("%.1fKB/s  %dKB/%dKB  %.2f%%  time:%02d:%02d:%02d")->second->text.c_str(),
					bytesReceived / 1024.f * 10, int(totalBytesReceived / 1024), int(totalBytesExpected / 1024), percent, hour, min, second));
			}
		}

		_remindText->setString(_global->userInformation->getGameText().find("文件正在下载中！请稍等！")->second->text);
};
}

void UpdateClient::downloadSuccess()
{
	_downloader->onFileTaskSuccess = [this](const cocos2d::network::DownloadTask& task)
	{
		ShellExecute(NULL, L"open", stringToWstring(_fileName), NULL, NULL, SW_SHOWNORMAL);
		Director::getInstance()->end();
		//_progressText->setString(_global->userInformation->getGameText().find("下载成功")->second->text +
		//	_global->userInformation->getGameText().find("存放路径")->second->text + task.identifier + " ]");
		//_remindText->setString(_global->userInformation->getGameText().find("点击确定退出游戏！")->second->text);
		//_explanText->setString(_global->userInformation->getGameText().find("下载成功说明")->second->text);

		//((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::百度网盘下载))))->setVisible(false);
		//((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::下载器下载))))->setVisible(false);
		//((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::直接下载))))->setVisible(false);
		//((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::退出游戏))))->setVisible(false);
		//((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::确定))))->setVisible(true);
	};
}

void UpdateClient::downloadError()
{
	_downloader->onTaskError = [this](const cocos2d::network::DownloadTask& task,
		int errorCode,
		int errorCodeInternal,
		const std::string& errorStr)
	{
		_remindText->setString(_global->userInformation->getGameText().find("下载失败")->second->text);
		((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::下载器下载))))->setEnabled(true);
		((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::直接下载))))->setEnabled(true);
		((Button*)_dialog->getChildByName(to_string(static_cast<int>(Update_Button::退出游戏))))->setEnabled(true);

		_explanText->setString(StringUtils::format(
			_global->userInformation->getGameText().find("错误信息")->second->text.c_str()
			, task.identifier.c_str()
			, errorCode
			, errorCodeInternal
			, errorStr.c_str()));
		_explanText->setColor(Color3B::RED);
#ifdef DEBUG
		log("Failed to download : %s, identifier(%s) error code(%d), internal error code(%d) desc(%s)"
			, task.requestURL.c_str()
			, task.identifier.c_str()
			, errorCode
			, errorCodeInternal
			, errorStr.c_str());
#endif // DEBUG
    };
}

LPCUWSTR UpdateClient::stringToWstring(string fileName)
{
	const size_t size = fileName.length() + 1;
	size_t convertedChars = 0;
	wchar_t* wstr = new wchar_t[fileName.length() - 1];
	mbstowcs_s(&convertedChars, wstr, size, fileName.c_str(), _TRUNCATE);
	return wstr;
}
