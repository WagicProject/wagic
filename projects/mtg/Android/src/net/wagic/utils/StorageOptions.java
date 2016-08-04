package net.wagic.utils;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Scanner;

import java.util.HashSet;
import java.util.Set;
import android.os.Build;

import android.os.Environment;
import android.util.Log;

public class StorageOptions
{
    private static final String      TAG     = StorageOptions.class.getCanonicalName();
    private static ArrayList<String> mMounts = new ArrayList<String>();
    private static ArrayList<String> mVold   = new ArrayList<String>();

    public static String[]           labels;
    public static String[]           paths;
    public static int                count   = 0;
    public static String             defaultMountPoint;

    public static void determineStorageOptions()
    {
        initializeMountPoints();
        if (findForcemount()){
            readMountsFileTest();
        }
        readMountsFile();
        readVoldFile();
        if (findForcemount()){
            removeDuplicates(mMounts);
        }
        compareMountsWithVold();
        testAndCleanMountsList();
        setProperties();
    }

    private static void initializeMountPoints()
    {
        try
        {
            defaultMountPoint = Environment.getExternalStorageDirectory().getCanonicalPath();
        } catch (Exception ioEx)
        {
            // an error occurred trying to get the canonical path, use '/mnt/sdcard' instead
            defaultMountPoint = "/mnt/sdcard";
        }
    }

    private static void readMountsFileTest()
    {
        /*
         * Test mountpoints storage -kevlahnota
         */

        try
        {
            Scanner scanner = new Scanner(new File("/proc/mounts"));
            while (scanner.hasNext())
            {
                String line = scanner.nextLine();
                if (line.startsWith("/"))
                {
                    String[] lineElements = line.split("\\s+");
                    if ("vfat".equals(lineElements[2]) || "fuse".equals(lineElements[2]) || "sdcardfs".equals(lineElements[2])) 
                    {
                        File mountPoint = new File(lineElements[1]);
                        if (!lineElements[1].equals(defaultMountPoint))
                            if (mountPoint.isDirectory() && mountPoint.canRead())
                                mMounts.add(lineElements[1]);
                    }
                }
            }
        } catch (FileNotFoundException fnfex)
        {
            // if proc/mount doesn't exist we just use
            Log.i(TAG, fnfex.getMessage() + ": assuming " + defaultMountPoint + " is the only mount point");
            mMounts.add(defaultMountPoint);
        } catch (Exception e)
        {
            Log.e(TAG, e.getMessage() + ": unknown exception while reading mounts file");
            mMounts.add(defaultMountPoint);
        }
    }

    private static void readMountsFile()
    {
        /*
         * Scan the /proc/mounts file and look for lines like this: /dev/block/vold/179:1 /mnt/sdcard vfat
         * rw,dirsync,nosuid,nodev,noexec,relatime,uid=1000,gid=1015,fmask=0602,dmask=0602,allow_utime=0020,codepage=cp437,iocharset=iso8859-1,shortname=mixed,utf8,errors=remount-ro 0 0
         * 
         * When one is found, split it into its elements and then pull out the path to the that mount point and add it to the arraylist
         */

        try
        {
            Scanner scanner = new Scanner(new File("/proc/mounts"));
            while (scanner.hasNext())
            {
                String line = scanner.nextLine();
                if (line.startsWith("/dev/block/vold/"))
                {
                    String[] lineElements = line.split(" ");
                    lineElements[1].replaceAll(":.*$", "");
                    mMounts.add(lineElements[1]);
                }
            }
        } catch (FileNotFoundException fnfex)
        {
            // if proc/mount doesn't exist we just use
            Log.i(TAG, fnfex.getMessage() + ": assuming " + defaultMountPoint + " is the only mount point");
            mMounts.add(defaultMountPoint);
        } catch (Exception e)
        {
            Log.e(TAG, e.getMessage() + ": unknown exception while reading mounts file");
            mMounts.add(defaultMountPoint);
        }
    }

