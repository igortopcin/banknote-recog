package br.usp.ime.banknote;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import android.app.Activity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.WindowManager;

public class MainActivity extends Activity implements CvCameraViewListener2,
		OnTouchListener {

	private static final String TAG = "BanknoteRecog::MainActivity";
	
	static {
		if (!OpenCVLoader.initDebug()) {
			// Handle initialization error
		} else {
			System.loadLibrary("banknote");
		}
	}

	private boolean captureFrame = false;
	private boolean initialized = false;
	private CameraBridgeViewBase cvCam;
	private Mat capturedImg;

	BaseLoaderCallback blc = new BaseLoaderCallback(this) {
		public void onManagerConnected(int status) {
			switch (status) {
			case (LoaderCallbackInterface.SUCCESS): {
				// Load the jni library
				System.loadLibrary("banknote");
				cvCam.enableView();
				cvCam.setOnTouchListener(MainActivity.this);
				Log.i(TAG, "Initializing lib");
				copyAssets();
				nInitTrainSamples(getExternalFilesDir(null).getAbsolutePath());
			}
			default:
				super.onManagerConnected(status);
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setContentView(R.layout.activity_main);
		cvCam = (CameraBridgeViewBase) findViewById(R.id.cvCamView);
		cvCam.setVisibility(CameraBridgeViewBase.VISIBLE);
		cvCam.setCvCameraViewListener(this);
	}

	@Override
	protected void onPause() {
		super.onPause();
		if (cvCam != null)
			cvCam.disableView();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
		if (cvCam != null)
			cvCam.disableView();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	public void onResume() {
		super.onResume();
		captureFrame = false;
		//OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_5, this, blc);
		cvCam.enableView();
		cvCam.setOnTouchListener(MainActivity.this);
		
		if (!initialized) {
			Log.i(TAG, "Initializing lib");
			copyAssets();
			nInitTrainSamples(getExternalFilesDir(null).getAbsolutePath());
			initialized = true;
		}
	}

	@Override
	public void onCameraViewStarted(int width, int height) {
		capturedImg = new Mat(height, width, CvType.CV_8UC3);
	}

	@Override
	public void onCameraViewStopped() {
		capturedImg.release();
	}

	@Override
	public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
		Mat rgba = inputFrame.rgba();
		if (captureFrame) {
			captureFrame = false;
			Imgproc.cvtColor(rgba, capturedImg, Imgproc.COLOR_RGBA2BGR, 3);
			
			Log.i(TAG, "Capturing frame!");
			String matchedText = nMatchImage(capturedImg.getNativeObjAddr());

			Bitmap bmp = Bitmap.createBitmap(capturedImg.cols(), capturedImg.rows(), Bitmap.Config.ARGB_8888);
			Utils.matToBitmap(capturedImg, bmp);

			ImageMatcherActivity.IMAGE = bmp;
			ImageMatcherActivity.CAPTION = matchedText;
			Intent i = new Intent(this, ImageMatcherActivity.class);
			startActivity(i);
		}
		return rgba;
	}

	@Override
	public boolean onTouch(View view, MotionEvent event) {
		captureFrame = true;
		return false;
	}

	private void copyAssets() {
		AssetManager assetManager = getAssets();
		String[] files = null;
		try {
			files = assetManager.list("");
		} catch (IOException e) {
			Log.e(TAG, "Failed to get asset file list.", e);
		}
		for (String filename : files) {
			InputStream in = null;
			OutputStream out = null;
			try {
				in = assetManager.open(filename);
				File outFile = new File(getExternalFilesDir(null), filename);
				Log.i(TAG, "Copying file: " + outFile.getAbsolutePath());
				out = new FileOutputStream(outFile);
				copyFile(in, out);
				in.close();
				in = null;
				out.flush();
				out.close();
				out = null;
			} catch (IOException e) {
				Log.e(TAG, "Failed to copy asset file: " + filename, e);
			}
		}
	}

	private void copyFile(InputStream in, OutputStream out) throws IOException {
		byte[] buffer = new byte[1024];
		int read;
		while ((read = in.read(buffer)) != -1) {
			out.write(buffer, 0, read);
		}
	}

	public native void nInitTrainSamples(String descriptors_dir);
	public native String nMatchImage(long addr);
}
