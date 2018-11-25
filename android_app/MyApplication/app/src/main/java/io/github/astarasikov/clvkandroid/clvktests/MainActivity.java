package io.github.astarasikov.clvkandroid.clvktests;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity {
    native void TestCLVK();

    static {
        System.loadLibrary("clvk_jni");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ((Button)findViewById(R.id.btn_test_clinfo)).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                TestCLVK();
            }
        });
    }
}
