package net.wagic.utils;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.jsoup.nodes.Node;

import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.Enumeration;

import net.lingala.zip4j.model.ZipParameters;
import net.lingala.zip4j.model.enums.CompressionMethod;

import java.io.*;
import java.net.URL;
import java.net.HttpURLConnection;
import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;

import android.graphics.*;
import android.app.ProgressDialog;
import org.libsdl.app.SDLActivity;

public class ImgDownloader {

    private static String readLineByLineJava8(String filePath) {
        StringBuilder contentBuilder = new StringBuilder();

        try {
            File file = new File(filePath);
            BufferedReader br = new BufferedReader(new FileReader(file));

            String st;
            while ((st = br.readLine()) != null)
                contentBuilder.append(st).append("\n");
        } catch (Exception e) {
            e.printStackTrace();
        }

        return contentBuilder.toString();
    }

    static HashMap<String, HashMap<String, String>> database;

    public static boolean loadDatabase(String path, String databaseurl) {
        database = new HashMap<String, HashMap<String, String>>();
        try {
            URL url = new URL(databaseurl);
            HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
            if (httpcon == null) {
                System.err.println("Error: Problem downloading or initializing database file, i will use the slow method...");
                database = null;
                return false;
            }
            httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
            httpcon.setConnectTimeout(30000);
            httpcon.setReadTimeout(30000);
            httpcon.setAllowUserInteraction(false);
            httpcon.setDoInput(true);
            httpcon.setDoOutput(false);
            InputStream in;
            try {
                in = new BufferedInputStream(httpcon.getInputStream());
            } catch (Exception ex) {
                try {
                    in = new BufferedInputStream(httpcon.getInputStream());
                } catch (Exception ex2) {
                    try {
                        in = new BufferedInputStream(httpcon.getInputStream());
                    } catch (Exception ex3) {
                        System.err.println("Error: Problem downloading or initializing database file, i will use the slow method...");
                        database = null;
                        return false;
                    }
                }
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] buf = new byte[1024];
            int n = 0;
            long millis = System.currentTimeMillis();
            boolean timeout = false;
            while (-1 != (n = in.read(buf)) && !timeout) {
                out.write(buf, 0, n);
                if (System.currentTimeMillis() - millis > 30000)
                    timeout = true;
            }
            if (timeout) {
                System.out.println("Warning: Timeout downloading database file, i will retry 2 times more...");
                buf = new byte[1024];
                n = 0;
                millis = System.currentTimeMillis();
                timeout = false;
                while (-1 != (n = in.read(buf)) && !timeout) {
                    out.write(buf, 0, n);
                    if (System.currentTimeMillis() - millis > 30000)
                        timeout = true;
                }
                if (timeout) {
                    System.out.println("Warning: Timeout downloading database file, i will retry 1 time more...");
                    buf = new byte[1024];
                    n = 0;
                    millis = System.currentTimeMillis();
                    timeout = false;
                    while (-1 != (n = in.read(buf)) && !timeout) {
                        out.write(buf, 0, n);
                        if (System.currentTimeMillis() - millis > 30000)
                            timeout = true;
                    }
                }
            }
            out.close();
            in.close();
            if (timeout) {
                System.err.println("Error: Timeout downloading database file, i will use the slow method...");
                return false;
            }
            byte[] response = out.toByteArray();
            String databasepath = path + File.separator + "CardImageLinks.csv";
            FileOutputStream fos = new FileOutputStream(databasepath);
            fos.write(response);
            fos.close();

            String lines = readLineByLineJava8(databasepath);
            String[] rows = lines.split("\n");
            for (int i = 1; i < rows.length; i++) {
                String[] cols = rows[i].split(";");
                if (database.get(cols[0]) == null)
                    database.put(cols[0], new HashMap<String, String>());
                database.get(cols[0]).put(cols[1], cols[2]);
            }
            File del = new File(databasepath);
            del.delete();
        } catch (Exception e) {
            System.err.println("Error: Problem downloading or initializing database file, i will use the slow method...");
            database = null;
            return false;
        }
        return true;
    }

