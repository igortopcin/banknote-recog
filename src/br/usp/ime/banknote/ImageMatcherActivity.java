package br.usp.ime.banknote;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.TextView;

public class ImageMatcherActivity extends Activity implements OnTouchListener {
	
	public static Bitmap IMAGE = null;
	public static String CAPTION = "No match";
	private ImageView imageView;
	private TextView textView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.image_viewer);
		imageView = (ImageView) findViewById(R.id.imageView);
		imageView.setOnTouchListener(this);
		if (IMAGE != null) {
			imageView.setImageBitmap(IMAGE);
		}
		textView = (TextView) findViewById(R.id.textView);
		textView.setText(CAPTION);
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		finish();
		return false;
	}

}