    private static void readVoldFile()
    {
        /*
         * Scan the /system/etc/vold.fstab file and look for lines like this: dev_mount sdcard /mnt/sdcard 1 /devices/platform/s3c-sdhci.0/mmc_host/mmc0
         * 
         * When one is found, split it into its elements and then pull out the path to the that mount point and add it to the arraylist
         */

        try
        {
            Scanner scanner = new Scanner(new File("/system/etc/vold.fstab"));
            while (scanner.hasNext())
            {
                String line = scanner.nextLine();
                if (line.startsWith("dev_mount"))
                {
                    String[] lineElements = line.split(" ");
                    lineElements[2] = lineElements[2].replaceAll(":.*$", "");
                    mVold.add(lineElements[2]);
                }
            }
        } catch (FileNotFoundException fnfex)
        {
            // if vold.fstab doesn't exist we use the value gathered from the Environment
            Log.i(TAG, fnfex.getMessage() + ": assuming " + defaultMountPoint + " is the only mount point");
            mMounts.add(defaultMountPoint);
        } catch (Exception e)
        {
            Log.e(TAG, e.getMessage() + ": unknown exception while reading vold.fstab file");
            mMounts.add(defaultMountPoint);
        }
    }
    
    private static ArrayList<String> removeDuplicates(ArrayList<String> list) 
    {
        ArrayList<String> result = new ArrayList<String>();

        HashSet<String> set = new HashSet<String>();

        for (String item : list) 
        {
            if (!set.contains(item)) 
            {
                result.add(item);
                set.add(item);
            }
        }
        return result;
    }

    private static void compareMountsWithVold()
    {
        /*
         * Sometimes the two lists of mount points will be different. We only want those mount points that are in both list.
         *
         * Compare the two lists together and remove items that are not in both lists.
         */

        if (mVold.size() > 0)
        {
            for (int i = 0; i < mMounts.size(); i++)
            {
                String mount = mMounts.get(i);
                if (!mVold.contains(mount))
                    mMounts.remove(i--);
            }
        }

        // don't need this anymore, clear the vold list to reduce memory
        // use and to prepare it for the next time it's needed.
        mVold.clear();
    }

    private static void testAndCleanMountsList()
    {
        /*
         * Now that we have a cleaned list of mount paths Test each one to make sure it's a valid and available path. If it is not, remove it from the list.
         */
        int t = 0;
        for (int i = 0; i < mMounts.size(); i++)
        {
            t++;
            String mount = mMounts.get(i);
            File root = new File(mount);
            if (!root.exists() || !root.isDirectory() || !root.canWrite())
                mMounts.remove(i--);
        }

        if (t == 0 && Build.VERSION.SDK_INT >= 16 && findForcemount())
        {
            //if none is found lets force it for Jellybean and above...
            if (System.getenv("EXTERNAL_STORAGE") != null)
            {
                File root = new File(System.getenv("EXTERNAL_STORAGE"));
                if (root.exists() && root.isDirectory() && root.canWrite())
                {
                    if(!isRooted())
                    {
                        File folder = new File(System.getenv("EXTERNAL_STORAGE")+"/Android/data/net.wagic.app/files");
                        folder.mkdirs();
                        mMounts.add(folder.toString());
                    }
                    else
                    {
                        mMounts.add(System.getenv("EXTERNAL_STORAGE"));
                    }
                }
            }

            if (System.getenv("SECONDARY_STORAGE") != null)
            {
                File root = new File(System.getenv("SECONDARY_STORAGE"));
                if (root.exists() && root.isDirectory() && root.canWrite())
                {
                    if(!isRooted())
                    {
                        File folder = new File(System.getenv("SECONDARY_STORAGE")+"/Android/data/net.wagic.app/files");
                        folder.mkdirs();
                        mMounts.add(folder.toString());
                    }
                    else
                    {
                        mMounts.add(System.getenv("SECONDARY_STORAGE"));
                    }
                }
            }
        }
    }

