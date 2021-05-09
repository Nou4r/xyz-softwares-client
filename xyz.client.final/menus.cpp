#include "menus.h"
#include <iostream>
#include <algorithm> 
#include <thread>
#include <vector>
#include "inject\mmi.h"
#include "protect\protection.h"
#include "bytes\prv_script.hpp"
#include "bytes\prv_spoofer.hpp"
#include "encrypt/md5.h"
#include "other\bytetofile.hpp"
#include "hwid.h"

#define COLOUR(x) x/255 
#define CENTER(width) ((ImGui::GetWindowSize().x - width) * 0.5f)
#ifdef _DEBUG
#define DEBUGLOG(msg) std::cout << msg << std::endl;   
#else
#define DEBUGLOG(msg)
#endif 

subscription sub;
std::vector< std::string > products;
std::vector< std::string > expires;

#pragma warning(disable : 4996)

#include <random>
#include <string>

template< typename ... Args >
std::string mixer(Args const& ... args)
{
	std::ostringstream stream;
	using List = int[];
	(void)List {
		0, ((void)(stream << args), 0) ...
	};
	return stream.str();
}
std::string random_string(std::string::size_type length)
{
	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	thread_local static std::mt19937 rg{ std::random_device{}() };
	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--)
		s += chrs[pick(rg)];

	return s;
}
inline bool FileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

int Menu::DLL(std::string filePath) {
	std::string targetProcess = _xor_("RuntimeBroker.exe");

	if (FileExists(filePath)) {
		Inject(filePath, targetProcess);
	}
}

Menu::Menu() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ImVec4(COLOUR(22.0f), COLOUR(24.0f), COLOUR(29.0f), 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(COLOUR(30.0f), COLOUR(31.0f), COLOUR(38.0f), 1.f);
	style.Colors[ImGuiCol_Border] = ImVec4(COLOUR(22.0f), COLOUR(24.0f), COLOUR(29.0f), 0.9f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(COLOUR(50.0f), COLOUR(50.0f), COLOUR(50.0f), 1.f);
	style.Colors[ImGuiCol_Button] = ImVec4(COLOUR(67.0f), COLOUR(38.0f), COLOUR(235.0f), 1.f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f);
	style.Colors[ImGuiCol_Header] = ImVec4(COLOUR(5.0f), COLOUR(116.0f), COLOUR(203.0f), 1.f);
	style.WindowRounding = 0.0f;
	style.FrameRounding = 2.f;
}

void Menu::mainMenu() {
	ImGui::SetNextWindowPos(ImVec2{ 0, 0 }, ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2{ ZER0_WIDTH,ZER0_HEIGHT }, ImGuiCond_Once);

	if (ImGui::Begin("\n", &AppOpen,
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoFocusOnAppearing))
	{
		if (state == state::loading) {
			loading();
		}
		else if (state == state::login) {
			login(state);
		}
		else if (state == state::cheats) {
			cheats();
		}
	}
	ImGui::End();
}

void Menu::login(int& loggedIn)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
	ImGui::PushFont(smallFont);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
		{
			if (my_texture) ImGui::Image((void*)my_texture, image_size);
		}
		ImGui::PopStyleVar();

		ImGui::SetCursorPosX(60);
		ImGui::Text(_xor_("Username: ").c_str());

		ImGui::SetCursorPosX(60);
		ImGui::InputText("", usernameBuf, sizeof(usernameBuf));

		ImGui::SetCursorPosX(60);
		ImGui::Text(_xor_("Password: ").c_str());

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 20));
		{
			ImGui::PushID(999);
			{
				ImGui::SetCursorPosX(60);
				ImGui::InputText("", passwordBuf, sizeof(passwordBuf), ImGuiInputTextFlags_Password);
			}
			ImGui::PopID();

			static bool rem = false;
			ImGui::SetCursorPosX(60);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 1);
			{
				ImGui::Checkbox(_xor_(" Remember Me").c_str(), &rem);
			}

			ImGui::PopStyleVar();
			ImGui::SetCursorPosX(CENTER(120));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
			{
				if (ImGui::Button(_xor_("Login").c_str(), ImVec2(120, 40))) {
					std::thread t = std::thread([this] {this->doLogin(); });
					t.detach();
				}
			}
			ImGui::PopStyleVar();

			// Login error message
			if (loginError) ImGui::OpenPopup(_xor_("Error").c_str());
			if (ImGui::BeginPopupModal(_xor_("Error").c_str(), &loginError, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
			{
				ImGui::Text(_xor_("Username or password invalid!").c_str());
				ImGui::EndPopup();
			}
		}
		ImGui::PopStyleVar();
	}
	ImGui::PopFont();
	ImGui::PopStyleVar();
}