    public static boolean fastDownloadCard(String set, String id, String name, String imgPath, String thumbPath, int ImgX, int ImgY, int ThumbX, int ThumbY, int Border, int BorderThumb) {
        if (database == null)
            return false;
        HashMap<String, String> subdb = database.get(set);
        if (subdb == null)
            return false;
        String imageurl = subdb.get(id);
        if (imageurl == null)
            return false;
        try {
            URL url = new URL(imageurl);
            if (url == null) {
                System.out.println("Warning: Problem fetching card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                return false;
            }
            HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
            if (httpcon == null) {
                System.out.println("Warning: Problem fetching card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                return false;
            }
            httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
            httpcon.setConnectTimeout(5000);
            httpcon.setReadTimeout(5000);
            httpcon.setAllowUserInteraction(false);
            httpcon.setDoInput(true);
            httpcon.setDoOutput(false);
            InputStream in;
            try {
                in = new BufferedInputStream(httpcon.getInputStream());
            } catch (Exception ex) {
                System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will retry 2 times more...");
                try {
                    in = new BufferedInputStream(httpcon.getInputStream());
                } catch (Exception ex2) {
                    System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will retry 1 time more...");
                    try {
                        in = new BufferedInputStream(httpcon.getInputStream());
                    } catch (Exception ex3) {
                        System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                        return false;
                    }
                }
            }
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            byte[] buf = new byte[1024];
            int n = 0;
            long millis = System.currentTimeMillis();
            boolean timeout = false;
            while (-1 != (n = in.read(buf)) && !timeout) {
                out.write(buf, 0, n);
                if (System.currentTimeMillis() - millis > 10000)
                    timeout = true;
            }
            if (timeout) {
                System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will retry 2 times more...");
                buf = new byte[1024];
                n = 0;
                millis = System.currentTimeMillis();
                timeout = false;
                while (-1 != (n = in.read(buf)) && !timeout) {
                    out.write(buf, 0, n);
                    if (System.currentTimeMillis() - millis > 10000)
                        timeout = true;
                }
                if (timeout) {
                    System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will retry 1 time more...");
                    buf = new byte[1024];
                    n = 0;
                    millis = System.currentTimeMillis();
                    timeout = false;
                    while (-1 != (n = in.read(buf)) && !timeout) {
                        out.write(buf, 0, n);
                        if (System.currentTimeMillis() - millis > 10000)
                            timeout = true;
                    }
                }
            }
            if (timeout) {
                System.out.println("Warning: Problem downloading card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                return false;
            }
            out.close();
            in.close();
            byte[] response = out.toByteArray();
            String cardimage = imgPath + File.separator + id + ".jpg";
            String thumbcardimage = thumbPath + File.separator + id + ".jpg";
            FileOutputStream fos = new FileOutputStream(cardimage);
            fos.write(response);
            fos.close();
            try {
                Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                if(Border > 0)
                    resized = Bitmap.createBitmap(resized, Border, Border, ImgX-2*Border, ImgY-2*Border);
                FileOutputStream fout = new FileOutputStream(cardimage);
                resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                fout.close();
            } catch (Exception e) {
                System.out.println("Warning: Problem resizing card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                return false;
            }
            try {
                Bitmap yourBitmapThumb = BitmapFactory.decodeFile(cardimage);
                Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmapThumb, ThumbX, ThumbY, true);
                if(BorderThumb > 0)
                    resizedThumb = Bitmap.createBitmap(resizedThumb, BorderThumb, BorderThumb, ThumbX-2*BorderThumb, ThumbY-2*BorderThumb);
                FileOutputStream fout = new FileOutputStream(thumbcardimage);
                resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                fout.close();
            } catch (Exception e) {
                System.out.println("Warning: Problem resizing card thumbnail: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
                return false;
            }
        } catch (Exception e) {
            System.out.println("Warning: Problem fetching card: " + name + " (" + id + ".jpg) from " + imageurl + ", i will try with slow method...");
            return false;
        }
        imageurl = subdb.get(id + "t");
        if (imageurl != null && !imageurl.isEmpty()) {
            System.err.println("The card: " + name + " (" + id + ".jpg) can create a token, i will try to download that image too as " + id + "t.jpg");
            try {
                URL url = new URL(imageurl);
                if (url == null) {
                    System.out.println("Warning: Problem fetching token: " + id + "t.jpg from " + imageurl + ", i will try with slow method...");
                    return false;
                }
                HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                if (httpcon == null) {
                    System.out.println("Warning: Problem fetching token: " + id + "t.jpg from " + imageurl + ", i will try with slow method...");
                    return false;
                }
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                httpcon.setConnectTimeout(5000);
                httpcon.setReadTimeout(5000);
                httpcon.setAllowUserInteraction(false);
                httpcon.setDoInput(true);
                httpcon.setDoOutput(false);
                InputStream intoken;
                try {
                    intoken = new BufferedInputStream(httpcon.getInputStream());
                } catch (Exception ex) {
                    System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will retry 2 times more...");
                    try {
                        intoken = new BufferedInputStream(httpcon.getInputStream());
                    } catch (Exception ex2) {
                        System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will retry 1 time more...");
                        try {
                            intoken = new BufferedInputStream(httpcon.getInputStream());
                        } catch (Exception ex3) {
                            System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will try with slow method...");
                            return false;
                        }
                    }
                }
                ByteArrayOutputStream outtoken = new ByteArrayOutputStream();
                byte[] buftoken = new byte[1024];
                int ntoken = 0;
                long millis = System.currentTimeMillis();
                boolean timeout = false;
                while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                    outtoken.write(buftoken, 0, ntoken);
                    if (System.currentTimeMillis() - millis > 10000)
                        timeout = true;
                }
                if (timeout) {
                    System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will retry 2 times more...");
                    buftoken = new byte[1024];
                    ntoken = 0;
                    millis = System.currentTimeMillis();
                    timeout = false;
                    while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                        outtoken.write(buftoken, 0, ntoken);
                        if (System.currentTimeMillis() - millis > 10000)
                            timeout = true;
                    }
                    if (timeout) {
                        System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will retry 1 time more...");
                        buftoken = new byte[1024];
                        ntoken = 0;
                        millis = System.currentTimeMillis();
                        timeout = false;
                        while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                            outtoken.write(buftoken, 0, ntoken);
                            if (System.currentTimeMillis() - millis > 10000)
                                timeout = true;
                        }
                    }
                }
                outtoken.close();
                intoken.close();
                if (timeout) {
                    System.out.println("Warning: Problem downloading token: " + id + "t.jpg from " + imageurl + ", i will try with slow method...");
                    return false;
                }
                byte[] responsetoken = outtoken.toByteArray();
                String tokenimage = imgPath + File.separator + id + "t.jpg";
                String tokenthumbimage = thumbPath + File.separator + id + "t.jpg";
                FileOutputStream fos2 = new FileOutputStream(tokenimage);
                fos2.write(responsetoken);
                fos2.close();
                try {
                    Bitmap yourBitmapToken = BitmapFactory.decodeFile(tokenimage);
                    Bitmap resizedToken = Bitmap.createScaledBitmap(yourBitmapToken, ImgX, ImgY, true);
                    if(Border > 0)
                        resizedToken = Bitmap.createBitmap(resizedToken, Border, Border, ImgX-2*Border, ImgY-2*Border);
                    FileOutputStream fout = new FileOutputStream(tokenimage);
                    resizedToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    fout.close();
                } catch (Exception e) {
                    System.out.println("Warning: Problem resizing token: " + id + "t.jpg) from " + imageurl + ", i will try with slow method...");
                    return false;
                }
                try {
                    Bitmap yourBitmapTokenThumb = BitmapFactory.decodeFile(tokenimage);
                    Bitmap resizedThumbToken = Bitmap.createScaledBitmap(yourBitmapTokenThumb, ThumbX, ThumbY, true);
                    if(BorderThumb > 0)
                        resizedThumbToken = Bitmap.createBitmap(resizedThumbToken, BorderThumb, BorderThumb, ThumbX-2*BorderThumb, ThumbY-2*BorderThumb);
                    FileOutputStream fout = new FileOutputStream(tokenthumbimage);
                    resizedThumbToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    fout.close();
                } catch (Exception e) {
                    System.out.println("Warning: Problem resizing token thumbnail: " + id + "t.jpg) from " + imageurl + ", i will try with slow method...");
                    return false;
                }
            } catch (Exception e) {
                System.out.println("Warning: Problem fetching token: " + id + "t.jpg from " + imageurl + ", i will try with slow method...");
                return false;
            }
        }
        return true;
    }

    public static String getSetInfo(String setName, boolean zipped, String path) {
        String cardsfilepath = "";
        boolean todelete = false;
        if (zipped) {
            File resFolder = new File(path + File.separator);
            File[] listOfFile = resFolder.listFiles();
            ZipFile zipFile = null;
            InputStream stream = null;
            File filePath = null;
            try {
                for (int i = 0; i < listOfFile.length; i++) {
                    if (listOfFile[i].getName().contains(".zip")) {
                        zipFile = new ZipFile(path + File.separator + listOfFile[i].getName());
                        break;
                    }
                }
                if (zipFile == null)
                    return "";
                Enumeration<? extends ZipEntry> e = zipFile.entries();
                while (e.hasMoreElements()) {
                    ZipEntry entry = e.nextElement();
                    String entryName = entry.getName();
                    if (entryName.contains("sets" + File.separator)) {
                        if (entryName.contains("_cards.dat")) {
                            String[] names = entryName.split(File.separator);
                            if (setName.equalsIgnoreCase(names[1])) {
                                stream = zipFile.getInputStream(entry);
                                byte[] buffer = new byte[1];
                                filePath = new File(path + File.separator + "_cards.dat");
                                try {
                                    FileOutputStream fos = new FileOutputStream(filePath);
                                    BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                                    int len;
                                    while ((len = stream.read(buffer)) != -1) {
                                        bos.write(buffer, 0, len);
                                    }
                                    fos.close();
                                    bos.close();
                                    cardsfilepath = filePath.getAbsolutePath();
                                    todelete = true;
                                } catch (Exception ex) {
                                }
                                break;
                            }
                        }
                    }
                }
            } catch (IOException ioe) {
            } finally {
                try {
                    if (zipFile != null) {
                        zipFile.close();
                    }
                } catch (IOException ioe) {
                }
            }
        } else {
            File setFolder = new File(path + File.separator + "sets" + File.separator + setName + File.separator);
            cardsfilepath = setFolder.getAbsolutePath() + File.separator + "_cards.dat";
        }
        String lines = readLineByLineJava8(cardsfilepath);
        if (todelete) {
            File del = new File(cardsfilepath);
            del.delete();
        }
        int totalcards = 0;
        String findStr = "total=";
        int lastIndex = lines.indexOf(findStr);
        String totals = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
        totalcards = Integer.parseInt(totals.split("=")[1]);
        findStr = "name=";
        lastIndex = lines.indexOf(findStr);
        String name = lines.substring(lastIndex, lines.indexOf("\n", lastIndex)).split("=")[1];
        return name + " (" + totalcards + " cards)";
    }

    public static String getSpecialCardUrl(String id) {
        String cardurl = "";

        if(id.equals("15208711t"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/c/9c138bf9-8be6-4f1a-a82c-a84938ab84f5.jpg?1562279137";
        else if(id.equals("15208712t"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/4/d453ee89-6122-4d51-989c-e78b046a9de3.jpg?1561758141";
        else if(id.equals("2050321t"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/8/18b9c83d-4422-4b95-9fc2-070ed6b5bdf6.jpg?1562701921";
        else if(id.equals("22010012t"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/4/84dc847c-7a37-4c7f-b02c-30b3e4c91fb6.jpg?1561757490";
        else if(id.equals("4143881t"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/a/8a73e348-5bf1-4465-978b-3f31408bade9.jpg?1561757530";
        else if(id.equals("8759611"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/1/41004bdf-8e09-4b2c-9e9c-26c25eac9854.jpg?1562493483";
        else if(id.equals("8759911"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/b/0b61d772-2d8b-4acf-9dd2-b2e8b03538c8.jpg?1562492461";
        else if(id.equals("8759511"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/2/d224c50f-8146-4c91-9401-04e5bd306d02.jpg?1562496100";
        else if(id.equals("8471611"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/4/84920a21-ee2a-41ac-a369-347633d10371.jpg?1562494702";
        else if(id.equals("8760011"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/2/42ba0e13-d20f-47f9-9c86-2b0b13c39ada.jpg?1562493487";
        else if(id.equals("7448911"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/a/ca03131a-9bd4-4fba-b95c-90f1831e86e7.jpg?1562879774";
        else if(id.equals("7453611"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/3/73636ca0-2309-4bb3-9300-8bd0c0bb5b31.jpg?1562877808";
        else if(id.equals("7447611"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/8/28f72260-c8f9-4c44-92b5-23cef6690fdd.jpg?1562876119";
        else if(id.equals("7467111"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/f/1fe2b76f-ddb7-49d5-933b-ccb06be5d46f.jpg?1562875903";
        else if(id.equals("7409311"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/5/758abd53-6ad2-406e-8615-8e48678405b4.jpg?1562877848";
        else if(id.equals("3896122t"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/9/59a00cac-53ae-46ad-8468-e6d1db40b266.jpg?1562542382";
        else if(id.equals("11492113t"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/b/5b9f471a-1822-4981-95a9-8923d83ddcbf.jpg?1562702075";
        else if(id.equals("3896523t"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/0/d0cd85cc-ad22-446b-8378-5eb69fee1959.jpg?1562840712";
        else if(id.equals("7897511"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/4/a4f4aa3b-c64a-4430-b1a2-a7fca87d0a22.jpg?1562763433";
        else if(id.equals("7868811"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/3/b3523b8e-065f-427c-8d5b-eb731ca91ede.jpg?1562763691";
        else if(id.equals("7868711"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/8/58164521-aeec-43fc-9db9-d595432dea6f.jpg?1564694999";
        else if(id.equals("7868611"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/3/33a8e5b9-6bfb-4ff2-a16d-3168a5412807.jpg?1562758927";
        else if(id.equals("7869111"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/d/9de1eebf-5725-438c-bcf0-f3a4d8a89fb0.jpg?1562762993";
        else if(id.equals("7860011"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/6/864ad989-19a6-4930-8efc-bbc077a18c32.jpg?1562762069";
        else if(id.equals("7867911"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/8/c8265c39-d287-4c5a-baba-f2f09dd80a1c.jpg?1562764226";
        else if(id.equals("7867811"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg?1564694995";
        else if(id.equals("7869511"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/2/f2ddf1a3-e6fa-4dd0-b80d-1a585b51b934.jpg?1562765664";
        else if(id.equals("7869411"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/e/6ee6cd34-c117-4d7e-97d1-8f8464bfaac8.jpg?1562761096";
        else if(id.equals("209163t"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/3/a3ea39a8-48d1-4a58-8662-88841eabec92.jpg?1562925559";
        else if(id.equals("111066t"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/7/a77c1ac0-5548-42b0-aa46-d532b3518632.jpg?1562578875";
        else if(id.equals("2050322t"))
            cardurl = "https://deckmaster.info/images/cards/M11/-239-hr.jpg";
        else if(id.equals("401721t"))
            cardurl = "https://deckmaster.info/images/cards/DDP/401721-hr.jpg";
        else if(id.equals("401722t"))
            cardurl = "https://deckmaster.info/images/cards/DDP/401722-hr.jpg";
        else if(id.equals("19784311t"))
            cardurl = "https://deckmaster.info/images/cards/AKH/-4173-hr.jpg";
        else if(id.equals("19784312t"))
            cardurl = "https://deckmaster.info/images/cards/BNG/-10-hr.jpg";
        else if(id.equals("19784313t"))
            cardurl = "https://deckmaster.info/images/cards/DDD/201843-hr.jpg";
        else if(id.equals("20787512t"))
            cardurl = "https://deckmaster.info/images/cards/SOM/-227-hr.jpg";
        else if(id.equals("20787511t"))
            cardurl = "https://deckmaster.info/images/cards/SOM/-226-hr.jpg";
        else if(id.equals("11492111t"))
            cardurl = "https://deckmaster.info/images/cards/TSP/-2841-hr.jpg";
        else if(id.equals("11492112t"))
            cardurl = "https://deckmaster.info/images/cards/TSP/-2840-hr.jpg";
        else if(id.equals("11492114t") || id.equals("16932t") || id.equals("293980t") || id.equals("293981t"))
            cardurl = "https://deckmaster.info/images/cards/DDN/386322-hr.jpg";
        else if(id.equals("11492115t") || id.equals("209162t") || id.equals("17010t") || id.equals("16997t"))
            cardurl = "https://deckmaster.info/images/cards/DDE/209162-hr.jpg";
        else if(id.equals("3896522t"))
            cardurl = "https://deckmaster.info/images/cards/C14/-474-hr.jpg";
        else if(id.equals("3896521t"))
            cardurl = "https://deckmaster.info/images/cards/C14/-472-hr.jpg";
        else if(id.equals("207998t"))
            cardurl = "https://deckmaster.info/images/cards/DDE/207998-hr.jpg";
        else if (id.equals("19784555t"))
            cardurl = "https://deckmaster.info/images/cards/DGM/-39-hr.jpg";
        else if (id.equals("19784612t"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-60-hr.jpg";
        else if (id.equals("19784613t"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-62-hr.jpg";
        else if (id.equals("19784611t"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-55-hr.jpg";
        else if (id.equals("4977511t"))
            cardurl = "https://deckmaster.info/images/cards/DST/-2819-hr.jpg";
        else if (id.equals("4977512t"))
            cardurl = "https://deckmaster.info/images/cards/DST/-2818-hr.jpg";
        else if(id.equals("111220t"))
            cardurl = "https://deckmaster.info/images/cards/DIS/111220-hr.jpg";
        else if(id.equals("383257t"))
            cardurl = "https://deckmaster.info/images/cards/M15/-109-hr.jpg";
        else if(id.equals("383290t"))
            cardurl = "https://deckmaster.info/images/cards/M15/-108-hr.jpg";
        else if(id.equals("378445t"))
            cardurl = "https://deckmaster.info/images/cards/BNG/-11-hr.jpg";
        else if(id.equals("378521t"))
            cardurl = "https://deckmaster.info/images/cards/DDO/394383-hr.jpg";
        else if(id.equals("16699t"))
            cardurl = "https://deckmaster.info/images/cards/NPH/-205-hr.jpg";
        else if(id.equals("16708t") || id.equals("17097t") || id.equals("17085t"))
            cardurl = "https://deckmaster.info/images/cards/M10/-292-hr.jpg";
        else if(id.equals("16710t"))
            cardurl = "https://deckmaster.info/images/cards/M11/-238-hr.jpg";
        else if(id.equals("16717t"))
            cardurl = "https://deckmaster.info/images/cards/MBS/-212-hr.jpg";
        else if(id.equals("16718t"))
            cardurl = "http://1.bp.blogspot.com/-0-mLvfUVgNk/VmdZWXWxikI/AAAAAAAAAUM/TVCIiZ_c67g/s1600/Spawn%2BToken.jpg";
        else if(id.equals("16729t"))
            cardurl = "https://deckmaster.info/images/cards/MRD/-2829-hr.jpg";
        else if (id.equals("53054t"))
            cardurl = "https://deckmaster.info/images/cards/BNG/-10-hr.jpg";
        else if(id.equals("52993t"))
            cardurl = "https://deckmaster.info/images/cards/TSP/-114916-hr.jpg";
        else if(id.equals("52973t"))
            cardurl = "https://deckmaster.info/images/cards/RTR/-62-hr.jpg";
        else if(id.equals("52593t") || id.equals("294265t"))
            cardurl = "https://deckmaster.info/images/cards/DDR/417498-hr.jpg";
        else if(id.equals("52492t"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/3/f32ad93f-3fd5-465c-ac6a-6f8fb57c19bd.jpg?1561758422";
        else if(id.equals("52418t"))
            cardurl = "https://deckmaster.info/images/cards/DDO/394383-hr.jpg";
        else if(id.equals("52398t"))
            cardurl = "https://deckmaster.info/images/cards/XLN/-5168-hr.jpg";
        else if(id.equals("52149t"))
            cardurl = "https://deckmaster.info/images/cards/GRN/-6433-hr.jpg";
        else if(id.equals("52136t"))
            cardurl = "https://deckmaster.info/images/cards/DDO/394407-hr.jpg";
        else if(id.equals("52637t") || id.equals("52945t"))
            cardurl = "https://deckmaster.info/images/cards/MBS/-216-hr.jpg";
        else if(id.equals("74272"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/5/45af7f55-9a69-43dd-969f-65411711b13e.jpg?1562487939";
        else if(id.equals("687701"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2437-hr.jpg";
        else if(id.equals("687702"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3069-hr.jpg";
        else if(id.equals("687703"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2443-hr.jpg";
        else if(id.equals("687704"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2444-hr.jpg";
        else if(id.equals("687705"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2450-hr.jpg";
        else if(id.equals("687713"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3175-hr.jpg";
        else if(id.equals("687712"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2624-hr.jpg";
        else if(id.equals("687711"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3168-hr.jpg";
        else if(id.equals("687710"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3161-hr.jpg";
        else if(id.equals("687709"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2485-hr.jpg";
        else if(id.equals("687752"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3085-hr.jpg";
        else if(id.equals("687707"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2478-hr.jpg";
        else if(id.equals("687751"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3083-hr.jpg";
        else if(id.equals("687720"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2652-hr.jpg";
        else if(id.equals("687719"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2650-hr.jpg";
        else if(id.equals("687718"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3178-hr.jpg";
        else if(id.equals("687717"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2641-hr.jpg";
        else if(id.equals("687716"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2634-hr.jpg";
        else if(id.equals("687715"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2631-hr.jpg";
        else if(id.equals("687714"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2630-hr.jpg";
        else if(id.equals("687722"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2550-hr.jpg";
        else if(id.equals("687721"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3183-hr.jpg";
        else if(id.equals("687734"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2398-hr.jpg";
        else if(id.equals("687708"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3086-hr.jpg";
        else if(id.equals("687732"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3158-hr.jpg";
        else if(id.equals("687731"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3157-hr.jpg";
        else if(id.equals("687755"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3156-hr.jpg";
        else if(id.equals("687730"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2603-hr.jpg";
        else if(id.equals("687729"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2576-hr.jpg";
        else if(id.equals("687728"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2573-hr.jpg";
        else if(id.equals("687727"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2570-hr.jpg";
        else if(id.equals("687726"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2568-hr.jpg";
        else if(id.equals("687725"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2559-hr.jpg";
        else if(id.equals("687724"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3131-hr.jpg";
        else if(id.equals("687723"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3128-hr.jpg";
        else if(id.equals("687740"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2759-hr.jpg";
        else if(id.equals("687739"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2755-hr.jpg";
        else if(id.equals("687738"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2432-hr.jpg";
        else if(id.equals("687737"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3053-hr.jpg";
        else if(id.equals("687756"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3054-hr.jpg";
        else if(id.equals("687736"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2408-hr.jpg";
        else if(id.equals("687735"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2403-hr.jpg";
        else if(id.equals("687733"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2729-hr.jpg";
        else if(id.equals("687706"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3082-hr.jpg";
        else if(id.equals("687750"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2748-hr.jpg";
        else if(id.equals("687748"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2747-hr.jpg";
        else if(id.equals("687749"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2746-hr.jpg";
        else if(id.equals("687742"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2743-hr.jpg";
        else if(id.equals("687743"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2744-hr.jpg";
        else if(id.equals("687744"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2745-hr.jpg";
        else if(id.equals("687745"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2763-hr.jpg";
        else if(id.equals("687746"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2764-hr.jpg";
        else if(id.equals("687747"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2765-hr.jpg";
        else if(id.equals("687741"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-2761-hr.jpg";
        else if(id.equals("687753"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3176-hr.jpg";
        else if(id.equals("687754"))
            cardurl = "https://deckmaster.info/images/cards/DKM/-3184-hr.jpg";
        else if(id.equals("242498"))
            cardurl = "https://deckmaster.info/images/cards/DKA/242498-hr.jpg";
        else if(id.equals("253431"))
            cardurl = "https://deckmaster.info/images/cards/DKA/253431-hr.jpg";
        else if(id.equals("262659"))
            cardurl = "https://deckmaster.info/images/cards/DKA/262659-hr.jpg";
        else if(id.equals("262698"))
            cardurl = "https://deckmaster.info/images/cards/DKA/262698-hr.jpg";
        else if(id.equals("244734"))
            cardurl = "https://deckmaster.info/images/cards/DKA/244734-hr.jpg";
        else if(id.equals("244712"))
            cardurl = "https://deckmaster.info/images/cards/DKA/244712-hr.jpg";
        else if(id.equals("227405"))
            cardurl = "https://deckmaster.info/images/cards/DKA/227405-hr.jpg";
        else if(id.equals("247122"))
            cardurl = "https://deckmaster.info/images/cards/DKA/247122-hr.jpg";
        else if(id.equals("244738"))
            cardurl = "https://deckmaster.info/images/cards/DKA/244738-hr.jpg";
        else if(id.equals("253429"))
            cardurl = "https://deckmaster.info/images/cards/DKA/253429-hr.jpg";
        else if(id.equals("242509"))
            cardurl = "https://deckmaster.info/images/cards/DKA/242509-hr.jpg";
        else if(id.equals("414422"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414422-hr.jpg";
        else if(id.equals("414325"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414325-hr.jpg";
        else if(id.equals("414347"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414347-hr.jpg";
        else if(id.equals("414392"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414392-hr.jpg";
        else if(id.equals("414305"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414305-hr.jpg";
        else if(id.equals("414500"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414500-hr.jpg";
        else if(id.equals("414471"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414471-hr.jpg";
        else if(id.equals("414480"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414480-hr.jpg";
        else if(id.equals("414449"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414449-hr.jpg";
        else if(id.equals("414514"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414514-hr.jpg";
        else if(id.equals("414497"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414497-hr.jpg";
        else if(id.equals("414478"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414478-hr.jpg";
        else if(id.equals("414442"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414442-hr.jpg";
        else if(id.equals("414358"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414358-hr.jpg";
        else if(id.equals("414408"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414408-hr.jpg";
        else if(id.equals("414465"))
            cardurl = "https://deckmaster.info/images/cards/EMN/414465-hr.jpg";
        else if(id.equals("227290"))
            cardurl = "https://deckmaster.info/images/cards/ISD/227290-hr.jpg";
        else if(id.equals("244687"))
            cardurl = "https://deckmaster.info/images/cards/ISD/244687-hr.jpg";
        else if(id.equals("222123"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222123-hr.jpg";
        else if(id.equals("222906"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222906-hr.jpg";
        else if(id.equals("227419"))
            cardurl = "https://deckmaster.info/images/cards/ISD/227419-hr.jpg";
        else if(id.equals("226755"))
            cardurl = "https://deckmaster.info/images/cards/ISD/226755-hr.jpg";
        else if(id.equals("221190"))
            cardurl = "https://deckmaster.info/images/cards/ISD/221190-hr.jpg";
        else if(id.equals("222115"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222115-hr.jpg";
        else if(id.equals("222183"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222183-hr.jpg";
        else if(id.equals("222114"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222114-hr.jpg";
        else if(id.equals("222117"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222117-hr.jpg";
        else if(id.equals("221222"))
            cardurl = "https://deckmaster.info/images/cards/ISD/221222-hr.jpg";
        else if(id.equals("222107"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222107-hr.jpg";
        else if(id.equals("221185"))
            cardurl = "https://deckmaster.info/images/cards/ISD/221185-hr.jpg";
        else if(id.equals("221173"))
            cardurl = "https://deckmaster.info/images/cards/ISD/221173-hr.jpg";
        else if(id.equals("222108"))
            cardurl = "https://deckmaster.info/images/cards/ISD/222108-hr.jpg";
        else if(id.equals("221215"))
            cardurl = "https://deckmaster.info/images/cards/ISD/221215-hr.jpg";
        else if(id.equals("227090"))
            cardurl = "https://deckmaster.info/images/cards/ISD/227090-hr.jpg";
        else if(id.equals("398442"))
            cardurl = "https://deckmaster.info/images/cards/ORI/398442-hr.jpg";
        else if(id.equals("398423"))
            cardurl = "https://deckmaster.info/images/cards/ORI/398423-hr.jpg";
        else if(id.equals("398435"))
            cardurl = "https://deckmaster.info/images/cards/ORI/398435-hr.jpg";
        else if(id.equals("398429"))
            cardurl = "https://deckmaster.info/images/cards/ORI/398429-hr.jpg";
        else if(id.equals("439843"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439843-hr.jpg";
        else if(id.equals("439835"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439835-hr.jpg";
        else if(id.equals("439825"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439825-hr.jpg";
        else if(id.equals("439839"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439839-hr.jpg";
        else if(id.equals("439827"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439827-hr.jpg";
        else if(id.equals("439816"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439816-hr.jpg";
        else if(id.equals("439819"))
            cardurl = "https://deckmaster.info/images/cards/RIX/439819-hr.jpg";
        else if(id.equals("439454"))
            cardurl = "https://deckmaster.info/images/cards/UST/439454-hr.jpg";
        else if(id.equals("435451"))
            cardurl = "https://deckmaster.info/images/cards/XLN/-5173-hr.jpg";
        else if (id.equals("1389"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/0/30345500-d430-4280-bfe3-de297309f136.jpg?1559597102";
        else if (id.equals("1390"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/a/5a240d1b-8430-4986-850d-32afa0e812b2.jpg?1559596752";
        else if (id.equals("1391"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/b/1b0f41e8-cf27-489b-812a-d566a75cf7f7.jpg?1559596849";
        else if (id.equals("2381"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/c/0c5c9379-b686-4823-b85a-eaf2c4b63205.jpg?1559603770";
        else if (id.equals("2382"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/0/10478e22-d1dd-4e02-81a7-d93ce71ed81d.jpg?1559604101";
        else if (id.equals("2383"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/0/50352268-88a6-4575-a5e1-cd8bef7f8286.jpg?1559603921";
        else if (id.equals("414789"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/e/2ef981a9-303e-4313-9265-77cc60323091.jpg?1562229658";
        else if (id.equals("414790"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/3/03e82924-899c-47b4-862a-7a27a96e285a.jpg?1562229261";
        else if (id.equals("414791"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/5/f57958e2-1e8f-48fa-816d-748ea2c7cb4e.jpg?1562230178";
        else if (id.equals("414792"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/f/3f89288a-9958-45c6-9bd2-24e6b3935171.jpg?1562229665";
        else if (id.equals("414793"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/5/85b37484-037a-497a-9820-97299d624daa.jpg?1562229691";
        else if (id.equals("205309"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/9/09e222f9-b7fc-49f0-8cef-9899aa333ecf.jpg?1562841209";
        else if (id.equals("205434"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/3/b3b6ad3d-a4d6-4ce9-bc0d-58fd83f83094.jpg?1562842861";
        else if (id.equals("205442"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/9/e9956850-0674-44e1-80e8-3875ef76d512.jpg?1562843350";
        else if (id.equals("205443"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/3/e3b5964a-78d8-453f-8cba-6ab01804054e.jpg?1562843341";
        else if (id.equals("205446"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/2/a262d93b-f95c-406c-9e54-cbd3ad14282f.jpg?1562842650";
        else if (id.equals("2743"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/6/4695653a-5c4c-4ff3-b80c-f4b6c685f370.jpg?1562907887";
        else if (id.equals("2744"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/a/6a90b49f-53b3-4ce0-92c1-bcd76d6981ea.jpg?1562914756";
        else if (id.equals("2745"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/d/ddca7e2e-bb0a-47ed-ade3-31900da992dc.jpg?1562936375";
        else if (id.equals("157871"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/3/4324380c-68b8-4955-ad92-76f921e6ffc1.jpg?1562829356";
        else if (id.equals("157886"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/a/3a310639-99ca-4a7e-9f65-731779f3ea46.jpg?1562828916";
        else if (id.equals("157889"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/e/1ee0be63-ec99-4291-b504-e17061c15a67.jpg?1562827639";
        else if (id.equals("158239"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/1/a1d2dedf-d0d8-42c5-a498-31e172a1b503.jpg?1562834034";
        else if (id.equals("2110"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/9/398a2b0f-0b91-408c-8083-3bc89873b69f.jpg?1559603803";
        else if (id.equals("2101"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/9/398a2b0f-0b91-408c-8083-3bc89873b69f.jpg?1559603803";
        else if (id.equals("3900"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/a/9ac60e8c-ef5b-4893-b3e5-4a54cb0a0d3a.jpg?1562592795";
        else if (id.equals("3981"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/f/4fa6c0d6-aa18-4c32-a641-1ec8e50a26f3.jpg?1562590659";
        else if (id.equals("426920"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/1/517b32e4-4b34-431f-8f3b-98a6cffc245a.jpg?1549941725";
        else if (id.equals("426915"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/e/eeac671f-2606-43ed-ad60-a69df5c150f6.jpg?1549941631";
        else if (id.equals("426914"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/4/a4b32135-7061-4278-a01a-4fcbaadc9706.jpg?1549941342";
        else if (id.equals("426917"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/c/9c6f5433-57cc-4cb3-8621-2575fcbff392.jpg?1549941629";
        else if (id.equals("426917t"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/d/2d1446ed-f114-421d-bb60-9aeb655e5adb.jpg?1562844787";
        else if (id.equals("426916"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/4/a47070a0-fd05-4ed9-a175-847a864478da.jpg?1549941630";
        else if (id.equals("426916t"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/a/1aea5e0b-dc4e-4055-9e13-1dfbc25a2f00.jpg?1562844782";
        else if (id.equals("47316011t"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/9/c994ea90-71f4-403f-9418-2b72cc2de14d.jpg?1569150300";
        else if (id.equals("47316012t"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/b/db951f76-b785-453e-91b9-b3b8a5c1cfd4.jpg?1569150303";
        else if (id.equals("47316013t"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/d/cd3ca6d5-4b2c-46d4-95f3-f0f2fa47f447.jpg?1569150305";
        else if (id.equals("426913"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/6/06c9e2e8-2b4c-4087-9141-6aa25a506626.jpg?1549941334";
        else if (id.equals("426912"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/3/937dbc51-b589-4237-9fce-ea5c757f7c48.jpg?1549941330";
        else if (id.equals("426919"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/c/5cf5c549-1e2a-4c47-baf7-e608661b3088.jpg?1549941724";
        else if (id.equals("426918"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/2/d2f3035c-ca27-40f3-ad73-c4e54bb2bcd7.jpg?1549941722";
        else if (id.equals("426926"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/e/fe1a4032-efbb-4f72-9181-994b2b35f598.jpg?1549941957";
        else if (id.equals("426925"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/6/c6f61e2b-e93b-4dda-95cf-9d0ff198c0a6.jpg?1549941949";
        else if (id.equals("426922"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/5/f59ea6f6-2dff-4e58-9166-57cac03f1d0a.jpg?1549941875";
        else if (id.equals("426921"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/4/6431d464-1f2b-42c4-ad38-67b7d0984080.jpg?1549941868";
        else if (id.equals("426924"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/1/11d84618-aca9-47dc-ae73-36a2c29f584c.jpg?1549941948";
        else if (id.equals("426923"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/9/b9623c8c-01b4-4e8f-a5b9-eeea408ec027.jpg?1549941877";
        else if (id.equals("3082"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/5/b5afe9b5-3be8-472a-95c3-2c34231bc042.jpg?1562770153";
        else if (id.equals("3083"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/5/b5afe9b5-3be8-472a-95c3-2c34231bc042.jpg?1562770153";
        else if (id.equals("3222"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/4/44be2d66-359e-4cc1-9670-119cb9c7d5f5.jpg?1562768261";
        else if (id.equals("3223"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/9/f9b0164c-2d4e-48ab-addd-322d9b504739.jpg?1562770860";
        else if (id.equals("912"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/c/fcc1004f-7cee-420a-9f0e-2986ed3ab852.jpg?1562942644";
        else if (id.equals("915"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/4/c4b610d3-2005-4347-bcda-c30b5b7972e5.jpg?1562931818";
        else if (id.equals("921"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/f/5f46783a-b91e-4829-a173-5515b09ca615.jpg?1562912566";
        else if (id.equals("922"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/1/31bf3f14-b5df-498b-a1bb-965885c82401.jpg?1562904228";
        else if (id.equals("923"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/8/18607bf6-ce11-41cb-b001-0c9538406ba0.jpg?1562899601";
        else if (id.equals("929"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/1/414d3cae-b8cf-4d53-bd6b-1aa83a828ba9.jpg?1562906979";
        else if (id.equals("946"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/9/f9d613d5-36a2-4633-b5af-64511bb29cc2.jpg?1562941972";
        else if (id.equals("947"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/0/c0b10fb7-8667-42bf-aeb6-35767a82917b.jpg?1562930986";
        else if (id.equals("74476"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/8/28f72260-c8f9-4c44-92b5-23cef6690fdd.jpg?1562876119";
        else if (id.equals("74489"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/a/ca03131a-9bd4-4fba-b95c-90f1831e86e7.jpg?1562879774";
        else if (id.equals("74536"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/3/73636ca0-2309-4bb3-9300-8bd0c0bb5b31.jpg?1562877808";
        else if (id.equals("74093"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/5/758abd53-6ad2-406e-8615-8e48678405b4.jpg?1562877848";
        else if (id.equals("74671"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/f/1fe2b76f-ddb7-49d5-933b-ccb06be5d46f.jpg?1562875903";
        else if (id.equals("376399"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/e/cec89c38-0b72-44b0-ac6c-7eb9503e1256.jpg?1562938742";
        else if (id.equals("451089") || id.equals("45108910t"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/8/58164521-aeec-43fc-9db9-d595432dea6f.jpg?1564694999";
        else if (id.equals("451089t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/5/c5ad13b4-bbf5-4c98-868f-4d105eaf8833.jpg?1592710082";
        else if (id.equals("470745"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/a/ca4caa4e-6b8f-4be8-b177-de2ebe2c9201.jpg?1567044873";
        else if (id.equals("470609"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/9/e9d5aee0-5963-41db-a22b-cfea40a967a3.jpg?1567044805";
        else if (id.equals("470738"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/e/fe3b32dc-f7e6-455c-b9d1-c7f8d6259179.jpg?1567044854";
        else if (id.equals("78686"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/3/33a8e5b9-6bfb-4ff2-a16d-3168a5412807.jpg?1562758927";
        else if (id.equals("78688"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/3/b3523b8e-065f-427c-8d5b-eb731ca91ede.jpg?1562763691";
        else if (id.equals("78687"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/9/49999b95-5e62-414c-b975-4191b9c1ab39.jpg?1562759856";
        else if (id.equals("75291"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/8/98d3bc63-8814-46e7-a6ee-dd5b94a8257e.jpg?1562762956";
        else if (id.equals("78679"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/8/c8265c39-d287-4c5a-baba-f2f09dd80a1c.jpg?1562764226";
        else if (id.equals("78678"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/7/77ffd913-8efa-48e5-a5cf-293d3068dbbf.jpg?1562761560";
        else if (id.equals("78691"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/d/9de1eebf-5725-438c-bcf0-f3a4d8a89fb0.jpg?1562762993";
        else if (id.equals("78695"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/2/f2ddf1a3-e6fa-4dd0-b80d-1a585b51b934.jpg?1562765664";
        else if (id.equals("78694"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/e/6ee6cd34-c117-4d7e-97d1-8f8464bfaac8.jpg?1562761096";
        else if (id.equals("78600"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/6/864ad989-19a6-4930-8efc-bbc077a18c32.jpg?1562762069";
        else if (id.equals("78975"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/4/a4f4aa3b-c64a-4430-b1a2-a7fca87d0a22.jpg?1562763433";
        else if (id.equals("2832"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/5/85bcd723-780b-45ca-9476-d28270350013.jpg?1562922034";
        else if (id.equals("2802"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/f/bfc43585-55ac-4d58-9e80-b19a7c8c8662.jpg?1562933573";
        else if (id.equals("446807"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg?1564694995";
        else if (id.equals("247175"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/d/fd9920a0-78c2-4cc8-82e6-ea3a1e23b314.jpg?1562942793";
        else if (id.equals("247182"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/1/91a2217c-8478-479b-a146-2d78f407a36f.jpg?1562922037";
        else if (id.equals("122075"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/5/5526c510-bd33-4fac-8941-f19bd0997557.jpg?1562183342";
        else if (id.equals("121236"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/5/1566c8a2-aaca-4ce0-a36b-620ea6f135cb.jpg?1562177467";
        else if (id.equals("244724"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/b/cb09041b-4d09-4cae-9e85-b859edae885b.jpg?1562942950";
        else if (id.equals("262675"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/2/a2c044c0-3625-4bdf-9445-b462394cecae.jpg?1562933422";
        else if (id.equals("226735"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/d/9d9c1c46-7aa7-464c-87b0-b29b9663daef.jpg?1562932220";
        else if (id.equals("253433"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/1/b150d71f-11c9-40d6-a461-4967ef437315.jpg?1562936877";
        else if (id.equals("226721"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/d/9d9c1c46-7aa7-464c-87b0-b29b9663daef.jpg?1562932220";
        else if (id.equals("227417"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/a/6aef77b3-4b38-4902-9f7a-dc18b5bb9da9.jpg?1562920184";
        else if (id.equals("243229"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/c/7c5a3c09-5656-4975-ba03-2d809903ed18.jpg?1562924292";
        else if (id.equals("242537"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/3/932d753d-9584-4ad8-9a5e-a3524184f961.jpg?1562929672";
        else if (id.equals("253426"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/3/1303e02a-ef69-4817-bca5-02c74774b811.jpg?1562899503";
        else if (id.equals("262875"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/a/aae6fb12-b252-453b-bca7-1ea2a0d6c8dc.jpg?1562935354";
        else if (id.equals("222178"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/5/f500cb95-d5ea-4cf2-920a-f1df45a9059b.jpg?1562953084";
        else if (id.equals("249422"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/0/e00ae92c-af6d-4a00-b102-c6d3bcc394b4.jpg?1562948371";
        else if (id.equals("247125"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/6/b6edac85-78e7-4e90-b538-b67c88bb5c62.jpg?1562938113";
        else if (id.equals("262694"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/f/6f35e364-81d9-4888-993b-acc7a53d963c.jpg?1562921188";
        else if (id.equals("244740"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/8/683af377-c491-4f62-900c-6b83d75c33c9.jpg?1562919527";
        else if (id.equals("262699"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/a/aae6fb12-b252-453b-bca7-1ea2a0d6c8dc.jpg?1562935354";
        else if (id.equals("262857"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/8/9831e3cc-659b-4408-b5d8-a27ae2738680.jpg?1562930830";
        else if (id.equals("414499"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/7/078b2103-15ce-456d-b092-352fa7222935.jpg?1562723962";
        else if (id.equals("414496"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/e/3e2011f0-a640-4579-bd67-1dfbc09b8c09.jpg?1562731266";
        else if (id.equals("414448"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/6/460f7733-c0a6-4439-a313-7b26ae6ee15b.jpg?1562732302";
        else if (id.equals("414346"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/2/22e816af-df55-4a3f-a6e7-0ff3bb1b45b5.jpg?1540920747";
        else if (id.equals("414464"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/8/f89f116a-1e8e-4ae7-be39-552e4954f229.jpg?1562756276";
        else if (id.equals("414349"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/0/30c3d4c1-dc3d-4529-9d6e-8c16149cf6da.jpg?1562729197";
        else if (id.equals("414470"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/6/a63c30c0-369a-4a75-b352-edab4d263d1b.jpg?1562745465";
        else if (id.equals("414350"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/0/30c3d4c1-dc3d-4529-9d6e-8c16149cf6da.jpg?1562729197";
        else if (id.equals("414357"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/e/1eb4ddf4-f695-412d-be80-b93392432498.jpg?1562726998";
        else if (id.equals("414479"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/d/0dbaef61-fa39-4ea7-bc21-445401c373e7.jpg?1562724612";
        else if (id.equals("414477"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/e/ee648500-a213-4aa4-a97c-b7223c11bebd.jpg?1562754423";
        else if (id.equals("414407"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/5/25baac6c-5bb4-4ecc-b1d5-fced52087bd9.jpg?1562727704";
        else if (id.equals("414421"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/f/7f95145a-41a1-478e-bf8a-ea8838d6f9b1.jpg?1562740440";
        else if (id.equals("414429"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/9/0900e494-962d-48c6-8e78-66a489be4bb2.jpg?1562724107";
        else if (id.equals("414304"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/7/27907985-b5f6-4098-ab43-15a0c2bf94d5.jpg?1562728142";
        else if (id.equals("414313"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/6/b6867ddd-f953-41c6-ba36-86ae2c14c908.jpg?1562747201";
        else if (id.equals("414314"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/6/b6867ddd-f953-41c6-ba36-86ae2c14c908.jpg?1562747201";
        else if (id.equals("414319"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/7/c75c035a-7da9-4b36-982d-fca8220b1797.jpg?1562749301";
        else if (id.equals("414324"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/a/9a55b60a-5d90-4f73-984e-53fdcc0366e4.jpg?1562744017";
        else if (id.equals("414441"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/b/0b0eab47-af62-4ee8-99cf-a864fadade2d.jpg?1562724176";
        else if (id.equals("456235"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/1/01ce2601-ae94-4ab5-bbd2-65f47281ca28.jpg?1544060145";
        else if (id.equals("452980"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/4/44614c6d-5508-4077-b825-66d5d684086c.jpg?1557465654";
        else if (id.equals("452979"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/2/92162888-35ea-4f4f-ab99-64dd3104e230.jpg?1557465657";
        else if (id.equals("452977"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/a/4a82084e-b178-442b-8007-7b2a70f3fbba.jpg?1557465653";
        else if (id.equals("452978"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/5/054a4e4f-8baa-41cf-b24c-d068e8b9a070.jpg?1557465656";
        else if (id.equals("452975"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/e/1e4e9e35-6cbc-4997-beff-d1a22d87545e.jpg?1557465652";
        else if (id.equals("452976"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/e/feb4b39f-d309-49ba-b427-240b7fdc1099.jpg?1557465650";
        else if (id.equals("452973"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/c/ace631d1-897a-417e-8628-0170713f03d3.jpg?1557465649";
        else if (id.equals("452974"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/0/e0644c92-4d67-475e-8c8e-0e2c493682fb.jpg?1557465652";
        else if (id.equals("452971"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/d/ad454e7a-06c9-4694-ae68-7b1431e00077.jpg?1557465646";
        else if (id.equals("452972"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/9/890ac54c-6fd7-4e46-8ce4-8926c6975f60.jpg?1557465648";
        else if (id.equals("430840"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/6/76f21f0b-aaa5-4677-8398-cef98c6fac2a.jpg?1562803878";
        else if (id.equals("430842"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/9/f928e8e8-aa20-402c-85bd-59106e9b9cc7.jpg?1562820622";
        else if (id.equals("430841"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/c/2c25b8ef-6331-49df-9457-b8b4e44da2c9.jpg?1562793920";
        else if (id.equals("430844"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/3/0383401f-d453-4e8f-82d2-5c016acc2591.jpg?1562787667";
        else if (id.equals("430843"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/c/1ca644e3-4fb3-4d38-b714-e3d7459bd8b9.jpg?1562791344";
        else if (id.equals("430846"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/7/7713ba59-dd4c-4b49-93a7-292728df86b8.jpg?1562803886";
        else if (id.equals("430845"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/5/054b07d8-99ae-430b-8e54-f9601fa572e7.jpg?1562787788";
        else if (id.equals("430837"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/9/d998db65-8785-4ee9-940e-fa9ab62e180f.jpg?1562816967";
        else if (id.equals("430839"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/4/0468e488-94ce-4ae3-abe4-7782673a7e62.jpg?1562787748";
        else if (id.equals("430838"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/c/1c1ead90-10d8-4217-80e4-6f40320c5569.jpg?1562791309";
        else if (id.equals("2470"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/f/af976f42-3d56-4e32-8294-970a276a4bf3.jpg?1562927660";
        else if (id.equals("2469"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/d/3d0006f6-2f96-453d-9145-eaefa588efbc.jpg?1562906229";
        else if (id.equals("2466"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/5/75b67eb2-b60e-46b4-9d48-11c284957bec.jpg?1562916780";
        else if (id.equals("2480"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/1/f16df768-06de-43a0-b548-44fb0887490b.jpg?1562940406";
        else if (id.equals("2635"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/8/7880e815-53e7-43e0-befd-e368f00a75d8.jpg?1562917281";
        else if (id.equals("221209"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/b/7bf864db-4754-433d-9d77-6695f78f6c09.jpg?1562832669";
        else if (id.equals("227415"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/b/bb90a6f1-c7f2-4c2e-ab1e-59c5c7937841.jpg?1562836209";
        else if (id.equals("221211"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/8/88db324f-11f1-43d3-a897-f4e3caf8d642.jpg?1562833493";
        else if (id.equals("221212"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/8/f8b8f0b4-71e1-4822-99a1-b1b3c2f10cb2.jpg?1562839966";
        else if (id.equals("244683"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/b/2b14ed17-1a35-4c49-ac46-3cad42d46c14.jpg?1562827887";
        else if (id.equals("222915"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/4/e42a0a3d-a987-4b24-b9d4-27380a12e093.jpg?1562838647";
        else if (id.equals("222112"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/d/cd5435d0-789f-4c42-8efc-165c072404a2.jpg?1562837238";
        else if (id.equals("222118"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/5/25b54a1d-e201-453b-9173-b04e06ee6fb7.jpg?1562827580";
        else if (id.equals("222105"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/3/8325c570-4d74-4e65-891c-3e153abf4bf9.jpg?1562833164";
        else if (id.equals("222111"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/2/028aeebc-4073-4595-94da-02f9f96ea148.jpg?1562825445";
        else if (id.equals("222016"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/8/58ae9cbc-d88d-42df-ab76-63ab5d05c023.jpg?1562830610";
        else if (id.equals("222124"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/b/4b43b0cb-a5a3-47b4-9b6b-9d2638222bb6.jpg?1562829761";
        else if (id.equals("226749"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/1/11bf83bb-c95b-4b4f-9a56-ce7a1816307a.jpg?1562826346";
        else if (id.equals("221179"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/b/ebf5e16f-a8bd-419f-b5ca-8c7fce09c4f1.jpg?1562839206";
        else if (id.equals("245251"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/4/b4160322-ff40-41a4-887a-73cd6b85ae45.jpg?1562835830";
        else if (id.equals("245250"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/4/b4160322-ff40-41a4-887a-73cd6b85ae45.jpg?1562835830";
        else if (id.equals("222186"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/1/6151cae7-92a4-4891-a952-21def412d3e4.jpg?1562831128";
        else if (id.equals("227072"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/3/13896468-e3d0-4bcb-b09e-b5c187aecb03.jpg?1562826506";
        else if (id.equals("227061"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/3/13896468-e3d0-4bcb-b09e-b5c187aecb03.jpg?1562826506";
        else if (id.equals("227409"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/7/57f0907f-74f4-4d86-93df-f2e50c9d0b2f.jpg?1562830557";
        else if (id.equals("222189"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/d/dd8ca448-f734-4cb9-b1d5-790eed9a4b2d.jpg?1562838270";
        else if (id.equals("227084"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/c/ec00d2d2-6597-474a-9353-345bbedfe57e.jpg?1562839216";
        else if (id.equals("447354"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/b/7b215968-93a6-4278-ac61-4e3e8c3c3943.jpg?1566971561";
        else if (id.equals("447355"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/b/7b215968-93a6-4278-ac61-4e3e8c3c3943.jpg?1566971561";
        else if (id.equals("184714"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/7/1777f69c-869e-414e-afe3-892714a6032a.jpg?1562867836";
        else if (id.equals("202605"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/f/5f6529eb-79ff-4ddc-9fae-38326324f7e6.jpg?1562917476";
        else if (id.equals("202443"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/8/082cf845-5a24-4f00-bad2-a3d0d07f59e6.jpg?1562896910";
        else if (id.equals("398438"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/f/ff0063da-ab6b-499d-8e87-8b34d46f0372.jpg?1562209457";
        else if (id.equals("398432"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/f/ff0063da-ab6b-499d-8e87-8b34d46f0372.jpg?1562209457";
        else if (id.equals("398434"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/2/02d6d693-f1f3-4317-bcc0-c21fa8490d38.jpg?1562005031";
        else if (id.equals("398441"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/f/9f25e1cf-eeb4-458d-8fb2-b3a2f86bdd54.jpg?1562033824";
        else if (id.equals("398422"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/0/b0d6caf0-4fa8-4ec5-b7f4-1307474d1b13.jpg?1562036951";
        else if (id.equals("398428"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/8/58c39df6-b237-40d1-bdcb-2fe5d05392a9.jpg?1562021001";
        else if (id.equals("6528"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/d/ade6a71a-e8ec-4d41-8a39-3eacf0097c8b.jpg?1562936067";
        else if (id.equals("4259"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/c/7c93d4e9-7fd6-4814-b86b-89b92d1dad3b.jpg?1562446874";
        else if (id.equals("439824"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/6/66d9d524-3611-48d9-86c9-48e509e8ae70.jpg?1555428581";
        else if (id.equals("439826"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/d/1d94ff37-f04e-48ee-8253-d62ab07f0632.jpg?1555428604";
        else if (id.equals("439834"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/1/c16ba84e-a0cc-4c6c-9b80-713247b8fef9.jpg?1555040973";
        else if (id.equals("439818"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/8/d81c4b3f-81c2-403b-8a5d-c9415f73a1f9.jpg?1555040854";
        else if (id.equals("439815"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/e/8e7554bc-8583-4059-8895-c3845bc27ae3.jpg?1555428629";
        else if (id.equals("439838"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/0/303d51ab-b9c4-4647-950f-291daabe7b81.jpg?1555041001";
        else if (id.equals("439842"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/9/397ba02d-f347-46f7-b028-dd4ba55faa2f.jpg?1555427909";
        else if (id.equals("457365"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/5/b5873efa-d573-4435-81ad-48df2ca5c7f2.jpg?1551138454";
        else if (id.equals("457366"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/1/d1dbc559-c78c-4675-9582-9c28f8151bc7.jpg?1549415048";
        else if (id.equals("457367"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/b/9bd15da6-2b86-4dba-951d-318c7d9a5dde.jpg?1549415053";
        else if (id.equals("457368"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/0/00320106-ce51-46a9-b0f9-79b3baf4e505.jpg?1549415058";
        else if (id.equals("457369"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/b/ab0ba4ef-9e82-4177-a80f-8fa6f6a5bd60.jpg?1549416398";
        else if (id.equals("457370"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/7/075bbe5d-d0f3-4be3-a3a6-072d5d3d614c.jpg?1549414568";
        else if (id.equals("457371"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/6/f6200937-3146-4972-ab83-051ade3b7a52.jpg?1551138470";
        else if (id.equals("457372"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/0/50ae0831-f3ba-4535-bfb6-feefbbc15275.jpg?1551138459";
        else if (id.equals("457373"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/e/2eefd8c1-96ce-4d7a-8aaf-29c35d634dda.jpg?1551138529";
        else if (id.equals("457374"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/0/0070651d-79aa-4ea6-b703-6ecd3528b548.jpg?1551138527";
        else if (id.equals("1158"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/3/c3591170-645f-4645-bc39-b90b7b6ddac7.jpg?1559597137";
        else if (id.equals("409826"))
            cardurl = "https://deckmaster.info/images/cards/SOI/409826-hr.jpg";
        else if (id.equals("409899"))
            cardurl = "https://deckmaster.info/images/cards/SOI/409899-hr.jpg";
        else if (id.equals("84716"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/4/84920a21-ee2a-41ac-a369-347633d10371.jpg?1562494702";
        else if (id.equals("87600"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/2/42ba0e13-d20f-47f9-9c86-2b0b13c39ada.jpg?1562493487";
        else if (id.equals("87599"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/b/0b61d772-2d8b-4acf-9dd2-b2e8b03538c8.jpg?1562492461";
        else if (id.equals("87595"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/2/d224c50f-8146-4c91-9401-04e5bd306d02.jpg?1562496100";
        else if (id.equals("87596"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/1/41004bdf-8e09-4b2c-9e9c-26c25eac9854.jpg?1562493483";
        else if (id.equals("106631"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/c/ac2e32d0-f172-4934-9d73-1bc2ab86586e.jpg?1562781784";
        else if (id.equals("9668"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/7/17c18690-cf8c-4006-a981-6258d18ba538.jpg?1562799066";
        else if (id.equals("9749"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/f/3fcefcab-8988-47e8-89bb-9b76f14c9d8b.jpg?1562799089";
        else if (id.equals("9780"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/9/a9f9c279-e382-4feb-9575-196e7cf5d7dc.jpg?1562799139";
        else if (id.equals("9844"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/9/a9f9c279-e382-4feb-9575-196e7cf5d7dc.jpg?1562799139";
        else if (id.equals("456821"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/6/468d5308-2a6c-440e-a8d0-1c5e084afb82.jpg?1547517180";
        else if (id.equals("74358"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/9/8987644d-5a31-4a4e-9a8a-3d6260ed0fd6.jpg?1562488870";
        else if (id.equals("73956"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/0/c01e8089-c3a9-413b-ae2d-39ede87516d3.jpg?1562489378";
        else if (id.equals("74242"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/5/85cbebbb-7ea4-4140-933f-186cad08697d.jpg?1562488867";
        else if (id.equals("74322"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/9/49dd5a66-101d-4f88-b1ba-e2368203d408.jpg?1562488377";
        else if (id.equals("4429"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/8/3884bede-df28-42e8-9ac9-ae03118b1985.jpg?1562800239";
        else if (id.equals("113522"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/8/b8a3cdfe-0289-474b-b9c4-07e8c6588ec5.jpg?1562933997";
        else if (id.equals("51733"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/7/27118cbb-a386-4145-8716-961ed0f653bf.jpg?1562902951";
        else if (id.equals("52362"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/b/9bd7a7f1-2221-4565-8c6e-1815def3bd2c.jpg?1562546811";
        else if (id.equals("52415"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/8/8825493a-878d-4df3-8d7a-98518358d678.jpg?1562546240";
        else if(id.equals("53214t"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/4/1449862b-309e-4c58-ac94-13d1acdd363f.jpg?1562541935";
        else if(id.equals("53179t"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/9/d9623e74-3b94-4842-903f-ed52931bdf6a.jpg?1562636919";
        else if(id.equals("16806"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/1/f1bb8fb5-32f2-444d-85cb-de84657b21bd.jpg?1561758404";
        else if(id.equals("16807"))
            cardurl = "https://img.scryfall.com/cards/large/back/f/1/f1bb8fb5-32f2-444d-85cb-de84657b21bd.jpg?1561758404";
        else if(id.equals("16808"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/e/2eb08fc5-29a4-4911-ac94-dc5ff2fc2ace.jpg?1561756860";
        else if(id.equals("16809"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/e/9e5180da-d757-415c-b92d-090ad5c1b658.jpg?1561757695";
        else if(id.equals("16809t"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/e/8ee8b915-afd3-4fad-8aef-7e9cbbbbc2e4.jpg?1561757559";
        else if(id.equals("16751"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/9/39a89c44-1aa7-4f2e-909b-d821ec2b7948.jpg?1561756358";
        else if(id.equals("17639t"))
            cardurl = "https://img.scryfall.com/cards/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg?1562921132";
        else if(id.equals("16740t"))
            cardurl = "https://deckmaster.info/images/cards/AER/-3992-hr.jpg";
        else if (id.equals("53143t") || id.equals("17717t") || id.equals("17705t") || id.equals("17669t") || id.equals("17661t")
                || id.equals("17645t") || id.equals("17573t") || id.equals("17549t") || id.equals("17537t") || id.equals("17513t")
                || id.equals("17429t") || id.equals("17417t") || id.equals("17405t") || id.equals("17393t") || id.equals("17285t")
                || id.equals("17273t") || id.equals("17249t") || id.equals("17141t") || id.equals("17129t") || id.equals("17117t")
                || id.equals("17105t") || id.equals("17093t") || id.equals("17081t") || id.equals("17866t") || id.equals("294460t"))
            cardurl = "https://deckmaster.info/images/cards/DDE/209162-hr.jpg";
        else if(id.endsWith("53141t"))
            cardurl = "https://deckmaster.info/images/cards/C14/-487-hr.jpg";
        else if(id.equals("53134t"))
            cardurl = "https://deckmaster.info/images/cards/DDD/201844-hr.jpg";
        else if(id.equals("16981t") || id.equals("16978t") || id.equals("16967t") || id.equals("17841t")
                || id.equals("17850t") || id.equals("17852t"))
            cardurl = "https://deckmaster.info/images/cards/EVG/159047-hr.jpg";
        else if (id.equals("16975t") || id.equals("17848t"))
            cardurl = "https://deckmaster.info/images/cards/BNG/-10-hr.jpg";
        else if (id.equals("16933t") || id.equals("476107t"))
            cardurl = "https://deckmaster.info/images/cards/WWK/-262-hr.jpg";
        else if(id.equals("16885t"))
            cardurl = "https://deckmaster.info/images/cards/ALA/-327-hr.jpg";
        else if(id.equals("16847t"))
            cardurl = "https://deckmaster.info/images/cards/DDQ/409653-hr.jpg";
        else if(id.equals("17656t") || id.equals("17500t") || id.equals("17080t"))
            cardurl = "https://deckmaster.info/images/cards/CON/-319-hr.jpg";
        else if(id.equals("17538t"))
            cardurl = "https://deckmaster.info/images/cards/MRD/-2829-hr.jpg";
        else if (id.equals("17501t") || id.equals("17494t") || id.equals("17354t") || id.equals("17062t"))
            cardurl = "https://deckmaster.info/images/cards/DDQ/409655-hr.jpg";
        else if (id.equals("17493t"))
            cardurl = "https://deckmaster.info/images/cards/VMA/-4464-hr.jpg";
        else if (id.equals("17358t"))
            cardurl = "https://deckmaster.info/images/cards/DDD/201842-hr.jpg";
        else if(id.equals("17207t"))
            cardurl = "https://deckmaster.info/images/cards/M15/-98-hr.jpg";
        else if(id.equals("17071t"))
            cardurl = "https://deckmaster.info/images/cards/C17/-5043-hr.jpg";
        else if(id.equals("17069t"))
            cardurl = "https://deckmaster.info/images/cards/GK1_SELESN/-6550-hr.jpg";
        else if(id.equals("17060t"))
            cardurl = "https://deckmaster.info/images/cards/GTC/-48-hr.jpg";
        else if(id.equals("17061t"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/c/acd51eed-bd5a-417a-811d-fbd1c08a3715.jpg?1561757812";
        else if(id.equals("17955"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/8/b86ac828-7b49-4663-a718-99fcac904568.jpg?1561756381";
        else if(id.equals("476097t") || id.equals("293685t") || id.equals("293652t"))
            cardurl = "https://deckmaster.info/images/cards/DDQ/409656-hr.jpg";
        else if(id.equals("999901t"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/0/40b79918-22a7-4fff-82a6-8ebfe6e87185.jpg?1561897497";
        else if(id.equals("19462t") || id.equals("19463t") || id.equals("19464t") || id.equals("19465t"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/2/d2f51f4d-eb6d-4503-b9a4-559db1b9b16f.jpg?1574710411";
        else if(id.equals("19476t") || id.equals("19477t"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/4/340fb06f-4bb0-4d23-b08c-8b1da4a8c2ad.jpg?1574709457";
        else if(id.equals("159127"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/e/1e14cf3a-3c5a-4c22-88d1-1b19660b2e2a.jpg?1559592579";
        else if(id.equals("159130"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/4/f4c21c0d-91ee-4c2c-bfa4-81bb07106842.jpg?1559592507";
        else if(id.equals("159132"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/0/b03bc922-782b-4254-897c-90d202b4cda4.jpg?1559592285";
        else if(id.equals("159764"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/8/98f443cb-55bb-4e83-826a-98261287bfd3.jpg?1559592330";
        else if(id.equals("159832"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/b/0b5f694c-11da-41af-9997-0aff93619248.jpg?1559592387";
        else if(id.equals("159237"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/e/1e76a75a-7125-4957-ab7a-8e7ead21d002.jpg?1559592440";
        else if(id.equals("159136"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/a/fa740755-244f-4658-a9e2-aa4cf6742808.jpg?1559592290";
        else if(id.equals("294381t"))
            cardurl = "https://deckmaster.info/images/cards/KTK/-460-hr.jpg";
        else if(id.equals("294366t"))
            cardurl = "https://deckmaster.info/images/cards/ORI/-848-hr.jpg";
        else if (id.equals("294235t") || id.equals("293899t"))
            cardurl = "https://deckmaster.info/images/cards/DDP/401720-hr.jpg";
        else if(id.equals("293497t"))
            cardurl = "https://deckmaster.info/images/cards/M13/-72-hr.jpg";
        else if(id.equals("476370"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/4/14b28eae-e8ed-4b99-b6ec-86d0716ec473.jpg?1581480007";
        else if(id.equals("479417"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/f/efcbd4ef-3bf4-4f22-9069-2a11c9619a43.jpg?1583965446";
        else if(id.equals("482713t"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/9/29d42a00-299d-47d3-ba03-e63812d57931.jpg?1586452636";
        else if(id.equals("4827131t"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/d/1dee8c94-cdc8-42b2-a393-0c0c8e439125.jpg?1586453065";
        else if(id.equals("484902t") || id.equals("484904t"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/2/720f3e68-84c0-462e-a0d1-90236ccc494a.jpg?1562539782";
        else if(id.equals("294690t"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/8/082c3bad-3fea-4c3f-8263-4b16139bb32a.jpg?1562701890";
        else if(id.equals("47963911t"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/9/f918b740-1984-4090-8886-9e290a698b95.jpg?1586451994";
        else if(id.equals("479634t"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/9/a9cc7c63-5d13-4fd6-af9d-4a26c2bab8e6.jpg?1588521003";
        else if(id.equals("485469t"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/3/4306be80-d7c9-4bcf-a3de-4bf159475546.jpg?1592323967";
        else if(id.equals("489663"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/6/d605c780-a42a-4816-8fb9-63e3114a8246.jpg?1592762067";
        else if(id.equals("48966310t"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/b/fbdf8dc1-1b10-4fce-97b9-1f5600500cc1.jpg?1592324494";
        else if(id.equals("48966311t"))
            cardurl = "https://img.scryfall.com/cards/large/front/4/f/4f8107b3-8539-4b9c-8d0d-c512c940838f.jpg?1592324480";
        else if(id.equals("489987t"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/b/fb248ba0-2ee7-4994-be57-2bcc8df29680.jpg?1596043510";
        else if(id.equals("489822t"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/7/c7e7822b-f155-4f3f-b835-ec64f3a71307.jpg?1596044257";
        else if(id.equals("489930t"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/9/791f5fa0-f972-455e-9802-ff299853607f.jpg?1596044240";
        else if(id.equals("491334"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/6/96d1a254-01a8-4590-8878-690c5bfb4a95.jpg?1596139673";
        else if(id.equals("491335"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/c/ec386bc3-137b-49b5-8380-8daff470f0bc.jpg?1596139680";
        else if(id.equals("491344"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/b/db149aaa-3da9-48c4-92cc-b3d804285290.jpg?1596139686";
        else if(id.equals("491345"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/2/32301593-f16a-4a46-8a4e-eecedd2a9013.jpg?1596139691";
        else if(id.equals("491346"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/b/eb49805c-8546-463d-b78c-f4ea109851b4.jpg?1596139696";
        else if(id.equals("491347"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/d/dd595e2f-65e4-46e8-9d28-f94ac308b275.jpg?1596139702";
        else if(id.equals("491348"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/9/f9baef6e-a086-41d4-a20e-486f01d72406.jpg?1596139709";
        else if(id.equals("491349"))
            cardurl = "https://img.scryfall.com/cards/large/front/e/c/ec136ce7-bad4-4ebb-ab00-b86de3d209a7.jpg?1596139714";
        else if(id.equals("491350"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/a/cacaf5ec-6745-4584-9175-36c98742958f.jpg?1596139721";
        else if(id.equals("491351"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/c/6c821158-f71a-48f9-b6b4-b0e605f22bec.jpg?1596278384";
        else if(id.equals("491352"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/9/b9a50516-a20f-4e6e-b4f2-0049b673f942.jpg?1596139732";
        else if(id.equals("491353"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/5/95702503-8f2d-46c8-abdb-6edd6c431d19.jpg?1596278403";
        else if(id.equals("491354"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/3/73731e45-51bb-4188-a54d-fdaa4bdfaf1f.jpg?1596139744";
        else if(id.equals("491355"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/3/d35575d0-0b10-4d1b-b5a2-a9f36f9eada4.jpg?1596139751";
        else if(id.equals("491356"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/2/62d2058c-3f20-4566-b366-93a2cbbe682f.jpg?1596139757";
        else if(id.equals("491357"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/b/5b8c11ba-533d-48c9-821c-3fec846bca97.jpg?1596139762";
        else if(id.equals("491358"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/8/187abedf-c2eb-453b-bea0-a10afa399e03.jpg?1596139769";
        else if(id.equals("491359"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/0/20c9c856-af15-40b1-a799-1c2066df2099.jpg?1596139775";
        else if(id.equals("491360"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/c/1c3fc61c-c26e-47f3-a1eb-f6f10f8469e2.jpg?1596139781";
        else if(id.equals("491361"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/0/8008977f-b164-4ab7-a38a-25b382c6a16f.jpg?1596139788";
        else if(id.equals("491362"))
            cardurl = "https://img.scryfall.com/cards/large/front/d/a/dac080ef-8f40-43a2-8440-b457b6074b69.jpg?1596139794";
        else if(id.equals("491363"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/6/86f670f9-c5b7-4eb0-a7d0-d16513fadf74.jpg?1596139800";
        else if(id.equals("491364"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/c/3c9847f3-5a4c-4b49-8e25-e444d1446bf9.jpg?1596139806";
        else if(id.equals("491365"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/a/2abc5ac8-b944-4b71-b022-c78183eb92c3.jpg?1596139812";
        else if(id.equals("491366"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/c/0ccd5597-2d4e-4f3e-94b7-46783486853a.jpg?1596139818";
        else if(id.equals("491367"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/e/0e94d334-e043-42f2-ba5c-be497d82f2c8.jpg?1596139824";
        else if(id.equals("491368"))
            cardurl = "https://img.scryfall.com/cards/large/front/8/b/8bc6178b-16e7-4089-974f-7048b9632fc2.jpg?1596139831";
        else if(id.equals("491369"))
            cardurl = "https://img.scryfall.com/cards/large/front/7/0/70eab734-875b-4b76-901b-3ac7d2133ad9.jpg?1596139837";
        else if(id.equals("491370"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/d/6d7cd274-ed83-475a-9b4f-adb9c780a6f4.jpg?1596139842";
        else if(id.equals("491371"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/4/547e3aa5-d88a-4418-ab9d-dd65385f031b.jpg?1596139849";
        else if(id.equals("491372"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/c/5cc0b4eb-8ee9-4213-8194-02e7d63428d3.jpg?1596139855";
        else if(id.equals("491373"))
            cardurl = "https://img.scryfall.com/cards/large/front/0/a/0a469d00-1416-48dd-ad91-eb6f3fb4b42b.jpg?1596139861";
        else if(id.equals("491374"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/2/22cd80dd-1c57-423c-81e2-9a956901565f.jpg?1596139867";
        else if(id.equals("491375"))
            cardurl = "https://img.scryfall.com/cards/large/front/3/a/3a195efe-8c4f-479d-bd0f-563ee4bb49a1.jpg?1596139873";
        else if(id.equals("491376"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/3/93c0681d-97da-4363-b75a-079c209e7e4a.jpg?1596139878";
        else if(id.equals("491377"))
            cardurl = "https://img.scryfall.com/cards/large/front/f/0/f097accb-28ad-4b22-b615-103c74e07708.jpg?1596139884";
        else if(id.equals("491378"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/9/69e55604-56da-44b5-aa78-f5de76ce9d20.jpg?1596139891";
        else if(id.equals("491379"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/4/546e0452-5304-41fa-9e3a-a3fa5a571315.jpg?1596139896";
        else if(id.equals("491380"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/3/936335d7-1c4a-4fcd-80ff-cd4d4fcab8c4.jpg?1596139903";
        else if(id.equals("491381"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/7/c75672e0-fa2d-43c5-9381-e17f2fd6d3bc.jpg?1596139909";
        else if(id.equals("491377t"))
            cardurl = "https://img.scryfall.com/cards/large/front/a/6/a6ee0db9-ac89-4ab6-ac2e-8a7527d9ecbd.jpg?1596045113";
        else if(id.equals("491372t"))
            cardurl = "https://img.scryfall.com/cards/large/front/c/f/cf371056-43dd-41ab-8d05-b16a8bdc8d28.jpg?1596045227";
        else if(id.equals("491365t"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/e/9ecc467e-b345-446c-b9b7-5f164e6651a4.jpg?1596043489";
        else if(id.equals("295116t") || id.equals("295103t"))
            cardurl = "https://img.scryfall.com/cards/large/front/2/d/2d1446ed-f114-421d-bb60-9aeb655e5adb.jpg?1562844787";
        else if(id.equals("295077t"))
            cardurl = "https://img.scryfall.com/cards/large/front/6/a/6aaa8539-8d21-4da1-8410-d4354078390f.jpg?1562844799";
        else if(id.equals("295041t"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/a/1aea5e0b-dc4e-4055-9e13-1dfbc25a2f00.jpg?1562844782";
        else if(id.equals("294952t") || id.equals("294950t"))
            cardurl = "https://img.scryfall.com/cards/large/front/b/5/b5bd6905-79be-4d2c-a343-f6e6a181b3e6.jpg?1562844819";
        else if(id.equals("491633t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/1/91098481-46c2-49bf-8123-e9cab2f22b84.jpg?1600466796";
        else if(id.equals("491633"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/4/c470539a-9cc7-4175-8f7c-c982b6072b6d.jpg?1601064253";
        else if(id.equals("491634"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/c/4/c470539a-9cc7-4175-8f7c-c982b6072b6d.jpg?1601064253";
        else if(id.equals("491641"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/6/366e9845-019d-47cc-adb8-8fbbaad35b6d.jpg?1601064385";
        else if(id.equals("491642"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/3/6/366e9845-019d-47cc-adb8-8fbbaad35b6d.jpg?1601064385";
        else if(id.equals("491649"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/d/ada9a974-8f1f-4148-bd61-200fc14714b2.jpg?1601064484";
        else if(id.equals("491650"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/a/d/ada9a974-8f1f-4148-bd61-200fc14714b2.jpg?1601064484";
        else if(id.equals("491654"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/6/b6e6be8c-41c3-4348-a8dd-b40ceb24e9b4.jpg?1601064504";
        else if(id.equals("491655"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/6/b6e6be8c-41c3-4348-a8dd-b40ceb24e9b4.jpg?1601064504";
        else if(id.equals("491662"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/f/2/f25d56f9-aa54-4657-9ac9-e93fbba3e715.jpg?1601064554";
        else if(id.equals("491663"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/f/2/f25d56f9-aa54-4657-9ac9-e93fbba3e715.jpg?1601064554";
        else if(id.equals("491666"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/1/014027c4-7f9d-4096-b308-ea4be574c0d4.jpg?1601064584";
        else if(id.equals("491667"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/0/1/014027c4-7f9d-4096-b308-ea4be574c0d4.jpg?1601064584";
        else if(id.equals("491673"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/f/5f411f08-45dd-4d73-8894-daf51c175150.jpg?1601064619";
        else if(id.equals("491674"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/5/f/5f411f08-45dd-4d73-8894-daf51c175150.jpg?1601064619";
        else if(id.equals("491688"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/a/5adcb500-8c77-4925-8e2c-1243502827d1.jpg?1601065203";
        else if(id.equals("491689"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/5/a/5adcb500-8c77-4925-8e2c-1243502827d1.jpg?1601065203";
        else if(id.equals("491693"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/0/301750a7-d1fd-435e-bfa8-9d2fb22ad627.jpg?1601065373";
        else if(id.equals("491694"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/3/0/301750a7-d1fd-435e-bfa8-9d2fb22ad627.jpg?1601065373";
        else if(id.equals("491706"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/9/193071fe-180b-4d35-ba78-9c16675c29fc.jpg?1601065398";
        else if(id.equals("491707"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/1/9/193071fe-180b-4d35-ba78-9c16675c29fc.jpg?1601065398";
        else if(id.equals("491711"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/1/11568cdf-6148-494c-8b98-f5ca5797d775.jpg?1601065596";
        else if(id.equals("491712"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/1/1/11568cdf-6148-494c-8b98-f5ca5797d775.jpg?1601065596";
        else if(id.equals("491718"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/9/890eee8d-a339-4143-adfa-1b17ec10c099.jpg?1601065664";
        else if(id.equals("491719"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/9/890eee8d-a339-4143-adfa-1b17ec10c099.jpg?1601065664";
        else if(id.equals("491723"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/7/67f4c93b-080c-4196-b095-6a120a221988.jpg?1601065680";
        else if(id.equals("491724"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/6/7/67f4c93b-080c-4196-b095-6a120a221988.jpg?1601065680";
        else if(id.equals("491725"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/2/32779721-b021-4bd4-95d1-4a19b78d9faa.jpg?1601065728";
        else if(id.equals("491726"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/3/2/32779721-b021-4bd4-95d1-4a19b78d9faa.jpg?1601065728";
        else if(id.equals("491741"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/c/7c04c734-354d-4925-8161-7052110951df.jpg?1601065747";
        else if(id.equals("491742"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/7/c/7c04c734-354d-4925-8161-7052110951df.jpg?1601065747";
        else if(id.equals("491747"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/0/609d3ecf-f88d-4268-a8d3-4bf2bcf5df60.jpg?1601065770";
        else if(id.equals("491748"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/6/0/609d3ecf-f88d-4268-a8d3-4bf2bcf5df60.jpg?1601065770";
        else if(id.equals("491757"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/6/e63f8b20-f45b-4293-9aac-cdc021939be6.jpg?1601065790";
        else if(id.equals("491758"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/e/6/e63f8b20-f45b-4293-9aac-cdc021939be6.jpg?1601065790";
        else if(id.equals("491770"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/8/98496d5b-1519-4f0c-8b46-0a43be643dfb.jpg?1601065841";
        else if(id.equals("491771"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/9/8/98496d5b-1519-4f0c-8b46-0a43be643dfb.jpg?1601065841";
        else if(id.equals("491773"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/8/d8ed0335-daa6-4dbe-a94d-4d56c8cfd093.jpg?1601065859";
        else if(id.equals("491774"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/d/8/d8ed0335-daa6-4dbe-a94d-4d56c8cfd093.jpg?1601065859";
        else if(id.equals("491786"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/5/75240bbc-adc7-48ff-9523-c79776d710d3.jpg?1601065917";
        else if(id.equals("491787"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/7/5/75240bbc-adc7-48ff-9523-c79776d710d3.jpg?1601065917";
        else if(id.equals("491802"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/c/bc7239ea-f8aa-4a6f-87bd-c35359635673.jpg?1601065932";
        else if(id.equals("491803"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/c/bc7239ea-f8aa-4a6f-87bd-c35359635673.jpg?1601065932";
        else if(id.equals("491807"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/8/782ca27f-9f18-476c-b582-89c06fb2e322.jpg?1601065955";
        else if(id.equals("491808"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/7/8/782ca27f-9f18-476c-b582-89c06fb2e322.jpg?1601065955";
        else if(id.equals("491809"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/6/a69541db-3f4e-412f-aa8e-dec1e74f74dc.jpg?1601066019";
        else if(id.equals("491810"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/a/6/a69541db-3f4e-412f-aa8e-dec1e74f74dc.jpg?1601066019";
        else if(id.equals("491818"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/2/228e551e-023a-4c9a-8f32-58dae6ffdf7f.jpg?1601066052";
        else if(id.equals("491819"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/2/2/228e551e-023a-4c9a-8f32-58dae6ffdf7f.jpg?1601066052";
        else if(id.equals("491825"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/5/c5cb3052-358d-44a7-8cfd-cd31b236494a.jpg?1601066068";
        else if(id.equals("491826"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/c/5/c5cb3052-358d-44a7-8cfd-cd31b236494a.jpg?1601066068";
        else if(id.equals("491835"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/f/2f632537-63bf-4490-86e6-e6067b9c1a3b.jpg?1601066084";
        else if(id.equals("491836"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/2/f/2f632537-63bf-4490-86e6-e6067b9c1a3b.jpg?1601066084";
        else if(id.equals("491839"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/9/99535539-aa73-41ed-86ab-21c97b92620d.jpg?1601066100";
        else if(id.equals("491840"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/9/9/99535539-aa73-41ed-86ab-21c97b92620d.jpg?1601066100";
        else if(id.equals("491859"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/3/235d1ffc-72aa-40a2-95dc-3f6a8d495061.jpg?1601066116";
        else if(id.equals("491860"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/2/3/235d1ffc-72aa-40a2-95dc-3f6a8d495061.jpg?1601066116";
        else if(id.equals("491864"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/1/61bd69ea-1e9e-46b0-b1a1-ed7fdbe3deb6.jpg?1601066131";
        else if(id.equals("491865"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/6/1/61bd69ea-1e9e-46b0-b1a1-ed7fdbe3deb6.jpg?1601066131";
        else if(id.equals("491866"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/a/3a7fd24e-84d8-405d-86e4-0571a9e23cc2.jpg?1601066149";
        else if(id.equals("491867"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/3/a/3a7fd24e-84d8-405d-86e4-0571a9e23cc2.jpg?1601066149";
        else if(id.equals("491909"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/5/0511e232-2a72-40f5-a400-4f7ebc442d17.jpg?1601066169";
        else if(id.equals("491910"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/0/5/0511e232-2a72-40f5-a400-4f7ebc442d17.jpg?1601066169";
        else if(id.equals("491911"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/2/d24c3d51-795d-4c01-a34a-3280fccd2d78.jpg?1601066301";
        else if(id.equals("491912"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/d/2/d24c3d51-795d-4c01-a34a-3280fccd2d78.jpg?1601066301";
        else if(id.equals("491913"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/4/b4b99ebb-0d54-4fe5-a495-979aaa564aa8.jpg?1601066318";
        else if(id.equals("491914"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/4/b4b99ebb-0d54-4fe5-a495-979aaa564aa8.jpg?1601066318";
        else if(id.equals("491915"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/a/da57eb54-5199-4a56-95f7-f6ac432876b1.jpg?1601066338";
        else if(id.equals("491916"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/d/a/da57eb54-5199-4a56-95f7-f6ac432876b1.jpg?1601066338";
        else if(id.equals("491918"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/5/6559047e-6ede-4815-a3a0-389062094f9d.jpg?1601066358";
        else if(id.equals("491919"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/6/5/6559047e-6ede-4815-a3a0-389062094f9d.jpg?1601066358";
        else if(id.equals("491920"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/6/2668ac91-6cda-4f81-a08d-4fc5f9cb35b2.jpg?1601066375";
        else if(id.equals("491921"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/2/6/2668ac91-6cda-4f81-a08d-4fc5f9cb35b2.jpg?1601066375";
        else if(id.equals("495098"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/e/ae92e656-6c9d-48c3-a238-5a11c2c62ec8.jpg?1599831846";
        else if(id.equals("495099"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/8/589a324f-4466-4d4a-8cfb-806a041d7c1f.jpg?1599831830";
        else if(id.equals("495100"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/9/1967d4a8-6cc4-4a4d-9d24-93257de35e6d.jpg?1599831973";
        else if(id.equals("495101"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/c/3c6a38dd-e8f5-420f-9576-66937c71286b.jpg?1599832044";
        else if(id.equals("495102"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/b/2b90e88b-60a3-4d1d-bb8c-14633e5005a5.jpg?1599832083";
        else if(id.equals("29530711"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/9/4912a0a5-2fec-4c6b-9545-9ab0c4e25268.jpg?1599764491";
        else if(id.equals("1750411"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/f/8f047a8b-6c94-4b99-bcaa-10680400ee25.jpg?1562073449";
        else if(id.equals("5176911"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/b/cbbd8a12-d916-4fb1-994a-7d4a3e2ae2ab.jpg?1562935938";
        else if(id.equals("44680711"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg?1564694995";
        else if(id.equals("295726t") || id.equals("295673t") || id.equals("295532t"))
            cardurl = "https://deckmaster.info/images/cards/KLD/-3289-hr.jpg";
        else if(id.equals("295632t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/e/1ebc91a9-23e0-4ca1-bc6d-e710ad2efb31.jpg?1561756762";
        else if(id.equals("295802"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/c/4cb8d03e-e1d2-451e-97a8-141082f92501.jpg?1598627140";
        else if(id.equals("497724t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/6/661cbde4-9444-4259-b2cf-7c8f9814a071.jpg?1604666353";
        else if(id.equals("295810t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/5/458f44dd-83f1-497e-b5d0-e3417eb9dfec.jpg?1592672533";
        else if(id.equals("476226"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/a/caa7922e-3313-4f12-b91e-95aaa2d76cc2.jpg?1574133191";
        else if(id.equals("476217"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/9/a9a6cf9c-3560-435c-b0ec-8653a9dc7776.jpg?1578369133";
        else if(id.equals("503619"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/7/97502411-5c93-434c-b77b-ceb2c32feae7.jpg?1608253263";
        else if(id.equals("503620"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/9/7/97502411-5c93-434c-b77b-ceb2c32feae7.jpg?1608253263";
        else if(id.equals("503626"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/6/3606519e-5677-4c21-a34e-be195b6669fa.jpg?1611000242";
        else if(id.equals("503627"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/3/6/3606519e-5677-4c21-a34e-be195b6669fa.jpg?1611000242";
        else if(id.equals("503646"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/d/5d131784-c1a3-463e-a37b-b720af67ab62.jpg?1611323243";
        else if(id.equals("503647"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/5/d/5d131784-c1a3-463e-a37b-b720af67ab62.jpg?1611323243";
        else if(id.equals("503657"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/f/a/fab2fca4-a99f-4ffe-9c02-edb6e0be2358.jpg?1611862762";
        else if(id.equals("503658"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/f/a/fab2fca4-a99f-4ffe-9c02-edb6e0be2358.jpg?1611862762";
        else if(id.equals("503700"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/d/9dfdb73d-b001-4a59-b79e-8c8c1baea116.jpg?1610397419";
        else if(id.equals("503701"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/9/d/9dfdb73d-b001-4a59-b79e-8c8c1baea116.jpg?1610397419";
        else if(id.equals("503721"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/4/14dc88ee-bba9-4625-af0d-89f3762a0ead.jpg?1610586381";
        else if(id.equals("503722"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/1/4/14dc88ee-bba9-4625-af0d-89f3762a0ead.jpg?1610586381";
        else if(id.equals("503724"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/a/ea7e4c65-b4c4-4795-9475-3cba71c50ea5.jpg?1610153366";
        else if(id.equals("503725"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/e/a/ea7e4c65-b4c4-4795-9475-3cba71c50ea5.jpg?1610153366";
        else if(id.equals("503734"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/4/44657ab1-0a6a-4a5f-9688-86f239083821.jpg?1611054410";
        else if(id.equals("503735"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/4/4/44657ab1-0a6a-4a5f-9688-86f239083821.jpg?1611054410";
        else if(id.equals("503766"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/2/22a6a5f1-1405-4efb-af3e-e1f58d664e99.jpg?1610652603";
        else if(id.equals("503767"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/2/2/22a6a5f1-1405-4efb-af3e-e1f58d664e99.jpg?1610652603";
        else if(id.equals("503781"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/f/6/f6cd7465-9dd0-473c-ac5e-dd9e2f22f5f6.jpg?1610295185";
        else if(id.equals("503782"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/f/6/f6cd7465-9dd0-473c-ac5e-dd9e2f22f5f6.jpg?1610295185";
        else if(id.equals("503793"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/6/c697548f-925b-405e-970a-4e78067d5c8e.jpg?1610996900";
        else if(id.equals("503794"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/c/6/c697548f-925b-405e-970a-4e78067d5c8e.jpg?1610996900";
        else if(id.equals("503796"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/7/b76bed98-30b1-4572-b36c-684ada06826c.jpg?1610368590";
        else if(id.equals("503797"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/7/b76bed98-30b1-4572-b36c-684ada06826c.jpg?1610368590";
        else if(id.equals("503867"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/6/b6de14ae-0132-4261-af00-630bf15918cd.jpg?1608226844";
        else if(id.equals("503868"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/6/b6de14ae-0132-4261-af00-630bf15918cd.jpg?1608226844";
        else if(id.equals("503869"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/c/0ce39a19-f51d-4a35-ae80-5b82eb15fcff.jpg?1608058149";
        else if(id.equals("503870"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/0/c/0ce39a19-f51d-4a35-ae80-5b82eb15fcff.jpg?1608058149";
        else if(id.equals("503872"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/7/87a4e5fe-161f-42da-9ca2-67c8e8970e94.jpg?1608057969";
        else if(id.equals("503873"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/7/87a4e5fe-161f-42da-9ca2-67c8e8970e94.jpg?1608057969";
        else if(id.equals("503879"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/e/7ef37cb3-d803-47d7-8a01-9c803aa2eadc.jpg?1608058266";
        else if(id.equals("503880"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/7/e/7ef37cb3-d803-47d7-8a01-9c803aa2eadc.jpg?1608058266";
        else if(id.equals("503837t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/4/54a1c6a9-3531-4432-9157-e4400dbc89fd.jpg?1611206522";
        else if(id.equals("503841t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/f/df826c7d-5508-4e21-848c-91bc3e3f447a.jpg?1611206331";
        else if(id.equals("473148"))
            cardurl = "https://img.scryfall.com/cards/large/front/5/d/5dca90ef-1c17-4dcc-9fef-dab9ee92f590.jpg?1572490726";
        else if(id.equals("473127t"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/4/94057dc6-e589-4a29-9bda-90f5bece96c4.jpg?1572489125";
        else if(id.equals("295910t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/b/7b993828-e139-4cb6-a329-487accc1c515.jpg?1563073064";
        else if(id.equals("296315t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/d/ed58cd8c-b11a-4109-b789-0eb92eaf0184.jpg?1614969127";
        else if(id.equals("296247t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/7/07027a7c-5843-4d78-9b86-8799363c0b82.jpg?1591319174";
        else if(id.equals("296217t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/7/e72daa68-0680-431c-a616-b3693fd58813.jpg?1614969153";
        else if(id.equals("296145t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/b/cbcb0668-e88c-4462-b079-34f140c0277e.jpg?1614969101";
        else if(id.equals("295986t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/a/4a2144f2-d4be-419e-bfca-116cedfdf18b.jpg?1614968857";
        else if(id.equals("518429t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/f/6/f62080da-a11b-4da3-bb8f-57f543bf076a.jpg?1618767682";
        else if(id.equals("513482"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/8/18a2bdc8-b705-4eb5-b3a5-ff2e2ab8f312.jpg?1617901976";
        else if(id.equals("513483"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/1/8/18a2bdc8-b705-4eb5-b3a5-ff2e2ab8f312.jpg?1617901976";
        else if(id.equals("513624"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/9/d9131fc3-018a-4975-8795-47be3956160d.jpg?1617452954";
        else if(id.equals("513625"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/d/9/d9131fc3-018a-4975-8795-47be3956160d.jpg?1617452954";
        else if(id.equals("513626"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/2/c204b7ca-0904-40fa-b20c-92400fae20b8.jpg?1617583842";
        else if(id.equals("513627"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/c/2/c204b7ca-0904-40fa-b20c-92400fae20b8.jpg?1617583842";
        else if(id.equals("513628"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/a/ba09360a-067e-48a5-bdc5-a19fd066a785.jpg?1617453022";
        else if(id.equals("513629"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/b/a/ba09360a-067e-48a5-bdc5-a19fd066a785.jpg?1617453022";
        else if(id.equals("513630"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/d/0dba25e3-2b4f-45d4-965f-3834bcb359ee.jpg?1617453074";
        else if(id.equals("513631"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/0/d/0dba25e3-2b4f-45d4-965f-3834bcb359ee.jpg?1617453074";
        else if(id.equals("513632"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/7/d7148d24-373e-4485-860b-c3429c2337f2.jpg?1618163722";
        else if(id.equals("513633"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/d/7/d7148d24-373e-4485-860b-c3429c2337f2.jpg?1618163722";
        else if(id.equals("513634"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/b/8b45dc40-6827-46a7-a9b7-802be698d053.jpg?1617453130";
        else if(id.equals("513635"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/b/8b45dc40-6827-46a7-a9b7-802be698d053.jpg?1617453130";
        else if(id.equals("513636"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/e/8e4e0f81-f92b-4a3a-bb29-adcc3de211b4.jpg?1617363020";
        else if(id.equals("513637"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/e/8e4e0f81-f92b-4a3a-bb29-adcc3de211b4.jpg?1617363020";
        else if(id.equals("513638"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/a/aaa1e6be-08cc-4ccc-b2de-3511613e4fd0.jpg?1617612926";
        else if(id.equals("513639"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/a/a/aaa1e6be-08cc-4ccc-b2de-3511613e4fd0.jpg?1617612926";
        else if(id.equals("513640"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/b/5bd9b5cf-f018-48af-a081-995ce8ecc539.jpg?1617453238";
        else if(id.equals("513641"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/5/b/5bd9b5cf-f018-48af-a081-995ce8ecc539.jpg?1617453238";
        else if(id.equals("513642"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/8/18c16872-3675-4a4d-962a-2e17ad6f3886.jpg?1618326108";
        else if(id.equals("513643"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/1/8/18c16872-3675-4a4d-962a-2e17ad6f3886.jpg?1618326108";
        else if(id.equals("513644"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/9/8982ff88-8595-4363-8cde-6e87fb57a2d8.jpg?1617400731";
        else if(id.equals("513645"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/9/8982ff88-8595-4363-8cde-6e87fb57a2d8.jpg?1617400731";
        else if(id.equals("513646"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/3/938cee8f-ac2c-49a5-9ff7-1367d0edfabe.jpg?1617453576";
        else if(id.equals("513647"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/9/3/938cee8f-ac2c-49a5-9ff7-1367d0edfabe.jpg?1617453576";
        else if(id.equals("513648"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/7/87463b68-3642-41c7-a11c-67d524759b60.jpg?1617453416";
        else if(id.equals("513649"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/7/87463b68-3642-41c7-a11c-67d524759b60.jpg?1617453416";
        else if(id.equals("513650"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/c/8cfd0887-0c83-4b33-a85e-8b8ec5bf758d.jpg?1617453475";
        else if(id.equals("513651"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/back/8/c/8cfd0887-0c83-4b33-a85e-8b8ec5bf758d.jpg?1617453475";
        else if(id.equals("513652"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/5/65008352-bc7e-40b2-a832-b46813e5dc4c.jpg?1617453527";
        else if(id.equals("513652t") || id.equals("513638t") || id.equals("513543t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/0/d0ddbe3e-4a66-494d-9304-7471232549bf.jpg?1617626099";
        else if(id.equals("513634t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/1/910f48ab-b04e-4874-b31d-a86a7bc5af14.jpg?1617626097";
        else if(id.equals("296380t"))
            cardurl = "https://deckmaster.info/images/cards/KLD/-3287-hr.jpg";
        else if(id.equals("530447"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/f/6f509dbe-6ec7-4438-ab36-e20be46c9922.jpg?1626139695";
        else if(id.equals("530448"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/9/59b11ff8-f118-4978-87dd-509dc0c8c932.jpg?1626297774";
        else if(id.equals("530449"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/7/0/70b284bd-7a8f-4b60-8238-f746bdc5b236.jpg?1626297777";
        else if(id.equals("530448t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/4/1425e965-7eea-419c-a7ec-c8169fa9edbf.jpg?1626139812";
        else if(id.equals("530447t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/f/a/fa6fdb57-82f3-4695-b1fa-1f301ea4ef83.jpg?1626139846";
        else if(id.equals("527514t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/4/c49e8e79-8673-41c2-a1ad-273c37e27aca.jpg?1625767076";
        else if(id.equals("527507t"))
            cardurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/b/6b2c8f52-1580-42d5-8434-c4c70e31e31b.jpg?1626139372";

        return cardurl;
    }

    public static String getSpecialTokenUrl(String id) {
        String tokenurl = "";

        if(id.equals("121236t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-2-hr.jpg";
        else if (id.equals("380486t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-5-hr.jpg";
        else if (id.equals("52181t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-9-hr.jpg";
        else if (id.equals("262699t") || id.equals("262875t") || id.equals("262857t") || id.equals("53054t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-10-hr.jpg";
        else if(id.equals("378445t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-11-hr.jpg";
        else if (id.equals("380482t"))
            tokenurl = "https://deckmaster.info/images/cards/THS/-21-hr.jpg";
        else if (id.equals("184589t"))
            tokenurl = "https://deckmaster.info/images/cards/M14/-28-hr.jpg";
        else if (id.equals("368951t") || id.equals("426025t"))
            tokenurl = "https://deckmaster.info/images/cards/DGM/-39-hr.jpg";
        else if (id.equals("380487t") || id.equals("414506t"))
            tokenurl = "https://deckmaster.info/images/cards/JOU/-41-hr.jpg";
        else if (id.equals("114917t") || id.equals("52353t"))
            tokenurl = "https://deckmaster.info/images/cards/JOU/-43-hr.jpg";
        else if(id.equals("455911t") || id.equals("294389t"))
            tokenurl = "https://deckmaster.info/images/cards/GTC/-51-hr.jpg";
        else if(id.equals("234849t") || id.equals("366401t") || id.equals("366340t")
                || id.equals("366375t") || id.equals("460772t"))
            tokenurl = "https://deckmaster.info/images/cards/RTR/-61-hr.jpg";
        else if(id.equals("52973t"))
            tokenurl = "https://deckmaster.info/images/cards/RTR/-62-hr.jpg";
        else if (id.equals("48096t"))
            tokenurl = "https://deckmaster.info/images/cards/CNS/-89-hr.jpg";
        else if(id.equals("383290t"))
            tokenurl = "https://deckmaster.info/images/cards/M15/-108-hr.jpg";
        else if(id.equals("51984t"))
            tokenurl = "https://deckmaster.info/images/cards/DKA/-169-hr.jpg";
        else if(id.equals("439331t"))
            tokenurl = "https://deckmaster.info/images/cards/ISD/-177-hr.jpg";
        else if(id.equals("52494t") || id.equals("293206t") || id.equals("294605t"))
            tokenurl = "https://deckmaster.info/images/cards/NPH/-204-hr.jpg";
        else if(id.equals("294598t"))
            tokenurl = "https://deckmaster.info/images/cards/NPH/-205-hr.jpg";
        else if(id.equals("423817t") || id.equals("423700t") || id.equals("183017t") || id.equals("383129t") ||
                id.equals("6164t") || id.equals("456522t") || id.equals("456545t") || id.equals("397624t") ||
                id.equals("52637t") || id.equals("52945t") || id.equals("53460t") || id.equals("53473t") ||
                id.equals("420600t") || id.equals("294436t") || id.equals("489333t") || id.equals("495977t") ||
                id.equals("295775t") || id.equals("295714t") || id.equals("295698t") || id.equals("295635t") ||
                id.equals("296365t"))
            tokenurl = "https://deckmaster.info/images/cards/MBS/-216-hr.jpg";
        else if (id.equals("53057t") || id.equals("425825t"))
            tokenurl = "https://deckmaster.info/images/cards/SOM/-226-hr.jpg";
        else if(id.equals("140233t") || id.equals("191239t") || id.equals("205957t") || id.equals("423797t") ||
                id.equals("51861t"))
            tokenurl = "https://deckmaster.info/images/cards/M11/-234-hr.jpg";
        else if (id.equals("271227t"))
            tokenurl = "https://deckmaster.info/images/cards/WWK/-265-hr.jpg";
        else if (id.equals("53461t"))
            tokenurl = "https://deckmaster.info/images/cards/WWK/-266-hr.jpg";
        else if (id.equals("185704t"))
            tokenurl = "https://deckmaster.info/images/cards/ZEN/-277-hr.jpg";
        else if(id.equals("78975t"))
            tokenurl = "https://deckmaster.info/images/cards/ZEN/-281-hr.jpg";
        else if(id.equals("294401t"))
            tokenurl = "https://deckmaster.info/images/cards/ARB/-316-hr.jpg";
        else if (id.equals("175105t") || id.equals("295412t"))
            tokenurl = "https://deckmaster.info/images/cards/ALA/-325-hr.jpg";
        else if (id.equals("376496t") || id.equals("376549t") || id.equals("294519t"))
            tokenurl = "https://deckmaster.info/images/cards/ALA/-327-hr.jpg";
        else if (id.equals("247202t"))
            tokenurl = "https://deckmaster.info/images/cards/EVE/-338-hr.jpg";
        else if (id.equals("376546t"))
            tokenurl = "https://deckmaster.info/images/cards/SHM/-352-hr.jpg";
        else if (id.equals("244668t"))
            tokenurl = "https://deckmaster.info/images/cards/SHM/-356-hr.jpg";
        else if(id.equals("294507t"))
            tokenurl = "https://deckmaster.info/images/cards/SHM/-358-hr.jpg";
        else if(id.equals("294514t"))
            tokenurl = "https://deckmaster.info/images/cards/SHM/-360-hr.jpg";
        else if (id.equals("457111t") || id.equals("51931t"))
            tokenurl = "https://deckmaster.info/images/cards/MOR/-362-hr.jpg";
        else if (id.equals("376578t") || id.equals("152553t"))
            tokenurl = "https://deckmaster.info/images/cards/LRW/-365-hr.jpg";
        else if (id.equals("153166t"))
            tokenurl = "https://deckmaster.info/images/cards/LRW/-367-hr.jpg";
        else if(id.equals("83236t") || id.equals("45390t") || id.equals("965t") || id.equals("966t") ||
                id.equals("52750t"))
            tokenurl = "https://deckmaster.info/images/cards/8ED/-391-hr.jpg";
        else if(id.equals("294426t"))
            tokenurl = "https://deckmaster.info/images/cards/KTK/-457-hr.jpg";
        else if (id.equals("19878t"))
            tokenurl = "https://deckmaster.info/images/cards/C14/-482-hr.jpg";
        else if (id.equals("126166t"))
            tokenurl = "https://deckmaster.info/images/cards/C14/-487-hr.jpg";
        else if (id.equals("202474t") || id.equals("1098t") || id.equals("2024t") || id.equals("3766t") || id.equals("11183t") || id.equals("902t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-884-hr.jpg";
        else if (id.equals("202590t") || id.equals("2073t") || id.equals("1027t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-892-hr.jpg";
        else if (id.equals("3809t") || id.equals("2792t") || id.equals("1422t") || id.equals("159826t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-886-hr.jpg";
        else if (id.equals("407540t") || id.equals("407672t") || id.equals("407525t") || id.equals("293194t"))
            tokenurl = "https://deckmaster.info/images/cards/BFZ/-944-hr.jpg";
        else if (id.equals("460768t"))
            tokenurl = "https://deckmaster.info/images/cards/C15/-2009-hr.jpg";
        else if (id.equals("201124t") || id.equals("3118t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2029-hr.jpg";
        else if (id.equals("184730t") || id.equals("3192t") || id.equals("3193t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2028-hr.jpg";
        else if (id.equals("25910t"))
            tokenurl = "https://deckmaster.info/images/cards/AP/-2032-hr.jpg";
        else if (id.equals("6142t"))
            tokenurl = "https://deckmaster.info/images/cards/EX/-2035-hr.jpg";
        else if (id.equals("34929t"))
            tokenurl = "https://deckmaster.info/images/cards/JUD/-2043-hr.jpg";
        else if (id.equals("1649t") || id.equals("201182t"))
            tokenurl = "https://deckmaster.info/images/cards/LE/-2046-hr.jpg";
        else if (id.equals("4854t") || id.equals("376556t"))
            tokenurl = "https://deckmaster.info/images/cards/TE/-2059-hr.jpg";
        else if (id.equals("4771t"))
            tokenurl = "https://deckmaster.info/images/cards/TE/-2060-hr.jpg";
        else if (id.equals("9667t"))
            tokenurl = "https://deckmaster.info/images/cards/UG/-2062-hr.jpg";
        else if (id.equals("74265t"))
            tokenurl = "https://deckmaster.info/images/cards/UNH/-2064-hr.jpg";
        else if (id.equals("73953t"))
            tokenurl = "https://deckmaster.info/images/cards/UNH/-2065-hr.jpg";
        else if (id.equals("25956t"))
            tokenurl = "https://deckmaster.info/images/cards/AP/-2069-hr.jpg";
        else if (id.equals("184598t") || id.equals("2959t"))
            tokenurl = "https://deckmaster.info/images/cards/HM/-2070-hr.jpg";
        else if (id.equals("111046t"))
            tokenurl = "https://deckmaster.info/images/cards/PLC/-2071-hr.jpg";
        else if (id.equals("27634t") || id.equals("3227t") || id.equals("159097t") || id.equals("294453t"))
            tokenurl = "https://deckmaster.info/images/cards/PS/-2072-hr.jpg";
        else if (id.equals("3148t"))
            tokenurl = "https://deckmaster.info/images/cards/AL/-2156-hr.jpg";
        else if(id.equals("26815t") || id.equals("51774t"))
            tokenurl = "https://deckmaster.info/images/cards/AP/-2163-hr.jpg";
        else if (id.equals("1534t"))
            tokenurl = "https://deckmaster.info/images/cards/LE/-2165-hr.jpg";
        else if (id.equals("130314t"))
            tokenurl = "https://deckmaster.info/images/cards/FUT/-2168-hr.jpg";
        else if (id.equals("116383t"))
            tokenurl = "https://deckmaster.info/images/cards/TSP/-2170-hr.jpg";
        else if (id.equals("124344t"))
            tokenurl = "https://deckmaster.info/images/cards/PLC/-2172-hr.jpg";
        else if (id.equals("376404t"))
            tokenurl = "https://deckmaster.info/images/cards/OGW/-2189-hr.jpg";
        else if (id.equals("409810t") || id.equals("409805t") || id.equals("409953t") || id.equals("409997t") ||
                id.equals("410032t") || id.equals("293377t") || id.equals("294345t") || id.equals("295471t"))
            tokenurl = "https://deckmaster.info/images/cards/SOI/-2404-hr.jpg";
        else if (id.equals("3242t"))
            tokenurl = "https://deckmaster.info/images/cards/MI/-2828-hr.jpg";
        else if (id.equals("21382t"))
            tokenurl = "https://deckmaster.info/images/cards/PR/-2835-hr.jpg";
        else if (id.equals("293348t") || id.equals("293058t"))
            tokenurl = "https://deckmaster.info/images/cards/EMN/-2857-hr.jpg";
        else if (id.equals("416746t"))
            tokenurl = "https://deckmaster.info/images/cards/V16/-3110-hr.jpg";
        else if (id.equals("46168t"))
            tokenurl = "https://deckmaster.info/images/cards/KLD/-3287-hr.jpg";
        else if(id.equals("423843t") || id.equals("423739t") || id.equals("423718t") || id.equals("423736t") ||
                id.equals("423691t") || id.equals("423743t") || id.equals("423769t") || id.equals("423670t") ||
                id.equals("423796t") || id.equals("423680t") || id.equals("423693t") || id.equals("52046t")  ||
                id.equals("52791t")  || id.equals("53426t")  || id.equals("53432t")  || id.equals("294273t") ||
                id.equals("293046t") || id.equals("293107t") || id.equals("293548t") || id.equals("294419t") ||
                id.equals("295769t") || id.equals("295726t") || id.equals("295719t") || id.equals("295696t") ||
                id.equals("295675t") || id.equals("295673t") || id.equals("295661t") || id.equals("295612t") ||
                id.equals("295609t") || id.equals("295605t") || id.equals("295598t") || id.equals("295597t") ||
                id.equals("295574t") || id.equals("295538t") || id.equals("295535t") || id.equals("295532t") ||
                id.equals("295529t") || id.equals("295525t") || id.equals("295524t") || id.equals("295520t") ||
                id.equals("295513t") || id.equals("295506t") || id.equals("295502t") || id.equals("293737t"))
            tokenurl = "https://deckmaster.info/images/cards/KLD/-3289-hr.jpg";
        else if (id.equals("265141t"))
            tokenurl = "https://deckmaster.info/images/cards/VMA/-4465-hr.jpg";
        else if(id.equals("383077t"))
            tokenurl = "https://deckmaster.info/images/cards/VMA/-4469-hr.jpg";
        else if(id.equals("53274t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-4995-hr.jpg";
        else if(id.equals("53244t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5000-hr.jpg";
        else if(id.equals("53240t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5003-hr.jpg";
        else if(id.equals("53299t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5005-hr.jpg";
        else if(id.equals("53246t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5006-hr.jpg";
        else if(id.equals("53259t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5009-hr.jpg";
        else if(id.equals("53264t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5010-hr.jpg";
        else if(id.equals("53289t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5017-hr.jpg";
        else if(id.equals("53300t"))
            tokenurl = "https://deckmaster.info/images/cards/PZ2/-5018-hr.jpg";
        else if (id.equals("401697t") || id.equals("401692t") || id.equals("401701t"))
            tokenurl = "https://deckmaster.info/images/cards/C17/-5050-hr.jpg";
        else if (id.equals("376397t") || id.equals("107557t"))
            tokenurl = "https://deckmaster.info/images/cards/CMA/-5709-hr.jpg";
        else if(id.equals("52398t"))
            tokenurl = "https://deckmaster.info/images/cards/XLN/-5168-hr.jpg";
        else if (id.equals("435411t") || id.equals("435410t"))
            tokenurl = "https://deckmaster.info/images/cards/XLN/-5173-hr.jpg";
        else if (id.equals("1686t") || id.equals("2881t") || id.equals("201231t"))
            tokenurl = "https://deckmaster.info/images/cards/A25/-5648-hr.jpg";
        else if (id.equals("439843t"))
            tokenurl = "https://deckmaster.info/images/cards/RIX/-5473-hr.jpg";
        else if(id.equals("447070t") || id.equals("53480t"))
            tokenurl = "https://deckmaster.info/images/cards/GS1/-5944-hr.jpg";
        else if(id.equals("53190t"))
            tokenurl = "https://deckmaster.info/images/cards/CM2/-6027-hr.jpg";
        else if (id.equals("452760t"))
            tokenurl = "https://deckmaster.info/images/cards/M19/-6036.jpg";
        else if(id.equals("53453t"))
            tokenurl = "https://deckmaster.info/images/cards/C18/-6244-hr.jpg";
        else if(id.equals("53438t"))
            tokenurl = "https://deckmaster.info/images/cards/C18/-6247-hr.jpg";
        else if(id.equals("53463t"))
            tokenurl = "https://deckmaster.info/images/cards/C18/-6252-hr.jpg";
        else if(id.equals("52149t"))
            tokenurl = "https://deckmaster.info/images/cards/GRN/-6433-hr.jpg";
        else if (id.equals("89110t") || id.equals("456379t"))
            tokenurl = "https://deckmaster.info/images/cards/GK1_SELESN/-6550-hr.jpg";
        else if (id.equals("3832t"))
            tokenurl = "https://deckmaster.info/images/cards/GK1_DIMIR/-6541-hr.jpg";
        else if (id.equals("116384t") || id.equals("376564t") || id.equals("52993t"))
            tokenurl = "https://deckmaster.info/images/cards/TSP/-114916-hr.jpg";
        else if(id.equals("17841t") || id.equals("17850t") || id.equals("17852t") || id.equals("19444t") ||
                id.equals("294101t") || id.equals("294226t"))
            tokenurl = "https://deckmaster.info/images/cards/EVG/159047-hr.jpg";
        else if(id.equals("383392t"))
            tokenurl = "https://deckmaster.info/images/cards/DDD/201842-hr.jpg";
        else if (id.equals("5610t") || id.equals("416754t"))
            tokenurl = "https://deckmaster.info/images/cards/DDE/207998-hr.jpg";
        else if (id.equals("5173t"))
            tokenurl = "https://deckmaster.info/images/cards/DDE/209163-hr.jpg";
        else if(id.equals("378521t") || id.equals("52418t"))
            tokenurl= "https://deckmaster.info/images/cards/DDO/394383-hr.jpg";
        else if(id.equals("52136t"))
            tokenurl= "https://deckmaster.info/images/cards/DDO/394407-hr.jpg";
        else if (id.equals("293619t") || id.equals("294261t") || id.equals("293585t"))
            tokenurl = "https://deckmaster.info/images/cards/DDP/401720-hr.jpg";
        else if (id.equals("271158t") || id.equals("401703t"))
            tokenurl = "https://deckmaster.info/images/cards/DDP/401721-hr.jpg";
        else if (id.equals("88973t") || id.equals("368549t"))
            tokenurl = "https://deckmaster.info/images/cards/DDQ/409655-hr.jpg";
        else if (id.equals("53454t"))
            tokenurl = "https://deckmaster.info/images/cards/DDQ/409656-hr.jpg";
        else if (id.equals("416829t"))
            tokenurl = "https://deckmaster.info/images/cards/CN2/416829-hr.jpg";
        else if (id.equals("417465t") || id.equals("294137t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417494-hr.jpg";
        else if (id.equals("417480t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417495-hr.jpg";
        else if (id.equals("417481t") || id.equals("293725t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417496-hr.jpg";
        else if (id.equals("417447t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417497-hr.jpg";
        else if(id.equals("3392t") || id.equals("220535t") || id.equals("376253t") || id.equals("376390t") ||
                id.equals("401643t") || id.equals("417451t") || id.equals("417424t") || id.equals("51908t") ||
                id.equals("52593t") || id.equals("53161t") || id.equals("53439t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417498-hr.jpg";
        else if (id.equals("21381t") || id.equals("40198t"))
            tokenurl = "https://img.scryfall.com/cards/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg?1562921132";
        else if (id.equals("461099t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/d/e/de7ba875-f77b-404f-8b75-4ba6f81da410.jpg?1557575978";
        else if (id.equals("426909t") || id.equals("426705t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/8/98956e73-04e4-4d7f-bda5-cfa78eb71350.jpg?1562844807";
        else if (id.equals("426897t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/8/a8f339c6-2c0d-4631-849b-44d4360b5131.jpg?1562844814";
        else if (id.equals("457139t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/0/105e687e-7196-4010-a6b7-cfa42d998fa4.jpg?1560096976";
        else if (id.equals("470549t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/7/7/7711a586-37f9-4560-b25d-4fb339d9cd55.jpg?1565299650";
        else if (id.equals("113527t") || id.equals("376321t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/5/b/5b9f471a-1822-4981-95a9-8923d83ddcbf.jpg?1562702075";
        else if (id.equals("114919t") || id.equals("247519t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/b/5/b5ddb67c-82fb-42d6-a4c2-11cd38eb128d.jpg?1562702281";
        else if (id.equals("8862t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/d/b/dbf33cc3-254f-4c5c-be22-3a2d96f29b80.jpg?1562936030";
        else if(id.equals("213757t") || id.equals("213734t") || id.equals("221554t") || id.equals("48049t") ||
                id.equals("46160t") || id.equals("47450t") || id.equals("376421t") || id.equals("213725t") ||
                id.equals("52492t") || id.equals("489268t") || id.equals("489458t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/f/3/f32ad93f-3fd5-465c-ac6a-6f8fb57c19bd.jpg?1561758422";
        else if (id.equals("247393t") || id.equals("247399t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg?1562636755";
        else if (id.equals("152998t") || id.equals("152963t") || id.equals("52364t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/5/959ed4bf-b276-45ed-b44d-c757e9c25846.jpg?1562702204";
        else if (id.equals("46703t") || id.equals("227151t") || id.equals("205298t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/0/a/0a9a25fd-1a4c-4d63-bbfa-296ef53feb8b.jpg?1562541933";
        else if (id.equals("394380t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/6/2/622397a1-6513-44b9-928a-388be06d4022.jpg?1562702085";
        else if (id.equals("1138t") || id.equals("2074t") || id.equals("640t") || id.equals("3814t") || id.equals("11530t") ||
                id.equals("43t") || id.equals("338t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/c/7/c75b81b5-5c84-45d4-832a-20c038372bc6.jpg?1561758040";
        else if (id.equals("275261t") || id.equals("271156t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg?1562636755";
        else if (id.equals("376455t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/e/9e0eeebf-7c4a-436b-8cb4-292e53783ff2.jpg?1562926847";
        else if(id.equals("414388t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/b/8/b8710a30-8314-49ef-b995-bd05454095be.jpg?1562636876";
        else if(id.equals("382874t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/8/3/83dcacd3-8707-4354-a1a5-9863d677d67f.jpg?1562702177";
        else if(id.equals("383065t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/8/5/8597029c-3b0d-476e-a6ee-48402f815dab.jpg?1561757494";
        else if(id.equals("414350t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/e/4/e4439a8b-ef98-428d-a274-53c660b23afe.jpg?1562636929";
        else if(id.equals("414349t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/e/4/e4439a8b-ef98-428d-a274-53c660b23afe.jpg?1562636929";
        else if(id.equals("414429t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/d/b/dbd994fc-f3f0-4c81-86bd-14ca63ec229b.jpg?1562636922";
        else if(id.equals("414314t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg?1562636737";
        else if(id.equals("414313t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg?1562636737";
        else if(id.equals("227061t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/5/f/5f68c2ab-5131-4620-920f-7ba99522ccf0.jpg?1562639825";
        else if(id.equals("227072t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/5/f/5f68c2ab-5131-4620-920f-7ba99522ccf0.jpg?1562639825";
        else if(id.equals("245250t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/5/a53f8031-aaa8-424c-929a-5478538a8cc6.jpg?1562639960";
        else if(id.equals("245251t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/5/a53f8031-aaa8-424c-929a-5478538a8cc6.jpg?1562639960";
        else if(id.equals("398441t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/e/5/e5ccae95-95c2-4d11-aa68-5c80ecf90fd2.jpg?1562640112";
        else if (id.equals("409826t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/e/0/e0a12a72-5cd9-4f1b-997d-7dabb65e9f51.jpg?1562086884";
        else if (id.equals("51939t") || id.equals("52121t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/b/9/b999a0fe-d2d0-4367-9abb-6ce5f3764f19.jpg?1562640005";
        else if (id.equals("52110t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/0/b/0bb628da-a02f-4d3e-b919-0c03821dd5f2.jpg?1561756633";
        else if (id.equals("473141t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/b/f/bf36408d-ed85-497f-8e68-d3a922c388a0.jpg?1567710130";
        else if(id.equals("53180t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg?1562636755";
        else if(id.equals("53118t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/0/3/03553980-53fa-4256-b478-c7e0e73e2b5b.jpg?1563132220";
        else if(id.equals("53268t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/6/c/6c1ffb14-9d92-4239-8694-61d156c9dba7.jpg?1562254006";
        else if(id.equals("53403t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/e/ae196fbc-c9ee-4dba-9eb3-52209908b898.jpg?1561757813";
        else if(id.equals("53408t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/0/e/0e80f154-9409-40fa-a564-6fc296498d80.jpg?1562898335";
        else if(id.equals("53417t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/2/9/29c4e4f2-0040-4490-b357-660d729ad9cc.jpg?1562636772";
        else if(id.equals("53326t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/7/4/748d267d-9c81-4dc0-92b7-eafb7691c6cc.jpg?1562636817";
        else if(id.equals("16787t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/e/8/e8a56b33-f720-4cbf-8015-59b5fd8ff756.jpg?1562941690";
        else if(id.equals("16759t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/f/3/f3b5665e-2b97-47c7-bbf9-6549c2c8a9f2.jpg?1562944002";
        else if(id.equals("456382t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/b/6/b64c5f80-4676-4860-be0e-20bcf2227405.jpg?1562540215";
        else if(id.equals("460464t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/4/94ed2eca-1579-411d-af6f-c7359c65de30.jpg?1562086876";
        else if(id.equals("19461t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/d/2/d2f51f4d-eb6d-4503-b9a4-559db1b9b16f.jpg?1574710411";
        else if(id.equals("19471t") || id.equals("19472t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/3/4/340fb06f-4bb0-4d23-b08c-8b1da4a8c2ad.jpg?1574709457";
        else if(id.equals("294089t") || id.equals("294717t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/8/b/8b4f81e2-916f-4af4-9e63-f4469e953122.jpg?1562702183";
        else if(id.equals("293323t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/2/f/2f4b7c63-8430-4ca4-baee-dc958d5bd22f.jpg?1557575919";
        else if (id.equals("74492t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/94/295/635032496473215708.jpg";
        else if (id.equals("3280t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/54/421/635032484680831888.jpg";
        else if (id.equals("107091t") || id.equals("295407t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/13/534/635032476540667501.jpg";
        else if (id.equals("184735t") || id.equals("376488t") || id.equals("3066t") || id.equals("121261t"))
            tokenurl = "https://i.pinimg.com/originals/a9/fb/37/a9fb37bdfa8f8013b7eb854d155838e2.jpg";
        else if (id.equals("205297t") || id.equals("50104t"))
            tokenurl = "https://i.pinimg.com/564x/cc/96/e3/cc96e3bdbe7e0f4bf1c0c1f942c073a9.jpg";
        else if (id.equals("3591t"))
            tokenurl = "https://i.pinimg.com/564x/6e/8d/fe/6e8dfeee2919a3efff210df56ab7b85d.jpg";
        else if (id.equals("136155t"))
            tokenurl = "https://i.pinimg.com/564x/5d/68/d6/5d68d67bef76bf90588a4afdc39dc60e.jpg";
        else if (id.equals("439538t"))
            tokenurl = "https://i.pinimg.com/originals/da/e3/31/dae3312aa1f15f876ebd363898847e23.jpg";
        else if(id.equals("397656t"))
            tokenurl = "https://i.pinimg.com/originals/3c/f4/55/3cf45588a840361b54a95141b335b76c.jpg";
        else if(id.equals("51789t") || id.equals("52682t"))
            tokenurl = "https://i.pinimg.com/originals/4c/40/ae/4c40ae9a4a4c8bb352b26bea0f277a26.jpg";
        else if (id.equals("3421t") || id.equals("15434t"))
            tokenurl = "https://www.mtg.onl/static/3c152b4fc1c64e3ce21022f53ec16559/4d406/PROXY_Cat_G_1_1.jpg";
        else if (id.equals("73976t"))
            tokenurl = "https://www.mtg.onl/static/8bbca3c195e798ca92b4a112275072e2/4d406/PROXY_Ape_G_1_1.jpg";
        else if (id.equals("49026t"))
            tokenurl = "https://www.mtg.onl/static/a9d81341e62e39e75075b573739f39d6/4d406/PROXY_Wirefly_2_2.jpg";
        else if (id.equals("3449t"))
            tokenurl = "https://www.mtg.onl/static/8c7fed1a0b8edd97c0fb0ceab24a654f/4d406/PROXY_Goblin_Scout_R_1_1.jpg";
        else if (id.equals("24624t"))
            tokenurl = "https://www.mtg.onl/static/6d717cba653ea9e3f6bd1419741671cb/4d406/PROXY_Minion_B_1_1.jpg";
        else if (id.equals("89051t") || id.equals("519129t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/0/a0b5e1f4-9206-40b6-9cf6-331f6a95d045.jpg?1618767339";
        else if (id.equals("72858t"))
            tokenurl = "https://www.mtg.onl/static/348314ede9097dd8f6dd018a6502d125/4d406/PROXY_Pincher_2_2.jpg";
        else if (id.equals("3113t"))
            tokenurl = "https://www.mtg.onl/static/fca7508d78c26e3daea78fd4640faf9a/4d406/PROXY_Orb_U_X_X.jpg";
        else if (id.equals("74027t"))
            tokenurl = "https://www.mtg.onl/static/48515f01d0fda15dd9308d3a528dae7b/4d406/PROXY_Spirit_W_3_3.jpg";
        else if (id.equals("23319t"))
            tokenurl = "https://www.mtg.onl/static/0f8b0552293c03a3a29614cc83024337/4d406/PROXY_Reflection_W_X_X.jpg";
        else if (id.equals("130638t"))
            tokenurl = "https://www.mtg.onl/static/20b01e1378e7b8e8b47066c52761fde2/4d406/PROXY_Giant_R_4_4.jpg";
        else if (id.equals("74411t"))
            tokenurl = "https://www.mtg.onl/static/5f65ea90850736160a28f3a5bd56744a/4d406/PROXY_Warrior_R_1_1.jpg";
        else if (id.equals("121156t"))
            tokenurl = "https://www.mtg.onl/static/3db04e8bdd45aac4bb25bb85cdb05ac0/4d406/PROXY_Wolf_G_1_1.jpg";
        else if (id.equals("126816t"))
            tokenurl = "https://www.mtg.onl/static/e25f8b900e6238d0047039da4690f1c4/4d406/PROXY_Knight_B_2_2.jpg";
        else if (id.equals("75291t"))
            tokenurl = "http://4.bp.blogspot.com/-y5Fanm3qvrU/Vmd4gGnl2DI/AAAAAAAAAWY/FCrS9FTgOJk/s1600/Tatsumasa%2BToken.jpg";
        else if (id.equals("26732t"))
            tokenurl = "http://1.bp.blogspot.com/-0-mLvfUVgNk/VmdZWXWxikI/AAAAAAAAAUM/TVCIiZ_c67g/s1600/Spawn%2BToken.jpg";
        else if (id.equals("47449t") || id.equals("52335t") || id.equals("295457t"))
            tokenurl = "https://1.bp.blogspot.com/-vrgXPWqThMw/XTyInczwobI/AAAAAAAADW4/SEceF3nunBkiCmHWfx6UxEUMF_gqdrvUQCLcBGAs/s1600/Kaldra%2BToken%2BUpdate.jpg";
        else if(id.equals("460140t") || id.equals("460146t"))
            tokenurl = "http://4.bp.blogspot.com/-jmiOVll5hDk/VmdvG_Hv7hI/AAAAAAAAAVg/oWYbn2yBPI8/s1600/White-Blue%2BBird%2BToken.jpg";
        else if (id.equals("5261t"))
            tokenurl = "https://static.cardmarket.com/img/5a0199344cad68eebeefca6fa24e52c3/items/1/MH1/376905.jpg";
        else if (id.equals("430686t"))
            tokenurl = "https://cdn.shopify.com/s/files/1/1601/3103/products/Token_45_2000x.jpg?v=1528922847";
        else if (id.equals("405191t"))
            tokenurl = "https://6d4be195623157e28848-7697ece4918e0a73861de0eb37d08968.ssl.cf1.rackcdn.com/108181_200w.jpg";
        else if (id.equals("476402t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/6/0/60466c78-155e-442b-8022-795e1e9de8df.jpg?1581901998";
        else if(id.equals("484904t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/2/1/21e89101-f1cf-4bbd-a1d5-c5d48512e0dd.jpg?1562539760";
        else if(id.equals("489168t"))
            tokenurl="https://img.scryfall.com/cards/large/front/d/e/dee1c2ee-d92e-409a-995a-b4c91620c918.jpg?1581901969";
        else if(id.equals("489401t"))
            tokenurl="https://i.pinimg.com/564x/01/b0/a2/01b0a289e1a28167cbf0f30532328d99.jpg";
        else if(id.equals("489171t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/4/f/4f8107b3-8539-4b9c-8d0d-c512c940838f.jpg?1592324480";
        else if(id.equals("489403t") || id.equals("489358t") || id.equals("489372t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/5/959ed4bf-b276-45ed-b44d-c757e9c25846.jpg";
        else if(id.equals("489562t") || id.equals("296282t"))
            tokenurl="https://img.scryfall.com/cards/large/front/c/f/cf9a289f-cd3f-42a0-9296-8c7cc7d01a91.jpg?1561758108";
        else if(id.equals("489363t"))
            tokenurl="https://img.scryfall.com/cards/large/front/8/3/83dcacd3-8707-4354-a1a5-9863d677d67f.jpg?1562702177";
        else if(id.equals("489900t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/8/6/8676704a-419e-4a00-a052-bca2ad34ecae.jpg?1596044091";
        else if(id.equals("489695t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/7/b/7becaa04-f142-4163-9286-00018b95c4ca.jpg?1596044169";
        else if(id.equals("489907t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/9/e/9ecc467e-b345-446c-b9b7-5f164e6651a4.jpg?1596043489";
        else if(id.equals("295082t"))
            tokenurl = "https://img.scryfall.com/cards/large/front/a/e/ae56d9e8-de05-456b-af32-b5992992ee15.jpg?1562639978";
        else if(id.equals("496035t") || id.equals("295423t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/d/ed666385-a2e7-4e1f-ad2c-babbfc0c50b3.jpg?1562640123";
        else if(id.equals("496036t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/1/e1eb3b8a-f9d3-4ce1-b171-ba7b0c3f4830.jpg?1562702405";
        else if(id.equals("495984t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/3/33bd708d-dc84-44d3-a563-536ade028bd0.jpg?1562702016";
        else if(id.equals("495971t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/7/67457137-64f2-413d-b62e-658b3f1b1043.jpg?1547509251";
        else if(id.equals("495958t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/b/e/be224180-a482-4b94-8a9d-3a92ee0eb34b.jpg?1562640020";
        else if(id.equals("295356t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/7/c7e7822b-f155-4f3f-b835-ec64f3a71307.jpg?1601138813";
        else if(id.equals("295376t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/b/cb8caa61-e294-4501-b357-a44abd77d09a.jpg?1601138497";
        else if(id.equals("295334t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/c/dcee70ef-6285-4f09-8a71-8b7960e8fa99.jpg?1562636925";
        else if(id.equals("295433t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/f/2f4b7c63-8430-4ca4-baee-dc958d5bd22f.jpg?1557575919";
        else if(id.equals("295428t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/5/3/5371de1b-db33-4db4-a518-e35c71aa72b7.jpg?1562702067";
        else if(id.equals("295377t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/e/ce90c48f-74fb-4e87-9e46-7f8c3d79cbb0.jpg?1562636904";
        else if(id.equals("295322t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/0/903e30f3-580e-4a14-989b-ae0632363407.jpg?1581902165";
        else if(id.equals("295234t"))
            tokenurl ="https://c1.scryfall.com/file/scryfall-cards/large/front/d/c/dc77b308-9d0c-492f-b3fe-e00d60470767.jpg?1563073222";
        else if(id.equals("295225t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/9/d9c95045-e806-4933-94a4-cb52ae1a215b.jpg?1562542413";
        else if(id.equals("295217t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/4/0419a202-6e32-4f0a-a032-72f6c00cae5e.jpg?1562639654";
        else if(id.equals("295556t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/2/623a08d1-f5ff-48b7-bdb6-54b8d7a4b931.jpg?1562639829";
        else if(id.equals("503330t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/5/c5ad13b4-bbf5-4c98-868f-4d105eaf8833.jpg?1592710082";
        else if(id.equals("503754t") || id.equals("503827t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/a/4ae9f454-4f8c-4123-9886-674bc439dfe7.jpg?1611206933";
        else if(id.equals("503846t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/d/3db39e3b-fad4-4c9b-911f-69883ac7e0e1.jpg?1611206884";
        else if(id.equals("503821t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/f/ef775ad0-b1a9-4254-ab6f-304558bb77a1.jpg?1611247501";
        else if(id.equals("508147t") || id.equals("508338t") || id.equals("508160t") || id.equals("508357t") ||
                id.equals("508354t") || id.equals("508349t") || id.equals("508343t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg?1611206824";
        else if(id.equals("295919t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg?1611206824";
        else if(id.equals("518457t") || id.equals("518473t") || id.equals("518468t") || id.equals("518463t") || id.equals("518460t") ||
                id.equals("518422t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/1/910f48ab-b04e-4874-b31d-a86a7bc5af14.jpg?1617626097";
        else if(id.equals("518467t") || id.equals("518410t") || id.equals("518436t") || id.equals("518308t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/9/c9deae5c-80d4-4701-b425-91853b7ee03b.jpg?1617626050";
        else if(id.equals("518461t") || id.equals("518432t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/d/0/d0ddbe3e-4a66-494d-9304-7471232549bf.jpg?1617626099";
        else if(id.equals("518310t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/d/3d0b9b88-705e-4df0-8a93-3e240b81355b.jpg?1617626092";
        else if(id.equals("513663t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/1/a/1a2d027f-8996-4761-a776-47cd428f6779.jpg?1618766925";
        else if(id.equals("522245t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/3/7/37e32ba6-108a-421f-9dad-3d03f7ebe239.jpg?1623113548";
        else if(id.equals("296413t"))
            tokenurl = "https://i.pinimg.com/564x/af/cc/4c/afcc4c87d67c9651838fed09217c7eed.jpg";
        else if(id.equals("296410t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/4/94057dc6-e589-4a29-9bda-90f5bece96c4.jpg?1572489125";
        else if(id.equals("527539t") || id.equals("527477t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/3/a3a684b7-27e0-4d9e-a064-9e03c6e50c89.jpg?1626139418";
        else if(id.equals("527351t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/c/e/ce3c0bd9-8a37-4164-9937-f35d1c210fe8.jpg?1626139016";
        else if(id.equals("527378t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/6/86881c5f-df5e-4f50-b554-e4c49d5316f9.jpg?1625676073";
        else if(id.equals("532511t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/a/3/a378702b-d074-4402-b423-2ca8f44fce7c.jpg?1572370699";
        else if(id.equals("532519t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/2/62cafc0a-cd02-4265-aa1f-b8a6cb7cc8db.jpg?1592710150";
        else if(id.equals("532527t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/8/e/8e3b2942-d1a4-4d27-9d64-65712497ab2e.jpg?1561897504";
        else if(id.equals("532560t") || id.equals("532659t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/5/95483574-95b7-42a3-b700-616189163b0a.jpg?1598312392";
        else if(id.equals("531918t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/7/076f934b-a244-45f1-bcb3-7c5e882e9911.jpg?1594733476";
        else if(id.equals("532539t") || id.equals("531948t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/4/4/44a4ef4a-a026-424e-88ff-e2bb77aaf05d.jpg?1625974889";
        else if(id.equals("532482t") || id.equals("532493t") || id.equals("532491t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/e/4/e43a205e-43ea-4b3e-92ab-c2ee2172a50a.jpg?1572489150";
        else if(id.equals("532599t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/9/c/9c8fe0d7-5c40-45fe-b3d8-47852380845e.jpg?1562542392";
        else if(id.equals("531833t"))
            tokenurl = "https://i.pinimg.com/564x/04/dc/04/04dc041251acb96f97327d67e9c8fe23.jpg";
        else if(id.equals("532489t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/6/0/60842b1a-6ae7-4b3b-a23f-0d94a3d89884.jpg?1562639827";
        else if(id.equals("531921t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/2/3/2300635e-7771-4676-a5a5-29a9d8f49f1a.jpg?1604194799";
        else if(id.equals("531928t") || id.equals("531933t"))
            tokenurl = "https://c1.scryfall.com/file/scryfall-cards/large/front/0/b/0b08d210-01cb-46c5-9150-4dfb47f50ae7.jpg?1626139197";
        return tokenurl;
    }

    public static boolean hasToken(String id) {
        if (id.equals("456378") || id.equals("2912") || id.equals("1514") || id.equals("364") || id.equals("69") || id.equals("369012") ||
                id.equals("417759") || id.equals("386476") || id.equals("456371") || id.equals("456360") || id.equals("391958") || id.equals("466959") ||
                id.equals("466813") || id.equals("201176") || id.equals("202483") || id.equals("3546") || id.equals("425949") || id.equals("426027") ||
                id.equals("425853") || id.equals("425846") || id.equals("426036") || id.equals("370387") || id.equals("29955") || id.equals("29989") ||
                id.equals("19741") || id.equals("19722") || id.equals("19706") || id.equals("24597") || id.equals("24617") || id.equals("24563") ||
                id.equals("253539") || id.equals("277995") || id.equals("265415") || id.equals("289225") || id.equals("289215") || id.equals("253529") ||
                id.equals("253641") || id.equals("270957") || id.equals("401685") || id.equals("89116") || id.equals("5183") || id.equals("5177") ||
                id.equals("209289") || id.equals("198171") || id.equals("10419") || id.equals("470542") || id.equals("29992") || id.equals("666") ||
                id.equals("2026") || id.equals("45395") || id.equals("442021") || id.equals("423758") || id.equals("426930") || id.equals("998") ||
                id.equals("446163") || id.equals("378411") || id.equals("376457") || id.equals("470749") || id.equals("450641") || id.equals("470623") ||
                id.equals("470620") || id.equals("470754") || id.equals("470750") || id.equals("470739") || id.equals("470708") || id.equals("470581") ||
                id.equals("470578") || id.equals("470571") || id.equals("470552") || id.equals("394490") || id.equals("114921") || id.equals("49775")  ||
                id.equals("473123") || id.equals("473160") || id.equals("16743")  || id.equals("16741") || id.equals("294493") || id.equals("293253") ||
                id.equals("293198") || id.equals("479634") || id.equals("479702") || id.equals("489837") || id.equals("489861") || id.equals("491359") ||
                id.equals("294872") || id.equals("295110") || id.equals("294842") || id.equals("295067") || id.equals("491767") || id.equals("295386")  ||
                id.equals("295229") || id.equals("295387") || id.equals("295206") || id.equals("295706") || id.equals("497549")  || id.equals("497666") ||
                id.equals("503860") || id.equals("522280") || id.equals("522111") || id.equals("527288") || id.equals("531927"))
            return false;
        return true;
    }

    public static Document findTokenPage(String imageurl, String name, String set, String[] availableSets, String tokenstats, String color, SDLActivity parent) throws Exception {
        Document doc = null;
        Elements outlinks = null;
        try {
            doc = Jsoup.connect(imageurl + "t" + set.toLowerCase()).get();
            if (doc != null) {
                outlinks = doc.select("body a");
                if (outlinks != null) {
                    for (int k = 0; k < outlinks.size() && parent.downloadInProgress; k++) {
                        while (parent.paused && parent.downloadInProgress) {
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                        }
                        if (!parent.downloadInProgress)
                            break;
                        String linktoken = outlinks.get(k).attributes().get("href");
                        if (linktoken != null && !linktoken.isEmpty()) {
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if (tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if (stats != null) {
                                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                                        while (parent.paused && parent.downloadInProgress) {
                                            try {
                                                Thread.sleep(1000);
                                            } catch (InterruptedException e) {
                                            }
                                        }
                                        if (!parent.downloadInProgress)
                                            break;
                                        String a = stats.get(j).attributes().get("content");
                                        if (stats.get(j).attributes().get("content").contains(tokenstats) &&
                                                stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                                            return tokendoc;
                                        }
                                    }
                                }
                            } catch (Exception e) {
                            }
                        }
                    }
                }
            }
        } catch (Exception e) {
        }
        System.out.println("Warning: Token " + name + " has not been found in " + set + " tokens, i will search for it in https://deckmaster.info");
        String json = "";
        try {
            URL url = new URL("https://deckmaster.info/includes/ajax.php?action=cardSearch&searchString=" + name);
            HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
            if (httpcon != null) {
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                InputStream stream = httpcon.getInputStream();
                if (stream != null) {
                    int i;
                    while ((i = stream.read()) != -1) {
                        json = json + ((char) i);
                    }
                }
            }
        } catch (Exception e) {
        }
        List<String> urls = new ArrayList<String>();
        String[] tok = json.split(",");
        for (int i = 0; i < tok.length; i++) {
            if (tok[i].contains("multiverseid")) {
                String id = tok[i].split(":")[1].replace("\"", "");
                urls.add(id);
            }
        }
        for (int i = 0; i < urls.size() && parent.downloadInProgress; i++) {
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;
            try {
                Document tokendoc = Jsoup.connect("https://deckmaster.info/card.php?multiverseid=" + urls.get(i)).get();
                if (tokendoc == null)
                    continue;
                Elements stats = tokendoc.select("head meta");
                if (stats != null) {
                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                        while (parent.paused && parent.downloadInProgress) {
                            try {
                                Thread.sleep(1000);
                            } catch (InterruptedException e) {
                            }
                        }
                        if (!parent.downloadInProgress)
                            break;
                        if (stats.get(j).attributes().get("content").contains("Token Creature")
                                && stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                            stats = tokendoc.select("body textarea");
                            if (stats != null) {
                                for (int y = 0; y < stats.size() && parent.downloadInProgress; y++) {
                                    while (parent.paused && parent.downloadInProgress) {
                                        try {
                                            Thread.sleep(1000);
                                        } catch (InterruptedException e) {
                                        }
                                    }
                                    if (!parent.downloadInProgress)
                                        break;
                                    List<Node> nodes = stats.get(y).childNodes();
                                    if (nodes != null) {
                                        for (int p = 0; p < nodes.size() && parent.downloadInProgress; p++) {
                                            while (parent.paused && parent.downloadInProgress) {
                                                try {
                                                    Thread.sleep(1000);
                                                } catch (InterruptedException e) {
                                                }
                                            }
                                            if (!parent.downloadInProgress)
                                                break;
                                            if (stats.get(y).childNode(p).attributes().get("#text").contains(tokenstats)) {
                                                if (!color.equals("(C)")) {
                                                    if (stats.get(y).childNode(p).attributes().get("#text").contains(color))
                                                        return tokendoc;
                                                } else {
                                                    if (!stats.get(y).childNode(p).attributes().get("#text").contains("(U") &&
                                                            !stats.get(y).childNode(p).attributes().get("#text").contains("(G") &&
                                                            !stats.get(y).childNode(p).attributes().get("#text").contains("(B") &&
                                                            !stats.get(y).childNode(p).attributes().get("#text").contains("(R") &&
                                                            !stats.get(y).childNode(p).attributes().get("#text").contains("(W"))
                                                        return tokendoc;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } catch (Exception e) {
            }
        }
        System.out.println("Warning: Token " + name + " has not been found in https://deckmaster.info so i will search for it between any other set in " + imageurl + " (it may take a long time)");
        for (int i = 0; i < availableSets.length; i++) {
            String currentSet = availableSets[i].toLowerCase().split(" - ")[0];
            if (!currentSet.equalsIgnoreCase(set)) {
                try {
                    doc = Jsoup.connect(imageurl + "t" + currentSet).get();
                    if (doc == null)
                        continue;
                    outlinks = doc.select("body a");
                    if (outlinks != null) {
                        for (int k = 0; k < outlinks.size() && parent.downloadInProgress; k++) {
                            while (parent.paused && parent.downloadInProgress) {
                                try {
                                    Thread.sleep(1000);
                                } catch (InterruptedException e) {
                                }
                            }
                            if (!parent.downloadInProgress)
                                break;
                            String linktoken = outlinks.get(k).attributes().get("href");
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if (tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if (stats != null) {
                                    for (int j = 0; j < stats.size() && parent.downloadInProgress; j++) {
                                        while (parent.paused && parent.downloadInProgress) {
                                            try {
                                                Thread.sleep(1000);
                                            } catch (InterruptedException e) {
                                            }
                                        }
                                        if (!parent.downloadInProgress)
                                            break;
                                        String a = stats.get(j).attributes().get("content");
                                        if (stats.get(j).attributes().get("content").contains(tokenstats) && stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                                            System.out.println("Token " + name + " has been found between " + currentSet.toUpperCase() + " tokens, i will use this one");
                                            return tokendoc;
                                        }
                                    }
                                }
                            } catch (Exception e) {
                            }
                        }
                    }
                } catch (Exception e) {
                }
            }
        }
        System.err.println("Error: Token " + name + " has not been found in any set of " + imageurl);
        throw new Exception();
    }

    public static String DownloadCardImages(String set, String[] availableSets, String targetres, String basePath, String destinationPath, ProgressDialog progressBarDialog, SDLActivity parent, boolean skipDownloaded, boolean borderless) throws IOException {
        try {
            File oldzip = new File(destinationPath + File.separator + set + File.separator + set + ".zip");
            if(oldzip.exists() && skipDownloaded)
                return "";
            else
                oldzip.delete();
        } catch (Exception e) {
        }
        String res = "";

        String baseurl = "https://gatherer.wizards.com/Pages/Card/Details.aspx?multiverseid=";
        String imageurl = "https://scryfall.com/sets/";

        Integer ImgX = 0;
        Integer ImgY = 0;
        Integer ThumbX = 0;
        Integer ThumbY = 0;
        Integer Border = 0;
        Integer BorderThumb = 0;

        if (targetres.equals("High")) {
            ImgX = 672;
            ImgY = 936;
            ThumbX = 124;
            ThumbY = 176;
        } else if (targetres.equals("Medium")) {
            ImgX = 488;
            ImgY = 680;
            ThumbX = 90;
            ThumbY = 128;
        } else if (targetres.equals("Low")) {
            ImgX = 244;
            ImgY = 340;
            ThumbX = 45;
            ThumbY = 64;
        } else if (targetres.equals("Tiny")) {
            ImgX = 180;
            ImgY = 255;
            ThumbX = 45;
            ThumbY = 64;
        }

        File baseFolder = new File(basePath);
        File[] listOfFiles = baseFolder.listFiles();
        Map<String, String> mappa = new HashMap<String, String>();
        ZipFile zipFile = null;
        InputStream stream = null;
        File filePath = null;
        try {
            zipFile = new ZipFile(basePath + File.separator + parent.RES_FILENAME);
            Enumeration<? extends ZipEntry> e = zipFile.entries();
            while (e.hasMoreElements()) {
                ZipEntry entry = e.nextElement();
                String entryName = entry.getName();
                if (entryName != null && entryName.contains("sets" + File.separator)) {
                    if (entryName.contains("_cards.dat")) {
                        String[] names = entryName.split(File.separator);
                        if (set.equalsIgnoreCase(names[1])) {
                            stream = zipFile.getInputStream(entry);
                            byte[] buffer = new byte[1];
                            filePath = new File(basePath + File.separator + "_cards.dat");
                            try {
                                FileOutputStream fos = new FileOutputStream(filePath);
                                BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                                int len;
                                while ((len = stream.read(buffer)) != -1) {
                                    bos.write(buffer, 0, len);
                                }
                                fos.close();
                                bos.close();
                            } catch (Exception ex) {
                                System.out.println("Error extracting zip file" + ex);
                            }
                            break;
                        }
                    }
                }
            }
        } catch (IOException ioe) {
            System.out.println("Error opening zip file" + ioe);
        } finally {
            try {
                if (zipFile != null) {
                    zipFile.close();
                }
            } catch (IOException ioe) {
                System.out.println("Error while closing zip file" + ioe);
            }
        }

        String lines = readLineByLineJava8(filePath.toString());
        File del = new File(filePath.toString());
        del.delete();
        int totalcards = 0;
        String findStr = "total=";
        int lastIndex = lines.indexOf(findStr);
        String totals = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
        totalcards = Integer.parseInt(totals.split("=")[1]);
        
        if(borderless){
            findStr = "year=";
            lastIndex = lines.indexOf(findStr);
            String date = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
            int year = Integer.parseInt(date.split("=")[1].split("-")[0]);
            int month = 1;
            int day = 1;
            if(date.split("=")[1].split("-").length > 1) {
                month = Integer.parseInt(date.split("=")[1].split("-")[1]);
                if(date.split("=")[1].split("-").length > 2) {
                    day = Integer.parseInt(date.split("=")[1].split("-")[2]);
                }
            }
            if(year > 2014 || (year == 2014 && month > 6) || (year == 2014 && month == 6 && day > 15)){
                Border = (int)Math.round(ImgX*0.02);
                BorderThumb = (int)Math.round(ThumbX*0.02);
            } else {
                Border = (int)Math.round(ImgX*0.04);
                BorderThumb = (int)Math.round(ThumbX*0.04);
            }
        } 
       
        while (lines.contains("[card]")) {
            findStr = "[card]";
            lastIndex = lines.indexOf(findStr);
            String id = null;
            String rarity = null;
            String primitive = null;
            int a = lines.indexOf("primitive=", lastIndex);
            if (a > 0) {
                if (lines.substring(a, lines.indexOf("\n", a)).split("=").length > 1)
                    primitive = lines.substring(a, lines.indexOf("\n", a)).split("=")[1];
            }
            int b = lines.indexOf("id=", lastIndex);
            if (b > 0) {
                if (lines.substring(b, lines.indexOf("\n", b)).replace("-", "").split("=").length > 1)
                    id = lines.substring(b, lines.indexOf("\n", b)).replace("-", "").split("=")[1];
            }
            int d = lines.indexOf("rarity=", lastIndex);
            if (d > 0) {
                if (lines.substring(d, lines.indexOf("\n", d)).split("=").length > 1)
                    rarity = lines.substring(d, lines.indexOf("\n", d)).split("=")[1].toLowerCase();
            }
            if (rarity == null || !rarity.equals("t") || set.equals("DKA") || set.equals("EMN") ||
                    set.equals("ISD") || set.equals("ORI") || set.equals("RIX") || set.equals("V17") ||
                    set.equals("UNH") || set.equals("XLN") || set.equals("SOI") || set.equals("SOK") ||
                    set.equals("BOK") || set.equals("CHK") || set.equals("ZNR") || set.equals("KHM") ||
                    set.equals("STX"))
                rarity = "";
            if(id != null && (id.equals("209162") || id.equals("209163") || id.equals("401721") || id.equals("401722") || id.equals("491633")))
                rarity = "t";
            if(id != null && (id.equals("1750411") || id.equals("5176911") || id.equals("44680711") || id.equals("29530711") || id.equals("45108910")
                    || id.equals("530447") || id.equals("530448") || id.equals("530449")))
                rarity = "";
            int c = lines.indexOf("[/card]", lastIndex);
            if (c > 0)
                lines = lines.substring(c + 8);
            if (primitive != null && id != null && !id.equalsIgnoreCase("null"))
                mappa.put(id + rarity, primitive);
            if(id.equals("503837"))
                mappa.put("503837t", "Koma's Coil");
            if(id.equals("503841"))
                mappa.put("503841t", "Shard");
            if(id.equals("513652"))
                mappa.put("513652t", "Pest");
            if(id.equals("513638"))
                mappa.put("513638t", "Pest");
            if(id.equals("513543"))
                mappa.put("513543t", "Pest");
            if(id.equals("513634"))
                mappa.put("513634t", "Fractal");
            if(id.equals("530447"))
                mappa.put("530447t", "Skeleton");
            if(id.equals("530448"))
                mappa.put("530448t", "Goblin");
            if (id.equals("114921")) {
                mappa.put("11492111t", "Citizen");
                mappa.put("11492112t", "Camarid");
                mappa.put("11492113t", "Thrull");
                mappa.put("11492114t", "Goblin");
                mappa.put("11492115t", "Saproling");
            }
        }

        progressBarDialog.setProgress(0);
        progressBarDialog.setMax(totalcards);

        File imgPath = new File(destinationPath + set + File.separator);
        if (!imgPath.exists()) {
            System.out.println("creating directory: " + imgPath.getName());
            boolean result = false;
            try {
                imgPath.mkdir();
                result = true;
            } catch (SecurityException se) {
                System.err.println(imgPath + " not created");
                System.exit(1);
            }
            if (result) {
                System.out.println(imgPath + " created");
            }
        }

        File thumbPath = new File(destinationPath + set + File.separator + "thumbnails" + File.separator);
        if (!thumbPath.exists()) {
            System.out.println("creating directory: " + thumbPath.getName());
            boolean result = false;
            try {
                thumbPath.mkdir();
                result = true;
            } catch (SecurityException se) {
                System.err.println(thumbPath + " not created");
                System.exit(1);
            }
            if (result) {
                System.out.println(thumbPath + " created");
            }
        }

        String scryset = set;
        if (scryset.equalsIgnoreCase("MRQ"))
            scryset = "MMQ";
        else if (scryset.equalsIgnoreCase("AVN"))
            scryset = "DDH";
        else if (scryset.equalsIgnoreCase("BVC"))
            scryset = "DDQ";
        else if (scryset.equalsIgnoreCase("CFX"))
            scryset = "CON";
        else if (scryset.equalsIgnoreCase("DM"))
            scryset = "DKM";
        else if (scryset.equalsIgnoreCase("EVK"))
            scryset = "DDO";
        else if (scryset.equalsIgnoreCase("EVT"))
            scryset = "DDF";
        else if (scryset.equalsIgnoreCase("FVD"))
            scryset = "DRB";
        else if (scryset.equalsIgnoreCase("FVE"))
            scryset = "V09";
        else if (scryset.equalsIgnoreCase("FVL"))
            scryset = "V11";
        else if (scryset.equalsIgnoreCase("FVR"))
            scryset = "V10";
        else if (scryset.equalsIgnoreCase("HVM"))
            scryset = "DDL";
        else if (scryset.equalsIgnoreCase("IVG"))
            scryset = "DDJ";
        else if (scryset.equalsIgnoreCase("JVV"))
            scryset = "DDM";
        else if (scryset.equalsIgnoreCase("KVD"))
            scryset = "DDG";
        else if (scryset.equalsIgnoreCase("PDS"))
            scryset = "H09";
        else if (scryset.equalsIgnoreCase("PVC"))
            scryset = "DDE";
        else if (scryset.equalsIgnoreCase("RV"))
            scryset = "3ED";
        else if (scryset.equalsIgnoreCase("SVT"))
            scryset = "DDK";
        else if (scryset.equalsIgnoreCase("VVK"))
            scryset = "DDI";
        else if (scryset.equalsIgnoreCase("ZVE"))
            scryset = "DDP";

        for (int y = 0; y < mappa.size() && parent.downloadInProgress; y++) {
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;

            String id = mappa.keySet().toArray()[y].toString();
            progressBarDialog.incrementProgressBy((int) (1));
            if (fastDownloadCard(set, id, mappa.get(id), imgPath.getAbsolutePath(), thumbPath.getAbsolutePath(), ImgX, ImgY, ThumbX, ThumbY, Border, BorderThumb))
                continue;
            String specialcardurl = getSpecialCardUrl(id);
            if (!specialcardurl.isEmpty()) {
                URL url = new URL(specialcardurl);
                HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                if (httpcon == null) {
                    System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + ", i will not download it...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    break;
                }
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                httpcon.setConnectTimeout(5000);
                httpcon.setReadTimeout(5000);
                httpcon.setAllowUserInteraction(false);
                httpcon.setDoInput(true);
                httpcon.setDoOutput(false);
                InputStream in = null;
                try {
                    in = new BufferedInputStream(httpcon.getInputStream());
                } catch (Exception ex) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                    try {
                        in = new BufferedInputStream(url.openStream());
                    } catch (Exception ex2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                        try {
                            in = new BufferedInputStream(url.openStream());
                        } catch (Exception ex3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            break;
                        }
                    }
                }
                ByteArrayOutputStream out = new ByteArrayOutputStream();
                byte[] buf = new byte[1024];
                int n = 0;
                long millis = System.currentTimeMillis();
                boolean timeout = false;
                while (-1 != (n = in.read(buf)) && !timeout) {
                    out.write(buf, 0, n);
                    if (System.currentTimeMillis() - millis > 10000)
                        timeout = true;
                }
                if (timeout) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                    buf = new byte[1024];
                    n = 0;
                    millis = System.currentTimeMillis();
                    timeout = false;
                    while (-1 != (n = in.read(buf)) && !timeout) {
                        out.write(buf, 0, n);
                        if (System.currentTimeMillis() - millis > 10000)
                            timeout = true;
                    }
                    if (timeout) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                        buf = new byte[1024];
                        n = 0;
                        millis = System.currentTimeMillis();
                        timeout = false;
                        while (-1 != (n = in.read(buf)) && !timeout) {
                            out.write(buf, 0, n);
                            if (System.currentTimeMillis() - millis > 10000)
                                timeout = true;
                        }
                    }
                }
                out.close();
                in.close();
                if (timeout) {
                    System.err.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg) from, i will not retry anymore...");
                    break;
                }
                byte[] response = out.toByteArray();
                String cardimage = imgPath + File.separator + id + ".jpg";
                String thumbcardimage = thumbPath + File.separator + id + ".jpg";
                FileOutputStream fos = new FileOutputStream(cardimage);
                fos.write(response);
                fos.close();
                try {
                    Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                    Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                    if(Border > 0)
                        resized = Bitmap.createBitmap(resized, Border, Border, ImgX-2*Border, ImgY-2*Border);
                    FileOutputStream fout = new FileOutputStream(cardimage);
                    resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    fout.close();
                } catch (Exception e) {
                    System.err.println("Error: Problem resizing card: " + mappa.get(id) + " (" + id + ".jpg), image may be corrupted...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    break;
                }
                try {
                    Bitmap yourBitmapthumb = BitmapFactory.decodeFile(cardimage);
                    Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmapthumb, ThumbX, ThumbY, true);
                    if(BorderThumb > 0)
                        resizedThumb = Bitmap.createBitmap(resizedThumb, BorderThumb, BorderThumb, ThumbX-2*BorderThumb, ThumbY-2*BorderThumb);
                    FileOutputStream fout = new FileOutputStream(thumbcardimage);
                    resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    fout.close();
                } catch (Exception e) {
                    System.err.println("Error: Problem resizing card thumbnail: " + mappa.get(id) + " (" + id + ".jpg, image may be corrupted...");
                    res = mappa.get(id) + " - " + set + File.separator + "thumbnails" + File.separator + id + ".jpg\n" + res;
                    break;
                }
                continue;
            }
            if (id.endsWith("t"))
                continue;
            Document doc = null;
            String cardname = mappa.get(id);
            Elements divs = new Elements();
            int k;
            if(scryset.equals("TD2") || scryset.equals("PRM") || scryset.equals("TD0") || scryset.equals("PZ1") || scryset.equals("PZ2")
                    || scryset.equals("PHPR") || scryset.equals("PGRU") || scryset.equals("PIDW") || scryset.equals("ANA") || scryset.equals("HTR")
                    || scryset.equals("HTR17") || scryset.equals("PI13") || scryset.equals("PI14") || scryset.equals("PSAL") || scryset.equals("PS11")
                    || scryset.equals("PDTP") || scryset.equals("PDP10") || scryset.equals("PDP11") || scryset.equals("PDP12") || scryset.equals("PDP13")
                    || scryset.equals("PDP14") || scryset.equals("DPA") || scryset.equals("PMPS") || scryset.equals("PMPS06") || scryset.equals("PMPS07")
                    || scryset.equals("PMPS08") || scryset.equals("PMPS09") || scryset.equals("PMPS10") || scryset.equals("PMPS11") || scryset.equals("GN2")
                    || scryset.equals("PAL00") || scryset.equals("PAL01") || scryset.equals("PAL02") || scryset.equals("PAL03") || scryset.equals("PAL04")
                    || scryset.equals("PAL05") || scryset.equals("PAL06") || scryset.equals("PAL99") || scryset.equals("PARL") || scryset.equals("HA1")
                    || scryset.equals("SLD") || scryset.equals("MB1") || scryset.equals("HA2") || scryset.equals("HA3") || scryset.equals("SS3")
                    || scryset.equals("AKR") || scryset.equals("ANB") || scryset.equals("PLIST") || scryset.equals("KLR") || scryset.equals("CC1")
                    || scryset.equals("ATH") || scryset.equals("HA4") || scryset.equals("TSR") || scryset.equals("HA5") || scryset.equals("H1R")
                    || scryset.equals("HTR18") || scryset.equals("HTR19")){
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    Elements outlinks = doc.select("body a");
                    if (outlinks != null) {
                        for (int h = 0; h < outlinks.size(); h++) {
                            String linkcard = outlinks.get(h).attributes().get("href");
                            if(linkcard == null)
                                continue;
                            String strtork[] = linkcard.toLowerCase().split("/");
                            if(strtork.length <= 0)
                                continue;
                            String nametocmp = strtork[strtork.length - 1];
                            if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                try {
                                    doc = Jsoup.connect(linkcard).get();
                                    if (doc == null)
                                        continue;
                                    Elements metadata = doc.select("head meta");
                                    if (metadata != null) {
                                        for (int j = 0; j < metadata.size(); j++) {
                                            if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                h = outlinks.size();
                                                break;
                                            }
                                        }
                                    }
                                } catch (Exception ex) {
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        Elements outlinks = doc.select("body a");
                        if (outlinks != null) {
                            for (int h = 0; h < outlinks.size(); h++) {
                                String linkcard = outlinks.get(h).attributes().get("href");
                                if(linkcard == null)
                                    continue;
                                String strtork[] = linkcard.toLowerCase().split("/");
                                if(strtork.length <= 0)
                                    continue;
                                String nametocmp = strtork[strtork.length - 1];
                                if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                    try {
                                        doc = Jsoup.connect(linkcard).get();
                                        if (doc == null)
                                            continue;
                                        Elements metadata = doc.select("head meta");
                                        if (metadata != null) {
                                            for (int j = 0; j < metadata.size(); j++) {
                                                if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                    h = outlinks.size();
                                                    break;
                                                }
                                            }
                                        }
                                    } catch (Exception ex) {
                                    }
                                }
                            }
                        }
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            Elements outlinks = doc.select("body a");
                            if (outlinks != null) {
                                for (int h = 0; h < outlinks.size(); h++) {
                                    String linkcard = outlinks.get(h).attributes().get("href");
                                    if(linkcard == null)
                                        continue;
                                    String strtork[] = linkcard.toLowerCase().split("/");
                                    if(strtork.length <= 0)
                                        continue;
                                    String nametocmp = strtork[strtork.length - 1];
                                    if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                        try {
                                            doc = Jsoup.connect(linkcard).get();
                                            if (doc == null)
                                                continue;
                                            Elements metadata = doc.select("head meta");
                                            if (metadata != null) {
                                                for (int j = 0; j < metadata.size(); j++) {
                                                    if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                        h = outlinks.size();
                                                        break;
                                                    }
                                                }
                                            }
                                        } catch (Exception ex) {
                                        }
                                    }
                                }
                            }
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
            } else {
                try {
                    doc = Jsoup.connect(baseurl + id).get();
                } catch (Exception e) {
                    System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(baseurl + id).get();
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(baseurl + id).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
                if (doc == null) {
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    continue;
                }
                divs = doc.select("body div");
                if (divs == null) {
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    continue;
                }

                for (k = 0; k < divs.size(); k++)
                    if (divs.get(k).childNodes().size() > 0 && divs.get(k).childNode(0).toString().toLowerCase().contains("card name"))
                        break;
                if (k >= divs.size()) {
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl + id + ", i can't download it...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    continue;
                }
                cardname = divs.get(k + 1).childNode(0).attributes().get("#text").replace("\r\n", "").trim();
            }
            while (parent.paused && parent.downloadInProgress) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                }
            }
            if (!parent.downloadInProgress)
                break;

            if (scryset.equals("UST") || scryset.equals("S00") || scryset.equals("V17") || scryset.equals("XLN") || scryset.equals("SOI")
                    || scryset.startsWith("GK1_") || scryset.startsWith("GK2_")){
                cardname = cardname.replace(" (a)", "");
                cardname = cardname.replace(" (b)", "");
                cardname = cardname.replace(" (c)", "");
                cardname = cardname.replace(" (d)", "");
                cardname = cardname.replace(" (e)", "");
                cardname = cardname.replace(" (f)", "");
                cardname = cardname.replace(" ...", "");
                String deckutrl = "https://deckmaster.info/card.php?multiverseid=";
                try {
                    doc = Jsoup.connect(deckutrl + id).get();
                } catch (Exception e) {
                    System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(deckutrl + id).get();
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(deckutrl + id).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + deckutrl + id + ", i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
            } else if(targetres.equals("High") && !scryset.equals("TD2") && !scryset.equals("PRM") && !scryset.equals("TD0") && !scryset.equals("PZ2")
                    && !scryset.equals("PHPR") && !scryset.equals("PGRU") && !scryset.equals("PGRU") && !scryset.equals("ANA") && !scryset.equals("HTR")
                    && !scryset.equals("HTR17") && !scryset.equals("PI13") && !scryset.equals("PI14") && !scryset.equals("PSAL") && !scryset.equals("PS11")
                    && !scryset.equals("PDTP") && !scryset.equals("PDP10") && !scryset.equals("PDP11") && !scryset.equals("PDP12") && !scryset.equals("PDP13")
                    && !scryset.equals("PDP14") && !scryset.equals("DPA") && !scryset.equals("PMPS") && !scryset.equals("PMPS06") && !scryset.equals("PMPS07")
                    && !scryset.equals("PMPS08") && !scryset.equals("PMPS09") && !scryset.equals("PMPS10") && !scryset.equals("PMPS11") && !scryset.equals("GN2")
                    && !scryset.equals("PAL00") && !scryset.equals("PAL01") && !scryset.equals("PAL02") && !scryset.equals("PAL03") && !scryset.equals("PAL04")
                    && !scryset.equals("PAL05") && !scryset.equals("PAL06") && !scryset.equals("PAL99") && !scryset.equals("PARL") && !scryset.equals("HA1")
                    && !scryset.equals("SLD") && !scryset.equals("MB1") && !scryset.equals("HA2") && !scryset.equals("HA3") && !scryset.equals("SS3")
                    && !scryset.equals("AKR") && !scryset.equals("ANB") && !scryset.equals("PLIST") && !scryset.equals("KLR") && !scryset.equals("CC1")
                    && !scryset.equals("ATH") && !scryset.equals("HA4") && !scryset.equals("TSR") && !scryset.equals("HA5") && !scryset.equals("H1R")
                    && !scryset.equals("HTR18") && !scryset.equals("HTR19")){
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    Elements outlinks = doc.select("body a");
                    if (outlinks != null) {
                        for (int h = 0; h < outlinks.size(); h++) {
                            String linkcard = outlinks.get(h).attributes().get("href");
                            if(linkcard == null)
                                continue;
                            String strtork[] = linkcard.toLowerCase().split("/");
                            if(strtork.length <= 0)
                                continue;
                            String nametocmp = strtork[strtork.length - 1];
                            if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                try {
                                    doc = Jsoup.connect(linkcard).get();
                                    if (doc == null)
                                        continue;
                                    Elements metadata = doc.select("head meta");
                                    if (metadata != null) {
                                        for (int j = 0; j < metadata.size(); j++) {
                                            if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                h = outlinks.size();
                                                break;
                                            }
                                        }
                                    }
                                } catch (Exception ex) {
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        Elements outlinks = doc.select("body a");
                        if (outlinks != null) {
                            for (int h = 0; h < outlinks.size(); h++) {
                                String linkcard = outlinks.get(h).attributes().get("href");
                                if(linkcard == null)
                                    continue;
                                String strtork[] = linkcard.toLowerCase().split("/");
                                if(strtork.length <= 0)
                                    continue;
                                String nametocmp = strtork[strtork.length - 1];
                                if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                    try {
                                        doc = Jsoup.connect(linkcard).get();
                                        if (doc == null)
                                            continue;
                                        Elements metadata = doc.select("head meta");
                                        if (metadata != null) {
                                            for (int j = 0; j < metadata.size(); j++) {
                                                if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                    h = outlinks.size();
                                                    break;
                                                }
                                            }
                                        }
                                    } catch (Exception ex) {
                                    }
                                }
                            }
                        }
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            Elements outlinks = doc.select("body a");
                            if (outlinks != null) {
                                for (int h = 0; h < outlinks.size(); h++) {
                                    String linkcard = outlinks.get(h).attributes().get("href");
                                    if(linkcard == null)
                                        continue;
                                    String strtork[] = linkcard.toLowerCase().split("/");
                                    if(strtork.length <= 0)
                                        continue;
                                    String nametocmp = strtork[strtork.length - 1];
                                    if(nametocmp.equals(cardname.toLowerCase().replace(" ", "-"))){
                                        try {
                                            doc = Jsoup.connect(linkcard).get();
                                            if (doc == null)
                                                continue;
                                            Elements metadata = doc.select("head meta");
                                            if (metadata != null) {
                                                for (int j = 0; j < metadata.size(); j++) {
                                                    if (metadata.get(j).attributes().get("content").toLowerCase().equals(cardname.toLowerCase())) {
                                                        h = outlinks.size();
                                                        break;
                                                    }
                                                }
                                            }
                                        } catch (Exception ex) {
                                        }
                                    }
                                }
                            }
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
            } else if(!scryset.equals("TD2") && !scryset.equals("PRM") && !scryset.equals("TD0") && !scryset.equals("PZ1") && !scryset.equals("PZ2")
                    && !scryset.equals("PHPR") && !scryset.equals("PGRU") && !scryset.equals("PGRU") && !scryset.equals("ANA") && !scryset.equals("HTR")
                    && !scryset.equals("HTR17") && !scryset.equals("PI13") && !scryset.equals("PI14") && !scryset.equals("PSAL") && !scryset.equals("PS11")
                    && !scryset.equals("PDTP") && !scryset.equals("PDP10") && !scryset.equals("PDP11") && !scryset.equals("PDP12") && !scryset.equals("PDP13")
                    && !scryset.equals("PDP14") && !scryset.equals("DPA") && !scryset.equals("PMPS") && !scryset.equals("PMPS06") && !scryset.equals("PMPS07")
                    && !scryset.equals("PMPS08") && !scryset.equals("PMPS09") && !scryset.equals("PMPS10") && !scryset.equals("PMPS11") && !scryset.equals("GN2")
                    && !scryset.equals("PAL00") && !scryset.equals("PAL01") && !scryset.equals("PAL02") && !scryset.equals("PAL03") && !scryset.equals("PAL04")
                    && !scryset.equals("PAL05") && !scryset.equals("PAL06") && !scryset.equals("PAL99") && !scryset.equals("PARL") && !scryset.equals("HA1")
                    && !scryset.equals("SLD") && !scryset.equals("MB1") && !scryset.equals("HA2") && !scryset.equals("HA3") && !scryset.equals("SS3")
                    && !scryset.equals("AKR") && !scryset.equals("ANB") && !scryset.equals("PLIST") && !scryset.equals("KLR") && !scryset.equals("CC1")
                    && !scryset.equals("ATH") && !scryset.equals("HA4") && !scryset.equals("TSR") && !scryset.equals("HA5") && !scryset.equals("H1R")
                    && !scryset.equals("HTR18") && !scryset.equals("HTR19")){
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                } catch (Exception e) {
                    System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    } catch (Exception e2) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will not retry anymore...");
                            res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
            }

            if (doc == null) {
                System.err.println("Error: Problem fetching card: " + mappa.get(id) + " (" + id + ".jpg), i will not download it...");
                res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                continue;
            }

            Elements imgs = doc.select("body img");
            if (imgs == null) {
                System.err.println("Error: Problem fetching card: " + mappa.get(id) + " (" + id + ".jpg), i will not download it...");
                res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                continue;
            }

            for (int i = 0; i < imgs.size() && parent.downloadInProgress; i++) {
                while (parent.paused && parent.downloadInProgress) {
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {
                    }
                }
                if (!parent.downloadInProgress)
                    break;

                String title = imgs.get(i).attributes().get("alt");
                if(title.isEmpty())
                    title = imgs.get(i).attributes().get("title");
                else
                    title = title.split("from")[0];
                if (title.replace("(" + scryset + ")","").replace("(NEM)","").trim().toLowerCase().equals(cardname.toLowerCase())) {
                    String CardImage = imgs.get(i).attributes().get("src");
                    if (CardImage.isEmpty())
                        CardImage = imgs.get(i).attributes().get("data-src");
                    CardImage = CardImage.replace("/normal/", "/large/");
                    URL url = new URL(CardImage);
                    HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                    if (httpcon == null) {
                        System.err.println("Error: Problem fetching card: " + mappa.get(id) + " (" + id + ".jpg), i will not download it...");
                        res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                        break;
                    }
                    httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                    httpcon.setConnectTimeout(5000);
                    httpcon.setReadTimeout(5000);
                    httpcon.setAllowUserInteraction(false);
                    httpcon.setDoInput(true);
                    httpcon.setDoOutput(false);
                    InputStream in = null;
                    try {
                        in = new BufferedInputStream(httpcon.getInputStream());
                    } catch (IOException ex) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                        try {
                            in = new BufferedInputStream(httpcon.getInputStream());
                        } catch (IOException ex2) {
                            System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                            try {
                                in = new BufferedInputStream(httpcon.getInputStream());
                            } catch (IOException ex3) {
                                System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will not retry anymore...");
                                res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                                break;
                            }
                        }
                    }
                    ByteArrayOutputStream out = new ByteArrayOutputStream();
                    byte[] buf = new byte[1024];
                    int n = 0;
                    long millis = System.currentTimeMillis();
                    boolean timeout = false;
                    while (-1 != (n = in.read(buf)) && !timeout) {
                        out.write(buf, 0, n);
                        if (System.currentTimeMillis() - millis > 10000)
                            timeout = true;
                    }
                    if (timeout) {
                        System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 2 times more...");
                        buf = new byte[1024];
                        n = 0;
                        millis = System.currentTimeMillis();
                        timeout = false;
                        while (-1 != (n = in.read(buf)) && !timeout) {
                            out.write(buf, 0, n);
                            if (System.currentTimeMillis() - millis > 10000)
                                timeout = true;
                        }
                        if (timeout) {
                            System.out.println("Warning: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg), i will retry 1 time more...");
                            buf = new byte[1024];
                            n = 0;
                            millis = System.currentTimeMillis();
                            timeout = false;
                            while (-1 != (n = in.read(buf)) && !timeout) {
                                out.write(buf, 0, n);
                                if (System.currentTimeMillis() - millis > 10000)
                                    timeout = true;
                            }
                        }
                    }
                    out.close();
                    in.close();
                    if (timeout) {
                        System.err.println("Error: Problem downloading card: " + mappa.get(id) + " (" + id + ".jpg) from, i will not retry anymore...");
                        break;
                    }
                    byte[] response = out.toByteArray();
                    String cardimage = imgPath + File.separator + id + ".jpg";
                    String thumbcardimage = thumbPath + File.separator + id + ".jpg";
                    FileOutputStream fos = new FileOutputStream(cardimage);
                    fos.write(response);
                    fos.close();
                    try {
                        Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                        Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                        if(Border > 0)
                            resized = Bitmap.createBitmap(resized, Border, Border, ImgX-2*Border, ImgY-2*Border);
                        FileOutputStream fout = new FileOutputStream(cardimage);
                        resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                        fout.close();
                    } catch (Exception e) {
                        System.err.println("Error: Problem resizing card: " + mappa.get(id) + " (" + id + ".jpg), image may be corrupted...");
                        res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                        break;
                    }
                    try {
                        Bitmap yourBitmapthumb = BitmapFactory.decodeFile(cardimage);
                        Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmapthumb, ThumbX, ThumbY, true);
                        if(BorderThumb > 0)
                            resizedThumb = Bitmap.createBitmap(resizedThumb, BorderThumb, BorderThumb, ThumbX-2*BorderThumb, ThumbY-2*BorderThumb);
                        FileOutputStream fout = new FileOutputStream(thumbcardimage);
                        resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                        fout.close();
                    } catch (Exception e) {
                        System.err.println("Error: Problem resizing card thumbnail: " + mappa.get(id) + " (" + id + ".jpg), image may be corrupted...");
                        res = mappa.get(id) + " - " + set + File.separator + "thumbnails" + File.separator + id + ".jpg\n" + res;
                        break;
                    }
                    String text = "";
                    if(scryset.equals("PRM") || scryset.equals("TD0") || scryset.equals("PZ1") || scryset.equals("PZ2") || scryset.equals("PHPR")
                            || scryset.equals("PGRU") || scryset.equals("PGRU") || scryset.equals("ANA") || scryset.equals("HTR") || scryset.equals("HTR17")
                            || scryset.equals("PI13") || scryset.equals("PI14") || scryset.equals("PSAL") || scryset.equals("PS11") || scryset.equals("PDTP")
                            || scryset.equals("PDP10") || scryset.equals("PDP11") || scryset.equals("PDP12") || scryset.equals("PDP13") || scryset.equals("PDP14")
                            || scryset.equals("DPA") || scryset.equals("PMPS") || scryset.equals("PMPS06") || scryset.equals("PMPS07") || scryset.equals("PMPS08")
                            || scryset.equals("PMPS09") || scryset.equals("PMPS10") || scryset.equals("PMPS11") || scryset.equals("GN2") || scryset.equals("PAL00")
                            || scryset.equals("PAL01") || scryset.equals("PAL02") || scryset.equals("PAL03") || scryset.equals("PAL04") || scryset.equals("PAL05")
                            || scryset.equals("PAL06") || scryset.equals("PAL99") || scryset.equals("PARL") || scryset.equals("HA1") || scryset.equals("SLD")
                            || scryset.equals("MB1") || scryset.equals("HA2") || scryset.equals("HA3") || scryset.equals("SS3") || scryset.equals("AKR")
                            || scryset.equals("ANB") || scryset.equals("PLIST") || scryset.equals("KLR") || scryset.equals("CC1") || scryset.equals("ATH")
                            || scryset.equals("HA4") || scryset.equals("TSR") || scryset.equals("HA5") || scryset.equals("H1R") || scryset.equals("HTR18")
                            || scryset.equals("HTR19")){
                        Elements metadata = doc.select("head meta");
                        if(metadata != null) {
                            for (int j = 0; j < metadata.size(); j++){
                                if(metadata.get(j).attributes().get("property").equals("og:description")){
                                    if(metadata.get(j).attributes().get("content").split("•").length > 3){
                                        text = metadata.get(j).attributes().get("content").split("•")[3].trim();
                                        if (text.contains("(" + scryset + ")"))
                                            text = metadata.get(j).attributes().get("content").split("•")[2].trim();
                                        if (text.contains("Illustrated by"))
                                            text = metadata.get(j).attributes().get("content").split("•")[1].trim();
                                        text = text.replace("&#39;", "'");
                                        break;
                                    } else
                                        break;
                                }
                            }
                        }
                    } else {
                        for (k = 0; k < divs.size(); k++)
                            if (divs.get(k).childNodes().size() > 0 && divs.get(k).childNode(0).toString().toLowerCase().contains("card text"))
                                break;
                        if (k < divs.size()) {
                            Element tex = divs.get(k + 1);
                            for (int z = 0; z < divs.get(k + 1).childNodes().size(); z++) {
                                for (int u = 0; u < divs.get(k + 1).childNode(z).childNodes().size(); u++) {
                                    if (divs.get(k + 1).childNode(z).childNode(u).childNodes().size() > 1) {
                                        for (int w = 0; w < divs.get(k + 1).childNode(z).childNode(u).childNodes().size(); w++) {
                                            if (divs.get(k + 1).childNode(z).childNode(u).childNode(w).hasAttr("alt")) {
                                                String newtext = divs.get(k + 1).childNode(z).childNode(u).childNode(w).attributes().get("alt").trim();
                                                newtext = newtext.replace("Green", "{G}");
                                                newtext = newtext.replace("White", "{W}");
                                                newtext = newtext.replace("Black", "{B}");
                                                newtext = newtext.replace("Blue", "{U}");
                                                newtext = newtext.replace("Red", "{R}");
                                                newtext = newtext.replace("Tap", "{T}");
                                                text = text + newtext;
                                            } else
                                                text = text + " " + divs.get(k + 1).childNode(z).childNode(u).childNode(w).toString().replace("\r\n", "").trim() + " ";
                                            text = text.replace("} .", "}.");
                                            text = text.replace("} :", "}:");
                                            text = text.replace("} ,", "},");
                                        }
                                    } else {
                                        if (divs.get(k + 1).childNode(z).childNode(u).hasAttr("alt")) {
                                            String newtext = divs.get(k + 1).childNode(z).childNode(u).attributes().get("alt").trim();
                                            newtext = newtext.replace("Green", "{G}");
                                            newtext = newtext.replace("White", "{W}");
                                            newtext = newtext.replace("Black", "{B}");
                                            newtext = newtext.replace("Blue", "{U}");
                                            newtext = newtext.replace("Red", "{R}");
                                            newtext = newtext.replace("Tap", "{T}");
                                            text = text + newtext;
                                        } else
                                            text = text + " " + divs.get(k + 1).childNode(z).childNode(u).toString().replace("\r\n", "").trim() + " ";
                                        text = text.replace("} .", "}.");
                                        text = text.replace("} :", "}:");
                                        text = text.replace("} ,", "},");
                                    }
                                    if (z > 0 && z < divs.get(k + 1).childNodes().size() - 1)
                                        text = text + " -- ";
                                    text = text.replace("<i>", "");
                                    text = text.replace("</i>", "");
                                    text = text.replace("<b>", "");
                                    text = text.replace("</b>", "");
                                    text = text.replace(" -- (", " (");
                                    text = text.replace("  ", " ");
                                }
                            }
                        }
                    }
                    if (hasToken(id) && ((text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) || (text.trim().toLowerCase().contains("put") && text.trim().toLowerCase().contains("token")))) {
                        System.out.println("The card: " + mappa.get(id) + " (" + id + ".jpg) can create a token, i will try to download that image too as " + id + "t.jpg");
                        boolean tokenfound = false;
                        String arrays[] = text.trim().split(" ");
                        String nametoken = "";
                        String nametocheck = "";
                        String tokenstats = "";
                        String color = "";
                        String color1 = "";
                        String color2 = "";
                        for (int l = 1; l < arrays.length - 1; l++) {
                            if (arrays[l].equalsIgnoreCase("creature") && arrays[l + 1].toLowerCase().contains("token")) {
                                nametoken = arrays[l - 1];
                                if (l - 3 > 0) {
                                    tokenstats = arrays[l - 3];
                                    color1 = arrays[l - 2];
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 4 > 0) {
                                        tokenstats = arrays[l - 4];
                                        color1 = arrays[l - 3];
                                    }
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 5 > 0) {
                                        tokenstats = arrays[l - 5];
                                        color1 = arrays[l - 4];
                                        color2 = arrays[l - 2];
                                    }
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 6 > 0) {
                                        tokenstats = arrays[l - 6];
                                        color1 = arrays[l - 5];
                                        color2 = arrays[l - 3];
                                    }
                                }
                                if (!tokenstats.contains("/")) {
                                    if (l - 7 > 0) {
                                        tokenstats = arrays[l - 7];
                                        color1 = arrays[l - 6];
                                        color2 = arrays[l - 4];
                                    }
                                }
                                if (nametoken.equalsIgnoreCase("artifact")) {
                                    if (l - 2 > 0)
                                        nametoken = arrays[l - 2];
                                    if (l - 4 > 0) {
                                        tokenstats = arrays[l - 4];
                                        color1 = arrays[l - 3];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 5 > 0) {
                                            tokenstats = arrays[l - 5];
                                            color1 = arrays[l - 4];
                                        }
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 6 > 0) {
                                            tokenstats = arrays[l - 6];
                                            color1 = arrays[l - 5];
                                            color2 = arrays[l - 3];
                                        }
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 7 > 0) {
                                            tokenstats = arrays[l - 7];
                                            color1 = arrays[l - 6];
                                            color2 = arrays[l - 4];
                                        }
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 8 > 0) {
                                            tokenstats = arrays[l - 8];
                                            color1 = arrays[l - 7];
                                            color2 = arrays[l - 5];
                                        }
                                    }
                                }
                                if (!tokenstats.contains("/"))
                                    tokenstats = "";

                                if (color1.toLowerCase().contains("white"))
                                    color1 = "W";
                                else if (color1.toLowerCase().contains("blue"))
                                    color1 = "U";
                                else if (color1.toLowerCase().contains("black"))
                                    color1 = "B";
                                else if (color1.toLowerCase().contains("red"))
                                    color1 = "R";
                                else if (color1.toLowerCase().contains("green"))
                                    color1 = "G";
                                else if (color1.toLowerCase().contains("colorless"))
                                    color1 = "C";
                                else
                                    color1 = "";

                                if (color2.toLowerCase().contains("white"))
                                    color2 = "W";
                                else if (color1.toLowerCase().contains("blue"))
                                    color2 = "U";
                                else if (color1.toLowerCase().contains("black"))
                                    color2 = "B";
                                else if (color1.toLowerCase().contains("red"))
                                    color2 = "R";
                                else if (color1.toLowerCase().contains("green"))
                                    color2 = "G";
                                else
                                    color2 = "";

                                if (!color1.isEmpty()) {
                                    color = "(" + color1 + color2 + ")";
                                }
                                break;
                            } else if (arrays[l].equalsIgnoreCase("put") && arrays[l + 3].toLowerCase().contains("token")) {
                                nametoken = arrays[l + 2];
                                for (int j = 1; j < arrays.length - 1; j++) {
                                    if (arrays[j].contains("/")) {
                                        tokenstats = arrays[j];
                                        color = arrays[j + 1];
                                    }
                                }
                                if (color.toLowerCase().contains("white"))
                                    color = "(W)";
                                else if (color.toLowerCase().contains("blue"))
                                    color = "(U)";
                                else if (color.toLowerCase().contains("black"))
                                    color = "(B)";
                                else if (color.toLowerCase().contains("red"))
                                    color = "(R)";
                                else if (color.toLowerCase().contains("green"))
                                    color = "(G)";
                                else if (color.toLowerCase().contains("colorless"))
                                    color = "(C)";
                                else
                                    color = "";
                                break;
                            }
                        }
                        String specialtokenurl = getSpecialTokenUrl(id + "t");
                        Elements imgstoken;
                        if (!specialtokenurl.isEmpty()) {
                            try {
                                doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            } catch (Exception ex) {
                                System.err.println("Error: Problem occurring while searching for token: " + nametoken + " (" + id + "t.jpg), i will not download it...");
                                res = nametoken + " - " + set + File.separator + id + "t.jpg\n" + res;
                                break;
                            }
                            if (doc == null)
                                break;
                            imgstoken = doc.select("body img");
                            if (imgstoken == null)
                                break;
                            tokenfound = true;
                        } else {
                            if (nametoken.isEmpty() || tokenstats.isEmpty()) {
                                tokenfound = false;
                                if (nametoken.isEmpty())
                                    nametoken = "Unknown";
                                nametocheck = mappa.get(id);
                                doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                            } else {
                                try {
                                    doc = findTokenPage(imageurl, nametoken, scryset, availableSets, tokenstats, color, parent);
                                    tokenfound = true;
                                    nametocheck = nametoken;
                                } catch (Exception e) {
                                    tokenfound = false;
                                    nametocheck = mappa.get(id);
                                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                                }
                            }
                            if (doc == null)
                                break;
                            imgstoken = doc.select("body img");
                            if (imgstoken == null)
                                break;
                        }
                        for (int p = 0; p < imgstoken.size() && parent.downloadInProgress; p++) {
                            while (parent.paused && parent.downloadInProgress) {
                                try {
                                    Thread.sleep(1000);
                                } catch (InterruptedException e) {
                                }
                            }
                            if (!parent.downloadInProgress)
                                break;

                            String titletoken = imgstoken.get(p).attributes().get("alt");
                            if (titletoken.isEmpty())
                                titletoken = imgstoken.get(p).attributes().get("title");
                            if (titletoken.toLowerCase().contains(nametocheck.toLowerCase())) {
                                String CardImageToken = imgstoken.get(p).attributes().get("src");
                                if (CardImageToken.isEmpty())
                                    CardImageToken = imgstoken.get(p).attributes().get("data-src");
                                CardImageToken = CardImageToken.replace("/normal/", "/large/");
                                URL urltoken = new URL(CardImageToken);
                                if (!specialtokenurl.isEmpty())
                                    urltoken = new URL(specialtokenurl);
                                HttpURLConnection httpcontoken = (HttpURLConnection) urltoken.openConnection();
                                if (httpcontoken == null) {
                                    System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t.jpg), i will not download it...");
                                    res = nametoken + " - " + set + File.separator + id + "t.jpg\n" + res;
                                    break;
                                }
                                httpcontoken.addRequestProperty("User-Agent", "Mozilla/4.76");
                                httpcontoken.setConnectTimeout(5000);
                                httpcontoken.setReadTimeout(5000);
                                httpcontoken.setAllowUserInteraction(false);
                                httpcontoken.setDoInput(true);
                                httpcontoken.setDoOutput(false);
                                InputStream intoken = null;
                                try {
                                    intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                } catch (IOException ex) {
                                    System.out.println("Warning: Problem downloading token: " + nametoken + " (" + id + "t.jpg), i will retry 2 times more...");
                                    try {
                                        intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                    } catch (IOException ex2) {
                                        System.out.println("Warning: Problem downloading token: " + nametoken + " (" + id + "t.jpg), i will retry 1 time more...");
                                        try {
                                            intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                        } catch (IOException ex3) {
                                            System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t.jpg), i will not retry anymore...");
                                            res = nametoken + " - " + set + File.separator + id + "t.jpg\n" + res;
                                            break;
                                        }
                                    }
                                }
                                ByteArrayOutputStream outtoken = new ByteArrayOutputStream();
                                byte[] buftoken = new byte[1024];
                                int ntoken = 0;
                                millis = System.currentTimeMillis();
                                timeout = false;
                                while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                                    outtoken.write(buftoken, 0, ntoken);
                                    if (System.currentTimeMillis() - millis > 10000)
                                        timeout = true;
                                }
                                if (timeout) {
                                    System.out.println("Warning: Problem downloading token: " + id + "t.jpg from, i will retry 2 times more...");
                                    buftoken = new byte[1024];
                                    ntoken = 0;
                                    millis = System.currentTimeMillis();
                                    timeout = false;
                                    while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                                        outtoken.write(buftoken, 0, ntoken);
                                        if (System.currentTimeMillis() - millis > 10000)
                                            timeout = true;
                                    }
                                    if (timeout) {
                                        System.out.println("Warning: Problem downloading token: " + id + "t.jpg from, i will retry 1 time more...");
                                        buftoken = new byte[1024];
                                        ntoken = 0;
                                        millis = System.currentTimeMillis();
                                        timeout = false;
                                        while (-1 != (ntoken = intoken.read(buftoken)) && !timeout) {
                                            outtoken.write(buftoken, 0, ntoken);
                                            if (System.currentTimeMillis() - millis > 10000)
                                                timeout = true;
                                        }
                                    }
                                }
                                outtoken.close();
                                intoken.close();
                                if (timeout) {
                                    System.err.println("Error: Problem downloading token: " + id + "t.jpg from, i will not retry anymore...");
                                    break;
                                }
                                byte[] responsetoken = outtoken.toByteArray();
                                String tokenimage = imgPath + File.separator + id + "t.jpg";
                                String tokenthumbimage = thumbPath + File.separator + id + "t.jpg";
                                if (!tokenfound && !id.equals("464007")) {
                                    System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t.jpg) i will use the same image of its source card");
                                    res = nametoken + " - " + set + File.separator + id + "t.jpg\n" + res;
                                }
                                FileOutputStream fos2 = new FileOutputStream(tokenimage);
                                fos2.write(responsetoken);
                                fos2.close();
                                try {
                                    Bitmap yourBitmapToken = BitmapFactory.decodeFile(tokenimage);
                                    Bitmap resizedToken = Bitmap.createScaledBitmap(yourBitmapToken, ImgX, ImgY, true);
                                    if(Border > 0)
                                        resizedToken = Bitmap.createBitmap(resizedToken, Border, Border, ImgX-2*Border, ImgY-2*Border);
                                    FileOutputStream fout = new FileOutputStream(tokenimage);
                                    resizedToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                    fout.close();
                                } catch (Exception e) {
                                    System.err.println("Error: Problem resizing token: " + id + "t.jpg, image may be corrupted...");
                                    res = nametoken + " - " + set + File.separator + "thumbnails" + File.separator + id + "t.jpg\n" + res;
                                    break;
                                }
                                try {
                                    Bitmap yourBitmapTokenthumb = BitmapFactory.decodeFile(tokenimage);
                                    Bitmap resizedThumbToken = Bitmap.createScaledBitmap(yourBitmapTokenthumb, ThumbX, ThumbY, true);
                                    if(BorderThumb > 0)
                                        resizedThumbToken = Bitmap.createBitmap(resizedThumbToken, BorderThumb, BorderThumb, ThumbX-2*BorderThumb, ThumbY-2*BorderThumb);
                                    FileOutputStream fout = new FileOutputStream(tokenthumbimage);
                                    resizedThumbToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                    fout.close();
                                } catch (Exception e) {
                                    System.err.println("Error: Problem resizing token thumbnail: " + id + "t.jpg, image may be corrupted...");
                                    res = nametoken + " - " + set + File.separator + "thumbnails" + File.separator + id + "t.jpg\n" + res;
                                    break;
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

        while (parent.paused && parent.downloadInProgress) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        }

        if (parent.downloadInProgress) {
            try {
                ZipParameters zipParameters = new ZipParameters();
                zipParameters.setCompressionMethod(CompressionMethod.STORE);
                File folder = new File(destinationPath + set + File.separator);
                File[] listOfFile = folder.listFiles();
                net.lingala.zip4j.ZipFile zipped = new net.lingala.zip4j.ZipFile(destinationPath + File.separator + set + File.separator + set + ".zip");
                for (int i = 0; i < listOfFile.length; i++) {
                    if (listOfFile[i].isDirectory()) {
                        zipped.addFolder(listOfFile[i], zipParameters);
                    } else {
                        zipped.addFile(listOfFile[i], zipParameters);
                    }
                }
                File destFolder = new File(destinationPath + set + File.separator);
                listOfFiles = destFolder.listFiles();
                for (int u = 0; u < listOfFiles.length; u++) {
                    if (!listOfFiles[u].getName().contains(".zip")) {
                        if (listOfFiles[u].isDirectory()) {
                            File[] listOfSubFiles = listOfFiles[u].listFiles();
                            for (int j = 0; j < listOfSubFiles.length; j++)
                                listOfSubFiles[j].delete();
                        }
                        listOfFiles[u].delete();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return res;
    }
}
