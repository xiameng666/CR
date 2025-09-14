package com.kr.test;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import com.kr.test.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'test' library on application startup.
    static {
        System.loadLibrary("test");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());

        binding.btnShow.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showFieldsMethods();
            }
        });

        binding.btnUpdate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                CFoo foo = new CFoo();
                Log.v("kr", "n1:"+CFoo.n1 + " f1:"+CFoo.f1 + " n:"+foo.n + " f:"+foo.f);
                updateFields(foo);
                Log.v("kr", "n1:"+CFoo.n1 + " f1:"+CFoo.f1 + " n:"+foo.n + " f:"+foo.f);
            }
        });
    }

    /**
     * A native method that is implemented by the 'test' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native void showFieldsMethods();
    public native void updateFields(Object obj);
}