    private static void setProperties()
    {
        Log.d(TAG, "setProperties()");
        /*
         * At this point all the paths in the list should be valid. Build the public properties.
         */

        ArrayList<String> mLabels = new ArrayList<String>();

        int i = 1;
        if(findForcemount()){
            for (String path : mMounts)
            {//with forcemount menu
                if ("/mnt/sdcard".equalsIgnoreCase(path) || "/storage/sdcard0".equalsIgnoreCase(path))
                    mLabels.add("Internal SD " + "[" + path + "]");
                else if (path.contains("emulated"))
                    mLabels.add("Emulated SD " + " [" + path + "]");
                else
                    mLabels.add("External SD " + " [" + path + "]");
            }
        }
        else
        {
            for (String path : mMounts)
            {
                // TODO: /mnt/sdcard is assumed to always mean internal storage. Use this comparison until there is a better way to do this
                if ("/mnt/sdcard".equalsIgnoreCase(path))
                    mLabels.add("Built-in Storage");
                else
                    mLabels.add("External SD Card " + i++);
            }
        }
        
        labels = new String[mLabels.size()];
        mLabels.toArray(labels);

        paths = new String[mMounts.size()];
        mMounts.toArray(paths);

        count = Math.min(labels.length, paths.length);

        // don't need this anymore, clear the mounts list to reduce memory
        // use and to prepare it for the next time it's needed.
        mMounts.clear();
    }
    
    private static boolean isExternalStorageReadOnly() {
        String extStorageState = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(extStorageState)) {
            return true;
        }
        return false;
    }
 
    private static boolean isExternalStorageAvailable() {
        String extStorageState = Environment.getExternalStorageState();
        if (Environment.MEDIA_MOUNTED.equals(extStorageState)) {
            return true;
        }
        return false;
    }
    
    /**
    * Checks if the device is rooted.
    *
    * @return <code>true</code> if the device is rooted, <code>false</code> otherwise.
    */
    public static boolean isRooted()
    {
        // get from build info
        String buildTags = android.os.Build.TAGS;
        if (buildTags != null && buildTags.contains("test-keys"))
        {
            return true;
        }

        // check if /system/app/Superuser.apk is present
        try
        {
            File file = new File("/system/app/Superuser.apk");
            if (file.exists())
            {
                return true;
            }
        } catch (Exception e1)
        {
            // ignore
        }
        try
        {
            File file = new File("/system/app/Superuser/Superuser.apk");
            if (file.exists())
            {
                return true;
            }
        } catch (Exception e1)
        {
            // ignore
        }
        //SuperSU
        try
        {
            File file = new File("/system/app/SuperSU.apk");
            if (file.exists())
            {
                return true;
            }
        } catch (Exception e1)
        {
            // ignore
        }
        try
        {
            File file = new File("/system/app/SuperSU/SuperSU.apk");
            if (file.exists())
            {
                return true;
            }
        } catch (Exception e1)
        {
            // ignore
        }
        // try executing commands
        return canExecuteCommand("/system/xbin/which su")
                || canExecuteCommand("/system/bin/which su") || canExecuteCommand("which su");
    }

    // executes a command on the system
    private static boolean canExecuteCommand(String command)
    {
        boolean executedSuccesfully;
        try
        {
            Runtime.getRuntime().exec(command);
            executedSuccesfully = true;
        } catch (Exception e)
        {
            executedSuccesfully = false;
        }

        return executedSuccesfully;
    }

    private static boolean findForcemount()
    {
        Log.d(TAG, "findForcemount()");
        try
        {
            File file = new File(System.getenv("EXTERNAL_STORAGE") + "/forcemount");
            if (file.exists())
            {
                return true;
            }
        } catch (Exception e)
        {
            Log.w(TAG, e.getMessage());
            return false;
        }
        return false;
    }
}
