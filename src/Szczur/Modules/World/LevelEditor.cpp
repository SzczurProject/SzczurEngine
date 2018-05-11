#include "LevelEditor.hpp"

#include <iostream>
#include <experimental/filesystem>

#ifdef OS_WINDOWS
#include <Shellapi.h>
#endif

#include <ImGui/imgui.h>

#include "Szczur/Utility/SFML3D/RenderTarget.hpp"
#include "Szczur/Utility/SFML3D/RectangleShape.hpp"
#include "Szczur/Utility/Convert/Windows1250.hpp"

#include "Szczur/modules/FileSystem/FileDialog.hpp"
#include "Szczur/modules/FileSystem/DirectoryDialog.hpp"

namespace rat {
    LevelEditor::LevelEditor(SceneManager& scenes) :
	_scenes(scenes) {
		_freeCamera.move({1000.f,500.f,2000.f});
    }

    void LevelEditor::render(sf3d::RenderTarget& target) {
		auto* scene = _scenes.getCurrentScene();
		if(scene) {
			_renderBar();
			if(_ifRenderObjectsList)
				_renderObjectsList();
			if(_anySelected)
				_renderFocusedObjectsParams();
			if(_ifRenderArmatureDisplayManager)
				_renderArmatureDisplayManager();
			if(_ifRenderDisplayDataManager)
				_renderDisplayDataManager();

			sf3d::RectangleShape rect({100.f, 100.f});
			rect.setColor({1.f, 1.f, 0.f, 0.2f});
			rect.setOrigin({50.f, 50.f, 0.f});

			glDisable(GL_DEPTH_TEST);
			scene->forEach([&](const std::string& group, Entity& entity){
				rect.setPosition(entity.getPosition());
				if(_focusedObject == entity.getID() && _anySelected) {
					rect.setSize({100.f, 100.f});
					rect.setOrigin({50.f, 50.f, 0.f});
					rect.setColor({1.f, 0.3f, 1.f, 0.4f});
					target.draw(rect);
					rect.setSize({80.f, 80.f});
					rect.setOrigin({40.f, 40.f, 0.f});
					rect.setColor({0.f, 1.f, 1.f, 0.4f});
					target.draw(rect);
					// rect.setOutlineColor({1.f, 1.f, 0.f, 0.8f})
					// rect.setOutlineThickness(2.f);
				}
				else {
					rect.setSize({100.f, 100.f});
					rect.setOrigin({50.f, 50.f, 0.f});
					rect.setColor({0.7f, 0.f, 0.8f, 0.4f});
					target.draw(rect);
					rect.setSize({80.f, 80.f});
					rect.setOrigin({40.f, 40.f, 0.f});
					rect.setColor({1.f, 1.f, 0.f, 0.4f});
					target.draw(rect);
					// rect.setOutlineThickness(0.f);
				}
			});
			glEnable(GL_DEPTH_TEST);
		}
    }

    void LevelEditor::update(InputManager& input, Camera& camera) {
		auto* scene = _scenes.getCurrentScene();
		bool cameraFocused = _focusedObject == scene->getCameraID() && _anySelected;
		if(ImGui::IsAnyItemActive() == false && !cameraFocused)
			_freeCamera.processEvents(input);
		sf3d::View view;
		if(cameraFocused) {
			auto* cam = scene->getEntity("single", scene->getCameraID());
			view.setRotation(cam->getRotation());
			view.setCenter(cam->getPosition());
		}
		else {
			//std::cout << _freeCamera.position.x << ' ' << _freeCamera.position.y << '\n';
			view.setRotation(_freeCamera.rotation);
			view.setCenter(_freeCamera.position);
		}
		camera.setView(view);
    }

