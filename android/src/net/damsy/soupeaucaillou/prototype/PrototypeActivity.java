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
package net.damsy.soupeaucaillou.prototype;

import net.damsy.soupeaucaillou.SacActivity;
import net.damsy.soupeaucaillou.heriswap.api.NameInputAPI;
import android.content.Context;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RelativeLayout;

public class PrototypeActivity extends SacActivity implements SensorEventListener{
	static {
        System.loadLibrary("prototype");
    }
	
	@Override
	public boolean canShowAppRater() {
		return false;
	}
	
	@Override
	public int[] getSwarmBoards() {
		return HeriswapSecret.boardsSwarm;
	}
	
	@Override
	public int getSwarmGameID() {
		return HeriswapSecret.Swarm_gameID;
	}
	
	@Override
	public String getSwarmGameKey() {
		return HeriswapSecret.Swarm_gameKey;
	}
	
	@Override
	public String getBundleKey() {
		return TILEMATCH_BUNDLE_KEY;
	}
	
	@Override
	public String getCharboostAppId() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public String getCharboostAppSignature() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public int getGLViewId() {
		// TODO Auto-generated method stub
		return R.id.surfaceviewclass;
	}
	
	@Override
	public int getLayoutId() {
		return R.layout.main;
	}
	
	static public final String Tag = "HeriswapJ";
	static final String TILEMATCH_BUNDLE_KEY = "plop";
	static public final String HERISWAP_SHARED_PREF = "HeriswapPref";
	
	byte[] renderingSystemState;
	
	static public View playerNameInputView;
	static public EditText nameEdit;
	static public String playerName;

	public SharedPreferences preferences;
	static public Button[] oldName;

	@Override
    protected void onCreate(Bundle savedInstanceState) {
		Log.i(PrototypeActivity.Tag, "-> onCreate [" + savedInstanceState);
        super.onCreate(savedInstanceState);

        RelativeLayout rl = (RelativeLayout) findViewById(R.id.parent_frame);
        playerNameInputView = findViewById(R.id.enter_name);
        nameEdit = (EditText) findViewById(R.id.player_name_input);
        rl.bringChildToFront(playerNameInputView);
        playerNameInputView.setVisibility(View.GONE);

        Button b = (Button) findViewById(R.id.name_save);
        b.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				playerName = filterPlayerName(nameEdit.getText().toString());

				//NOLOGLog.i(HeriswapActivity.Tag, "Player name: '" + playerName + "'");
				if (playerName != null && playerName.length() > 0) {
					playerNameInputView.setVisibility(View.GONE);
					NameInputAPI.nameReady = true;
					// hide keyboard
					InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
					mgr.hideSoftInputFromWindow(nameEdit.getWindowToken(), 0);
				}
			}
		});
        oldName = new Button[3];
        oldName[0] = (Button)findViewById(R.id.reuse_name_1);
        oldName[1] = (Button)findViewById(R.id.reuse_name_2);
        oldName[2] = (Button)findViewById(R.id.reuse_name_3);
        for (int i=0 ;i<3; i++) {
        	oldName[i].setOnClickListener( new View.OnClickListener() {
				public void onClick(View v) {
	        		playerName = ((Button)v).getText().toString();
	        		playerNameInputView.setVisibility(View.GONE);
	        		NameInputAPI.nameReady = true;
					// hide keyboard
					InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
					mgr.hideSoftInputFromWindow(nameEdit.getWindowToken(), 0);
	        	}
        	});
        }
        
        SensorManager sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        setListeners(sensorManager, this);
    }
	
	public void setListeners(SensorManager sensorManager, SensorEventListener mEventListener)
    {
        sensorManager.registerListener(mEventListener, sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), 
                SensorManager.SENSOR_DELAY_NORMAL);
        sensorManager.registerListener(mEventListener, sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD), 
                SensorManager.SENSOR_DELAY_NORMAL);
    }

  
    final float[] mValuesMagnet      = new float[3];
    final float[] mValuesAccel       = new float[3];
    
    final float[] mRotationMatrix    = new float[9];
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		
	} 
	@Override
	public void onSensorChanged(SensorEvent event) {
		// Handle the events for which we registered
        switch (event.sensor.getType()) {
            case Sensor.TYPE_ACCELEROMETER:
                System.arraycopy(event.values, 0, mValuesAccel, 0, 3);
                break;

            case Sensor.TYPE_MAGNETIC_FIELD:
                System.arraycopy(event.values, 0, mValuesMagnet, 0, 3);
                break;
        }
        SensorManager.getRotationMatrix(mRotationMatrix, null, mValuesAccel, mValuesMagnet);
        SensorManager.getOrientation(mRotationMatrix, mValuesOrientation);
	}
	
    private String filterPlayerName(String name) {
    	String n = name.trim();
    	return n.replaceAll("[^a-zA-Z0-9 ]"," ").substring(0, Math.min(11, n.length()));
    }
}
