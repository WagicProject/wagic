package org.libsdl.app;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.VelocityTracker;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;

import com.google.ads.*;


import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.URL;
import java.net.URLConnection;


/**
    SDL Activity
*/
public class SDLActivity extends Activity {

	
    // Main components
    private static AdView mAdView;	
    private static SDLActivity mSingleton;
    private static SDLSurface mSurface;
    
    // Audio
    private static Thread mAudioThread;
    private static AudioTrack mAudioTrack;

    //Resource download 
    public static final int DIALOG_DOWNLOAD_PROGRESS = 0;
    public static final int DIALOG_DOWNLOAD_ERROR = 1;    
    private ProgressDialog mProgressDialog;
    private AlertDialog mErrorDialog;
    public String mErrorMessage = "";
    public Boolean mErrorHappened = false;
	public final static String RES_FOLDER = "/sdcard/Wagic/Res/";
    public static final String RES_FILENAME = "core_0180.zip";
    public static final String RES_URL = "http://wagic.googlecode.com/files/";
   
    private void startDownload() {
        String url = RES_URL + RES_FILENAME;
        new DownloadFileAsync().execute(url);
    }
    
    public void downloadError(String errorMessage) {
    	mErrorHappened = true;
    	mErrorMessage = errorMessage;
    }
    
    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
		case DIALOG_DOWNLOAD_PROGRESS:
			mProgressDialog = new ProgressDialog(this);
			mProgressDialog.setMessage("Downloading resource files (" + RES_FILENAME + ")");
			mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
			mProgressDialog.setCancelable(false);
			mProgressDialog.show();
			return mProgressDialog;
		case DIALOG_DOWNLOAD_ERROR:
	        //prepare alertDialog
	    	AlertDialog.Builder builder = new AlertDialog.Builder(this);
	    	builder.setMessage(mErrorMessage)
	    	       .setCancelable(false)
	    	       .setPositiveButton("Exit", new DialogInterface.OnClickListener() {
	    	           public void onClick(DialogInterface dialog, int id) {
	    	        	   System.exit(0);
	    	           }
	    	       });
	    	mErrorDialog = builder.create(); 			
			mErrorDialog.show();
			return mErrorDialog;
		default:
			return null;
        }
    }   
    
    // Load the .so
    static {
        System.loadLibrary("SDL");
        //System.loadLibrary("SDL_image");
        //System.loadLibrary("SDL_mixer");
        //System.loadLibrary("SDL_ttf");
        System.loadLibrary("main");
    }

    
    //create main application
    public void mainDisplay() {
        FrameLayout _videoLayout = new FrameLayout(this);
        
        //mGLView = new DemoGLSurfaceView(this);
       
        
        // Set up the surface
        mSurface = new SDLSurface(getApplication());
        
        //setContentView(mSurface);
        SurfaceHolder holder = mSurface.getHolder();
        holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
       
        // Create the adView
        mAdView = new AdView(this, AdSize.BANNER, "a14e9009f88864f"); //"a14dc0ab7b27413" <-- for the alpha);       
        
        _videoLayout.addView(mSurface, new LayoutParams(LayoutParams.FILL_PARENT,
        		  LayoutParams.FILL_PARENT));
        _videoLayout.addView(mAdView, new LayoutParams(LayoutParams.WRAP_CONTENT
        		  , LayoutParams.WRAP_CONTENT, Gravity.BOTTOM + Gravity.CENTER_HORIZONTAL) );
        		       // mGLView.setFocusableInTouchMode(true);
        		       // mGLView.setFocusable(true);
        		         //         adView.requestFreshAd();
        setContentView(_videoLayout,
        		new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
        mSurface.requestFocus(); 
        		        

        AdRequest request = new AdRequest();

        request.addTestDevice(AdRequest.TEST_EMULATOR);
        request.addTestDevice("1DA1E070BC7D4ABE8BA77EB73C2CB0AA"); //wololo's phone
        request.addTestDevice("C386F3830A9789C649098A817BF54C04"); //xawotihs's tablet
             
        // Initiate a generic request to load it with an ad
        mAdView.loadAd(request);      	
    }
    // Setup
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Log.v("SDL", "onCreate()");
        super.onCreate(savedInstanceState);
        
        // So we can call stuff from static callbacks
        mSingleton = this;
          
        File file = new File(RES_FOLDER + RES_FILENAME);
        if (file.exists()) {
        	mainDisplay();
        } else {
        	FrameLayout _videoLayout = new FrameLayout(this);
        	setContentView(_videoLayout,
                new LayoutParams(LayoutParams.FILL_PARENT, LayoutParams.FILL_PARENT));
        	startDownload();        	
        }
             
    }

    // Events
    @Override
    protected void onPause() {
        //Log.v("SDL", "onPause()");
        super.onPause();
        SDLActivity.nativePause();   
    }
    
    @Override
    protected void onResume() {
        Log.v("SDL", "onResume()");
        super.onResume();
        SDLActivity.nativeResume();
    }


    @Override
    public void onDestroy() {   
    	Log.v("SDL", "onDestroy()");

    	super.onDestroy();    	
    	mSurface.onDestroy();
    }
 
    // Handler for Messages coming from JGE
    // Suggested syntax for JGE messages is a string separated by the ":" symbol
    protected void processJGEMsg (String command)
    {   
    	if (null == command) {
    		return;
    	}
    	
        if ((command.compareTo("entergamestate:menu") == 0)
        		|| (command.compareTo("enterduelphase:end") == 0)) {
        	mAdView.setVisibility(View.VISIBLE);
        } else if (command.compareTo("leavegamestate:menu") == 0) {
        	mAdView.setVisibility(View.INVISIBLE);
        }   
    }
    
    // Messages from the SDLMain thread
    static int COMMAND_CHANGE_TITLE = 1;
    static int COMMAND_JGE_MSG = 2;

    // Handler for the messages
    Handler commandHandler = new Handler() {
        public void handleMessage(Message msg) {
            if (msg.arg1 == COMMAND_CHANGE_TITLE) {
                setTitle((String)msg.obj);
            }   
            if (msg.arg1 == COMMAND_JGE_MSG) {
                processJGEMsg((String)msg.obj);
            }                
        }
    };

    // Send a message from the SDLMain thread
    void sendCommand(int command, Object data) {
        Message msg = commandHandler.obtainMessage();
        msg.arg1 = command;
        msg.obj = data;
        commandHandler.sendMessage(msg);
    }
    
    // C functions we call
    public static native void nativeInit();
    public static native void nativeQuit();
    public static native void nativePause();
    public static native void nativeResume();    
    public static native void onNativeResize(int x, int y, int format);
    public static native void onNativeKeyDown(int keycode);
    public static native void onNativeKeyUp(int keycode);
    public static native void onNativeTouch(int index, int action, float x, 
                                            float y, float p);
	public static native void onNativeFlickGesture(float xVelocity, float yVelocity);
    public static native void onNativeAccel(float x, float y, float z);
    public static native void nativeRunAudioThread();


    // Java functions called from C
    // Receive a message from the SDLMain thread
    public static void jgeSendCommand(String command) {
    	mSingleton.sendCommand(COMMAND_JGE_MSG, command);
    }    
    
    public static boolean createGLContext(int majorVersion, int minorVersion) {
        return mSurface.initEGL(majorVersion, minorVersion);
    }

    public static void flipBuffers() {
        mSurface.flipEGL();
    }
    
    public static void setActivityTitle(String title) {
        // Called from SDLMain() thread and can't directly affect the view
        mSingleton.sendCommand(COMMAND_CHANGE_TITLE, title);
    }

    // Audio
    private static Object buf;
    
    public static Object audioInit(int sampleRate, boolean is16Bit, boolean isStereo, int desiredFrames) {
        int channelConfig = isStereo ? AudioFormat.CHANNEL_CONFIGURATION_STEREO : AudioFormat.CHANNEL_CONFIGURATION_MONO;
        int audioFormat = is16Bit ? AudioFormat.ENCODING_PCM_16BIT : AudioFormat.ENCODING_PCM_8BIT;
        int frameSize = (isStereo ? 2 : 1) * (is16Bit ? 2 : 1);
        
        Log.v("SDL", "SDL audio: wanted " + (isStereo ? "stereo" : "mono") + " " + (is16Bit ? "16-bit" : "8-bit") + " " + ((float)sampleRate / 1000f) + "kHz, " + desiredFrames + " frames buffer");
        
        // Let the user pick a larger buffer if they really want -- but ye
        // gods they probably shouldn't, the minimums are horrifyingly high
        // latency already
        desiredFrames = Math.max(desiredFrames, (AudioTrack.getMinBufferSize(sampleRate, channelConfig, audioFormat) + frameSize - 1) / frameSize);
        
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
                channelConfig, audioFormat, desiredFrames * frameSize, AudioTrack.MODE_STREAM);
        
        audioStartThread();
        
        Log.v("SDL", "SDL audio: got " + ((mAudioTrack.getChannelCount() >= 2) ? "stereo" : "mono") + " " + ((mAudioTrack.getAudioFormat() == AudioFormat.ENCODING_PCM_16BIT) ? "16-bit" : "8-bit") + " " + ((float)mAudioTrack.getSampleRate() / 1000f) + "kHz, " + desiredFrames + " frames buffer");
        
        if (is16Bit) {
            buf = new short[desiredFrames * (isStereo ? 2 : 1)];
        } else {
            buf = new byte[desiredFrames * (isStereo ? 2 : 1)]; 
        }
        return buf;
    }
    
    public static void audioStartThread() {
        mAudioThread = new Thread(new Runnable() {
            public void run() {
                mAudioTrack.play();
                nativeRunAudioThread();
            }
        });
        
        // I'd take REALTIME if I could get it!
        mAudioThread.setPriority(Thread.MAX_PRIORITY);
        mAudioThread.start();
    }
    
    public static void audioWriteShortBuffer(short[] buffer) {
        for (int i = 0; i < buffer.length; ) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w("SDL", "SDL audio: error return from write(short)");
                return;
            }
        }
    }
    
    public static void audioWriteByteBuffer(byte[] buffer) {
        for (int i = 0; i < buffer.length; ) {
            int result = mAudioTrack.write(buffer, i, buffer.length - i);
            if (result > 0) {
                i += result;
            } else if (result == 0) {
                try {
                    Thread.sleep(1);
                } catch(InterruptedException e) {
                    // Nom nom
                }
            } else {
                Log.w("SDL", "SDL audio: error return from write(short)");
                return;
            }
        }
    }

    public static void audioQuit() {
        if (mAudioThread != null) {
            try {
                mAudioThread.join();
            } catch(Exception e) {
                Log.v("SDL", "Problem stopping audio thread: " + e);
            }
            mAudioThread = null;

            //Log.v("SDL", "Finished waiting for audio thread");
        }

        if (mAudioTrack != null) {
            mAudioTrack.stop();
            mAudioTrack = null;
        }
    }
    
    
    class DownloadFileAsync extends AsyncTask<String, String, String> {
    	@Override
    	protected void onPreExecute() {
    		super.onPreExecute();
    		showDialog(DIALOG_DOWNLOAD_PROGRESS);   		
    	}

    	@Override
    	protected String doInBackground(String... aurl) {
    		int count;

	    	try {
	
	    		//
	    		// Prepare the sdcard folders in order to download the resource file
	    		//
	    		String state = Environment.getExternalStorageState();
	    		if (! Environment.MEDIA_MOUNTED.equals(state)) {
	    			mSingleton.downloadError("cannot write to SD Card, please check your sd card");
	    			return null;
	    		}
	    		File resDirectory = new File(RES_FOLDER); //TODO use getExternalStorageDirectory() and update the C++ code
	    		resDirectory.mkdirs(); 	    		
	    		
		    	URL url = new URL(aurl[0]);
		    	URLConnection conexion = url.openConnection();
		    	conexion.connect();
		
		    	int lenghtOfFile = conexion.getContentLength();
		    	Log.d("ANDRO_ASYNC", "Length of file: " + lenghtOfFile);
		
		    	InputStream input = new BufferedInputStream(url.openStream());
	    		// create a File object for the output file
	    		File outputFile = new File(resDirectory, RES_FILENAME + ".tmp");

		    	
		    	OutputStream output = new FileOutputStream(outputFile);
		
		    	byte data[] = new byte[1024];
		
		    	long total = 0;
	
	    		while ((count = input.read(data)) != -1) {
	    			total += count;
	    			publishProgress(""+(int)((total*100)/lenghtOfFile));
	    			output.write(data, 0, count);
	    		}
	
	    		output.flush();
	    		output.close();
	    		input.close();
	    	} catch (Exception e) {
	    		mSingleton.downloadError("An error happened while downloading the resources. It could be that our server is temporarily down, that your device is not connected to a network, or that we cannot write to " + RES_FOLDER + ". Please check your phone settings and try again. For more help please go to http://wagic.net");
	    	}
	    	return null;
    	}
    	
    	protected void onProgressUpdate(String... progress) {
    		 Log.d("ANDRO_ASYNC",progress[0]);
    		 mProgressDialog.setProgress(Integer.parseInt(progress[0]));
    	}

    	@Override
    	protected void onPostExecute(String unused) {
    		if (mErrorHappened) {
    			dismissDialog(DIALOG_DOWNLOAD_PROGRESS); 
    			showDialog(DIALOG_DOWNLOAD_ERROR);
    			return;
    		}
    		//rename the temporary file into the final filename
    		File preFile = new File(RES_FOLDER + RES_FILENAME + ".tmp");
    		File postFile = new File(RES_FOLDER + RES_FILENAME);
    		preFile.renameTo(postFile);
    		
    		dismissDialog(DIALOG_DOWNLOAD_PROGRESS); 
    		//Start game;
    		mSingleton.mainDisplay();
    	}
    }
       
    
    
}

