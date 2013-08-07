package co.mwater.opencvactivity;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

/*
 * Cordova/Phonegap plugin.
 * 
 * Call using action "process" with parameters processId, processParams, title.
 * Call using action "processList" to get a list of processIds available
 */
public class OpenCVActivityPlugin extends CordovaPlugin {
	private static final String TAG = OpenCVActivityPlugin.class.getSimpleName();
	
	private static int REQUEST_OPENCV_ACTIVITY = 1234;
	CallbackContext currentCallbackContext;

	@Override
	public boolean execute(String action, JSONArray args,
			CallbackContext callbackContext) throws JSONException {
		if ("process".equals(action)) {
			// Launch OpenCV
			currentCallbackContext = callbackContext;
			Intent intent = new Intent(cordova.getActivity(), OpenCVActivity.class);
			intent.putExtra("processId", args.getString(0));
			intent.putExtra("title", args.getString(2));
			
			JSONArray paramsJSON = args.getJSONArray(1);
			String[] params = new String[paramsJSON.length()];
			for (int i=0;i<params.length;i++)
				params[i]=paramsJSON.getString(i);
			intent.putExtra("processParams", params);
			cordova.startActivityForResult(this, intent, REQUEST_OPENCV_ACTIVITY);
			return true;
		}
		if ("processList".equals(action)) {
			Log.i(TAG, "processList called");
			
			// Get a list of processes available
			JSONArray list = new JSONArray();
			list.put("ec-plate");
			list.put("demo");
			callbackContext.success(list);
			return true;
		}
		return false;
	}

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent intent) {
		if (requestCode == REQUEST_OPENCV_ACTIVITY) {
			// Call Javascript with results
			try {
				if (resultCode == Activity.RESULT_OK) {
					JSONObject res = new JSONObject(intent.getStringExtra("result"));
					currentCallbackContext.success(res);
				}
				else 
					currentCallbackContext.error("Request failed");
			} catch (JSONException e) {
				currentCallbackContext.error(e.getLocalizedMessage());
			}
		}
	}
}
