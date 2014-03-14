#include "SaveManager.h"

#include "util/DataFileParser.h"

#include <fstream>
#include <PrototypeGame.h>

void SaveManager::init(PrototypeGame* prototypeGame) { 
	this->game = prototypeGame;
}

void SaveManager::load() {
    FileBuffer fb = game->gameThreadContext->assetAPI->loadFile(
        game->gameThreadContext->assetAPI->getWritableAppDatasPath() + "/save.ini");
    DataFileParser dfp;
    dfp.load(fb, "save.ini");

    std::string key, value;
    for (unsigned i = 0; i < dfp.sectionSize(""); ++i) {
        dfp.get("", i, key, &value);
        map[key] = value;
    }
}

void SaveManager::save() {
	std::ofstream ofs(game->gameThreadContext->assetAPI->getWritableAppDatasPath() + "/save.ini");
    LOGI(game->gameThreadContext->assetAPI->getWritableAppDatasPath() + "/save.ini");
    for (auto & item : map) {
        LOGI(item.first << " = " << item.second);
        ofs << item.first << " = " << item.second << std::endl;
    }
    ofs << std::endl;
}
