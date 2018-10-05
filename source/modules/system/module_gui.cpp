#include "mcv_platform.h"
#include "module_gui.h"
#include "render/render_objects.h"
#include "gui/gui_parser.h"
#include "gui/controllers/gui_menu_buttons_controller.h"
#include "gui/widgets/gui_bar.h"
#include "gui/gui_controller.h"

using namespace GUI;

CModuleGUI::CModuleGUI(const std::string& name)
	: IModule(name)
{}

bool CModuleGUI::start()
{
	_orthoCamera.setOrthographicGUI(width, height);

	_technique = Resources.get("gui.tech")->as<CRenderTechnique>();
	_quadMesh = Resources.get("unit_quad_xy.mesh")->as<CRenderMesh>();
	_fontTexture = Resources.get("data/textures/gui/font.dds")->as<CTexture>();

	initializeWidgetStructure();
	
	return true;
}

void CModuleGUI::initializeWidgetStructure() {

	//Initializing all the functions for the buttons of GUI

	//MAIN-MENU
	auto mm_newGameCB = []() {
		CEngine::get().getModules().changeGameState("map_intro");
	};
	auto mm_continueCB = []() {
		//CEngine::get().getGUI().outOfMainMenu();
	};
	auto mm_optionsCB = []() {
		//activateWidget("main_menu_buttons");
	};
	auto mm_exitCB = []() {
		exit(0);
	};
	
	//PAUSE-MENU
	auto pm_resumeGame = []() {
		EngineGUI.closePauseMenu();
	};
	auto pm_restartLevel = []() {
        //EngineLogic.execSystemScriptDelayed("gameManager:resetToCheckpoint()", 2.f);
		CEngine::get().getGameManager().resetLevel();
	};
	auto pm_RestartFromCheckPoint = []() {
        //EngineLogic.execSystemScriptDelayed("gameManager:resetToCheckpoint()", 2.f);
		CEngine::get().getGameManager().resetToCheckpoint();
	};
	auto pm_Controls = []() {
		EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::CONTROLS)->makeChildsFadeIn(0.08,0,false);
		EngineGUI.activateWidget(CModuleGUI::EGUIWidgets::BACK_BUTTON)->makeChildsFadeIn(0.08, 0, true);
		EngineGUI.deactivateController(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE_BUTTONS);
		//activateWidget("main_menu_buttons");
	};
	auto pm_Exit = []() {
		exit(0);
	};
	auto pm_Back = []() {
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::BACK_BUTTON)->makeChildsFadeOut(0.08, 0, true);
		EngineGUI.getWidget(CModuleGUI::EGUIWidgets::CONTROLS)->makeChildsFadeOut(0.08, 0, false);
		EngineLogic.execSystemScriptDelayed("backFromControls();", 0.08f);
	};

	auto pm_Dead = []() {
		CEngine::get().getGameManager().resetToCheckpoint();
	};

	CMenuButtonsController* mmc = new CMenuButtonsController();

	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BUTTONS, "data/gui/main_menu_buttons.json", mmc);
	mmc = (CMenuButtonsController*)getWidgetController(EGUIWidgets::MAIN_MENU_BUTTONS);
	mmc->registerOption("new_game", mm_newGameCB);
	mmc->registerOption("continue", mm_continueCB);
	mmc->registerOption("options", mm_optionsCB);
	mmc->registerOption("exit", mm_exitCB);
	mmc->setCurrentOption(0);
	

	CMenuButtonsController* pmc = new CMenuButtonsController();
	registerWigdetStruct(EGUIWidgets::INGAME_MENU_PAUSE_BUTTONS, "data/gui/pause_menu_buttons.json", pmc);

	pmc = (CMenuButtonsController*)getWidgetController(EGUIWidgets::INGAME_MENU_PAUSE_BUTTONS);
	pmc->registerOption("resume_game", pm_resumeGame);
	pmc->registerOption("restart", pm_restartLevel);
	pmc->registerOption("restart_checkpoint", pm_RestartFromCheckPoint);
	pmc->registerOption("controls", pm_Controls);
	pmc->registerOption("pause_exit", pm_Exit);
	pmc->setCurrentOption(0);


	CMenuButtonsController* dmc = new CMenuButtonsController();
	registerWigdetStruct(EGUIWidgets::DEAD_MENU_BUTTONS, "data/gui/dead_menu_buttons.json", dmc);
	dmc = (CMenuButtonsController*)getWidgetController(EGUIWidgets::DEAD_MENU_BUTTONS);
	dmc->registerOption("restart_dead", pm_Dead);
	dmc->setCurrentOption(0);

	CMenuButtonsController* bbc = new CMenuButtonsController();
	registerWigdetStruct(EGUIWidgets::BACK_BUTTON, "data/gui/back_button.json", bbc);
	bbc = (CMenuButtonsController*)getWidgetController(EGUIWidgets::BACK_BUTTON);
	bbc->registerOption("back", pm_Back);
	bbc->setCurrentOption(0);


	registerWigdetStruct(EGUIWidgets::MAIN_MENU_BACKGROUND, "data/gui/main_menu_background.json");
	registerWigdetStruct(EGUIWidgets::SOUND_GRAPH, "data/gui/sound_graph.json");
	registerWigdetStruct(EGUIWidgets::INGAME_STAMINA_BAR, "data/gui/ingame.json");
	registerWigdetStruct(EGUIWidgets::INGAME_MENU_PAUSE, "data/gui/pause_menu_background.json");
	registerWigdetStruct(EGUIWidgets::DEAD_MENU_BACKGROUND, "data/gui/dead_menu_background.json");
	registerWigdetStruct(EGUIWidgets::CONTROLS, "data/gui/controls.json");
	registerWigdetStruct(EGUIWidgets::LOADING_SPRITE, "data/gui/loading.json");
	
}