void Menu::cheats() {
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);

	ImGui::PushFont(smallFont);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	{
		ImGui::SetCursorPosX(CENTER(image_size.x));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
		{
			if (my_texture) {
				ImGui::Image((void*)my_texture, image_size);
			}
		}
		ImGui::PopStyleVar();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 20);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);

		ImGui::SetCursorPosX(65);
		ImGui::Text(_xor_("Your subscriptions:").c_str());

		ImGui::SetCursorPosX(65);

		static bool hasSubscription = false;
		if (ImGui::ListBoxHeader(_xor_("##subs").c_str(), subscriptions.size(), 5))
		{
			size_t prodSize = std::size(products);

			if (prodSize > 0) {
				for (int i = 0; i < prodSize; i++) {
					if (ImGui::Selectable(products[i].c_str(), currentCheat == i)) {
						currentCheat = i;
					}
				}
				hasSubscription = true;
			}
			else {
				ImGui::Selectable(_xor_("No Subscriptions").c_str(), currentCheat == 0);
				hasSubscription = false;
			}
			ImGui::ListBoxFooter();
		}

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 7);

		if (hasSubscription) {
			ImGui::SetCursorPosX(115);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
			{
				if (ImGui::Button(_xor_("Launch").c_str(), ImVec2(120, 40))) {
					state = state::loading;
					std::thread t = std::thread([this] {
						std::string randDLL = mixer(random_string(32), _xor_(".dll"));
						std::string tempPath = getenv(_xor_("temp").c_str());
						std::string finalPath = mixer(tempPath, _xor_("\\"), randDLL);

						if (products[currentCheat] == "Spoofer") {
							utils::CreateFileFromMemory(finalPath, reinterpret_cast<const char*>(spoofer_final), sizeof(spoofer_final));
						}
						else if (products[currentCheat] == "Script") {
							utils::CreateFileFromMemory(finalPath, reinterpret_cast<const char*>(private_bytes), sizeof(private_bytes));
						}

						Menu::DLL(finalPath);
						});
					t.detach();
				}
			}
			ImGui::PopStyleVar();
			ImGui::SetCursorPosX(120);
			static std::string e1 = _xor_("Expires in ");  static std::string e2 = _xor_(" hours");
			std::string test = (e1 + expires.at(currentCheat).c_str() + e2).c_str();
			ImGui::Text(test.c_str());
		}
		else {
			ImGui::SetCursorPosX(115);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
			{
				if (ImGui::Button(_xor_("Store").c_str(), ImVec2(120, 40))) {
					//TODO setup purchase link on click
				}
			}
			ImGui::PopStyleVar();
		}
	}
	ImGui::PopFont();

	ImGui::PopStyleVar();
}

void Menu::loading() {
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 25);
	ImGui::SetCursorPosX(CENTER(image_size.x));
	ImGui::PushFont(mediumFont);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 35));
	{
		if (my_texture) {
			ImGui::Image((void*)my_texture, image_size);
		}

		ImGui::SetCursorPosX(CENTER(45) - 22.5f);

		ImGui::SetCursorPosX(CENTER(ImGui::CalcTextSize(loadingType.c_str()).x));
		ImGui::Text(loadingType.c_str());
	}
	ImGui::PopFont();
	ImGui::PopStyleVar();
}

void Menu::doLogin() {
	std::string successMsg = _xor_("SUCCESS_LOGIN");
	std::string failedMsg = _xor_("FAILED_LOGIN");

	std::string sendInfo = m_Client->sendrecieve(mixer(usernameBuf, _xor_(";"), passwordBuf, _xor_(";"), getHWinfo64()));

	if (sendInfo.size() == successMsg.size()) {
		std::string usersubscriptions = m_Client->sendrecieve(_xor_("REQUEST_SUBS"));
		this->subscriptions.clear();

		std::stringstream ss(usersubscriptions); std::string token;
		while (std::getline(ss, token, '|')) {
			std::stringstream ss2(token); std::string token2;

			sub.products = ss2.str();
			size_t n = std::count(sub.products.begin(), sub.products.end(), ',');

			for (int i = 0; i < n; i++) {
				std::getline(ss2, sub.products, ',');
				products.push_back(sub.products.substr(0, sub.products.find(_xor_(";"))).data());
				expires.push_back(sub.products.substr(sub.products.find(_xor_(";")) + 1).data());
			}

			std::getline(ss2, sub.lenght, ',');
			this->subscriptions.push_back(sub);
		}
		this->state = state::cheats;
	}
	if (sendInfo.size() == failedMsg.size()) {
		this->state == state::login;
		loginError = true;
	}
}

std::unique_ptr<Menu> m_Menu;