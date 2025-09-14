package com.kr.test;

import android.util.Log;

public class CFoo {
    public int n = 100;
    public float f = 3.14f;
    static public int n1 = 200;
    static public float f1 = 3.14f;

    public void show(){
        Log.v("kr", "show");
    }
    public static void showStatic() {
        Log.v("kr", "showStatic");
    }

    public static void main(String[] args) {
        System.out.println("hello world");
    }
}