void CModuleGUI::registerWigdetStruct(EGUIWidgets wdgt_type, std::string wdgt_path, GUI::CController *wdgt_controller) {

	WidgetStructure wdgt_struct;
	CParser parser;
	wdgt_struct._widgetName = parser.parseFile(wdgt_path);
	wdgt_struct._type = wdgt_type;
	wdgt_struct._widget = getWidget(wdgt_struct._widgetName);
	wdgt_struct._controller = wdgt_controller;
	_widgetStructureMap[wdgt_type] = wdgt_struct;
}

bool CModuleGUI::stop()
{
	return true;
}

void CModuleGUI::update(float delta)
{
	if (EngineInput[VK_DOWN].getsPressed())
	{
		//deactivateWidget(EGUIWidgets::MAIN_MENU_BUTTONS);
	}
	if (EngineInput[VK_UP].getsPressed())
	{
		//activateWidget(EGUIWidgets::MAIN_MENU_BACKGROUND);
	}

	for (auto& wdgt : _activeWidgets)
	{
		wdgt->updateAll(delta);
	}
	if (buttons_state) {
		for (auto& controller : _controllers)
		{
			controller->update(delta);
		}
	}

}

bool CModuleGUI::getWidgetStructureEnabled(EGUIWidgets wdgt) {

	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	return wdgt_struct.enabled;
}

void CModuleGUI::activateController(EGUIWidgets wdgt) {

	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];

	if (wdgt_struct._controller != nullptr) {
		registerController(wdgt_struct._controller);
	}

}

void CModuleGUI::deactivateController(EGUIWidgets wdgt) {

	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	if (wdgt_struct._controller != nullptr) {
		unregisterController(wdgt_struct._controller);
	}
}

void CModuleGUI::renderGUI()
{
	for (auto& wdgt : _activeWidgets)
	{
		wdgt->renderAll();
	}
}

void CModuleGUI::registerWidget(CWidget* wdgt)
{
	_registeredWidgets.push_back(wdgt);
}

CWidget* CModuleGUI::getWidget(const std::string& name, bool recursive) const
{
	for (auto& rwdgt : _registeredWidgets)
	{
		if (rwdgt->getName() == name)
		{
			return rwdgt;
		}
	}

	if (recursive)
	{
		for (auto& rwdgt : _registeredWidgets)
		{
			CWidget* wdgt = rwdgt->getChild(name, true);
			if (wdgt)
			{
				return wdgt;
			}
		}
	}

	return nullptr;
}

CWidget* CModuleGUI::getWidget(EGUIWidgets wdgt_type) {
	
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt_type];
	CWidget* wdgt = wdgt_struct._widget;
	if (wdgt != nullptr) {
		return wdgt;
	}
	return nullptr;
}

GUI::CController* CModuleGUI::getWidgetController(EGUIWidgets wdgt_type) {

	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt_type];
	CController* controller = wdgt_struct._controller;
	return controller;
}

CWidget* CModuleGUI::activateWidget(EGUIWidgets wdgt)
{
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	if (wdgt_struct.enabled) return nullptr;
	CWidget* widgt = getWidget(wdgt_struct._widgetName);
	if (widgt)
	{
		wdgt_struct.enabled = true;
		_widgetStructureMap[wdgt] = wdgt_struct;
		_activeWidgets.push_back(widgt);
		if (wdgt_struct._controller != nullptr) {
			registerController(wdgt_struct._controller);
		}
		return widgt;
	}
	else {
		return nullptr;
	}

	
}

