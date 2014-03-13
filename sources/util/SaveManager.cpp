#include "SaveManager.h"

#include "util/DataFileParser.h"

#include <fstream>

void SaveManager::init(AssetAPI* assetAPI) { 
	this->assetAPI = assetAPI;
}

void SaveManager::load() {
    FileBuffer fb = assetAPI->loadFile(assetAPI->getWritableAppDatasPath() + "/progression.ini");
    DataFileParser dfp;
    dfp.load(fb, "progression.ini");

	LOGT("Do something with the loaded file!");
}


void SaveManager::save() {
	std::ofstream ofs(assetAPI->getWritableAppDatasPath() + "/progression.ini");

	FileBuffer fb;
	LOGT("Fill the filebuffer before writting it in file!");

	ofs << fb.data;
}