/**
    SDLSurface. This is what we draw on, so we need to know when it's created
    in order to do anything useful. 

    Because of this, that's where we set up the SDL thread
*/
class SDLSurface extends SurfaceView implements SurfaceHolder.Callback, 
    View.OnKeyListener, View.OnTouchListener, SensorEventListener  {

    // This is what SDL runs in. It invokes SDL_main(), eventually
    private Thread mSDLThread;       
    
    // EGL private objects
    private EGLContext  mEGLContext;
    private EGLSurface  mEGLSurface;
    private EGLDisplay  mEGLDisplay;
    private EGLConfig   mEGLConfig;

    // Sensors
    private static SensorManager mSensorManager;

	private static VelocityTracker mVelocityTracker;

	final private Object mSemSurface;
	private Boolean mSurfaceValid;
	

    void startSDLThread() {
        if (mSDLThread == null) {
            mSDLThread = new Thread(new SDLMain(), "SDLThread"); 
            mSDLThread.start();       
        }    	
    }
    
	/**
    Simple nativeInit() runnable
	*/
	class SDLMain implements Runnable {
	    public void run() {
	        // Runs SDL_main()
            SDLActivity.nativeInit();

            SDLActivity.nativeQuit();
            
	        Log.v("SDL", "SDL thread terminated");
	        
        	// On exit, tear everything down for a fresh restart next time.
            System.exit(0);        
	    }    
	}	
			
    // Startup    
    public SDLSurface(Context context) {
        super(context);
    	mSemSurface = new Object();
    	mSurfaceValid = false;
        getHolder().addCallback(this); 
    
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
        setOnKeyListener(this); 
        setOnTouchListener(this);   

        mSensorManager = (SensorManager)context.getSystemService("sensor");
    }

    // Called when we have a valid drawing surface
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v("SDL", "surfaceCreated()");

        enableSensor(Sensor.TYPE_ACCELEROMETER, true);
    }

    public void onDestroy() {
        // Send a quit message to the application
    	//should that be in SDLActivity "onDestroy" instead ?
        
        SDLActivity.nativeQuit();

        // Now wait for the SDL thread to quit
        if (mSDLThread != null) {
            try {
                mSDLThread.join();
            } catch(Exception e) {
                Log.v("SDL", "Problem stopping thread: " + e);
            }
            mSDLThread = null;
          
            //Log.v("SDL", "Finished waiting for SDL thread");
        }     	
    }
    
    // Called when we lose the surface
    public void surfaceDestroyed(SurfaceHolder holder) {
        Log.v("SDL", "surfaceDestroyed()");
        synchronized(mSemSurface) {
        	mSurfaceValid = false;
        	mSemSurface.notifyAll();
        }
        enableSensor(Sensor.TYPE_ACCELEROMETER, false);
    }
    
    // Called when the surface is resized
    public void surfaceChanged(SurfaceHolder holder,
                               int format, int width, int height) {
        Log.v("SDL", "surfaceChanged()");

        int sdlFormat = 0x85151002; // SDL_PIXELFORMAT_RGB565 by default
        switch (format) {
        case PixelFormat.A_8:
            Log.v("SDL", "pixel format A_8");
            break;
        case PixelFormat.LA_88:
            Log.v("SDL", "pixel format LA_88");
            break;
        case PixelFormat.L_8:
            Log.v("SDL", "pixel format L_8");
            break;
        case PixelFormat.RGBA_4444:
            Log.v("SDL", "pixel format RGBA_4444");
            sdlFormat = 0x85421002; // SDL_PIXELFORMAT_RGBA4444
            break;
        case PixelFormat.RGBA_5551:
            Log.v("SDL", "pixel format RGBA_5551");
            sdlFormat = 0x85441002; // SDL_PIXELFORMAT_RGBA5551
            break;
        case PixelFormat.RGBA_8888:
            Log.v("SDL", "pixel format RGBA_8888");
            sdlFormat = 0x86462004; // SDL_PIXELFORMAT_RGBA8888
            break;
        case PixelFormat.RGBX_8888:
            Log.v("SDL", "pixel format RGBX_8888");
            sdlFormat = 0x86262004; // SDL_PIXELFORMAT_RGBX8888
            break;
        case PixelFormat.RGB_332:
            Log.v("SDL", "pixel format RGB_332");
            sdlFormat = 0x84110801; // SDL_PIXELFORMAT_RGB332
            break;
        case PixelFormat.RGB_565:
            Log.v("SDL", "pixel format RGB_565");
            sdlFormat = 0x85151002; // SDL_PIXELFORMAT_RGB565
            break;
        case PixelFormat.RGB_888:
            Log.v("SDL", "pixel format RGB_888");
            // Not sure this is right, maybe SDL_PIXELFORMAT_RGB24 instead?
            sdlFormat = 0x86161804; // SDL_PIXELFORMAT_RGB888
            break;
        default:
            Log.v("SDL", "pixel format unknown " + format);
            break;
        }
        SDLActivity.onNativeResize(width, height, sdlFormat);    
        
        // Now start up the C app thread
        startSDLThread();   
    }

    // unused
    public void onDraw(Canvas canvas) {}


    // EGL functions
    public boolean initEGL(int majorVersion, int minorVersion) {
        Log.v("SDL", "Starting up OpenGL ES " + majorVersion + "." + minorVersion);

        try {
        	
            EGL10 egl = (EGL10)EGLContext.getEGL();

            EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

            int[] version = new int[2];
            egl.eglInitialize(dpy, version);

            int EGL_OPENGL_ES_BIT = 1;
            int EGL_OPENGL_ES2_BIT = 4;
            int renderableType = 0;
            if (majorVersion == 2) {
                renderableType = EGL_OPENGL_ES2_BIT;
            } else if (majorVersion == 1) {
                renderableType = EGL_OPENGL_ES_BIT;
            }
            int[] configSpec = {
                //EGL10.EGL_DEPTH_SIZE,   16,
                EGL10.EGL_RENDERABLE_TYPE, renderableType,
                EGL10.EGL_NONE
            };
            EGLConfig[] configs = new EGLConfig[1];
            int[] num_config = new int[1];
            if (!egl.eglChooseConfig(dpy, configSpec, configs, 1, num_config) || num_config[0] == 0) {
                Log.e("SDL", "No EGL config available");
                return false;
            }
            mEGLConfig = configs[0];

            EGLContext ctx = egl.eglCreateContext(dpy, mEGLConfig, EGL10.EGL_NO_CONTEXT, null);
            if (ctx == EGL10.EGL_NO_CONTEXT) {
                Log.e("SDL", "Couldn't create context");
                return false;
            }

            mEGLContext = ctx;
            mEGLDisplay = dpy;
            
            if (!createSurface(this.getHolder())) {
            	return false;
            }

        } catch(Exception e) {
            Log.v("SDL", e + "");
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("SDL", s.toString());
            }
        }

        return true;
    }
    
    
    public Boolean createSurface(SurfaceHolder holder) {
        /*
         *  The window size has changed, so we need to create a new
         *  surface.
         */
    	EGL10 egl = (EGL10)EGLContext.getEGL();
        if (mEGLSurface != null) {

            /*
             * Unbind and destroy the old EGL surface, if
             * there is one.
             */
            egl.eglMakeCurrent(mEGLDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
            egl.eglDestroySurface(mEGLDisplay, mEGLSurface);
        }

        /*
         * Create an EGL surface we can render into.
         */
        mEGLSurface = egl.eglCreateWindowSurface(mEGLDisplay, mEGLConfig, holder, null);
        if (mEGLSurface == EGL10.EGL_NO_SURFACE) {
            Log.e("SDL", "Couldn't create surface");
            return false;
        }        

        /*
         * Before we can issue GL commands, we need to make sure
         * the context is current and bound to a surface.
         */
        if (!egl.eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext)) {
            Log.e("SDL", "Couldn't make context current");
            return false;
        }        
        
        mSurfaceValid = true;

        return true;
    }
    

    // EGL buffer flip
    public void flipEGL() {
    	
    	if (!mSurfaceValid)
    	{
    		createSurface(this.getHolder());
    	}
    	
        try {
            EGL10 egl = (EGL10)EGLContext.getEGL();

            egl.eglWaitNative(EGL10.EGL_NATIVE_RENDERABLE, null);

            // drawing here

            egl.eglWaitGL();

            egl.eglSwapBuffers(mEGLDisplay, mEGLSurface);

            
        } catch(Exception e) {
            Log.v("SDL", "flipEGL(): " + e);
            for (StackTraceElement s : e.getStackTrace()) {
                Log.v("SDL", s.toString());
            }
        }
                 
    }

    // Key events
    public boolean onKey(View  v, int keyCode, KeyEvent event) {

        if (event.getAction() == KeyEvent.ACTION_DOWN) {
            //Log.v("SDL", "key down: " + keyCode);
            SDLActivity.onNativeKeyDown(keyCode);
            return true;
        }
        else if (event.getAction() == KeyEvent.ACTION_UP) {
            //Log.v("SDL", "key up: " + keyCode);
            SDLActivity.onNativeKeyUp(keyCode);
            return true;
        }
        
        return false;
    }

    // Touch events
    public boolean onTouch(View v, MotionEvent event) {
    
		for (int index = 0; index < event.getPointerCount(); ++index)
		{
			int action = event.getActionMasked();
			float x = event.getX(index);
			float y = event.getY(index);
			float p = event.getPressure(index);

	        // TODO: Anything else we need to pass?        
	        SDLActivity.onNativeTouch(index, action, x, y, p);
		}
        
		// account for 'flick' type gestures by monitoring velocity
		if (event.getActionIndex() == 0)
		{
			if (event.getAction() == MotionEvent.ACTION_DOWN)
			{
				mVelocityTracker = VelocityTracker.obtain();
				mVelocityTracker.clear();
				mVelocityTracker.addMovement(event);
			}
			else if (event.getAction() == MotionEvent.ACTION_MOVE)
			{
				mVelocityTracker.addMovement(event);
			}
			else if (event.getAction() == MotionEvent.ACTION_UP)
			{
				mVelocityTracker.addMovement(event);

				// calc velocity
				mVelocityTracker.computeCurrentVelocity(1000);
				float xVelocity = mVelocityTracker.getXVelocity(0);
				float yVelocity = mVelocityTracker.getYVelocity(0);
				if ( Math.abs(xVelocity) > 300 || Math.abs(yVelocity) > 300)
					SDLActivity.onNativeFlickGesture(xVelocity, yVelocity);

				mVelocityTracker.recycle();
			}
		}

		return true;
    }

    // Sensor events
    public void enableSensor(int sensortype, boolean enabled) {
        // TODO: This uses getDefaultSensor - what if we have >1 accels?
        if (enabled) {
            mSensorManager.registerListener(this, 
                            mSensorManager.getDefaultSensor(sensortype), 
                            SensorManager.SENSOR_DELAY_GAME, null);
        } else {
            mSensorManager.unregisterListener(this, 
                            mSensorManager.getDefaultSensor(sensortype));
        }
    }
    
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        // TODO
    }

    public void onSensorChanged(SensorEvent event) {
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            SDLActivity.onNativeAccel(event.values[0],
                                      event.values[1],
                                      event.values[2]);
        }
    }    
}