	void FreeCamera::processEvents(InputManager& input) {

		velocity = 50.f;
		if(input.isKept(Keyboard::LShift)) {
			velocity = 200.f;
		}

		if(input.isKept(Keyboard::W)) {
			move({
				velocity * glm::sin(glm::radians(rotation.y)),
				0.f,
				-velocity * glm::cos(glm::radians(rotation.y))
			});
		}
		if(input.isKept(Keyboard::S))
			move({
				-velocity * glm::sin(glm::radians(rotation.y)),
				0.f,
				velocity * glm::cos(glm::radians(rotation.y))
			});
		if(input.isKept(Keyboard::D)) {
			move(glm::vec3{
				velocity * glm::cos(glm::radians(rotation.y)),
				0.f,
				velocity * glm::sin(glm::radians(rotation.y))
			});
		}
		if(input.isKept(Keyboard::A)) {
			move(glm::vec3{
				-velocity * glm::cos(glm::radians(rotation.y)),
				0.f,
				-velocity * glm::sin(glm::radians(rotation.y))
			});
		}
		if(input.isKept(Keyboard::Space))
			move({0.f, velocity, 0.f});
		if(input.isKept(Keyboard::LControl))
			move({0.f, -velocity, 0.f});
		if(rotating) {
			auto mouse = input.getMousePosition();
			rotate({
				(mouse.y - previousMouse.y)/10.f,
				(mouse.x - previousMouse.x)/10.f,
				0.f
			});
			previousMouse = mouse;
		}
		if(input.isPressed(Mouse::Right)) {
			rotating = true;
			previousMouse = input.getMousePosition();
		}
		if(input.isReleased(Mouse::Right)) {
			rotating = false;
		}
	}