void CModuleGUI::deactivateWidget(EGUIWidgets wdgt)
{
	WidgetStructure wdgt_struct = _widgetStructureMap[wdgt];
	CWidget* widgt = getWidget(wdgt_struct._widgetName);
	for (auto it = _activeWidgets.begin(); it != _activeWidgets.end();) {
		if (*it == widgt) {
			_activeWidgets.erase(it);
			break;
		}
		it++;
	}
	wdgt_struct.enabled = false;
	_widgetStructureMap[wdgt] = wdgt_struct;
	if (wdgt_struct._controller != nullptr) {
		unregisterController(wdgt_struct._controller);
	}
}

void CModuleGUI::enableWidget(const std::string& name, bool status)
{
    // Maybe we should replace this with the deactivateWidget 
    CWidget* widgt = getWidget(name, true);

    if(widgt)
        widgt->enable(status); 
}

void CModuleGUI::registerController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it == _controllers.end())
	{
		controller->start();
		_controllers.push_back(controller);
	}
}

void CModuleGUI::unregisterController(GUI::CController* controller)
{
	auto it = std::find(_controllers.begin(), _controllers.end(), controller);
	if (it != _controllers.end())
	{
		_controllers.erase(it);
	}
}

CCamera& CModuleGUI::getCamera()
{
	return _orthoCamera;
}

MVariants& CModuleGUI::getVariables()
{
	return _variables;
}

void CModuleGUI::renderTexture(const MAT44& world, const CTexture* texture, const VEC2& minUV, const VEC2& maxUV, const VEC4& color)
{
	assert(_technique && _quadMesh);

	cb_object.obj_world = world;
	cb_object.obj_color = VEC4(1, 1, 1, 1);
	cb_object.updateGPU();

	cb_gui.minUV = minUV;
	cb_gui.maxUV = maxUV;
	cb_gui.tint_color = color; 
	cb_gui.updateGPU();

	_technique->activate();
	if (texture)
		texture->activate(TS_ALBEDO);

	_quadMesh->activateAndRender();
}

void CModuleGUI::renderCustomTexture(const std::string & tech, const MAT44& world, const CTexture* texture, const ConfigParams & params)
{
    assert(_technique && _quadMesh);

    cb_object.obj_world = world;
    cb_object.obj_color = VEC4(1, 1, 1, 1);
    cb_object.updateGPU();

    cb_gui.minUV = params.minUV;
    cb_gui.maxUV = params.maxUV;
    cb_gui.tint_color = params.color;
    cb_gui.gui_var1 = params.var;
    cb_gui.updateGPU();

    const CRenderTechnique * c_technique = Resources.get(tech)->as<CRenderTechnique>();
    assert(c_technique);
    c_technique->activate();

    if (texture)
        texture->activate(TS_ALBEDO);

    _quadMesh->activateAndRender();
}

void CModuleGUI::renderText(const MAT44& world, const std::string& text, const VEC4& color)
{
	assert(_fontTexture);

	int cellsPerRow = 8;
	float cellSize = 1.f / 8.f;
	char firstCharacter = ' ';
	for (size_t i = 0; i < text.size(); ++i)
	{
		char c = text[i];

		int cell = c - firstCharacter;
		int row = cell / cellsPerRow;
		int col = cell % cellsPerRow;

		VEC2 minUV = VEC2(col * cellSize, row * cellSize);
		VEC2 maxUV = minUV + VEC2(1, 1) * cellSize;
		VEC2 gap = (float)i * VEC2(1, 0);
		MAT44 w = MAT44::CreateTranslation(gap.x, gap.y, 0.f) * world;

		renderTexture(w, _fontTexture, minUV, maxUV, color);
	}
}

void CModuleGUI::setButtonsState(bool state) {
	buttons_state = state;
}

bool CModuleGUI::getButtonsState() {
	return buttons_state;
}

void CModuleGUI::closePauseMenu() {
	EngineGUI.setButtonsState(false);
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE)->makeChildsFadeOut(0.08, 0, false);
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::INGAME_MENU_PAUSE_BUTTONS)->makeChildsFadeOut(0.08, 0, true);
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::CONTROLS)->makeChildsFadeOut(0.08, 0, false);
	EngineGUI.getWidget(CModuleGUI::EGUIWidgets::BACK_BUTTON)->makeChildsFadeOut(0.08, 0, true);
	EngineLogic.execSystemScriptDelayed("unPauseGame();", 0.08f);
}