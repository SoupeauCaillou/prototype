/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
package net.damsy.soupeaucaillou.fond_ecran;

import net.damsy.soupeaucaillou.SacActivity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class Fond_ecranActivity extends SacActivity {
	static {
        System.loadLibrary("fond_ecran");
    }
 
	@Override
	public boolean canShowAppRater() {
		return false;
	}
	
	@Override
	public int[] getSwarmBoards() {
		return null;
	}
	
	@Override
	public int getSwarmGameID() {
		return 0;
	}
	
	@Override
	public String getSwarmGameKey() {
		return null;
	}
	
	@Override
	public String getBundleKey() {
		return PROTOTYPE_BUNDLE_KEY;
	}

    public int getLayoutId() {
        return R.layout.main;
    }
	public int getParentViewId() {
        return R.id.parent_frame;
    }
    public String getCharboostAppId() {
        return null;
    }
    public String getRevMobAppId() {
        return null;
    }
	public String getCharboostAppSignature() {
        return null;
    }
    public View getNameInputView() {
        return null;
    }
	public EditText getNameInputEdit() {
        return null;
    }
	public Button getNameInputButton() {
        return null;
    }
	public boolean giftizEnabled() {
		return false;
	}
	
	static public final String Tag = "Fond_ecran";
	static final String PROTOTYPE_BUNDLE_KEY = "plop";
	static public final String PROTOTYPE_SHARED_PREF = "Fond_ecranPref";
	
	byte[] renderingSystemState;
	
	static public View playerNameInputView;
	static public EditText nameEdit;
	static public String playerName;

	public SharedPreferences preferences;
	static public Button[] oldName;

	@Override
    protected void onCreate(Bundle savedInstanceState) {
		Log.i(Fond_ecranActivity.Tag, "-> onCreate [" + savedInstanceState);
        super.onCreate(savedInstanceState);
    }
}