    void LevelEditor::_renderBar() {
		if(ImGui::BeginMainMenuBar()) {
			if(ImGui::BeginMenu("Files")) {
				if(ImGui::MenuItem("Load")) {
					std::string relative = _getRelativePathFromExplorer("Load world", ".\\Editor\\Saves", "Worlds (*.world)|*.world");
					// std::cout<<"--l-"<<relative<<std::endl;
					if(relative != "") {
						try {
							_scenes.loadFromFile(relative);
                            _focusedObject = -1;
                            _anySelected = false;
						}
						catch(...) {}
					}
				}
				if(ImGui::MenuItem("Save")) {
					std::string relative = _getRelativePathFromExplorer("Save world", ".\\Editor\\Saves", "Worlds (*.world)|*.world", true);
					// std::cout<<"--s-"<<relative<<std::endl;
					if(relative != "") {
						try {
							_scenes.saveToFile(relative);
						}
						catch(...) {}
					}
				}

				if (ImGui::MenuItem("Show in explorer")) {
					std::string current = std::experimental::filesystem::current_path().string();

#ifdef OS_WINDOWS
					ShellExecute(NULL, "open", current.c_str(), NULL, NULL, SW_SHOWDEFAULT);
#endif
				}

				ImGui::Separator();

				if(ImGui::MenuItem("Exit")) {
					std::cout << "Exit\n";
				}
				ImGui::EndMenu();
			}
			if(ImGui::BeginMenu("Tools")) {
				ImGui::MenuItem("Objects List", nullptr, &_ifRenderObjectsList);
				ImGui::MenuItem("Display Data Manager", nullptr, &_ifRenderDisplayDataManager);
				ImGui::MenuItem("Armature Data Manager", nullptr, &_ifRenderArmatureDisplayManager);
				ImGui::EndMenu();
			}
		}
		ImGui::EndMainMenuBar();
	}
    void LevelEditor::_renderDisplayDataManager() {
		static char enteredText[255];
		if(ImGui::Begin("Display Data Manager", &_ifRenderDisplayDataManager)) {
			auto& spriteDisplayDataHolder = _scenes.getCurrentScene()->getSpriteDisplayDataHolder();
			if(ImGui::InputText("", enteredText, 255)) {
			}
			ImGui::SameLine();
			if(ImGui::Button("Add")) {
				try{
					spriteDisplayDataHolder.emplace_back(enteredText);
				}
				catch(const std::exception& exc) {
				}
				for(int i = 0; i<255; ++i)
					enteredText[i] = '\0';
			}
			ImGui::Separator();
			if(ImGui::BeginChild("Datas")) {
				for(auto it = spriteDisplayDataHolder.begin(); it!=spriteDisplayDataHolder.end(); ++it) {
					if(ImGui::SmallButton("-")) {
						spriteDisplayDataHolder.erase(it);
						--it;
						continue;
					}
					ImGui::SameLine();
					ImGui::Text(it->getName().c_str());
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
    void LevelEditor::_renderArmatureDisplayManager() {
		static char enteredText[255];
		if(ImGui::Begin("Armature Data Manager", &_ifRenderArmatureDisplayManager)) {
			auto& armatureDisplayDataHolder = _scenes.getCurrentScene()->getArmatureDisplayDataHolder();
			if(ImGui::InputText("", enteredText, 255)) {
			}
			ImGui::SameLine();
			if(ImGui::Button("Add")) {
				try{
					armatureDisplayDataHolder.emplace_back(enteredText);
				}
				catch(const std::exception& exc) {
				}
				for(int i = 0; i<255; ++i)
					enteredText[i] = '\0';
			}
			ImGui::Separator();
			if(ImGui::BeginChild("Datas")) {
				for(auto it = armatureDisplayDataHolder.begin(); it!=armatureDisplayDataHolder.end(); ++it) {
					if(ImGui::SmallButton("-")) {
						armatureDisplayDataHolder.erase(it);
						--it;
						continue;
					}
					ImGui::SameLine();
					ImGui::Text(mapWindows1250ToUtf8(it->getName()).c_str());
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
	}
    void LevelEditor::_renderFocusedObjectsParams() {
		if(ImGui::Begin("Object Parameters", &_anySelected)) {
			auto* scene = _scenes.getCurrentScene();
			Entity* focusedObject = scene->getEntity(_focusedObject);

			// Change components
			if(ImGui::Button("Change components...")) {
				ImGui::OpenPopup("Change components...##modal");
				ImGui::SetNextWindowSize(ImVec2(300,300));
			}
			_renderComponentsManager();

			if(focusedObject) {
				if(ImGui::CollapsingHeader("Base", ImGuiTreeNodeFlags_DefaultOpen)) {
					glm::vec3 position = focusedObject->getPosition();
					glm::vec3 origin = focusedObject->getOrigin();
					origin.y = -origin.y;
					glm::vec3 rotation = focusedObject->getRotation();
					glm::vec3 scale = focusedObject->getScale();
					char name[255];
					std::strcpy(&name[0], focusedObject->getName().c_str());
					ImGui::InputText("", name, 255);
					focusedObject->setName(name);
					ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position));
					ImGui::DragFloat3("Origin", reinterpret_cast<float*>(&origin));
					ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&rotation));
					ImGui::DragFloat2("Scale", reinterpret_cast<float*>(&scale), 0.01f);
					//ImGui::Checkbox("Locked", &(_focusedObject->locked));
					focusedObject->setPosition(position);
					focusedObject->setOrigin(origin);
					focusedObject->setRotation(rotation);
					focusedObject->setScale(scale);
				}

				if(auto* object = focusedObject->getComponentAs<SpriteComponent>(); object != nullptr) {
					if(ImGui::CollapsingHeader("Sprite Component")) {

						// Load texture button
						auto& spriteDisplayDataHolder = _scenes.getCurrentScene()->getSpriteDisplayDataHolder();
						if(ImGui::Button("Load texture...##sprite_component")) {
							std::string file = _getRelativePathFromExplorer("Select texture", ".\\Assets");
							std::cout<<file<<std::endl;
							if(file != "") {
								try {
									auto& it = spriteDisplayDataHolder.emplace_back(file);
									object->setSpriteDisplayData(&it);
								}
								catch(const std::exception& exc) {
									object->setSpriteDisplayData(nullptr);
								}
							}
						}
						ImGui::Text("Path:");
						ImGui::SameLine();
						ImGui::Text(object->getSpriteDisplayData() ? mapWindows1250ToUtf8(object->getSpriteDisplayData()->getName()).c_str() : "None");
					}
				}

				if(auto* object = focusedObject->getComponentAs<ArmatureComponent>(); object != nullptr) {
					if(ImGui::CollapsingHeader("Armature Component")) {

						//Load armature button
						auto& armatureDisplayDataHolder = _scenes.getCurrentScene()->getArmatureDisplayDataHolder();
						if(ImGui::Button("Load armature...##armature_component")) {
							// std::string directory = _getRelativeDirectoryPathFromExplorer("Select armature folder", "Assets");
							std::string directory = _getRelativePathFromExplorer("Select armature file", ".\\Assets");
							directory = std::experimental::filesystem::path(directory).parent_path().string();
							if(directory != "") {
								try {
									auto& it = armatureDisplayDataHolder.emplace_back(directory);
									object->setArmatureDisplayData(&it);
								}
								catch(const std::exception& exc) {
									object->setArmatureDisplayData(nullptr);
								}
							}
						}
						ImGui::Text("Path:");
						ImGui::SameLine();
						ImGui::Text(object->getArmatureDisplayData() ? mapWindows1250ToUtf8(object->getArmatureDisplayData()->getName()).c_str() : "None");

						// Select animation button
						if(auto* arm = object->getArmature(); arm && arm->getAnimation()) {
							ImGui::Button("Select animation##armature_component");

							ImGui::SetNextWindowSize({300.f, 200.f});
							if(ImGui::BeginPopupContextItem(NULL, 0)) {
								auto names = arm->getAnimation()->getAnimationNames();
								for(auto& it : names) {
									ImGui::PushID(it.c_str());
									if(ImGui::Selectable(it.c_str())) {
										arm->getAnimation()->play(it);
									}
									ImGui::PopID();
								}
								ImGui::EndPopup();
							}
							if(arm->getAnimation()->isPlaying()) {
								ImGui::SameLine();
								if(ImGui::Button("Stop##armature_component")) {
									arm->getAnimation()->play(arm->getAnimation()->getLastAnimationName());
									arm->getAnimation()->stop(arm->getAnimation()->getLastAnimationName());
								}
							}
						}
					}

					// if(ImGui::CollapsingHeader("Armature Component")) {
					// 	if(ImGui::BeginCombo(
					// 		"DisplayData",
					// 		( object->getArmatureDisplayData() )?object->getArmatureDisplayData()->getName().c_str() : "None"
					// 	)) {
					// 		if(ImGui::MenuItem("None", nullptr, object->getArmatureDisplayData() == nullptr))
					// 			object->setArmatureDisplayData(nullptr);
					// 		for(auto& it : scene->getArmatureDisplayData()) {
					// 			if(ImGui::MenuItem(it.getName().c_str(), nullptr, object->getArmatureDisplayData() == &it))
					// 				object->setArmatureDisplayData(&it);
					// 		}
					// 		ImGui::EndCombo();
					// 	}
					// 	if(auto* arm = object->getArmature(); arm) {
					// 		auto names = arm->getAnimation()->getAnimationNames();
					// 		for(auto& it : names) {
					// 			if(ImGui::Button(it.c_str())) {
					// 				arm->getAnimation()->play(it);
					// 			}
					// 		}
					// 	}
					// }
				}
			}
		}
		ImGui::End();
	}
    void LevelEditor::_renderObjectsList() {
		if(ImGui::Begin("Objects", &_ifRenderObjectsList)) {
			ImGui::Separator();
			if(ImGui::BeginChild("Objects")) {
				int i = 0;
				auto* scene = _scenes.getCurrentScene();
				for(auto& group : scene->getAllEntities()) {
					ImGui::PushID(i);

					ImGui::PushStyleColor(ImGuiCol_Header, {0.f, 0.f, 0.f, 0.f});
					if(ImGui::TreeNodeEx(group.first.c_str(), ImGuiTreeNodeFlags_FramePadding|ImGuiTreeNodeFlags_DefaultOpen|ImGuiTreeNodeFlags_Framed)) {
						ImGui::PopStyleColor();

						// Context menu for group. Options: Add object
						if(ImGui::BeginPopupContextItem("Group context menu")) {
							if(ImGui::Selectable("Add object##group_context_menu")) {
								Entity* ent = scene->addEntity(group.first);
								if(ent) {
									_focusedObject = ent->getID();
								}
								_anySelected = static_cast<bool>(ent);
							}
							ImGui::EndPopup();
						}

						for(int i2 = 0; i2<group.second.size(); ++i2) {
							Entity& object = group.second[i2];
							bool temp = object.getID() == _focusedObject && _anySelected;
							ImGui::PushID(i2);
							if(ImGui::Selectable(object.getName().c_str(), temp)) {
								if(!temp) {
									_focusedObject = object.getID();
								}
								_anySelected = !temp;
							}

							// Context menu for object. Options: Duplicate|Remove
							if(group.first != "single") {
								if(ImGui::BeginPopupContextItem("Object context menu")) {
									if(ImGui::Selectable("Duplicate##object_context_menu")) {
										Entity* ent = scene->duplicateEntity(group.second[i2].getID());
										if(ent) {
											_focusedObject = ent->getID();
										}
										_anySelected = static_cast<bool>(ent);
									}
									if(ImGui::Selectable("Remove##object_context_menu")) {
										if(temp)
											_anySelected = false;
										group.second.erase(group.second.begin() + i2);
										ImGui::EndPopup();
										ImGui::PopID();
										break;
									}
									ImGui::EndPopup();
								}
							}

							ImGui::PopID();
						}
						ImGui::TreePop();
					}
					else {
						ImGui::PopStyleColor();

						// Context menu for group. Options: Add object
						if(ImGui::BeginPopupContextItem("Group context menu")) {
							if(ImGui::Selectable("Add object##group_context_menu")) {
								Entity* ent = scene->addEntity(group.first);
								if(ent) {
									_focusedObject = ent->getID();
								}
								_anySelected = static_cast<bool>(ent);
							}
							ImGui::EndPopup();
						}

					}
					ImGui::Separator();
					ImGui::PopID();
					++i;
				}
			}
			ImGui::EndChild();

		}
		ImGui::End();
	}
	void LevelEditor::_renderComponentsManager() {
		if (ImGui::BeginPopupModal("Change components...##modal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {

			auto* focusedObject = _scenes.getCurrentScene()->getEntity(_focusedObject);

			static bool selectedComponents[2];
			if(ImGui::IsWindowAppearing()) {
				selectedComponents[0] = focusedObject->hasComponent<SpriteComponent>();
				selectedComponents[1] = focusedObject->hasComponent<ArmatureComponent>();
			}
			if(ImGui::Button("Accept", ImVec2(70,0))) {

				if(focusedObject->hasComponent<SpriteComponent>()!=selectedComponents[0]) {
					if(selectedComponents[0]) focusedObject->addComponent<SpriteComponent>();
					else focusedObject->removeComponent<SpriteComponent>();
				}
				if(focusedObject->hasComponent<ArmatureComponent>()!=selectedComponents[1]) {
					if(selectedComponents[1]) focusedObject->addComponent<ArmatureComponent>();
					else focusedObject->removeComponent<ArmatureComponent>();
				}

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if(ImGui::Button("Close", ImVec2(70,0))) {
				ImGui::CloseCurrentPopup();
			}

			ImGui::Checkbox("SpriteComponent", &selectedComponents[0]);
			ImGui::Checkbox("ArmatureComponent", &selectedComponents[1]);

			ImGui::EndPopup();
		}
	}

    std::string LevelEditor::_getRelativePathFromExplorer(const std::string& title, const std::string& directory, const std::string& filter, bool saveButton) {
    	namespace filesystem = std::experimental::filesystem;

		std::string file;
		if(saveButton) file = FileDialog::getSaveFileName(title, directory, filter);
		else file = FileDialog::getOpenFileName(title, directory, filter);
		if(file == "") return "";

		std::string current = filesystem::current_path().string();

		if(current == file.substr(0, current.size())) {
			return file.substr(current.size()+1);
		}

		return "";
    }
  //   std::string LevelEditor::_getRelativeDirectoryPathFromExplorer(const std::string& title, const std::string& directory) {
  //   	namespace filesystem = std::experimental::filesystem;

		// std::string dir;
		// dir = DirectoryDialog::getExistingDirectory(title, directory);
		// if(dir == "") return "";

		// std::string current = filesystem::current_path().string();

		// if(current == dir.substr(0, current.size())) {
		// 	return dir.substr(current.size()+1);
		// }

		// return "";
  //   }
}
