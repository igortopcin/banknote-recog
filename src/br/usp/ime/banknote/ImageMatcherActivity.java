package br.usp.ime.banknote;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

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
		Toast.makeText(this, CAPTION, Toast.LENGTH_LONG).show();
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		finish();
		return false;
	}

}
