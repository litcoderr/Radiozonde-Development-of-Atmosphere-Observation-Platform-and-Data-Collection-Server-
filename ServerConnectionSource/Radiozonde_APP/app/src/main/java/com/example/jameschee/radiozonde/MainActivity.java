package com.example.jameschee.radiozonde;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.RequestQueue;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.android.volley.toolbox.Volley;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    //GUI components
    private TextView tv_data_id;
    private TextView tv_data_status;
    private TextView tv_data;

    private Button bt_connect;
    private Button bt_disconnect;

    //Bluetooth components
    private static final String TAG = "BluetoothActivity";
    private final String DEVICE_ADDRESS = "98:D3:31:80:93:D2"; //address for our bt_module
    private final UUID PORT_UUID = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb");

    private BluetoothDevice device;
    private BluetoothSocket socket;
    private OutputStream outputStream;
    private InputStream inputStream;
    boolean deviceConnected=false;
    Thread thread;
    byte buffer[];
    int bufferPosition;
    boolean stopThread;

    boolean done = false;
    boolean start = true;
    String transfer = new String();

    int count = 1;

    String insertURL = "http://info-rne.net/radiozonde.php";
    RequestQueue requestQueue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        tv_data_id = (TextView) findViewById(R.id.data_id);
        tv_data_status = (TextView) findViewById(R.id.data_status);
        tv_data = (TextView) findViewById(R.id.data);

        bt_connect = (Button)findViewById(R.id.connect_btn);
        bt_disconnect = (Button)findViewById(R.id.disconnect_btn);
        setUiEnabled(false);

        requestQueue = Volley.newRequestQueue(getApplicationContext());

    }

    public void setUiEnabled(boolean bool)
    {
        bt_connect.setEnabled(!bool);
        bt_disconnect.setEnabled(bool);
    }

    public boolean BTinit(){
        boolean found=false;
        BluetoothAdapter bluetoothAdapter=BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) {
            Toast.makeText(getApplicationContext(),"Device doesnt Support Bluetooth",Toast.LENGTH_SHORT).show();
        }
        if(!bluetoothAdapter.isEnabled())
        {
            Intent enableAdapter = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableAdapter, 0);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        Set<BluetoothDevice> bondedDevices = bluetoothAdapter.getBondedDevices();
        if(bondedDevices.isEmpty())
        {
            Toast.makeText(getApplicationContext(),"Please Pair the Device first",Toast.LENGTH_SHORT).show();
        }
        else
        {
            for (BluetoothDevice iterator : bondedDevices)
            {
                if(iterator.getAddress().equals(DEVICE_ADDRESS))
                {
                    device=iterator;
                    found=true;
                    break;
                }
            }
        }
        return found;
    }

    public boolean BTconnect(){
        boolean connected=true;
        try {
            socket = device.createRfcommSocketToServiceRecord(PORT_UUID);
            socket.connect();
        } catch (IOException e) {
            e.printStackTrace();
            connected=false;
        }
        if(connected)
        {
            try {
                outputStream=socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                inputStream=socket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }


        return connected;
    }

    public void beginListenForData(){
        final Handler handler = new Handler();
        stopThread = false;
        buffer = new byte[1024];
        Thread thread  = new Thread(new Runnable()
        {
            public void run()
            {
                while(!Thread.currentThread().isInterrupted() && !stopThread)
                {
                    try
                    {
                        int byteCount = inputStream.available();
                        if(byteCount > 0)
                        {
                            byte[] rawBytes = new byte[byteCount];
                            inputStream.read(rawBytes);
                            final String string=new String(rawBytes,"UTF-8");
                            handler.post(new Runnable() {
                                public void run() {
                                    if(string.contains("\n")){
                                        done = true;
                                    }
                                    if(!done){
                                        if(start){
                                            tv_data.setText("");
                                        }
                                        tv_data.append(string);
                                        start = false;
                                    }else{
                                        tv_data.append(string);
                                        transfer = tv_data.getText().toString();
                                        tv_data_id.setText("DataID: "+Integer.toString(count));

                                        insertData(count,transfer);

                                        count++;
                                        transfer = new String();
                                        done = false;
                                        start = true;
                                    }
                                }
                            });
                        }
                    }
                    catch (IOException ex)
                    {
                        stopThread = true;
                    }
                }
            }
        });

        thread.start();
    }

    public void insertData(final int id, final String data){
        StringRequest request = new StringRequest(Request.Method.POST, insertURL, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {

                System.out.println(response.toString());
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {

            }
        }) {

            @Override
            protected Map<String, String> getParams() throws AuthFailureError {
                Map<String,String> parameters  = new HashMap<String, String>();
                parameters.put("id",Integer.toString(id));
                parameters.put("data",data);
                return parameters;
            }
        };
        requestQueue.add(request);
    }


    public void connectButtonPressed(View view){
        if(BTinit()){
            if(BTconnect()){
                setUiEnabled(true);
                deviceConnected=true;
                beginListenForData();
                tv_data_status.setText("Enabled");
            }
        }
    }

    public void disconnectedButtonPressed(View view)throws IOException{
        stopThread = true;
        outputStream.close();
        inputStream.close();
        socket.close();
        setUiEnabled(false);
        deviceConnected=false;
        tv_data_status.setText("\nConnection Closed!\n");
    }

}