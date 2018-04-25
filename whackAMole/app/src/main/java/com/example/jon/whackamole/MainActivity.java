package com.example.jon.whackamole;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.Random;
import java.util.UUID;

import static android.content.ContentValues.TAG;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private final static String TAG = MainActivity.class.getSimpleName();
    boolean PLAYER_1 = true;
    boolean PLAYER_2 = true;

    Button b00;
    Button b01;
    Button b02;

    Button b10;
    Button b11;
    Button b12;

    Button b20;
    Button b21;
    Button b22;

    TextView P1Count;
    TextView P2Count;

    Button bReset;

    TextView tvInfo;

    static int randValue = -1;

    int[][] boardStatus = new int[3][3];

    BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    private static int REQUEST_ENABLE_BT = 1;
    private static BluetoothSocket mmSocket;
    public static InputStream inStream;

    private static int numBytes;
    byte[] mmBuffer = new byte[1024];
    private static String readMessage;
    public static String lastCode = "0";
    public static long startTime = 0;
    String[] codeBuffer = new String[1024];
    long[] timeBuffer = new long[1024];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        b00 = (Button) findViewById(R.id.b00);
        b01 = (Button) findViewById(R.id.b01);
        b02 = (Button) findViewById(R.id.b02);

        b10 = (Button) findViewById(R.id.b10);
        b11 = (Button) findViewById(R.id.b11);
        b12 = (Button) findViewById(R.id.b12);

        b20 = (Button) findViewById(R.id.b20);
        b21 = (Button) findViewById(R.id.b21);
        b22 = (Button) findViewById(R.id.b22);

        P1Count = (TextView) findViewById(R.id.P1Count);
        P2Count = (TextView) findViewById(R.id.P2Count);

        bReset = (Button) findViewById(R.id.bReset);
        tvInfo = (TextView) findViewById(R.id.tvInfo);

        bReset.setOnClickListener(this);

        b00.setOnClickListener(this);
        b01.setOnClickListener(this);
        b02.setOnClickListener(this);

        b10.setOnClickListener(this);
        b11.setOnClickListener(this);
        b12.setOnClickListener(this);

        b20.setOnClickListener(this);
        b21.setOnClickListener(this);
        b22.setOnClickListener(this);

        initializeBoardStatus();

        if (mBluetoothAdapter == null) {
            Toast.makeText(getApplicationContext(),"Bluetooth not supported, exiting",Toast.LENGTH_SHORT).show();
            finish();
        }

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        BluetoothDevice device = mBluetoothAdapter.getRemoteDevice("20:14:04:16:31:86");

        try {
            mmSocket = device.createInsecureRfcommSocketToServiceRecord(UUID.fromString("00001101-0000-1000-8000-00805F9B34FB"));
            Toast.makeText(getApplicationContext(),"Socket creation succeeded",Toast.LENGTH_SHORT).show();
        }
        catch (IOException e) {
            Log.e(TAG, "Socket's create() method failed", e);
            Toast.makeText(getApplicationContext(),"Socket creation failed",Toast.LENGTH_SHORT).show();
        }

        mBluetoothAdapter.cancelDiscovery();

        try {
            // Connect to the remote device through the socket. This call blocks
            // until it succeeds or throws an exception.
            mmSocket.connect();
            Toast.makeText(getApplicationContext(),"Successful connection",Toast.LENGTH_SHORT).show();
        } catch (IOException connectException) {
            // Unable to connect; close the socket and return.
            try {
                Toast.makeText(getApplicationContext(),"Failed connection",Toast.LENGTH_SHORT).show();
                mmSocket.close();
            } catch (IOException closeException) {
                Log.e(TAG, "Could not close the client socket", closeException);
            }
        }

        try {
            inStream = mmSocket.getInputStream();
            Toast.makeText(getApplicationContext(),"Opened input stream",Toast.LENGTH_SHORT).show();
        }
        catch (IOException inputException) {
            Log.e(TAG, "Could not open input stream", inputException);
            Toast.makeText(getApplicationContext(),"Could not open input stream",Toast.LENGTH_SHORT).show();
        }

        while (true) {
            try {
                numBytes = MainActivity.inStream.read(mmBuffer);
                readMessage = new String(mmBuffer, 0, numBytes);
                if (readMessage.contains("a")) {
                    startTime = System.currentTimeMillis();
                    break;
                }
            }
            catch (IOException e) {};
        }

        new Thread(new Runnable() {
            public void run() {  //Read from Bluetooth thread
                while (true) {
                    try {
                        Thread.sleep(1000);
                    }
                    catch (InterruptedException e) {};

                    try {
                        // Read from the InputStream.
                        numBytes = MainActivity.inStream.read(mmBuffer);
                        for (int i = 0; i < numBytes-5; i++)
                        {
                            if (mmBuffer[i] == 0xFF) {
                                codeBuffer[i] = Byte.toString(mmBuffer[i+5]);
                                timeBuffer[i] = Byte.toUnsignedLong()
                            }
                        }
                        readMessage = new String(mmBuffer, 0, numBytes);
                    } catch (IOException e) {
                        Log.d(TAG, "Input stream was disconnected", e);
                    }

                    lastCode = Character.toString(readMessage.charAt(readMessage.length()-1));
                } }
        }).start();

        new Thread() //random number/update UI thread
        {
            private int randInterval = 0;
            public void run()
            {
                while (true) {
                    Random timeInterval = new Random();
                    randInterval = timeInterval.nextInt(2000 - 1000) + 1000;
                    try {
                        Thread.sleep(randInterval);
                    } catch (InterruptedException e) {
                    }
                    Random random = new Random();
                    randValue = random.nextInt(8);

                    runOnUiThread( new Runnable() {
                        @Override
                        public void run() {
                            switch (randValue) {
                                case 0:
                                    resetBoard();
                                    b00.setEnabled(true);
                                    boardStatus[0][0] = 1;
                                    b00.setText("X");
                                    break;
                                case 1:
                                    resetBoard();
                                    b01.setEnabled(true);
                                    boardStatus[0][1] = 1;
                                    b01.setText("X");
                                    break;
                                case 2:
                                    resetBoard();
                                    b02.setEnabled(true);
                                    boardStatus[0][2] = 1;
                                    b02.setText("X");
                                    break;
                                case 3:
                                    resetBoard();
                                    b10.setEnabled(true);
                                    boardStatus[1][0] = 1;
                                    b10.setText("X");
                                    break;
                                case 4:
                                    resetBoard();
                                    b11.setEnabled(true);
                                    boardStatus[1][1] = 1;
                                    b11.setText("X");
                                    break;
                                case 5:
                                    resetBoard();
                                    b12.setEnabled(true);
                                    boardStatus[1][2] = 1;
                                    b12.setText("X");
                                    break;
                                case 6:
                                    resetBoard();
                                    b20.setEnabled(true);
                                    boardStatus[2][0] = 1;
                                    b20.setText("X");
                                    break;
                                case 7:
                                    resetBoard();
                                    b21.setEnabled(true);
                                    boardStatus[2][1] = 1;
                                    b21.setText("X");
                                    break;
                                case 8:
                                    resetBoard();
                                    b22.setEnabled(true);
                                    boardStatus[2][2] = 1;
                                    b22.setText("X");
                                    break;
                            }
                        }
                    });
                 }
            }
        }.start();
    }

    @Override
    public void onClick(View view) {
        Log.d(TAG, "Inside onClick");
        PLAYER_1 = false;
        PLAYER_2 = false;

        setInfo(String.valueOf(lastCode));

        if (lastCode.equals("g")) {
            PLAYER_1 = true;
            lastCode = "0";
        }
        if (lastCode.equals("k")) {
            PLAYER_2 = true;
            lastCode = "0";
        }

        boolean resetButtonPressed = false;

        switch (view.getId()){
            case R.id.b00:
                if(boardStatus[0][0] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b00.setEnabled(false);
                break;

            case R.id.b01:
                if(boardStatus[0][1] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b01.setEnabled(false);
                break;

            case R.id.b02:
                if(boardStatus[0][2] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b02.setEnabled(false);
                break;

            case R.id.b10:
                if(boardStatus[1][0] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b10.setEnabled(false);
                break;

            case R.id.b11:
                if(boardStatus[1][1] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b11.setEnabled(false);
                break;

            case R.id.b12:
                if(boardStatus[1][2] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b12.setEnabled(false);
                break;

            case R.id.b20:
                if(boardStatus[2][0] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b20.setEnabled(false);
                break;

            case R.id.b21:
                if(boardStatus[2][1] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b21.setEnabled(false);
                break;

            case R.id.b22:
                if(boardStatus[2][2] == 1){
                    if (PLAYER_1)
                        P1Count.setText(String.valueOf(Integer.parseInt(P1Count.getText().toString())+1));
                    else if (PLAYER_2)
                        P2Count.setText(String.valueOf(Integer.parseInt(P2Count.getText().toString())+1));
                }
                b22.setEnabled(false);
                break;

            case R.id.bReset:
                resetButtonPressed = true;
                break;

            default:
                lastCode = "0";
                break;

        }

        if(resetButtonPressed){
            lastCode = "0";
            P1Count.setText(String.valueOf("0"));
            P2Count.setText(String.valueOf("0"));
        }
    }

    private void enableAllBoxes(boolean value){
        Log.d(TAG, "Inside enableAllBoxes");
        b00.setEnabled(value);
        b01.setEnabled(value);
        b02.setEnabled(value);

        b10.setEnabled(value);
        b11.setEnabled(value);
        b12.setEnabled(value);

        b20.setEnabled(value);
        b21.setEnabled(value);
        b22.setEnabled(value);
    }

    private void resetBoard(){
        Log.d(TAG, "Inside resetBoard");
        b00.setText("");
        b01.setText("");
        b02.setText("");

        b10.setText("");
        b11.setText("");
        b12.setText("");

        b20.setText("");
        b21.setText("");
        b22.setText("");

        enableAllBoxes(false);

        PLAYER_1 = true;

        initializeBoardStatus();
    }

    private void setInfo(String text){
        tvInfo.setText(text);
    }

    private void initializeBoardStatus(){
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                boardStatus[i][j] = -1;
            }
        }

    }
}
