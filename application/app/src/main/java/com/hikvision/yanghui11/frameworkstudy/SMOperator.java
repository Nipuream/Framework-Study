package com.hikvision.yanghui11.frameworkstudy;

import java.io.IOException;

/**
 * Created by yanghui11 on 2018/6/29.
 */

public class SMOperator {

    static {
        System.loadLibrary("sm");
    }

    public static native int write(int fd, byte[] buffer, int srcOffset, int destOffset, int count, int address) throws IOException;
    public static native int read(int fd, byte[] buffer, int srcOffset, int destOffset, int count, int address) throws IOException;
    public static native int mmap(int fd, int length, int mode);
    public static native void close(int fd);

}
