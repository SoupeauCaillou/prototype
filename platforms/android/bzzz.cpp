/*
    This file is part of Bzzz.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Bzzz is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Bzzz is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Bzzz.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "android/sacjnilib.h"
#include "../sources/BzzzGame.h"
/*
class BzzzGameThreadJNIEnvCtx : public GameThreadJNIEnvCtx {
	public:

    void init(JNIEnv* pEnv, jobject assetMgr) {
	    GameThreadJNIEnvCtx::init(pEnv, assetMgr);
    }

    void uninit(JNIEnv* pEnv) {
		if (env == pEnv) {
		}
		GameThreadJNIEnvCtx::uninit(pEnv);
    }
};
*/
GameHolder* GameHolder::build() {
	GameHolder* hld = new GameHolder();

/*
	BzzzGameThreadJNIEnvCtx* jniCtx = new BzzzGameThreadJNIEnvCtx();
	hld->gameThreadJNICtx = jniCtx;
*/
	hld->game = new BzzzGame(0, 0);
	return hld;
};
