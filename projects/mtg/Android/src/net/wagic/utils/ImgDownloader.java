package net.wagic.utils;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.jsoup.nodes.Node;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;

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

    public static JSONObject findCardJsonById(String multiverseId) {
        try{
            String apiUrl = "https://api.scryfall.com/cards/multiverse/" + multiverseId;

            URL url = new URL(apiUrl);
            HttpURLConnection connection = (HttpURLConnection) url.openConnection();
            connection.setRequestMethod("GET");

            BufferedReader reader = new BufferedReader(new InputStreamReader(connection.getInputStream()));
            StringBuilder response = new StringBuilder();
            String line;
            while ((line = reader.readLine()) != null) {
                response.append(line);
            }
            reader.close();

            JSONParser jsonParser = new JSONParser();
            JSONObject jsonObject = (JSONObject) jsonParser.parse(response.toString());
            return jsonObject;
        } catch(Exception e){
            return null;
        }
    }

    public static String findCardImageUrl(JSONObject jsonObject, String primitiveCardName, String multiverseId, String format){
        Map<String, String> imageUris = new HashMap<String, String>();
        if (jsonObject.get("image_uris") != null) {
            JSONObject imageUrisObject = (JSONObject) jsonObject.get("image_uris");
            if(imageUrisObject != null && jsonObject.get("name").equals(primitiveCardName))
                imageUris = (HashMap) imageUrisObject;
        } else if (jsonObject.get("card_faces") != null) {
            JSONArray faces = (JSONArray) jsonObject.get("card_faces");
            if(faces != null){
                for (Object o : faces) {
                    JSONObject imageUrisObject = (JSONObject) o;
                    if(imageUrisObject != null && imageUrisObject.get("name").equals(primitiveCardName)){
                        if (imageUrisObject.get("image_uris") != null) {
                            imageUrisObject = (JSONObject) imageUrisObject.get("image_uris");
                            if(imageUrisObject != null)
                                imageUris = (HashMap) imageUrisObject;
                        }
                    }
                }
            }
        } else {
            System.err.println("Cannot retrieve image url for card: " + primitiveCardName + "(" + multiverseId + ")");
            return "";
        }
        String imageUrl = imageUris.get(format);
        if(imageUrl == null){
            System.err.println("Cannot retrieve image url for card: " + primitiveCardName + "(" + multiverseId + ")");
            return "";
        }
        if(imageUrl.indexOf(".jpg") < imageUrl.length())
            imageUrl = imageUrl.substring(0, imageUrl.indexOf(".jpg")+4);
        return imageUrl;
    }

    public static String findTokenImageUrl(JSONObject jsonObject, String multiverseId, String format, String filterName){
        String imageUrl = "";
        try {
            Document document = Jsoup.connect((String )jsonObject.get("scryfall_uri")).get();
            if (document != null) {
                Element printsTable = document.selectFirst("table.prints-table");
                if (printsTable != null) {
                    Element tokenRow = null;
                    Elements rows = printsTable.select("tr");
                    int howmany = 0;
                    for (Element row : rows) {
                        if (row.text().contains(" Token,") && !row.text().contains("Faces,")) {
                            Element aElement = row.selectFirst("td > a");
                            String tokenName = aElement.text();
                            tokenName = tokenName.substring(0, tokenName.indexOf(" Token,"));
                            if(tokenName.equals(filterName)){
                                System.out.println("The token " + tokenName + " has been filtered for card: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
                            } else {
                                imageUrl = aElement.attr("data-card-image-front");
                                if(imageUrl != null){
                                    howmany++;
                                    if(imageUrl.indexOf(".jpg") < imageUrl.length())
                                        imageUrl = imageUrl.substring(0, imageUrl.indexOf(".jpg")+4);
                                }
                            }
                        }
                    }
                    if (howmany > 1) {
                        System.out.println("Found " + howmany  + " valid image urls for token created by: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("There was an error while retrieving the token image for card: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
            return "";
        }
        if(imageUrl == null) {
            System.err.println("There was an error while retrieving the token image for card: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
            return "";
        }
        return imageUrl.replace("large", format);
    }

    public static String findTokenName(JSONObject jsonObject, String multiverseId, String filterName){
        String tokenName = "";
        try {
            Document document = Jsoup.connect((String) jsonObject.get("scryfall_uri")).get();
            if (document != null) {
                Element printsTable = document.selectFirst("table.prints-table");
                if (printsTable != null) {
                    Element tokenRow = null;
                    Elements rows = printsTable.select("tr");
                    int howmany = 0;
                    for (Element row : rows) {
                        if (row.text().contains(" Token,") && !row.text().contains("Faces,")) {
                            Element aElement = row.selectFirst("td > a");
                            String tok = aElement.text();
                            if(tok != null) {
                                tok = tok.substring(0, tok.indexOf(" Token,"));
                                if (tok.equals(filterName)) {
                                    System.out.println("The token " + tok + " has been filtered for card: " + (String) jsonObject.get("name") + " (" + multiverseId + ")");
                                } else {
                                    howmany++;
                                    tokenName = tok;
                                }
                            }
                        }
                    }
                    if (howmany > 1) {
                        System.out.println("Found " + howmany  + " valid token name created by: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
                    }
                }
            }
        } catch (IOException e) {
            System.err.println("There was an error while retrieving the token name for card: " + (String)jsonObject.get("name") + " (" + multiverseId + ")");
            return "";
        }
        return tokenName;
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

    public static String getSpecialCardUrl(String id, String set) {
        String cardurl = "";

        if(id.equals("15208711t"))
            cardurl = "https://cards.scryfall.io/large/front/9/c/9c138bf9-8be6-4f1a-a82c-a84938ab84f5.jpg";
        else if(id.equals("15208712t"))
            cardurl = "https://cards.scryfall.io/large/front/d/4/d453ee89-6122-4d51-989c-e78b046a9de3.jpg";
        else if(id.equals("2050321t"))
            cardurl = "https://cards.scryfall.io/large/front/1/8/18b9c83d-4422-4b95-9fc2-070ed6b5bdf6.jpg";
        else if(id.equals("22010012t"))
            cardurl = "https://cards.scryfall.io/large/front/8/4/84dc847c-7a37-4c7f-b02c-30b3e4c91fb6.jpg";
        else if(id.equals("4143881t"))
            cardurl = "https://cards.scryfall.io/large/front/8/a/8a73e348-5bf1-4465-978b-3f31408bade9.jpg";
        else if(id.equals("8759611"))
            cardurl = "https://cards.scryfall.io/large/front/4/1/41004bdf-8e09-4b2c-9e9c-26c25eac9854.jpg";
        else if(id.equals("8759911"))
            cardurl = "https://cards.scryfall.io/large/front/0/b/0b61d772-2d8b-4acf-9dd2-b2e8b03538c8.jpg";
        else if(id.equals("8759511"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d224c50f-8146-4c91-9401-04e5bd306d02.jpg";
        else if(id.equals("8471611"))
            cardurl = "https://cards.scryfall.io/large/front/8/4/84920a21-ee2a-41ac-a369-347633d10371.jpg";
        else if(id.equals("8760011"))
            cardurl = "https://cards.scryfall.io/large/front/4/2/42ba0e13-d20f-47f9-9c86-2b0b13c39ada.jpg";
        else if(id.equals("7448911"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/ca03131a-9bd4-4fba-b95c-90f1831e86e7.jpg";
        else if(id.equals("7453611"))
            cardurl = "https://cards.scryfall.io/large/front/7/3/73636ca0-2309-4bb3-9300-8bd0c0bb5b31.jpg";
        else if(id.equals("7447611"))
            cardurl = "https://cards.scryfall.io/large/front/2/8/28f72260-c8f9-4c44-92b5-23cef6690fdd.jpg";
        else if(id.equals("7467111"))
            cardurl = "https://cards.scryfall.io/large/front/1/f/1fe2b76f-ddb7-49d5-933b-ccb06be5d46f.jpg";
        else if(id.equals("7409311"))
            cardurl = "https://cards.scryfall.io/large/front/7/5/758abd53-6ad2-406e-8615-8e48678405b4.jpg";
        else if(id.equals("3896122t"))
            cardurl = "https://cards.scryfall.io/large/front/5/9/59a00cac-53ae-46ad-8468-e6d1db40b266.jpg";
        else if(id.equals("11492113t"))
            cardurl = "https://cards.scryfall.io/large/front/5/b/5b9f471a-1822-4981-95a9-8923d83ddcbf.jpg";
        else if(id.equals("3896523t")) //Kraken 9/9
            cardurl = "https://cards.scryfall.io/large/front/d/0/d0cd85cc-ad22-446b-8378-5eb69fee1959.jpg";
        else if(id.equals("7897511"))
            cardurl = "https://cards.scryfall.io/large/front/a/4/a4f4aa3b-c64a-4430-b1a2-a7fca87d0a22.jpg";
        else if(id.equals("7868811"))
            cardurl = "https://cards.scryfall.io/large/front/b/3/b3523b8e-065f-427c-8d5b-eb731ca91ede.jpg";
        else if(id.equals("7868711"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/58164521-aeec-43fc-9db9-d595432dea6f.jpg";
        else if(id.equals("7868611"))
            cardurl = "https://cards.scryfall.io/large/front/3/3/33a8e5b9-6bfb-4ff2-a16d-3168a5412807.jpg";
        else if(id.equals("7869111"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9de1eebf-5725-438c-bcf0-f3a4d8a89fb0.jpg";
        else if(id.equals("7860011"))
            cardurl = "https://cards.scryfall.io/large/front/8/6/864ad989-19a6-4930-8efc-bbc077a18c32.jpg";
        else if(id.equals("7867911"))
            cardurl = "https://cards.scryfall.io/large/front/c/8/c8265c39-d287-4c5a-baba-f2f09dd80a1c.jpg";
        else if(id.equals("7867811"))
            cardurl = "https://cards.scryfall.io/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg";
        else if(id.equals("7869511"))
            cardurl = "https://cards.scryfall.io/large/front/f/2/f2ddf1a3-e6fa-4dd0-b80d-1a585b51b934.jpg";
        else if(id.equals("7869411"))
            cardurl = "https://cards.scryfall.io/large/front/6/e/6ee6cd34-c117-4d7e-97d1-8f8464bfaac8.jpg";
        else if(id.equals("209163t"))
            cardurl = "https://cards.scryfall.io/large/front/a/3/a3ea39a8-48d1-4a58-8662-88841eabec92.jpg";
        else if(id.equals("111066t"))
            cardurl = "https://cards.scryfall.io/large/front/a/7/a77c1ac0-5548-42b0-aa46-d532b3518632.jpg";
        else if(id.equals("2050322t") || id.equals("16710t")) //Ooze 1/1
            cardurl = "https://cards.scryfall.io/large/front/a/b/ab430ac0-6fbc-4361-8adc-0c13b399310f.jpg";
        else if(id.equals("401721t")) //Hellion 4/4
            cardurl = "https://cards.scryfall.io/large/front/d/a/da59fb40-b218-452f-b161-3bde15e30c74.jpg";
        else if(id.equals("401722t"))
            cardurl = "https://cards.scryfall.io/large/front/d/3/d365bf32-74a3-436b-b1cc-1a7d3254257a.jpg";
        else if(id.equals("19784311t") || id.equals("29669411t") || id.equals("53939511t")) //Snake 1/1 green
            cardurl = "https://cards.scryfall.io/large/front/2/a/2a452235-cebd-4e8f-b217-9b55fc1c3830.jpg";
        else if(id.equals("19784313t") || id.equals("29669413t") || id.equals("53939513t")) //Elephant 3/3
            cardurl = "https://cards.scryfall.io/large/front/2/d/2dbccfc7-427b-41e6-b770-92d73994bf3b.jpg";
        else if(id.equals("20787512t")) //Wurm T2 3/3
            cardurl = "https://cards.scryfall.io/large/front/a/6/a6ee0db9-ac89-4ab6-ac2e-8a7527d9ecbd.jpg";
        else if(id.equals("20787511t")) //Wurm T1 3/3
            cardurl = "https://cards.scryfall.io/large/front/b/6/b68e816f-f9ac-435b-ad0b-ceedbe72447a.jpg";
        else if(id.equals("11492111t")) //Citizen 1/1
            cardurl = "https://cards.scryfall.io/large/front/1/6/165164e7-5693-4d65-b789-8ed8a222365b.jpg";
        else if(id.equals("11492112t")) //Camarid 1/1
            cardurl = "https://www.mtg.onl/static/f5ec1ae8d4ec1a8be1c20ec315956bfa/4d406/PROXY_Camarid_U_1_1.jpg";
        else if(id.equals("11492114t") || id.equals("16932t") || id.equals("293980t") || id.equals("293981t") || id.equals("296660t")) //Goblin 1/1
            cardurl = "https://cards.scryfall.io/large/front/1/4/1425e965-7eea-419c-a7ec-c8169fa9edbf.jpg";
        else if(id.equals("3896522t")) //Whale 6/6
            cardurl = "https://cards.scryfall.io/large/front/8/7/87a6f719-3e2f-48ea-829d-77134a2a8432.jpg";
        else if(id.equals("3896521t")) //Fish 3/3
            cardurl = "https://cards.scryfall.io/large/front/3/a/3abd270d-55d0-40f8-9864-4a7d7b9310ff.jpg";
        else if(id.equals("207998t")) //Minion */*
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9930d11-4772-4fc2-abbd-9af0a9b23a3e.jpg";
        else if (id.equals("19784555t")) //Elemental */*
            cardurl = "https://cards.scryfall.io/large/front/8/6/8676704a-419e-4a00-a052-bca2ad34ecae.jpg";
        else if (id.equals("19784612t") || id.equals("53941712t")) //Centaur 3/3
            cardurl = "https://cards.scryfall.io/large/front/8/8/880d5dc1-ceec-4c5f-93c2-c88b7dbfcac2.jpg";
        else if (id.equals("19784613t") || id.equals("52973t") || id.equals("53941713t")) //Rhino 4/4
            cardurl = "https://cards.scryfall.io/large/front/1/3/1331008a-ae86-4640-b823-a73be766ac16.jpg";
        else if (id.equals("19784611t") || id.equals("53941711t")) //Knight 2/2
            cardurl = "https://cards.scryfall.io/large/front/1/b/1bc2969b-2176-4471-b316-9c80443866dd.jpg";
        else if (id.equals("4977511t")) //Elemental 2/2
            cardurl = "https://www.mtg.onl/static/acc7da698156ddfb2270f09ac7ae6f81/4d406/PROXY_Elemental_U_2_2.jpg";
        else if (id.equals("4977512t")) //Elemental 3/3
            cardurl = "https://www.mtg.onl/static/6c36d944a78a513c082c86b7f624b3b6/4d406/PROXY_Elemental_R_3_3.jpg";
        else if(id.equals("383257t")) //Land mine
            cardurl = "https://cards.scryfall.io/large/front/3/0/30093c6e-505e-4902-b535-707e364059b4.jpg";
        else if(id.equals("383290t")) //Treefolk Warrior */*
            cardurl = "https://cards.scryfall.io/large/front/2/5/2569593a-d2f2-414c-9e61-2c34e8a5832d.jpg";
        else if(id.equals("378445t")) //Gold
            cardurl = "https://cards.scryfall.io/large/front/0/c/0ca10abd-8d9d-4c0b-9d33-c3516abdf6b3.jpg";
        else if(id.equals("16699t"))//Myr 1/1
            cardurl = "https://cards.scryfall.io/large/front/d/b/dbad9b20-0b13-41b9-a84a-06b691ee6c71.jpg";
        else if(id.equals("16708t") || id.equals("17097t") || id.equals("17085t") || id.equals("541071t") || id.equals("541072t")) //Insect 1/1
            cardurl = "https://cards.scryfall.io/large/front/0/4/0436e71b-c1f9-4ca8-a29c-775da858a0cd.jpg";
        else if(id.equals("16717t")) //Germ 0/1
            cardurl = "https://cards.scryfall.io/large/front/4/4/4414f9fa-dfda-4714-9f87-cb5e8914b07a.jpg";
        else if(id.equals("16718t")) //Spawn 2/2
            cardurl = "http://1.bp.blogspot.com/-0-mLvfUVgNk/VmdZWXWxikI/AAAAAAAAAUM/TVCIiZ_c67g/s1600/Spawn%2BToken.jpg";
        else if(id.equals("16729t") || id.equals("17538t")) //Beast 5/5
            cardurl = "https://www.mtg.onl/static/115b4e620e7ac0442355b28e5dc03673/4d406/PROXY_Beast_G_5_5.jpg";
        else if(id.equals("52993t")) //Assembly-Worker 2/2
            cardurl = "https://cards.scryfall.io/large/front/e/7/e72daa68-0680-431c-a616-b3693fd58813.jpg";
        else if(id.equals("52593t") || id.equals("294265t")) //Plant 0/1
            cardurl = "https://cards.scryfall.io/large/front/f/a/fa0025fa-c530-4151-bcff-48425a4f1db5.jpg";
        else if(id.equals("52492t"))
            cardurl = "https://cards.scryfall.io/large/front/f/3/f32ad93f-3fd5-465c-ac6a-6f8fb57c19bd.jpg";
        else if(id.equals("52418t") || id.equals("378521t")) //Kraken 9/9
            cardurl= "https://cards.scryfall.io/large/front/d/0/d0cd85cc-ad22-446b-8378-5eb69fee1959.jpg";
        else if(id.equals("52398t")) //Illusion 2/2
            cardurl = "https://cards.scryfall.io/large/front/a/1/a10729a5-061a-4daf-91d6-0f6ce813a992.jpg";
        else if(id.equals("52149t") || id.equals("52136t")) //Soldier 1/1
            cardurl = "https://cards.scryfall.io/large/front/4/5/45907b16-af17-4237-ab38-9d7537fd30e8.jpg";
        else if(id.equals("52637t") || id.equals("52945t") || id.equals("296637t")) // Thopter 1/1
            cardurl = "https://cards.scryfall.io/large/front/5/a/5a4649cc-07fb-4ff0-9ac6-846763b799df.jpg";
        else if(id.equals("74272"))
            cardurl = "https://cards.scryfall.io/large/front/4/5/45af7f55-9a69-43dd-969f-65411711b13e.jpg";
        else if(id.equals("242498"))
            cardurl = "https://cards.scryfall.io/large/back/f/5/f500cb95-d5ea-4cf2-920a-f1df45a9059b.jpg";
        else if(id.equals("253431"))
            cardurl = "https://cards.scryfall.io/large/back/1/3/1303e02a-ef69-4817-bca5-02c74774b811.jpg";
        else if(id.equals("262659"))
            cardurl = "https://cards.scryfall.io/large/back/6/f/6f35e364-81d9-4888-993b-acc7a53d963c.jpg";
        else if(id.equals("262698"))
            cardurl = "https://cards.scryfall.io/large/back/a/2/a2c044c0-3625-4bdf-9445-b462394cecae.jpg";
        else if(id.equals("244734"))
            cardurl = "https://cards.scryfall.io/large/back/c/b/cb09041b-4d09-4cae-9e85-b859edae885b.jpg";
        else if(id.equals("244712"))
            cardurl = "https://cards.scryfall.io/large/back/7/c/7c5a3c09-5656-4975-ba03-2d809903ed18.jpg";
        else if(id.equals("227405"))
            cardurl = "https://cards.scryfall.io/large/back/6/a/6aef77b3-4b38-4902-9f7a-dc18b5bb9da9.jpg";
        else if(id.equals("247122"))
            cardurl = "https://cards.scryfall.io/large/back/b/6/b6edac85-78e7-4e90-b538-b67c88bb5c62.jpg";
        else if(id.equals("244738"))
            cardurl = "https://cards.scryfall.io/large/back/6/8/683af377-c491-4f62-900c-6b83d75c33c9.jpg";
        else if(id.equals("253429"))
            cardurl = "https://cards.scryfall.io/large/back/b/1/b150d71f-11c9-40d6-a461-4967ef437315.jpg";
        else if(id.equals("242509"))
            cardurl = "https://cards.scryfall.io/large/back/9/3/932d753d-9584-4ad8-9a5e-a3524184f961.jpg";
        else if(id.equals("687706") || id.equals("687751"))
            cardurl = "https://cards.scryfall.io/large/front/6/9/692c668d-3061-4bdf-921f-94af32b4878c.jpg";
        else if(id.equals("414422"))
            cardurl = "https://cards.scryfall.io/large/back/7/f/7f95145a-41a1-478e-bf8a-ea8838d6f9b1.jpg";
        else if(id.equals("414325"))
            cardurl = "https://cards.scryfall.io/large/back/9/a/9a55b60a-5d90-4f73-984e-53fdcc0366e4.jpg";
        else if(id.equals("414347"))
            cardurl = "https://cards.scryfall.io/large/back/2/2/22e816af-df55-4a3f-a6e7-0ff3bb1b45b5.jpg";
        else if(id.equals("414392"))
            cardurl = "https://cards.scryfall.io/large/front/7/0/70b94f21-4f01-46f8-ad50-e2bb0b68ea33.jpg";
        else if(id.equals("414305"))
            cardurl = "https://cards.scryfall.io/large/front/5/a/5a7a212e-e0b6-4f12-a95c-173cae023f93.jpg";
        else if(id.equals("414500"))
            cardurl = "https://cards.scryfall.io/large/back/0/7/078b2103-15ce-456d-b092-352fa7222935.jpg";
        else if(id.equals("414471"))
            cardurl = "https://cards.scryfall.io/large/back/a/6/a63c30c0-369a-4a75-b352-edab4d263d1b.jpg";
        else if(id.equals("414480"))
            cardurl = "https://cards.scryfall.io/large/back/0/d/0dbaef61-fa39-4ea7-bc21-445401c373e7.jpg";
        else if(id.equals("414449"))
            cardurl = "https://cards.scryfall.io/large/back/4/6/460f7733-c0a6-4439-a313-7b26ae6ee15b.jpg";
        else if(id.equals("414514")) //Eldrazi Horror 3/2
            cardurl = "https://cards.scryfall.io/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg";
        else if(id.equals("414497"))
            cardurl = "https://cards.scryfall.io/large/back/3/e/3e2011f0-a640-4579-bd67-1dfbc09b8c09.jpg";
        else if(id.equals("414478"))
            cardurl = "https://cards.scryfall.io/large/back/e/e/ee648500-a213-4aa4-a97c-b7223c11bebd.jpg";
        else if(id.equals("414442"))
            cardurl = "https://cards.scryfall.io/large/back/0/b/0b0eab47-af62-4ee8-99cf-a864fadade2d.jpg";
        else if(id.equals("414358"))
            cardurl = "https://cards.scryfall.io/large/back/1/e/1eb4ddf4-f695-412d-be80-b93392432498.jpg";
        else if(id.equals("414408"))
            cardurl = "https://cards.scryfall.io/large/back/2/5/25baac6c-5bb4-4ecc-b1d5-fced52087bd9.jpg";
        else if(id.equals("414465"))
            cardurl = "https://cards.scryfall.io/large/back/f/8/f89f116a-1e8e-4ae7-be39-552e4954f229.jpg";
        else if(id.equals("439843"))
            cardurl = "https://cards.scryfall.io/large/back/3/9/397ba02d-f347-46f7-b028-dd4ba55faa2f.jpg";
        else if(id.equals("439835"))
            cardurl = "https://cards.scryfall.io/large/back/c/1/c16ba84e-a0cc-4c6c-9b80-713247b8fef9.jpg";
        else if(id.equals("439825"))
            cardurl = "https://cards.scryfall.io/large/back/6/6/66d9d524-3611-48d9-86c9-48e509e8ae70.jpg";
        else if(id.equals("439839"))
            cardurl = "https://cards.scryfall.io/large/back/3/0/303d51ab-b9c4-4647-950f-291daabe7b81.jpg";
        else if(id.equals("439827"))
            cardurl = "https://cards.scryfall.io/large/back/1/d/1d94ff37-f04e-48ee-8253-d62ab07f0632.jpg";
        else if(id.equals("439816"))
            cardurl = "https://cards.scryfall.io/large/back/8/e/8e7554bc-8583-4059-8895-c3845bc27ae3.jpg";
        else if(id.equals("439819"))
            cardurl = "https://cards.scryfall.io/large/back/d/8/d81c4b3f-81c2-403b-8a5d-c9415f73a1f9.jpg";
        else if(id.equals("227290"))
            cardurl = "https://cards.scryfall.io/large/back/5/7/57f0907f-74f4-4d86-93df-f2e50c9d0b2f.jpg";
        else if(id.equals("244687"))
            cardurl = "https://cards.scryfall.io/large/back/2/b/2b14ed17-1a35-4c49-ac46-3cad42d46c14.jpg";
        else if(id.equals("222123"))
            cardurl = "https://cards.scryfall.io/large/back/4/b/4b43b0cb-a5a3-47b4-9b6b-9d2638222bb6.jpg";
        else if(id.equals("222906"))
            cardurl = "https://cards.scryfall.io/large/back/e/4/e42a0a3d-a987-4b24-b9d4-27380a12e093.jpg";
        else if(id.equals("227419"))
            cardurl = "https://cards.scryfall.io/large/back/b/b/bb90a6f1-c7f2-4c2e-ab1e-59c5c7937841.jpg";
        else if(id.equals("226755"))
            cardurl = "https://cards.scryfall.io/large/back/1/1/11bf83bb-c95b-4b4f-9a56-ce7a1816307a.jpg";
        else if(id.equals("221190"))
            cardurl = "https://cards.scryfall.io/large/back/5/8/58ae9cbc-d88d-42df-ab76-63ab5d05c023.jpg";
        else if(id.equals("222115"))
            cardurl = "https://cards.scryfall.io/large/back/0/2/028aeebc-4073-4595-94da-02f9f96ea148.jpg";
        else if(id.equals("222183"))
            cardurl = "https://cards.scryfall.io/large/back/d/d/dd8ca448-f734-4cb9-b1d5-790eed9a4b2d.jpg";
        else if(id.equals("222114"))
            cardurl = "https://cards.scryfall.io/large/back/2/5/25b54a1d-e201-453b-9173-b04e06ee6fb7.jpg";
        else if(id.equals("222117"))
            cardurl = "https://cards.scryfall.io/large/back/6/1/6151cae7-92a4-4891-a952-21def412d3e4.jpg";
        else if(id.equals("221222"))
            cardurl = "https://cards.scryfall.io/large/back/f/8/f8b8f0b4-71e1-4822-99a1-b1b3c2f10cb2.jpg";
        else if(id.equals("222107"))
            cardurl = "https://cards.scryfall.io/large/back/c/d/cd5435d0-789f-4c42-8efc-165c072404a2.jpg";
        else if(id.equals("221185"))
            cardurl = "https://cards.scryfall.io/large/back/7/b/7bf864db-4754-433d-9d77-6695f78f6c09.jpg";
        else if(id.equals("221173"))
            cardurl = "https://cards.scryfall.io/large/back/e/b/ebf5e16f-a8bd-419f-b5ca-8c7fce09c4f1.jpg";
        else if(id.equals("222108"))
            cardurl = "https://cards.scryfall.io/large/back/8/3/8325c570-4d74-4e65-891c-3e153abf4bf9.jpg";
        else if(id.equals("221215"))
            cardurl = "https://cards.scryfall.io/large/back/8/8/88db324f-11f1-43d3-a897-f4e3caf8d642.jpg";
        else if(id.equals("227090"))
            cardurl = "https://cards.scryfall.io/large/back/e/c/ec00d2d2-6597-474a-9353-345bbedfe57e.jpg";
        else if(id.equals("398442"))
            cardurl = "https://cards.scryfall.io/large/front/9/f/9f25e1cf-eeb4-458d-8fb2-b3a2f86bdd54.jpg";
        else if(id.equals("398423"))
            cardurl = "https://cards.scryfall.io/large/front/b/0/b0d6caf0-4fa8-4ec5-b7f4-1307474d1b13.jpg";
        else if(id.equals("398435"))
            cardurl = "https://cards.scryfall.io/large/front/0/2/02d6d693-f1f3-4317-bcc0-c21fa8490d38.jpg";
        else if(id.equals("398429"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/58c39df6-b237-40d1-bdcb-2fe5d05392a9.jpg";
        else if(id.equals("439454"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cb3587b9-e727-4f37-b4d6-1baa7316262f.jpg";
        else if(id.equals("435451"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6c78fee-c186-4209-8533-edd695b9836a.jpg";
        else if (id.equals("1389"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/30345500-d430-4280-bfe3-de297309f136.jpg";
        else if (id.equals("1390"))
            cardurl = "https://cards.scryfall.io/large/front/5/a/5a240d1b-8430-4986-850d-32afa0e812b2.jpg";
        else if (id.equals("1391"))
            cardurl = "https://cards.scryfall.io/large/front/1/b/1b0f41e8-cf27-489b-812a-d566a75cf7f7.jpg";
        else if (id.equals("2381"))
            cardurl = "https://cards.scryfall.io/large/front/0/c/0c5c9379-b686-4823-b85a-eaf2c4b63205.jpg";
        else if (id.equals("2382"))
            cardurl = "https://cards.scryfall.io/large/front/1/0/10478e22-d1dd-4e02-81a7-d93ce71ed81d.jpg";
        else if (id.equals("2383"))
            cardurl = "https://cards.scryfall.io/large/front/5/0/50352268-88a6-4575-a5e1-cd8bef7f8286.jpg";
        else if (id.equals("414789"))
            cardurl = "https://cards.scryfall.io/large/front/2/e/2ef981a9-303e-4313-9265-77cc60323091.jpg";
        else if (id.equals("414790"))
            cardurl = "https://cards.scryfall.io/large/front/0/3/03e82924-899c-47b4-862a-7a27a96e285a.jpg";
        else if (id.equals("414791"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f57958e2-1e8f-48fa-816d-748ea2c7cb4e.jpg";
        else if (id.equals("414792"))
            cardurl = "https://cards.scryfall.io/large/front/3/f/3f89288a-9958-45c6-9bd2-24e6b3935171.jpg";
        else if (id.equals("414793"))
            cardurl = "https://cards.scryfall.io/large/front/8/5/85b37484-037a-497a-9820-97299d624daa.jpg";
        else if (id.equals("205309"))
            cardurl = "https://cards.scryfall.io/large/front/0/9/09e222f9-b7fc-49f0-8cef-9899aa333ecf.jpg";
        else if (id.equals("205434"))
            cardurl = "https://cards.scryfall.io/large/front/b/3/b3b6ad3d-a4d6-4ce9-bc0d-58fd83f83094.jpg";
        else if (id.equals("205442"))
            cardurl = "https://cards.scryfall.io/large/front/e/9/e9956850-0674-44e1-80e8-3875ef76d512.jpg";
        else if (id.equals("205443"))
            cardurl = "https://cards.scryfall.io/large/front/e/3/e3b5964a-78d8-453f-8cba-6ab01804054e.jpg";
        else if (id.equals("205446"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a262d93b-f95c-406c-9e54-cbd3ad14282f.jpg";
        else if (id.equals("2743"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/4695653a-5c4c-4ff3-b80c-f4b6c685f370.jpg";
        else if (id.equals("2744"))
            cardurl = "https://cards.scryfall.io/large/front/6/a/6a90b49f-53b3-4ce0-92c1-bcd76d6981ea.jpg";
        else if (id.equals("2745"))
            cardurl = "https://cards.scryfall.io/large/front/d/d/ddca7e2e-bb0a-47ed-ade3-31900da992dc.jpg";
        else if (id.equals("157871"))
            cardurl = "https://cards.scryfall.io/large/front/4/3/4324380c-68b8-4955-ad92-76f921e6ffc1.jpg";
        else if (id.equals("157886"))
            cardurl = "https://cards.scryfall.io/large/front/3/a/3a310639-99ca-4a7e-9f65-731779f3ea46.jpg";
        else if (id.equals("157889"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1ee0be63-ec99-4291-b504-e17061c15a67.jpg";
        else if (id.equals("158239"))
            cardurl = "https://cards.scryfall.io/large/front/a/1/a1d2dedf-d0d8-42c5-a498-31e172a1b503.jpg";
        else if (id.equals("2110"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/398a2b0f-0b91-408c-8083-3bc89873b69f.jpg";
        else if (id.equals("2101"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/398a2b0f-0b91-408c-8083-3bc89873b69f.jpg";
        else if (id.equals("3900"))
            cardurl = "https://cards.scryfall.io/large/front/9/a/9ac60e8c-ef5b-4893-b3e5-4a54cb0a0d3a.jpg";
        else if (id.equals("3981"))
            cardurl = "https://cards.scryfall.io/large/front/4/f/4fa6c0d6-aa18-4c32-a641-1ec8e50a26f3.jpg";
        else if (id.equals("426920"))
            cardurl = "https://cards.scryfall.io/large/front/5/1/517b32e4-4b34-431f-8f3b-98a6cffc245a.jpg";
        else if (id.equals("426915"))
            cardurl = "https://cards.scryfall.io/large/front/e/e/eeac671f-2606-43ed-ad60-a69df5c150f6.jpg";
        else if (id.equals("426914"))
            cardurl = "https://cards.scryfall.io/large/front/a/4/a4b32135-7061-4278-a01a-4fcbaadc9706.jpg";
        else if (id.equals("426917"))
            cardurl = "https://cards.scryfall.io/large/front/9/c/9c6f5433-57cc-4cb3-8621-2575fcbff392.jpg";
        else if (id.equals("426917t"))
            cardurl = "https://cards.scryfall.io/large/front/2/d/2d1446ed-f114-421d-bb60-9aeb655e5adb.jpg";
        else if (id.equals("426916"))
            cardurl = "https://cards.scryfall.io/large/front/a/4/a47070a0-fd05-4ed9-a175-847a864478da.jpg";
        else if (id.equals("426916t"))
            cardurl = "https://cards.scryfall.io/large/front/1/a/1aea5e0b-dc4e-4055-9e13-1dfbc25a2f00.jpg";
        else if (id.equals("47316011t"))
            cardurl = "https://cards.scryfall.io/large/front/c/9/c994ea90-71f4-403f-9418-2b72cc2de14d.jpg";
        else if (id.equals("47316012t"))
            cardurl = "https://cards.scryfall.io/large/front/d/b/db951f76-b785-453e-91b9-b3b8a5c1cfd4.jpg";
        else if (id.equals("47316013t"))
            cardurl = "https://cards.scryfall.io/large/front/c/d/cd3ca6d5-4b2c-46d4-95f3-f0f2fa47f447.jpg";
        else if (id.equals("426913"))
            cardurl = "https://cards.scryfall.io/large/front/0/6/06c9e2e8-2b4c-4087-9141-6aa25a506626.jpg";
        else if (id.equals("426912"))
            cardurl = "https://cards.scryfall.io/large/front/9/3/937dbc51-b589-4237-9fce-ea5c757f7c48.jpg";
        else if (id.equals("426919"))
            cardurl = "https://cards.scryfall.io/large/front/5/c/5cf5c549-1e2a-4c47-baf7-e608661b3088.jpg";
        else if (id.equals("426918"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2f3035c-ca27-40f3-ad73-c4e54bb2bcd7.jpg";
        else if (id.equals("426926"))
            cardurl = "https://cards.scryfall.io/large/front/f/e/fe1a4032-efbb-4f72-9181-994b2b35f598.jpg";
        else if (id.equals("426925"))
            cardurl = "https://cards.scryfall.io/large/front/c/6/c6f61e2b-e93b-4dda-95cf-9d0ff198c0a6.jpg";
        else if (id.equals("426922"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f59ea6f6-2dff-4e58-9166-57cac03f1d0a.jpg";
        else if (id.equals("426921"))
            cardurl = "https://cards.scryfall.io/large/front/6/4/6431d464-1f2b-42c4-ad38-67b7d0984080.jpg";
        else if (id.equals("426924"))
            cardurl = "https://cards.scryfall.io/large/front/1/1/11d84618-aca9-47dc-ae73-36a2c29f584c.jpg";
        else if (id.equals("426923"))
            cardurl = "https://cards.scryfall.io/large/front/b/9/b9623c8c-01b4-4e8f-a5b9-eeea408ec027.jpg";
        else if (id.equals("3082"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5afe9b5-3be8-472a-95c3-2c34231bc042.jpg";
        else if (id.equals("3083"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5afe9b5-3be8-472a-95c3-2c34231bc042.jpg";
        else if (id.equals("3222"))
            cardurl = "https://cards.scryfall.io/large/front/4/4/44be2d66-359e-4cc1-9670-119cb9c7d5f5.jpg";
        else if (id.equals("3223"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f9b0164c-2d4e-48ab-addd-322d9b504739.jpg";
        else if (id.equals("912"))
            cardurl = "https://cards.scryfall.io/large/front/f/c/fcc1004f-7cee-420a-9f0e-2986ed3ab852.jpg";
        else if (id.equals("915"))
            cardurl = "https://cards.scryfall.io/large/front/c/4/c4b610d3-2005-4347-bcda-c30b5b7972e5.jpg";
        else if (id.equals("921"))
            cardurl = "https://cards.scryfall.io/large/front/5/f/5f46783a-b91e-4829-a173-5515b09ca615.jpg";
        else if (id.equals("922"))
            cardurl = "https://cards.scryfall.io/large/front/3/1/31bf3f14-b5df-498b-a1bb-965885c82401.jpg";
        else if (id.equals("923"))
            cardurl = "https://cards.scryfall.io/large/front/1/8/18607bf6-ce11-41cb-b001-0c9538406ba0.jpg";
        else if (id.equals("929"))
            cardurl = "https://cards.scryfall.io/large/front/4/1/414d3cae-b8cf-4d53-bd6b-1aa83a828ba9.jpg";
        else if (id.equals("946"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f9d613d5-36a2-4633-b5af-64511bb29cc2.jpg";
        else if (id.equals("947"))
            cardurl = "https://cards.scryfall.io/large/front/c/0/c0b10fb7-8667-42bf-aeb6-35767a82917b.jpg";
        else if (id.equals("74476"))
            cardurl = "https://cards.scryfall.io/large/front/2/8/28f72260-c8f9-4c44-92b5-23cef6690fdd.jpg";
        else if (id.equals("74489"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/ca03131a-9bd4-4fba-b95c-90f1831e86e7.jpg";
        else if (id.equals("74536"))
            cardurl = "https://cards.scryfall.io/large/front/7/3/73636ca0-2309-4bb3-9300-8bd0c0bb5b31.jpg";
        else if (id.equals("74093"))
            cardurl = "https://cards.scryfall.io/large/front/7/5/758abd53-6ad2-406e-8615-8e48678405b4.jpg";
        else if (id.equals("74671"))
            cardurl = "https://cards.scryfall.io/large/front/1/f/1fe2b76f-ddb7-49d5-933b-ccb06be5d46f.jpg";
        else if (id.equals("376399"))
            cardurl = "https://cards.scryfall.io/large/front/c/e/cec89c38-0b72-44b0-ac6c-7eb9503e1256.jpg";
        else if (id.equals("451089") || id.equals("45108910"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/58164521-aeec-43fc-9db9-d595432dea6f.jpg";
        else if (id.equals("451089t"))
            cardurl = "https://cards.scryfall.io/large/front/c/5/c5ad13b4-bbf5-4c98-868f-4d105eaf8833.jpg";
        else if (id.equals("470745"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/ca4caa4e-6b8f-4be8-b177-de2ebe2c9201.jpg";
        else if (id.equals("470609"))
            cardurl = "https://cards.scryfall.io/large/front/e/9/e9d5aee0-5963-41db-a22b-cfea40a967a3.jpg";
        else if (id.equals("470738"))
            cardurl = "https://cards.scryfall.io/large/front/f/e/fe3b32dc-f7e6-455c-b9d1-c7f8d6259179.jpg";
        else if (id.equals("78686"))
            cardurl = "https://cards.scryfall.io/large/front/3/3/33a8e5b9-6bfb-4ff2-a16d-3168a5412807.jpg";
        else if (id.equals("78688"))
            cardurl = "https://cards.scryfall.io/large/front/b/3/b3523b8e-065f-427c-8d5b-eb731ca91ede.jpg";
        else if (id.equals("78687"))
            cardurl = "https://cards.scryfall.io/large/front/4/9/49999b95-5e62-414c-b975-4191b9c1ab39.jpg";
        else if (id.equals("75291"))
            cardurl = "https://cards.scryfall.io/large/front/9/8/98d3bc63-8814-46e7-a6ee-dd5b94a8257e.jpg";
        else if (id.equals("78679"))
            cardurl = "https://cards.scryfall.io/large/front/c/8/c8265c39-d287-4c5a-baba-f2f09dd80a1c.jpg";
        else if (id.equals("78678"))
            cardurl = "https://cards.scryfall.io/large/front/7/7/77ffd913-8efa-48e5-a5cf-293d3068dbbf.jpg";
        else if (id.equals("78691"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9de1eebf-5725-438c-bcf0-f3a4d8a89fb0.jpg";
        else if (id.equals("78695"))
            cardurl = "https://cards.scryfall.io/large/front/f/2/f2ddf1a3-e6fa-4dd0-b80d-1a585b51b934.jpg";
        else if (id.equals("78694"))
            cardurl = "https://cards.scryfall.io/large/front/6/e/6ee6cd34-c117-4d7e-97d1-8f8464bfaac8.jpg";
        else if (id.equals("78600"))
            cardurl = "https://cards.scryfall.io/large/front/8/6/864ad989-19a6-4930-8efc-bbc077a18c32.jpg";
        else if (id.equals("78975"))
            cardurl = "https://cards.scryfall.io/large/front/a/4/a4f4aa3b-c64a-4430-b1a2-a7fca87d0a22.jpg";
        else if (id.equals("2832"))
            cardurl = "https://cards.scryfall.io/large/front/8/5/85bcd723-780b-45ca-9476-d28270350013.jpg";
        else if (id.equals("2802"))
            cardurl = "https://cards.scryfall.io/large/front/b/f/bfc43585-55ac-4d58-9e80-b19a7c8c8662.jpg";
        else if (id.equals("446807"))
            cardurl = "https://cards.scryfall.io/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg";
        else if (id.equals("247175"))
            cardurl = "https://cards.scryfall.io/large/front/f/d/fd9920a0-78c2-4cc8-82e6-ea3a1e23b314.jpg";
        else if (id.equals("247182"))
            cardurl = "https://cards.scryfall.io/large/front/9/1/91a2217c-8478-479b-a146-2d78f407a36f.jpg";
        else if (id.equals("122075"))
            cardurl = "https://cards.scryfall.io/large/front/5/5/5526c510-bd33-4fac-8941-f19bd0997557.jpg";
        else if (id.equals("121236"))
            cardurl = "https://cards.scryfall.io/large/front/1/5/1566c8a2-aaca-4ce0-a36b-620ea6f135cb.jpg";
        else if (id.equals("244724"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cb09041b-4d09-4cae-9e85-b859edae885b.jpg";
        else if (id.equals("262675"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a2c044c0-3625-4bdf-9445-b462394cecae.jpg";
        else if (id.equals("226735"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9d9c1c46-7aa7-464c-87b0-b29b9663daef.jpg";
        else if (id.equals("253433"))
            cardurl = "https://cards.scryfall.io/large/front/b/1/b150d71f-11c9-40d6-a461-4967ef437315.jpg";
        else if (id.equals("226721"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9d9c1c46-7aa7-464c-87b0-b29b9663daef.jpg";
        else if (id.equals("227417"))
            cardurl = "https://cards.scryfall.io/large/front/6/a/6aef77b3-4b38-4902-9f7a-dc18b5bb9da9.jpg";
        else if (id.equals("243229"))
            cardurl = "https://cards.scryfall.io/large/front/7/c/7c5a3c09-5656-4975-ba03-2d809903ed18.jpg";
        else if (id.equals("242537"))
            cardurl = "https://cards.scryfall.io/large/front/9/3/932d753d-9584-4ad8-9a5e-a3524184f961.jpg";
        else if (id.equals("253426"))
            cardurl = "https://cards.scryfall.io/large/front/1/3/1303e02a-ef69-4817-bca5-02c74774b811.jpg";
        else if (id.equals("262875"))
            cardurl = "https://cards.scryfall.io/large/front/a/a/aae6fb12-b252-453b-bca7-1ea2a0d6c8dc.jpg";
        else if (id.equals("222178"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f500cb95-d5ea-4cf2-920a-f1df45a9059b.jpg";
        else if (id.equals("249422"))
            cardurl = "https://cards.scryfall.io/large/front/e/0/e00ae92c-af6d-4a00-b102-c6d3bcc394b4.jpg";
        else if (id.equals("247125"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6edac85-78e7-4e90-b538-b67c88bb5c62.jpg";
        else if (id.equals("262694"))
            cardurl = "https://cards.scryfall.io/large/front/6/f/6f35e364-81d9-4888-993b-acc7a53d963c.jpg";
        else if (id.equals("244740"))
            cardurl = "https://cards.scryfall.io/large/front/6/8/683af377-c491-4f62-900c-6b83d75c33c9.jpg";
        else if (id.equals("262699"))
            cardurl = "https://cards.scryfall.io/large/front/a/a/aae6fb12-b252-453b-bca7-1ea2a0d6c8dc.jpg";
        else if (id.equals("262857"))
            cardurl = "https://cards.scryfall.io/large/front/9/8/9831e3cc-659b-4408-b5d8-a27ae2738680.jpg";
        else if (id.equals("414499"))
            cardurl = "https://cards.scryfall.io/large/front/0/7/078b2103-15ce-456d-b092-352fa7222935.jpg";
        else if (id.equals("414496"))
            cardurl = "https://cards.scryfall.io/large/front/3/e/3e2011f0-a640-4579-bd67-1dfbc09b8c09.jpg";
        else if (id.equals("414448"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/460f7733-c0a6-4439-a313-7b26ae6ee15b.jpg";
        else if (id.equals("414346"))
            cardurl = "https://cards.scryfall.io/large/front/2/2/22e816af-df55-4a3f-a6e7-0ff3bb1b45b5.jpg";
        else if (id.equals("414464"))
            cardurl = "https://cards.scryfall.io/large/front/f/8/f89f116a-1e8e-4ae7-be39-552e4954f229.jpg";
        else if (id.equals("414349"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/30c3d4c1-dc3d-4529-9d6e-8c16149cf6da.jpg";
        else if (id.equals("414470"))
            cardurl = "https://cards.scryfall.io/large/front/a/6/a63c30c0-369a-4a75-b352-edab4d263d1b.jpg";
        else if (id.equals("414350"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/30c3d4c1-dc3d-4529-9d6e-8c16149cf6da.jpg";
        else if (id.equals("414357"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1eb4ddf4-f695-412d-be80-b93392432498.jpg";
        else if (id.equals("414479"))
            cardurl = "https://cards.scryfall.io/large/front/0/d/0dbaef61-fa39-4ea7-bc21-445401c373e7.jpg";
        else if (id.equals("414477"))
            cardurl = "https://cards.scryfall.io/large/front/e/e/ee648500-a213-4aa4-a97c-b7223c11bebd.jpg";
        else if (id.equals("414407"))
            cardurl = "https://cards.scryfall.io/large/front/2/5/25baac6c-5bb4-4ecc-b1d5-fced52087bd9.jpg";
        else if (id.equals("414421"))
            cardurl = "https://cards.scryfall.io/large/front/7/f/7f95145a-41a1-478e-bf8a-ea8838d6f9b1.jpg";
        else if (id.equals("414429"))
            cardurl = "https://cards.scryfall.io/large/front/0/9/0900e494-962d-48c6-8e78-66a489be4bb2.jpg";
        else if (id.equals("414304"))
            cardurl = "https://cards.scryfall.io/large/front/2/7/27907985-b5f6-4098-ab43-15a0c2bf94d5.jpg";
        else if (id.equals("414313"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6867ddd-f953-41c6-ba36-86ae2c14c908.jpg";
        else if (id.equals("414314"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6867ddd-f953-41c6-ba36-86ae2c14c908.jpg";
        else if (id.equals("414319"))
            cardurl = "https://cards.scryfall.io/large/front/c/7/c75c035a-7da9-4b36-982d-fca8220b1797.jpg";
        else if (id.equals("414324"))
            cardurl = "https://cards.scryfall.io/large/front/9/a/9a55b60a-5d90-4f73-984e-53fdcc0366e4.jpg";
        else if (id.equals("414441"))
            cardurl = "https://cards.scryfall.io/large/front/0/b/0b0eab47-af62-4ee8-99cf-a864fadade2d.jpg";
        else if (id.equals("456235"))
            cardurl = "https://cards.scryfall.io/large/front/0/1/01ce2601-ae94-4ab5-bbd2-65f47281ca28.jpg";
        else if (id.equals("452980"))
            cardurl = "https://cards.scryfall.io/large/front/4/4/44614c6d-5508-4077-b825-66d5d684086c.jpg";
        else if (id.equals("452979"))
            cardurl = "https://cards.scryfall.io/large/front/9/2/92162888-35ea-4f4f-ab99-64dd3104e230.jpg";
        else if (id.equals("452977"))
            cardurl = "https://cards.scryfall.io/large/front/4/a/4a82084e-b178-442b-8007-7b2a70f3fbba.jpg";
        else if (id.equals("452978"))
            cardurl = "https://cards.scryfall.io/large/front/0/5/054a4e4f-8baa-41cf-b24c-d068e8b9a070.jpg";
        else if (id.equals("452975"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1e4e9e35-6cbc-4997-beff-d1a22d87545e.jpg";
        else if (id.equals("452976"))
            cardurl = "https://cards.scryfall.io/large/front/f/e/feb4b39f-d309-49ba-b427-240b7fdc1099.jpg";
        else if (id.equals("452973"))
            cardurl = "https://cards.scryfall.io/large/front/a/c/ace631d1-897a-417e-8628-0170713f03d3.jpg";
        else if (id.equals("452974"))
            cardurl = "https://cards.scryfall.io/large/front/e/0/e0644c92-4d67-475e-8c8e-0e2c493682fb.jpg";
        else if (id.equals("452971"))
            cardurl = "https://cards.scryfall.io/large/front/a/d/ad454e7a-06c9-4694-ae68-7b1431e00077.jpg";
        else if (id.equals("452972"))
            cardurl = "https://cards.scryfall.io/large/front/8/9/890ac54c-6fd7-4e46-8ce4-8926c6975f60.jpg";
        else if (id.equals("430840"))
            cardurl = "https://cards.scryfall.io/large/front/7/6/76f21f0b-aaa5-4677-8398-cef98c6fac2a.jpg";
        else if (id.equals("430842"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f928e8e8-aa20-402c-85bd-59106e9b9cc7.jpg";
        else if (id.equals("430841"))
            cardurl = "https://cards.scryfall.io/large/front/2/c/2c25b8ef-6331-49df-9457-b8b4e44da2c9.jpg";
        else if (id.equals("430844"))
            cardurl = "https://cards.scryfall.io/large/front/0/3/0383401f-d453-4e8f-82d2-5c016acc2591.jpg";
        else if (id.equals("430843"))
            cardurl = "https://cards.scryfall.io/large/front/1/c/1ca644e3-4fb3-4d38-b714-e3d7459bd8b9.jpg";
        else if (id.equals("430846"))
            cardurl = "https://cards.scryfall.io/large/front/7/7/7713ba59-dd4c-4b49-93a7-292728df86b8.jpg";
        else if (id.equals("430845"))
            cardurl = "https://cards.scryfall.io/large/front/0/5/054b07d8-99ae-430b-8e54-f9601fa572e7.jpg";
        else if (id.equals("430837"))
            cardurl = "https://cards.scryfall.io/large/front/d/9/d998db65-8785-4ee9-940e-fa9ab62e180f.jpg";
        else if (id.equals("430839"))
            cardurl = "https://cards.scryfall.io/large/front/0/4/0468e488-94ce-4ae3-abe4-7782673a7e62.jpg";
        else if (id.equals("430838"))
            cardurl = "https://cards.scryfall.io/large/front/1/c/1c1ead90-10d8-4217-80e4-6f40320c5569.jpg";
        else if (id.equals("2470"))
            cardurl = "https://cards.scryfall.io/large/front/a/f/af976f42-3d56-4e32-8294-970a276a4bf3.jpg";
        else if (id.equals("2469"))
            cardurl = "https://cards.scryfall.io/large/front/3/d/3d0006f6-2f96-453d-9145-eaefa588efbc.jpg";
        else if (id.equals("2466"))
            cardurl = "https://cards.scryfall.io/large/front/7/5/75b67eb2-b60e-46b4-9d48-11c284957bec.jpg";
        else if (id.equals("2480"))
            cardurl = "https://cards.scryfall.io/large/front/f/1/f16df768-06de-43a0-b548-44fb0887490b.jpg";
        else if (id.equals("2635"))
            cardurl = "https://cards.scryfall.io/large/front/7/8/7880e815-53e7-43e0-befd-e368f00a75d8.jpg";
        else if (id.equals("221209"))
            cardurl = "https://cards.scryfall.io/large/front/7/b/7bf864db-4754-433d-9d77-6695f78f6c09.jpg";
        else if (id.equals("227415"))
            cardurl = "https://cards.scryfall.io/large/front/b/b/bb90a6f1-c7f2-4c2e-ab1e-59c5c7937841.jpg";
        else if (id.equals("221211"))
            cardurl = "https://cards.scryfall.io/large/front/8/8/88db324f-11f1-43d3-a897-f4e3caf8d642.jpg";
        else if (id.equals("221212"))
            cardurl = "https://cards.scryfall.io/large/front/f/8/f8b8f0b4-71e1-4822-99a1-b1b3c2f10cb2.jpg";
        else if (id.equals("244683"))
            cardurl = "https://cards.scryfall.io/large/front/2/b/2b14ed17-1a35-4c49-ac46-3cad42d46c14.jpg";
        else if (id.equals("222915"))
            cardurl = "https://cards.scryfall.io/large/front/e/4/e42a0a3d-a987-4b24-b9d4-27380a12e093.jpg";
        else if (id.equals("222112"))
            cardurl = "https://cards.scryfall.io/large/front/c/d/cd5435d0-789f-4c42-8efc-165c072404a2.jpg";
        else if (id.equals("222118"))
            cardurl = "https://cards.scryfall.io/large/front/2/5/25b54a1d-e201-453b-9173-b04e06ee6fb7.jpg";
        else if (id.equals("222105"))
            cardurl = "https://cards.scryfall.io/large/front/8/3/8325c570-4d74-4e65-891c-3e153abf4bf9.jpg";
        else if (id.equals("222111"))
            cardurl = "https://cards.scryfall.io/large/front/0/2/028aeebc-4073-4595-94da-02f9f96ea148.jpg";
        else if (id.equals("222016"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/58ae9cbc-d88d-42df-ab76-63ab5d05c023.jpg";
        else if (id.equals("222124"))
            cardurl = "https://cards.scryfall.io/large/front/4/b/4b43b0cb-a5a3-47b4-9b6b-9d2638222bb6.jpg";
        else if (id.equals("226749"))
            cardurl = "https://cards.scryfall.io/large/front/1/1/11bf83bb-c95b-4b4f-9a56-ce7a1816307a.jpg";
        else if (id.equals("221179"))
            cardurl = "https://cards.scryfall.io/large/front/e/b/ebf5e16f-a8bd-419f-b5ca-8c7fce09c4f1.jpg";
        else if (id.equals("245251"))
            cardurl = "https://cards.scryfall.io/large/front/b/4/b4160322-ff40-41a4-887a-73cd6b85ae45.jpg";
        else if (id.equals("245250"))
            cardurl = "https://cards.scryfall.io/large/front/b/4/b4160322-ff40-41a4-887a-73cd6b85ae45.jpg";
        else if (id.equals("222186"))
            cardurl = "https://cards.scryfall.io/large/front/6/1/6151cae7-92a4-4891-a952-21def412d3e4.jpg";
        else if (id.equals("227072"))
            cardurl = "https://cards.scryfall.io/large/front/1/3/13896468-e3d0-4bcb-b09e-b5c187aecb03.jpg";
        else if (id.equals("227061"))
            cardurl = "https://cards.scryfall.io/large/front/1/3/13896468-e3d0-4bcb-b09e-b5c187aecb03.jpg";
        else if (id.equals("227409"))
            cardurl = "https://cards.scryfall.io/large/front/5/7/57f0907f-74f4-4d86-93df-f2e50c9d0b2f.jpg";
        else if (id.equals("222189"))
            cardurl = "https://cards.scryfall.io/large/front/d/d/dd8ca448-f734-4cb9-b1d5-790eed9a4b2d.jpg";
        else if (id.equals("227084"))
            cardurl = "https://cards.scryfall.io/large/front/e/c/ec00d2d2-6597-474a-9353-345bbedfe57e.jpg";
        else if (id.equals("447354"))
            cardurl = "https://cards.scryfall.io/large/front/7/b/7b215968-93a6-4278-ac61-4e3e8c3c3943.jpg";
        else if (id.equals("447355"))
            cardurl = "https://cards.scryfall.io/large/front/7/b/7b215968-93a6-4278-ac61-4e3e8c3c3943.jpg";
        else if (id.equals("184714"))
            cardurl = "https://cards.scryfall.io/large/front/1/7/1777f69c-869e-414e-afe3-892714a6032a.jpg";
        else if (id.equals("202605"))
            cardurl = "https://cards.scryfall.io/large/front/5/f/5f6529eb-79ff-4ddc-9fae-38326324f7e6.jpg";
        else if (id.equals("202443"))
            cardurl = "https://cards.scryfall.io/large/front/0/8/082cf845-5a24-4f00-bad2-a3d0d07f59e6.jpg";
        else if (id.equals("398438"))
            cardurl = "https://cards.scryfall.io/large/front/f/f/ff0063da-ab6b-499d-8e87-8b34d46f0372.jpg";
        else if (id.equals("398432"))
            cardurl = "https://cards.scryfall.io/large/front/f/f/ff0063da-ab6b-499d-8e87-8b34d46f0372.jpg";
        else if (id.equals("398434"))
            cardurl = "https://cards.scryfall.io/large/front/0/2/02d6d693-f1f3-4317-bcc0-c21fa8490d38.jpg";
        else if (id.equals("398441"))
            cardurl = "https://cards.scryfall.io/large/front/9/f/9f25e1cf-eeb4-458d-8fb2-b3a2f86bdd54.jpg";
        else if (id.equals("398422"))
            cardurl = "https://cards.scryfall.io/large/front/b/0/b0d6caf0-4fa8-4ec5-b7f4-1307474d1b13.jpg";
        else if (id.equals("398428"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/58c39df6-b237-40d1-bdcb-2fe5d05392a9.jpg";
        else if (id.equals("6528"))
            cardurl = "https://cards.scryfall.io/large/front/a/d/ade6a71a-e8ec-4d41-8a39-3eacf0097c8b.jpg";
        else if (id.equals("4259"))
            cardurl = "https://cards.scryfall.io/large/front/7/c/7c93d4e9-7fd6-4814-b86b-89b92d1dad3b.jpg";
        else if (id.equals("439824"))
            cardurl = "https://cards.scryfall.io/large/front/6/6/66d9d524-3611-48d9-86c9-48e509e8ae70.jpg";
        else if (id.equals("439826"))
            cardurl = "https://cards.scryfall.io/large/front/1/d/1d94ff37-f04e-48ee-8253-d62ab07f0632.jpg";
        else if (id.equals("439834"))
            cardurl = "https://cards.scryfall.io/large/front/c/1/c16ba84e-a0cc-4c6c-9b80-713247b8fef9.jpg";
        else if (id.equals("439818"))
            cardurl = "https://cards.scryfall.io/large/front/d/8/d81c4b3f-81c2-403b-8a5d-c9415f73a1f9.jpg";
        else if (id.equals("439815"))
            cardurl = "https://cards.scryfall.io/large/front/8/e/8e7554bc-8583-4059-8895-c3845bc27ae3.jpg";
        else if (id.equals("439838"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/303d51ab-b9c4-4647-950f-291daabe7b81.jpg";
        else if (id.equals("439842"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/397ba02d-f347-46f7-b028-dd4ba55faa2f.jpg";
        else if (id.equals("457365"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5873efa-d573-4435-81ad-48df2ca5c7f2.jpg";
        else if (id.equals("457366"))
            cardurl = "https://cards.scryfall.io/large/front/d/1/d1dbc559-c78c-4675-9582-9c28f8151bc7.jpg";
        else if (id.equals("457367"))
            cardurl = "https://cards.scryfall.io/large/front/9/b/9bd15da6-2b86-4dba-951d-318c7d9a5dde.jpg";
        else if (id.equals("457368"))
            cardurl = "https://cards.scryfall.io/large/front/0/0/00320106-ce51-46a9-b0f9-79b3baf4e505.jpg";
        else if (id.equals("457369"))
            cardurl = "https://cards.scryfall.io/large/front/a/b/ab0ba4ef-9e82-4177-a80f-8fa6f6a5bd60.jpg";
        else if (id.equals("457370"))
            cardurl = "https://cards.scryfall.io/large/front/0/7/075bbe5d-d0f3-4be3-a3a6-072d5d3d614c.jpg";
        else if (id.equals("457371"))
            cardurl = "https://cards.scryfall.io/large/front/f/6/f6200937-3146-4972-ab83-051ade3b7a52.jpg";
        else if (id.equals("457372"))
            cardurl = "https://cards.scryfall.io/large/front/5/0/50ae0831-f3ba-4535-bfb6-feefbbc15275.jpg";
        else if (id.equals("457373"))
            cardurl = "https://cards.scryfall.io/large/front/2/e/2eefd8c1-96ce-4d7a-8aaf-29c35d634dda.jpg";
        else if (id.equals("457374"))
            cardurl = "https://cards.scryfall.io/large/front/0/0/0070651d-79aa-4ea6-b703-6ecd3528b548.jpg";
        else if (id.equals("1158"))
            cardurl = "https://cards.scryfall.io/large/front/c/3/c3591170-645f-4645-bc39-b90b7b6ddac7.jpg";
        else if (id.equals("409826"))
            cardurl = "https://cards.scryfall.io/large/front/6/e/6e099a6a-97c4-42cd-aca6-5e1a2da0d5e5.jpg";
        else if (id.equals("409899"))
            cardurl = "https://cards.scryfall.io/large/front/f/8/f8bdc165-4c6f-47e6-8bda-877c0be3613b.jpg";
        else if (id.equals("84716"))
            cardurl = "https://cards.scryfall.io/large/front/8/4/84920a21-ee2a-41ac-a369-347633d10371.jpg";
        else if (id.equals("87600"))
            cardurl = "https://cards.scryfall.io/large/front/4/2/42ba0e13-d20f-47f9-9c86-2b0b13c39ada.jpg";
        else if (id.equals("87599"))
            cardurl = "https://cards.scryfall.io/large/front/0/b/0b61d772-2d8b-4acf-9dd2-b2e8b03538c8.jpg";
        else if (id.equals("87595"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d224c50f-8146-4c91-9401-04e5bd306d02.jpg";
        else if (id.equals("87596"))
            cardurl = "https://cards.scryfall.io/large/front/4/1/41004bdf-8e09-4b2c-9e9c-26c25eac9854.jpg";
        else if (id.equals("106631"))
            cardurl = "https://cards.scryfall.io/large/front/a/c/ac2e32d0-f172-4934-9d73-1bc2ab86586e.jpg";
        else if (id.equals("9668"))
            cardurl = "https://cards.scryfall.io/large/front/1/7/17c18690-cf8c-4006-a981-6258d18ba538.jpg";
        else if (id.equals("9749"))
            cardurl = "https://cards.scryfall.io/large/front/3/f/3fcefcab-8988-47e8-89bb-9b76f14c9d8b.jpg";
        else if (id.equals("9780"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9f9c279-e382-4feb-9575-196e7cf5d7dc.jpg";
        else if (id.equals("9844"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9f9c279-e382-4feb-9575-196e7cf5d7dc.jpg";
        else if (id.equals("456821"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/468d5308-2a6c-440e-a8d0-1c5e084afb82.jpg";
        else if (id.equals("74358"))
            cardurl = "https://cards.scryfall.io/large/front/8/9/8987644d-5a31-4a4e-9a8a-3d6260ed0fd6.jpg";
        else if (id.equals("73956"))
            cardurl = "https://cards.scryfall.io/large/front/c/0/c01e8089-c3a9-413b-ae2d-39ede87516d3.jpg";
        else if (id.equals("74242"))
            cardurl = "https://cards.scryfall.io/large/front/8/5/85cbebbb-7ea4-4140-933f-186cad08697d.jpg";
        else if (id.equals("74322"))
            cardurl = "https://cards.scryfall.io/large/front/4/9/49dd5a66-101d-4f88-b1ba-e2368203d408.jpg";
        else if (id.equals("4429"))
            cardurl = "https://cards.scryfall.io/large/front/3/8/3884bede-df28-42e8-9ac9-ae03118b1985.jpg";
        else if (id.equals("113522"))
            cardurl = "https://cards.scryfall.io/large/front/b/8/b8a3cdfe-0289-474b-b9c4-07e8c6588ec5.jpg";
        else if (id.equals("51733"))
            cardurl = "https://cards.scryfall.io/large/front/2/7/27118cbb-a386-4145-8716-961ed0f653bf.jpg";
        else if (id.equals("52362"))
            cardurl = "https://cards.scryfall.io/large/front/9/b/9bd7a7f1-2221-4565-8c6e-1815def3bd2c.jpg";
        else if (id.equals("52415"))
            cardurl = "https://cards.scryfall.io/large/front/8/8/8825493a-878d-4df3-8d7a-98518358d678.jpg";
        else if(id.equals("53214t"))
            cardurl = "https://cards.scryfall.io/large/front/1/4/1449862b-309e-4c58-ac94-13d1acdd363f.jpg";
        else if(id.equals("53179t"))
            cardurl = "https://cards.scryfall.io/large/front/d/9/d9623e74-3b94-4842-903f-ed52931bdf6a.jpg";
        else if(id.equals("16806"))
            cardurl = "https://cards.scryfall.io/large/front/f/1/f1bb8fb5-32f2-444d-85cb-de84657b21bd.jpg";
        else if(id.equals("16807"))
            cardurl = "https://cards.scryfall.io/large/back/f/1/f1bb8fb5-32f2-444d-85cb-de84657b21bd.jpg";
        else if(id.equals("16808"))
            cardurl = "https://cards.scryfall.io/large/front/2/e/2eb08fc5-29a4-4911-ac94-dc5ff2fc2ace.jpg";
        else if(id.equals("16809"))
            cardurl = "https://cards.scryfall.io/large/front/9/e/9e5180da-d757-415c-b92d-090ad5c1b658.jpg";
        else if(id.equals("16809t"))
            cardurl = "https://cards.scryfall.io/large/front/8/e/8ee8b915-afd3-4fad-8aef-7e9cbbbbc2e4.jpg";
        else if(id.equals("16751"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/39a89c44-1aa7-4f2e-909b-d821ec2b7948.jpg";
        else if(id.equals("17639t"))
            cardurl = "https://cards.scryfall.io/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg";
        else if(id.equals("16740t") || id.equals("294023t")) //Gremlin 2/2
            cardurl = "https://cards.scryfall.io/large/front/c/6/c6071fed-39c1-4f3b-a821-1611aedd8054.jpg";
        else if (id.equals("53143t") || id.equals("17717t") || id.equals("17705t") || id.equals("17669t") || id.equals("17661t")
                || id.equals("17645t") || id.equals("17573t") || id.equals("17549t") || id.equals("17537t") || id.equals("17513t")
                || id.equals("17429t") || id.equals("17417t") || id.equals("17405t") || id.equals("17393t") || id.equals("17285t")
                || id.equals("17273t") || id.equals("17249t") || id.equals("17141t") || id.equals("17129t") || id.equals("17117t")
                || id.equals("17105t") || id.equals("17093t") || id.equals("17081t") || id.equals("17866t") || id.equals("294460t")
                || id.equals("11492115t") || id.equals("209162t") || id.equals("17010t") || id.equals("16997t")) //Saproling 1/1
            cardurl = "https://cards.scryfall.io/large/front/3/4/34f6ffaa-6dee-49db-ac59-745eae5e75b2.jpg";
        else if(id.endsWith("53141t")) //Elf Druid 1/1
            cardurl = "https://cards.scryfall.io/large/front/4/5/458f44dd-83f1-497e-b5d0-e3417eb9dfec.jpg";
        else if(id.equals("53134t") || id.equals("54047313t")) //Beast 4/4
            cardurl = "https://cards.scryfall.io/large/front/0/6/06b5e4d2-7eac-4ee9-82aa-80a668705679.jpg";
        else if(id.equals("16981t") || id.equals("16978t") || id.equals("16967t") || id.equals("17841t")
                || id.equals("17850t") || id.equals("17852t")) // Elf Warrior 1/1
            cardurl = "https://cards.scryfall.io/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg";
        else if (id.equals("16975t") || id.equals("17848t") || id.equals("53054t") || id.equals("19784312t") || id.equals("29669412t") ||
                id.equals("53939512t") || id.equals("541105t") || id.equals("297091t") || id.equals("297083t") || id.equals("297084t")) // Wolf 2/2
            cardurl = "https://cards.scryfall.io/large/front/4/6/462ff49b-a004-4dab-a25b-65cb18c1bbec.jpg";
        else if (id.equals("16933t") || id.equals("476107t")) //Dragon 5/5
            cardurl = "https://cards.scryfall.io/large/front/9/9/993b3b90-74c3-479b-b3e6-3f1cd8f1da04.jpg";
        else if(id.equals("16885t")) //Thopter blue 1/1
            cardurl = "https://cards.scryfall.io/large/front/a/3/a3506ee6-a168-49a4-9814-2858194be60e.jpg";
        else if(id.equals("16847t")) //Angel 4/4
            cardurl = "https://cards.scryfall.io/large/front/1/6/1671013a-2c15-44f0-b4bc-057eb5f727db.jpg";
        else if(id.equals("17656t") || id.equals("17500t") || id.equals("17080t")) //Elemental 3/1
            cardurl = "https://cards.scryfall.io/large/front/d/a/da6283ba-1297-4c7d-8744-f530c04194cd.jpg";
        else if (id.equals("17501t") || id.equals("17494t") || id.equals("17354t") || id.equals("17062t") || id.equals("541103t") ||
                id.equals("541104t") || id.equals("297442t") || id.equals("297432t") || id.equals("297431t")) //Spirit 1/1
            cardurl = "https://cards.scryfall.io/large/front/b/3/b3c9a097-219b-4aaf-831f-cc0cddbcfaae.jpg";
        else if (id.equals("17493t")) //Bear 2/2
            cardurl = "https://cards.scryfall.io/large/front/c/8/c879d4a6-cef5-48f1-8c08-f5b59ec850de.jpg";
        else if (id.equals("473117t")) //Bear 2/2 ELD
            cardurl = "https://cards.scryfall.io/large/front/b/0/b0f09f9e-e0f9-4ed8-bfc0-5f1a3046106e.jpg";
        else if (id.equals("17358t") || id.equals("54047312t")) //Beast 3/3
            cardurl = "https://cards.scryfall.io/large/front/3/f/3fc3a29a-280d-4f2c-9a01-8cfead75f583.jpg";
        else if(id.equals("17207t")) //Sliver 1/1
            cardurl = "https://cards.scryfall.io/large/front/d/e/dec96e95-5580-4110-86ec-561007ab0f1e.jpg";
        else if(id.equals("17071t")) //Cat Warrior 2/2
            cardurl = "https://cards.scryfall.io/large/front/2/9/29c4e4f2-0040-4490-b357-660d729ad9cc.jpg";
        else if(id.equals("17069t")) //Voja 2/2
            cardurl = "https://cards.scryfall.io/large/front/2/8/2879010f-b752-4808-8531-d24e612de0d9.jpg";
        else if(id.equals("17060t") || id.equals("476037t") || id.equals("473092t") || id.equals("473062t")) //Rat 1/1
            cardurl = "https://cards.scryfall.io/large/front/1/a/1a85fe9d-ef18-46c4-88b0-cf2e222e30e4.jpg";
        else if(id.equals("17061t"))
            cardurl = "https://cards.scryfall.io/large/front/a/c/acd51eed-bd5a-417a-811d-fbd1c08a3715.jpg";
        else if(id.equals("17955"))
            cardurl = "https://cards.scryfall.io/large/front/b/8/b86ac828-7b49-4663-a718-99fcac904568.jpg";
        else if(id.equals("476097t") || id.equals("293685t") || id.equals("293652t") || id.equals("296820t") || id.equals("297499t")) //Zombie 2/2
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5bd6905-79be-4d2c-a343-f6e6a181b3e6.jpg";
        else if(id.equals("999901t")) //Monarch Token
            cardurl = "https://cards.scryfall.io/large/front/4/0/40b79918-22a7-4fff-82a6-8ebfe6e87185.jpg";
        else if(id.equals("999902t")) //City's Blessing
            cardurl = "https://cards.scryfall.io/large/front/b/a/ba64ed3e-93c5-406f-a38d-65cc68472122.jpg";
        else if(id.equals("19462t") || id.equals("19463t") || id.equals("19464t") || id.equals("19465t"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2f51f4d-eb6d-4503-b9a4-559db1b9b16f.jpg";
        else if(id.equals("19476t") || id.equals("19477t"))
            cardurl = "https://cards.scryfall.io/large/front/3/4/340fb06f-4bb0-4d23-b08c-8b1da4a8c2ad.jpg";
        else if(id.equals("159127"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1e14cf3a-3c5a-4c22-88d1-1b19660b2e2a.jpg";
        else if(id.equals("159130"))
            cardurl = "https://cards.scryfall.io/large/front/f/4/f4c21c0d-91ee-4c2c-bfa4-81bb07106842.jpg";
        else if(id.equals("159132"))
            cardurl = "https://cards.scryfall.io/large/front/b/0/b03bc922-782b-4254-897c-90d202b4cda4.jpg";
        else if(id.equals("159764"))
            cardurl = "https://cards.scryfall.io/large/front/9/8/98f443cb-55bb-4e83-826a-98261287bfd3.jpg";
        else if(id.equals("159832"))
            cardurl = "https://cards.scryfall.io/large/front/0/b/0b5f694c-11da-41af-9997-0aff93619248.jpg";
        else if(id.equals("159237"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1e76a75a-7125-4957-ab7a-8e7ead21d002.jpg";
        else if(id.equals("159136"))
            cardurl = "https://cards.scryfall.io/large/front/f/a/fa740755-244f-4658-a9e2-aa4cf6742808.jpg";
        else if(id.equals("294381t")) //Bear 4/4
            cardurl = "https://cards.scryfall.io/large/front/c/a/ca3dae7d-3880-4c0a-acfb-8fd227cf9fab.jpg";
        else if(id.equals("294366t")) //Elemental 2/2
            cardurl = "https://cards.scryfall.io/large/front/5/d/5dc134da-51b8-452d-b515-54def56fe0c7.jpg";
        else if (id.equals("294235t") || id.equals("293899t")) // Eldrazi Spawn 0/1
            cardurl = "https://cards.scryfall.io/large/front/7/7/7787eae2-7dfb-44ab-8e92-56fdfc0bb39e.jpg";
        else if(id.equals("293497t")) //Drake 2/2
            cardurl = "https://cards.scryfall.io/large/front/d/c/dcd1cef8-d78a-4bdb-8da0-a50ad199c691.jpg";
        else if(id.equals("476370"))
            cardurl = "https://cards.scryfall.io/large/front/1/4/14b28eae-e8ed-4b99-b6ec-86d0716ec473.jpg";
        else if(id.equals("479417"))
            cardurl = "https://cards.scryfall.io/large/front/e/f/efcbd4ef-3bf4-4f22-9069-2a11c9619a43.jpg";
        else if(id.equals("482713t"))
            cardurl = "https://cards.scryfall.io/large/front/2/9/29d42a00-299d-47d3-ba03-e63812d57931.jpg";
        else if(id.equals("4827131t"))
            cardurl = "https://cards.scryfall.io/large/front/1/d/1dee8c94-cdc8-42b2-a393-0c0c8e439125.jpg";
        else if(id.equals("484902t") || id.equals("484904t"))
            cardurl = "https://cards.scryfall.io/large/front/7/2/720f3e68-84c0-462e-a0d1-90236ccc494a.jpg";
        else if(id.equals("294690t"))
            cardurl = "https://cards.scryfall.io/large/front/0/8/082c3bad-3fea-4c3f-8263-4b16139bb32a.jpg";
        else if(id.equals("47963911t"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f918b740-1984-4090-8886-9e290a698b95.jpg";
        else if(id.equals("479634t"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9cc7c63-5d13-4fd6-af9d-4a26c2bab8e6.jpg";
        else if(id.equals("485469t"))
            cardurl = "https://cards.scryfall.io/large/front/4/3/4306be80-d7c9-4bcf-a3de-4bf159475546.jpg";
        else if(id.equals("489663"))
            cardurl = "https://cards.scryfall.io/large/front/d/6/d605c780-a42a-4816-8fb9-63e3114a8246.jpg";
        else if(id.equals("48966310t"))
            cardurl = "https://cards.scryfall.io/large/front/f/b/fbdf8dc1-1b10-4fce-97b9-1f5600500cc1.jpg";
        else if(id.equals("48966311t"))
            cardurl = "https://cards.scryfall.io/large/front/4/f/4f8107b3-8539-4b9c-8d0d-c512c940838f.jpg";
        else if(id.equals("489987t"))
            cardurl = "https://cards.scryfall.io/large/front/f/b/fb248ba0-2ee7-4994-be57-2bcc8df29680.jpg";
        else if(id.equals("489822t"))
            cardurl = "https://cards.scryfall.io/large/front/c/7/c7e7822b-f155-4f3f-b835-ec64f3a71307.jpg";
        else if(id.equals("489930t"))
            cardurl = "https://cards.scryfall.io/large/front/7/9/791f5fa0-f972-455e-9802-ff299853607f.jpg";
        else if(id.equals("491334"))
            cardurl = "https://cards.scryfall.io/large/front/9/6/96d1a254-01a8-4590-8878-690c5bfb4a95.jpg";
        else if(id.equals("491335"))
            cardurl = "https://cards.scryfall.io/large/front/e/c/ec386bc3-137b-49b5-8380-8daff470f0bc.jpg";
        else if(id.equals("491344"))
            cardurl = "https://cards.scryfall.io/large/front/d/b/db149aaa-3da9-48c4-92cc-b3d804285290.jpg";
        else if(id.equals("491345"))
            cardurl = "https://cards.scryfall.io/large/front/3/2/32301593-f16a-4a46-8a4e-eecedd2a9013.jpg";
        else if(id.equals("491346"))
            cardurl = "https://cards.scryfall.io/large/front/e/b/eb49805c-8546-463d-b78c-f4ea109851b4.jpg";
        else if(id.equals("491347"))
            cardurl = "https://cards.scryfall.io/large/front/d/d/dd595e2f-65e4-46e8-9d28-f94ac308b275.jpg";
        else if(id.equals("491348"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f9baef6e-a086-41d4-a20e-486f01d72406.jpg";
        else if(id.equals("491349"))
            cardurl = "https://cards.scryfall.io/large/front/e/c/ec136ce7-bad4-4ebb-ab00-b86de3d209a7.jpg";
        else if(id.equals("491350"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/cacaf5ec-6745-4584-9175-36c98742958f.jpg";
        else if(id.equals("491351"))
            cardurl = "https://cards.scryfall.io/large/front/6/c/6c821158-f71a-48f9-b6b4-b0e605f22bec.jpg";
        else if(id.equals("491352"))
            cardurl = "https://cards.scryfall.io/large/front/b/9/b9a50516-a20f-4e6e-b4f2-0049b673f942.jpg";
        else if(id.equals("491353"))
            cardurl = "https://cards.scryfall.io/large/front/9/5/95702503-8f2d-46c8-abdb-6edd6c431d19.jpg";
        else if(id.equals("491354"))
            cardurl = "https://cards.scryfall.io/large/front/7/3/73731e45-51bb-4188-a54d-fdaa4bdfaf1f.jpg";
        else if(id.equals("491355"))
            cardurl = "https://cards.scryfall.io/large/front/d/3/d35575d0-0b10-4d1b-b5a2-a9f36f9eada4.jpg";
        else if(id.equals("491356"))
            cardurl = "https://cards.scryfall.io/large/front/6/2/62d2058c-3f20-4566-b366-93a2cbbe682f.jpg";
        else if(id.equals("491357"))
            cardurl = "https://cards.scryfall.io/large/front/5/b/5b8c11ba-533d-48c9-821c-3fec846bca97.jpg";
        else if(id.equals("491358"))
            cardurl = "https://cards.scryfall.io/large/front/1/8/187abedf-c2eb-453b-bea0-a10afa399e03.jpg";
        else if(id.equals("491359"))
            cardurl = "https://cards.scryfall.io/large/front/2/0/20c9c856-af15-40b1-a799-1c2066df2099.jpg";
        else if(id.equals("491360"))
            cardurl = "https://cards.scryfall.io/large/front/1/c/1c3fc61c-c26e-47f3-a1eb-f6f10f8469e2.jpg";
        else if(id.equals("491361"))
            cardurl = "https://cards.scryfall.io/large/front/8/0/8008977f-b164-4ab7-a38a-25b382c6a16f.jpg";
        else if(id.equals("491362"))
            cardurl = "https://cards.scryfall.io/large/front/d/a/dac080ef-8f40-43a2-8440-b457b6074b69.jpg";
        else if(id.equals("491363"))
            cardurl = "https://cards.scryfall.io/large/front/8/6/86f670f9-c5b7-4eb0-a7d0-d16513fadf74.jpg";
        else if(id.equals("491364"))
            cardurl = "https://cards.scryfall.io/large/front/3/c/3c9847f3-5a4c-4b49-8e25-e444d1446bf9.jpg";
        else if(id.equals("491365"))
            cardurl = "https://cards.scryfall.io/large/front/2/a/2abc5ac8-b944-4b71-b022-c78183eb92c3.jpg";
        else if(id.equals("491366"))
            cardurl = "https://cards.scryfall.io/large/front/0/c/0ccd5597-2d4e-4f3e-94b7-46783486853a.jpg";
        else if(id.equals("491367"))
            cardurl = "https://cards.scryfall.io/large/front/0/e/0e94d334-e043-42f2-ba5c-be497d82f2c8.jpg";
        else if(id.equals("491368"))
            cardurl = "https://cards.scryfall.io/large/front/8/b/8bc6178b-16e7-4089-974f-7048b9632fc2.jpg";
        else if(id.equals("491369"))
            cardurl = "https://cards.scryfall.io/large/front/7/0/70eab734-875b-4b76-901b-3ac7d2133ad9.jpg";
        else if(id.equals("491370"))
            cardurl = "https://cards.scryfall.io/large/front/6/d/6d7cd274-ed83-475a-9b4f-adb9c780a6f4.jpg";
        else if(id.equals("491371"))
            cardurl = "https://cards.scryfall.io/large/front/5/4/547e3aa5-d88a-4418-ab9d-dd65385f031b.jpg";
        else if(id.equals("491372"))
            cardurl = "https://cards.scryfall.io/large/front/5/c/5cc0b4eb-8ee9-4213-8194-02e7d63428d3.jpg";
        else if(id.equals("491373"))
            cardurl = "https://cards.scryfall.io/large/front/0/a/0a469d00-1416-48dd-ad91-eb6f3fb4b42b.jpg";
        else if(id.equals("491374"))
            cardurl = "https://cards.scryfall.io/large/front/2/2/22cd80dd-1c57-423c-81e2-9a956901565f.jpg";
        else if(id.equals("491375"))
            cardurl = "https://cards.scryfall.io/large/front/3/a/3a195efe-8c4f-479d-bd0f-563ee4bb49a1.jpg";
        else if(id.equals("491376"))
            cardurl = "https://cards.scryfall.io/large/front/9/3/93c0681d-97da-4363-b75a-079c209e7e4a.jpg";
        else if(id.equals("491377"))
            cardurl = "https://cards.scryfall.io/large/front/f/0/f097accb-28ad-4b22-b615-103c74e07708.jpg";
        else if(id.equals("491378"))
            cardurl = "https://cards.scryfall.io/large/front/6/9/69e55604-56da-44b5-aa78-f5de76ce9d20.jpg";
        else if(id.equals("491379"))
            cardurl = "https://cards.scryfall.io/large/front/5/4/546e0452-5304-41fa-9e3a-a3fa5a571315.jpg";
        else if(id.equals("491380"))
            cardurl = "https://cards.scryfall.io/large/front/9/3/936335d7-1c4a-4fcd-80ff-cd4d4fcab8c4.jpg";
        else if(id.equals("491381"))
            cardurl = "https://cards.scryfall.io/large/front/c/7/c75672e0-fa2d-43c5-9381-e17f2fd6d3bc.jpg";
        else if(id.equals("491377t"))
            cardurl = "https://cards.scryfall.io/large/front/a/6/a6ee0db9-ac89-4ab6-ac2e-8a7527d9ecbd.jpg";
        else if(id.equals("491372t"))
            cardurl = "https://cards.scryfall.io/large/front/c/f/cf371056-43dd-41ab-8d05-b16a8bdc8d28.jpg";
        else if(id.equals("491365t"))
            cardurl = "https://cards.scryfall.io/large/front/9/e/9ecc467e-b345-446c-b9b7-5f164e6651a4.jpg";
        else if(id.equals("295116t") || id.equals("295103t"))
            cardurl = "https://cards.scryfall.io/large/front/2/d/2d1446ed-f114-421d-bb60-9aeb655e5adb.jpg";
        else if(id.equals("295077t"))
            cardurl = "https://cards.scryfall.io/large/front/6/a/6aaa8539-8d21-4da1-8410-d4354078390f.jpg";
        else if(id.equals("295041t"))
            cardurl = "https://cards.scryfall.io/large/front/1/a/1aea5e0b-dc4e-4055-9e13-1dfbc25a2f00.jpg";
        else if(id.equals("294952t") || id.equals("294950t"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5bd6905-79be-4d2c-a343-f6e6a181b3e6.jpg";
        else if(id.equals("491633t"))
            cardurl = "https://cards.scryfall.io/large/front/9/1/91098481-46c2-49bf-8123-e9cab2f22b84.jpg";
        else if(id.equals("491633"))
            cardurl = "https://cards.scryfall.io/large/front/c/4/c470539a-9cc7-4175-8f7c-c982b6072b6d.jpg";
        else if(id.equals("491634"))
            cardurl = "https://cards.scryfall.io/large/back/c/4/c470539a-9cc7-4175-8f7c-c982b6072b6d.jpg";
        else if(id.equals("491641"))
            cardurl = "https://cards.scryfall.io/large/front/3/6/366e9845-019d-47cc-adb8-8fbbaad35b6d.jpg";
        else if(id.equals("491642"))
            cardurl = "https://cards.scryfall.io/large/back/3/6/366e9845-019d-47cc-adb8-8fbbaad35b6d.jpg";
        else if(id.equals("491649"))
            cardurl = "https://cards.scryfall.io/large/front/a/d/ada9a974-8f1f-4148-bd61-200fc14714b2.jpg";
        else if(id.equals("491650"))
            cardurl = "https://cards.scryfall.io/large/back/a/d/ada9a974-8f1f-4148-bd61-200fc14714b2.jpg";
        else if(id.equals("491654"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6e6be8c-41c3-4348-a8dd-b40ceb24e9b4.jpg";
        else if(id.equals("491655"))
            cardurl = "https://cards.scryfall.io/large/back/b/6/b6e6be8c-41c3-4348-a8dd-b40ceb24e9b4.jpg";
        else if(id.equals("491662"))
            cardurl = "https://cards.scryfall.io/large/front/f/2/f25d56f9-aa54-4657-9ac9-e93fbba3e715.jpg";
        else if(id.equals("491663"))
            cardurl = "https://cards.scryfall.io/large/back/f/2/f25d56f9-aa54-4657-9ac9-e93fbba3e715.jpg";
        else if(id.equals("491666"))
            cardurl = "https://cards.scryfall.io/large/front/0/1/014027c4-7f9d-4096-b308-ea4be574c0d4.jpg";
        else if(id.equals("491667"))
            cardurl = "https://cards.scryfall.io/large/back/0/1/014027c4-7f9d-4096-b308-ea4be574c0d4.jpg";
        else if(id.equals("491673"))
            cardurl = "https://cards.scryfall.io/large/front/5/f/5f411f08-45dd-4d73-8894-daf51c175150.jpg";
        else if(id.equals("491674"))
            cardurl = "https://cards.scryfall.io/large/back/5/f/5f411f08-45dd-4d73-8894-daf51c175150.jpg";
        else if(id.equals("491688"))
            cardurl = "https://cards.scryfall.io/large/front/5/a/5adcb500-8c77-4925-8e2c-1243502827d1.jpg";
        else if(id.equals("491689"))
            cardurl = "https://cards.scryfall.io/large/back/5/a/5adcb500-8c77-4925-8e2c-1243502827d1.jpg";
        else if(id.equals("491693"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/301750a7-d1fd-435e-bfa8-9d2fb22ad627.jpg";
        else if(id.equals("491694"))
            cardurl = "https://cards.scryfall.io/large/back/3/0/301750a7-d1fd-435e-bfa8-9d2fb22ad627.jpg";
        else if(id.equals("491706"))
            cardurl = "https://cards.scryfall.io/large/front/1/9/193071fe-180b-4d35-ba78-9c16675c29fc.jpg";
        else if(id.equals("491707"))
            cardurl = "https://cards.scryfall.io/large/back/1/9/193071fe-180b-4d35-ba78-9c16675c29fc.jpg";
        else if(id.equals("491711"))
            cardurl = "https://cards.scryfall.io/large/front/1/1/11568cdf-6148-494c-8b98-f5ca5797d775.jpg";
        else if(id.equals("491712"))
            cardurl = "https://cards.scryfall.io/large/back/1/1/11568cdf-6148-494c-8b98-f5ca5797d775.jpg";
        else if(id.equals("491718"))
            cardurl = "https://cards.scryfall.io/large/front/8/9/890eee8d-a339-4143-adfa-1b17ec10c099.jpg";
        else if(id.equals("491719"))
            cardurl = "https://cards.scryfall.io/large/back/8/9/890eee8d-a339-4143-adfa-1b17ec10c099.jpg";
        else if(id.equals("491723"))
            cardurl = "https://cards.scryfall.io/large/front/6/7/67f4c93b-080c-4196-b095-6a120a221988.jpg";
        else if(id.equals("491724"))
            cardurl = "https://cards.scryfall.io/large/back/6/7/67f4c93b-080c-4196-b095-6a120a221988.jpg";
        else if(id.equals("491725"))
            cardurl = "https://cards.scryfall.io/large/front/3/2/32779721-b021-4bd4-95d1-4a19b78d9faa.jpg";
        else if(id.equals("491726"))
            cardurl = "https://cards.scryfall.io/large/back/3/2/32779721-b021-4bd4-95d1-4a19b78d9faa.jpg";
        else if(id.equals("491741"))
            cardurl = "https://cards.scryfall.io/large/front/7/c/7c04c734-354d-4925-8161-7052110951df.jpg";
        else if(id.equals("491742"))
            cardurl = "https://cards.scryfall.io/large/back/7/c/7c04c734-354d-4925-8161-7052110951df.jpg";
        else if(id.equals("491747"))
            cardurl = "https://cards.scryfall.io/large/front/6/0/609d3ecf-f88d-4268-a8d3-4bf2bcf5df60.jpg";
        else if(id.equals("491748"))
            cardurl = "https://cards.scryfall.io/large/back/6/0/609d3ecf-f88d-4268-a8d3-4bf2bcf5df60.jpg";
        else if(id.equals("491757"))
            cardurl = "https://cards.scryfall.io/large/front/e/6/e63f8b20-f45b-4293-9aac-cdc021939be6.jpg";
        else if(id.equals("491758"))
            cardurl = "https://cards.scryfall.io/large/back/e/6/e63f8b20-f45b-4293-9aac-cdc021939be6.jpg";
        else if(id.equals("491770"))
            cardurl = "https://cards.scryfall.io/large/front/9/8/98496d5b-1519-4f0c-8b46-0a43be643dfb.jpg";
        else if(id.equals("491771"))
            cardurl = "https://cards.scryfall.io/large/back/9/8/98496d5b-1519-4f0c-8b46-0a43be643dfb.jpg";
        else if(id.equals("491773"))
            cardurl = "https://cards.scryfall.io/large/front/d/8/d8ed0335-daa6-4dbe-a94d-4d56c8cfd093.jpg";
        else if(id.equals("491774"))
            cardurl = "https://cards.scryfall.io/large/back/d/8/d8ed0335-daa6-4dbe-a94d-4d56c8cfd093.jpg";
        else if(id.equals("491786"))
            cardurl = "https://cards.scryfall.io/large/front/7/5/75240bbc-adc7-48ff-9523-c79776d710d3.jpg";
        else if(id.equals("491787"))
            cardurl = "https://cards.scryfall.io/large/back/7/5/75240bbc-adc7-48ff-9523-c79776d710d3.jpg";
        else if(id.equals("491802"))
            cardurl = "https://cards.scryfall.io/large/front/b/c/bc7239ea-f8aa-4a6f-87bd-c35359635673.jpg";
        else if(id.equals("491803"))
            cardurl = "https://cards.scryfall.io/large/back/b/c/bc7239ea-f8aa-4a6f-87bd-c35359635673.jpg";
        else if(id.equals("491807"))
            cardurl = "https://cards.scryfall.io/large/front/7/8/782ca27f-9f18-476c-b582-89c06fb2e322.jpg";
        else if(id.equals("491808"))
            cardurl = "https://cards.scryfall.io/large/back/7/8/782ca27f-9f18-476c-b582-89c06fb2e322.jpg";
        else if(id.equals("491809"))
            cardurl = "https://cards.scryfall.io/large/front/a/6/a69541db-3f4e-412f-aa8e-dec1e74f74dc.jpg";
        else if(id.equals("491810"))
            cardurl = "https://cards.scryfall.io/large/back/a/6/a69541db-3f4e-412f-aa8e-dec1e74f74dc.jpg";
        else if(id.equals("491818"))
            cardurl = "https://cards.scryfall.io/large/front/2/2/228e551e-023a-4c9a-8f32-58dae6ffdf7f.jpg";
        else if(id.equals("491819"))
            cardurl = "https://cards.scryfall.io/large/back/2/2/228e551e-023a-4c9a-8f32-58dae6ffdf7f.jpg";
        else if(id.equals("491825"))
            cardurl = "https://cards.scryfall.io/large/front/c/5/c5cb3052-358d-44a7-8cfd-cd31b236494a.jpg";
        else if(id.equals("491826"))
            cardurl = "https://cards.scryfall.io/large/back/c/5/c5cb3052-358d-44a7-8cfd-cd31b236494a.jpg";
        else if(id.equals("491835"))
            cardurl = "https://cards.scryfall.io/large/front/2/f/2f632537-63bf-4490-86e6-e6067b9c1a3b.jpg";
        else if(id.equals("491836"))
            cardurl = "https://cards.scryfall.io/large/back/2/f/2f632537-63bf-4490-86e6-e6067b9c1a3b.jpg";
        else if(id.equals("491839"))
            cardurl = "https://cards.scryfall.io/large/front/9/9/99535539-aa73-41ed-86ab-21c97b92620d.jpg";
        else if(id.equals("491840"))
            cardurl = "https://cards.scryfall.io/large/back/9/9/99535539-aa73-41ed-86ab-21c97b92620d.jpg";
        else if(id.equals("491859"))
            cardurl = "https://cards.scryfall.io/large/front/2/3/235d1ffc-72aa-40a2-95dc-3f6a8d495061.jpg";
        else if(id.equals("491860"))
            cardurl = "https://cards.scryfall.io/large/back/2/3/235d1ffc-72aa-40a2-95dc-3f6a8d495061.jpg";
        else if(id.equals("491864"))
            cardurl = "https://cards.scryfall.io/large/front/6/1/61bd69ea-1e9e-46b0-b1a1-ed7fdbe3deb6.jpg";
        else if(id.equals("491865"))
            cardurl = "https://cards.scryfall.io/large/back/6/1/61bd69ea-1e9e-46b0-b1a1-ed7fdbe3deb6.jpg";
        else if(id.equals("491866"))
            cardurl = "https://cards.scryfall.io/large/front/3/a/3a7fd24e-84d8-405d-86e4-0571a9e23cc2.jpg";
        else if(id.equals("491867"))
            cardurl = "https://cards.scryfall.io/large/back/3/a/3a7fd24e-84d8-405d-86e4-0571a9e23cc2.jpg";
        else if(id.equals("491909"))
            cardurl = "https://cards.scryfall.io/large/front/0/5/0511e232-2a72-40f5-a400-4f7ebc442d17.jpg";
        else if(id.equals("491910"))
            cardurl = "https://cards.scryfall.io/large/back/0/5/0511e232-2a72-40f5-a400-4f7ebc442d17.jpg";
        else if(id.equals("491911"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d24c3d51-795d-4c01-a34a-3280fccd2d78.jpg";
        else if(id.equals("491912"))
            cardurl = "https://cards.scryfall.io/large/back/d/2/d24c3d51-795d-4c01-a34a-3280fccd2d78.jpg";
        else if(id.equals("491913"))
            cardurl = "https://cards.scryfall.io/large/front/b/4/b4b99ebb-0d54-4fe5-a495-979aaa564aa8.jpg";
        else if(id.equals("491914"))
            cardurl = "https://cards.scryfall.io/large/back/b/4/b4b99ebb-0d54-4fe5-a495-979aaa564aa8.jpg";
        else if(id.equals("491915"))
            cardurl = "https://cards.scryfall.io/large/front/d/a/da57eb54-5199-4a56-95f7-f6ac432876b1.jpg";
        else if(id.equals("491916"))
            cardurl = "https://cards.scryfall.io/large/back/d/a/da57eb54-5199-4a56-95f7-f6ac432876b1.jpg";
        else if(id.equals("491918"))
            cardurl = "https://cards.scryfall.io/large/front/6/5/6559047e-6ede-4815-a3a0-389062094f9d.jpg";
        else if(id.equals("491919"))
            cardurl = "https://cards.scryfall.io/large/back/6/5/6559047e-6ede-4815-a3a0-389062094f9d.jpg";
        else if(id.equals("491920"))
            cardurl = "https://cards.scryfall.io/large/front/2/6/2668ac91-6cda-4f81-a08d-4fc5f9cb35b2.jpg";
        else if(id.equals("491921"))
            cardurl = "https://cards.scryfall.io/large/back/2/6/2668ac91-6cda-4f81-a08d-4fc5f9cb35b2.jpg";
        else if(id.equals("495098"))
            cardurl = "https://cards.scryfall.io/large/front/a/e/ae92e656-6c9d-48c3-a238-5a11c2c62ec8.jpg";
        else if(id.equals("495099"))
            cardurl = "https://cards.scryfall.io/large/front/5/8/589a324f-4466-4d4a-8cfb-806a041d7c1f.jpg";
        else if(id.equals("495100"))
            cardurl = "https://cards.scryfall.io/large/front/1/9/1967d4a8-6cc4-4a4d-9d24-93257de35e6d.jpg";
        else if(id.equals("495101"))
            cardurl = "https://cards.scryfall.io/large/front/3/c/3c6a38dd-e8f5-420f-9576-66937c71286b.jpg";
        else if(id.equals("495102"))
            cardurl = "https://cards.scryfall.io/large/front/2/b/2b90e88b-60a3-4d1d-bb8c-14633e5005a5.jpg";
        else if(id.equals("29530711"))
            cardurl = "https://cards.scryfall.io/large/front/4/9/4912a0a5-2fec-4c6b-9545-9ab0c4e25268.jpg";
        else if(id.equals("1750411"))
            cardurl = "https://cards.scryfall.io/large/front/8/f/8f047a8b-6c94-4b99-bcaa-10680400ee25.jpg";
        else if(id.equals("5176911"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cbbd8a12-d916-4fb1-994a-7d4a3e2ae2ab.jpg";
        else if(id.equals("44680711"))
            cardurl = "https://cards.scryfall.io/large/front/a/0/a00a7180-49bd-4ead-852a-67b6b5e4b933.jpg";
        else if(id.equals("295726t") || id.equals("295673t") || id.equals("295532t")) //Servo 1/1
            cardurl = "https://cards.scryfall.io/large/front/d/7/d79e2bf1-d26d-4be3-a5ad-a43346ed445a.jpg";
        else if(id.equals("295632t"))
            cardurl = "https://cards.scryfall.io/large/front/1/e/1ebc91a9-23e0-4ca1-bc6d-e710ad2efb31.jpg";
        else if(id.equals("295802"))
            cardurl = "https://cards.scryfall.io/large/front/4/c/4cb8d03e-e1d2-451e-97a8-141082f92501.jpg";
        else if(id.equals("497724t"))
            cardurl = "https://cards.scryfall.io/large/front/6/6/661cbde4-9444-4259-b2cf-7c8f9814a071.jpg";
        else if(id.equals("295810t"))
            cardurl = "https://cards.scryfall.io/large/front/4/5/458f44dd-83f1-497e-b5d0-e3417eb9dfec.jpg";
        else if(id.equals("476226"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/caa7922e-3313-4f12-b91e-95aaa2d76cc2.jpg";
        else if(id.equals("476217"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9a6cf9c-3560-435c-b0ec-8653a9dc7776.jpg";
        else if(id.equals("503619"))
            cardurl = "https://cards.scryfall.io/large/front/9/7/97502411-5c93-434c-b77b-ceb2c32feae7.jpg";
        else if(id.equals("503620"))
            cardurl = "https://cards.scryfall.io/large/back/9/7/97502411-5c93-434c-b77b-ceb2c32feae7.jpg";
        else if(id.equals("503626"))
            cardurl = "https://cards.scryfall.io/large/front/3/6/3606519e-5677-4c21-a34e-be195b6669fa.jpg";
        else if(id.equals("503627"))
            cardurl = "https://cards.scryfall.io/large/back/3/6/3606519e-5677-4c21-a34e-be195b6669fa.jpg";
        else if(id.equals("503646"))
            cardurl = "https://cards.scryfall.io/large/front/5/d/5d131784-c1a3-463e-a37b-b720af67ab62.jpg";
        else if(id.equals("503647"))
            cardurl = "https://cards.scryfall.io/large/back/5/d/5d131784-c1a3-463e-a37b-b720af67ab62.jpg";
        else if(id.equals("503657"))
            cardurl = "https://cards.scryfall.io/large/front/f/a/fab2fca4-a99f-4ffe-9c02-edb6e0be2358.jpg";
        else if(id.equals("503658"))
            cardurl = "https://cards.scryfall.io/large/back/f/a/fab2fca4-a99f-4ffe-9c02-edb6e0be2358.jpg";
        else if(id.equals("503700"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9dfdb73d-b001-4a59-b79e-8c8c1baea116.jpg";
        else if(id.equals("503701"))
            cardurl = "https://cards.scryfall.io/large/back/9/d/9dfdb73d-b001-4a59-b79e-8c8c1baea116.jpg";
        else if(id.equals("503721"))
            cardurl = "https://cards.scryfall.io/large/front/1/4/14dc88ee-bba9-4625-af0d-89f3762a0ead.jpg";
        else if(id.equals("503722"))
            cardurl = "https://cards.scryfall.io/large/back/1/4/14dc88ee-bba9-4625-af0d-89f3762a0ead.jpg";
        else if(id.equals("503724"))
            cardurl = "https://cards.scryfall.io/large/front/e/a/ea7e4c65-b4c4-4795-9475-3cba71c50ea5.jpg";
        else if(id.equals("503725"))
            cardurl = "https://cards.scryfall.io/large/back/e/a/ea7e4c65-b4c4-4795-9475-3cba71c50ea5.jpg";
        else if(id.equals("503734"))
            cardurl = "https://cards.scryfall.io/large/front/4/4/44657ab1-0a6a-4a5f-9688-86f239083821.jpg";
        else if(id.equals("503735"))
            cardurl = "https://cards.scryfall.io/large/back/4/4/44657ab1-0a6a-4a5f-9688-86f239083821.jpg";
        else if(id.equals("503766"))
            cardurl = "https://cards.scryfall.io/large/front/2/2/22a6a5f1-1405-4efb-af3e-e1f58d664e99.jpg";
        else if(id.equals("503767"))
            cardurl = "https://cards.scryfall.io/large/back/2/2/22a6a5f1-1405-4efb-af3e-e1f58d664e99.jpg";
        else if(id.equals("503781"))
            cardurl = "https://cards.scryfall.io/large/front/f/6/f6cd7465-9dd0-473c-ac5e-dd9e2f22f5f6.jpg";
        else if(id.equals("503782"))
            cardurl = "https://cards.scryfall.io/large/back/f/6/f6cd7465-9dd0-473c-ac5e-dd9e2f22f5f6.jpg";
        else if(id.equals("503793"))
            cardurl = "https://cards.scryfall.io/large/front/c/6/c697548f-925b-405e-970a-4e78067d5c8e.jpg";
        else if(id.equals("503794"))
            cardurl = "https://cards.scryfall.io/large/back/c/6/c697548f-925b-405e-970a-4e78067d5c8e.jpg";
        else if(id.equals("503796"))
            cardurl = "https://cards.scryfall.io/large/front/b/7/b76bed98-30b1-4572-b36c-684ada06826c.jpg";
        else if(id.equals("503797"))
            cardurl = "https://cards.scryfall.io/large/back/b/7/b76bed98-30b1-4572-b36c-684ada06826c.jpg";
        else if(id.equals("503867"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6de14ae-0132-4261-af00-630bf15918cd.jpg";
        else if(id.equals("503868"))
            cardurl = "https://cards.scryfall.io/large/back/b/6/b6de14ae-0132-4261-af00-630bf15918cd.jpg";
        else if(id.equals("503869"))
            cardurl = "https://cards.scryfall.io/large/front/0/c/0ce39a19-f51d-4a35-ae80-5b82eb15fcff.jpg";
        else if(id.equals("503870"))
            cardurl = "https://cards.scryfall.io/large/back/0/c/0ce39a19-f51d-4a35-ae80-5b82eb15fcff.jpg";
        else if(id.equals("503872"))
            cardurl = "https://cards.scryfall.io/large/front/8/7/87a4e5fe-161f-42da-9ca2-67c8e8970e94.jpg";
        else if(id.equals("503873"))
            cardurl = "https://cards.scryfall.io/large/back/8/7/87a4e5fe-161f-42da-9ca2-67c8e8970e94.jpg";
        else if(id.equals("503879"))
            cardurl = "https://cards.scryfall.io/large/front/7/e/7ef37cb3-d803-47d7-8a01-9c803aa2eadc.jpg";
        else if(id.equals("503880"))
            cardurl = "https://cards.scryfall.io/large/back/7/e/7ef37cb3-d803-47d7-8a01-9c803aa2eadc.jpg";
        else if(id.equals("503837t"))
            cardurl = "https://cards.scryfall.io/large/front/5/4/54a1c6a9-3531-4432-9157-e4400dbc89fd.jpg";
        else if(id.equals("503841t"))
            cardurl = "https://cards.scryfall.io/large/front/d/f/df826c7d-5508-4e21-848c-91bc3e3f447a.jpg";
        else if(id.equals("473148"))
            cardurl = "https://cards.scryfall.io/large/front/5/d/5dca90ef-1c17-4dcc-9fef-dab9ee92f590.jpg";
        else if(id.equals("473127t"))
            cardurl = "https://cards.scryfall.io/large/front/9/4/94057dc6-e589-4a29-9bda-90f5bece96c4.jpg";
        else if(id.equals("295910t"))
            cardurl = "https://cards.scryfall.io/large/front/7/b/7b993828-e139-4cb6-a329-487accc1c515.jpg";
        else if(id.equals("296315t"))
            cardurl = "https://cards.scryfall.io/large/front/e/d/ed58cd8c-b11a-4109-b789-0eb92eaf0184.jpg";
        else if(id.equals("296247t"))
            cardurl = "https://cards.scryfall.io/large/front/0/7/07027a7c-5843-4d78-9b86-8799363c0b82.jpg";
        else if(id.equals("296217t"))
            cardurl = "https://cards.scryfall.io/large/front/e/7/e72daa68-0680-431c-a616-b3693fd58813.jpg";
        else if(id.equals("296145t"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cbcb0668-e88c-4462-b079-34f140c0277e.jpg";
        else if(id.equals("295986t"))
            cardurl = "https://cards.scryfall.io/large/front/4/a/4a2144f2-d4be-419e-bfca-116cedfdf18b.jpg";
        else if(id.equals("518429t"))
            cardurl = "https://cards.scryfall.io/large/front/f/6/f62080da-a11b-4da3-bb8f-57f543bf076a.jpg";
        else if(id.equals("513482"))
            cardurl = "https://cards.scryfall.io/large/front/1/8/18a2bdc8-b705-4eb5-b3a5-ff2e2ab8f312.jpg";
        else if(id.equals("513483"))
            cardurl = "https://cards.scryfall.io/large/back/1/8/18a2bdc8-b705-4eb5-b3a5-ff2e2ab8f312.jpg";
        else if(id.equals("513624"))
            cardurl = "https://cards.scryfall.io/large/front/d/9/d9131fc3-018a-4975-8795-47be3956160d.jpg";
        else if(id.equals("513625"))
            cardurl = "https://cards.scryfall.io/large/back/d/9/d9131fc3-018a-4975-8795-47be3956160d.jpg";
        else if(id.equals("513626"))
            cardurl = "https://cards.scryfall.io/large/front/c/2/c204b7ca-0904-40fa-b20c-92400fae20b8.jpg";
        else if(id.equals("513627"))
            cardurl = "https://cards.scryfall.io/large/back/c/2/c204b7ca-0904-40fa-b20c-92400fae20b8.jpg";
        else if(id.equals("513628"))
            cardurl = "https://cards.scryfall.io/large/front/b/a/ba09360a-067e-48a5-bdc5-a19fd066a785.jpg";
        else if(id.equals("513629"))
            cardurl = "https://cards.scryfall.io/large/back/b/a/ba09360a-067e-48a5-bdc5-a19fd066a785.jpg";
        else if(id.equals("513630"))
            cardurl = "https://cards.scryfall.io/large/front/0/d/0dba25e3-2b4f-45d4-965f-3834bcb359ee.jpg";
        else if(id.equals("513631"))
            cardurl = "https://cards.scryfall.io/large/back/0/d/0dba25e3-2b4f-45d4-965f-3834bcb359ee.jpg";
        else if(id.equals("513632"))
            cardurl = "https://cards.scryfall.io/large/front/d/7/d7148d24-373e-4485-860b-c3429c2337f2.jpg";
        else if(id.equals("513633"))
            cardurl = "https://cards.scryfall.io/large/back/d/7/d7148d24-373e-4485-860b-c3429c2337f2.jpg";
        else if(id.equals("513634"))
            cardurl = "https://cards.scryfall.io/large/front/8/b/8b45dc40-6827-46a7-a9b7-802be698d053.jpg";
        else if(id.equals("513635"))
            cardurl = "https://cards.scryfall.io/large/back/8/b/8b45dc40-6827-46a7-a9b7-802be698d053.jpg";
        else if(id.equals("513636"))
            cardurl = "https://cards.scryfall.io/large/front/8/e/8e4e0f81-f92b-4a3a-bb29-adcc3de211b4.jpg";
        else if(id.equals("513637"))
            cardurl = "https://cards.scryfall.io/large/back/8/e/8e4e0f81-f92b-4a3a-bb29-adcc3de211b4.jpg";
        else if(id.equals("513638"))
            cardurl = "https://cards.scryfall.io/large/front/a/a/aaa1e6be-08cc-4ccc-b2de-3511613e4fd0.jpg";
        else if(id.equals("513639"))
            cardurl = "https://cards.scryfall.io/large/back/a/a/aaa1e6be-08cc-4ccc-b2de-3511613e4fd0.jpg";
        else if(id.equals("513640"))
            cardurl = "https://cards.scryfall.io/large/front/5/b/5bd9b5cf-f018-48af-a081-995ce8ecc539.jpg";
        else if(id.equals("513641"))
            cardurl = "https://cards.scryfall.io/large/back/5/b/5bd9b5cf-f018-48af-a081-995ce8ecc539.jpg";
        else if(id.equals("513642"))
            cardurl = "https://cards.scryfall.io/large/front/1/8/18c16872-3675-4a4d-962a-2e17ad6f3886.jpg";
        else if(id.equals("513643"))
            cardurl = "https://cards.scryfall.io/large/back/1/8/18c16872-3675-4a4d-962a-2e17ad6f3886.jpg";
        else if(id.equals("513644"))
            cardurl = "https://cards.scryfall.io/large/front/8/9/8982ff88-8595-4363-8cde-6e87fb57a2d8.jpg";
        else if(id.equals("513645"))
            cardurl = "https://cards.scryfall.io/large/back/8/9/8982ff88-8595-4363-8cde-6e87fb57a2d8.jpg";
        else if(id.equals("513646"))
            cardurl = "https://cards.scryfall.io/large/front/9/3/938cee8f-ac2c-49a5-9ff7-1367d0edfabe.jpg";
        else if(id.equals("513647"))
            cardurl = "https://cards.scryfall.io/large/back/9/3/938cee8f-ac2c-49a5-9ff7-1367d0edfabe.jpg";
        else if(id.equals("513648"))
            cardurl = "https://cards.scryfall.io/large/front/8/7/87463b68-3642-41c7-a11c-67d524759b60.jpg";
        else if(id.equals("513649"))
            cardurl = "https://cards.scryfall.io/large/back/8/7/87463b68-3642-41c7-a11c-67d524759b60.jpg";
        else if(id.equals("513650"))
            cardurl = "https://cards.scryfall.io/large/front/8/c/8cfd0887-0c83-4b33-a85e-8b8ec5bf758d.jpg";
        else if(id.equals("513651"))
            cardurl = "https://cards.scryfall.io/large/back/8/c/8cfd0887-0c83-4b33-a85e-8b8ec5bf758d.jpg";
        else if(id.equals("513652"))
            cardurl = "https://cards.scryfall.io/large/front/6/5/65008352-bc7e-40b2-a832-b46813e5dc4c.jpg";
        else if(id.equals("513653"))
            cardurl = "https://cards.scryfall.io/large/back/6/5/65008352-bc7e-40b2-a832-b46813e5dc4c.jpg";
        else if(id.equals("513652t") || id.equals("513638t") || id.equals("513543t"))
            cardurl = "https://cards.scryfall.io/large/front/d/0/d0ddbe3e-4a66-494d-9304-7471232549bf.jpg";
        else if(id.equals("513634t"))
            cardurl = "https://cards.scryfall.io/large/front/9/1/910f48ab-b04e-4874-b31d-a86a7bc5af14.jpg";
        else if(id.equals("296380t")) // Construct */*
            cardurl = "https://cards.scryfall.io/large/front/c/5/c5eafa38-5333-4ef2-9661-08074c580a32.jpg";
        else if(id.equals("530447"))
            cardurl = "https://cards.scryfall.io/large/front/6/f/6f509dbe-6ec7-4438-ab36-e20be46c9922.jpg";
        else if(id.equals("530448"))
            cardurl = "https://cards.scryfall.io/large/front/5/9/59b11ff8-f118-4978-87dd-509dc0c8c932.jpg";
        else if(id.equals("530449"))
            cardurl = "https://cards.scryfall.io/large/front/7/0/70b284bd-7a8f-4b60-8238-f746bdc5b236.jpg";
        else if(id.equals("530448t"))
            cardurl = "https://cards.scryfall.io/large/front/1/4/1425e965-7eea-419c-a7ec-c8169fa9edbf.jpg";
        else if(id.equals("530447t"))
            cardurl = "https://cards.scryfall.io/large/front/f/a/fa6fdb57-82f3-4695-b1fa-1f301ea4ef83.jpg";
        else if(id.equals("527514t"))
            cardurl = "https://cards.scryfall.io/large/front/c/4/c49e8e79-8673-41c2-a1ad-273c37e27aca.jpg";
        else if(id.equals("527507t"))
            cardurl = "https://cards.scryfall.io/large/front/6/b/6b2c8f52-1580-42d5-8434-c4c70e31e31b.jpg";
        else if(id.equals("527307t"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9c981c9-3376-4f6e-b30d-859e5fc7347e.jpg";
        else if(id.equals("96946t") || id.equals("338397t"))
            cardurl = "https://static.cardmarket.com/img/065612b0892a18c27f4de6a50c5d0327/items/1/GK1/366030.jpg";
        else if(id.equals("439314"))
            cardurl = "https://cards.scryfall.io/large/front/2/4/24842e29-77ac-4904-bd8f-b2cd163dd357.jpg";
        else if(id.equals("439315"))
            cardurl = "https://cards.scryfall.io/large/back/2/4/24842e29-77ac-4904-bd8f-b2cd163dd357.jpg";
        else if(id.equals("439316"))
            cardurl = "https://cards.scryfall.io/large/front/d/7/d78cd000-3908-446d-b155-dd8af3d8f166.jpg";
        else if(id.equals("439317"))
            cardurl = "https://cards.scryfall.io/large/back/d/7/d78cd000-3908-446d-b155-dd8af3d8f166.jpg";
        else if(id.equals("439318"))
            cardurl = "https://cards.scryfall.io/large/front/c/e/ce0a6fa9-f664-4263-8deb-8112f860814c.jpg";
        else if(id.equals("439318t"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/462ff49b-a004-4dab-a25b-65cb18c1bbec.jpg";
        else if(id.equals("439319"))
            cardurl = "https://cards.scryfall.io/large/back/c/e/ce0a6fa9-f664-4263-8deb-8112f860814c.jpg";
        else if(id.equals("439320"))
            cardurl = "https://cards.scryfall.io/large/front/3/2/3216d161-a43d-4a55-a14b-098061805409.jpg";
        else if(id.equals("439320t"))
            cardurl = "https://static.cardmarket.com/img/a8462480806adfd76fb002d92e976d96/items/1/UST/313929.jpg";
        else if(id.equals("439321"))
            cardurl = "https://cards.scryfall.io/large/back/3/2/3216d161-a43d-4a55-a14b-098061805409.jpg";
        else if(id.equals("439321t"))
            cardurl = "https://static.cardmarket.com/img/a8462480806adfd76fb002d92e976d96/items/1/UST/313929.jpg";
        else if(id.equals("439322"))
            cardurl = "https://cards.scryfall.io/large/front/2/b/2b45fb19-450d-40bd-91c7-b5ace4a77f2a.jpg";
        else if(id.equals("439323"))
            cardurl = "https://cards.scryfall.io/large/front/e/b/eba8bb03-6093-4e2b-99a2-a3fc5d8eb659.jpg";
        else if(id.equals("439324"))
            cardurl = "https://cards.scryfall.io/large/front/7/e/7e0cfe44-9b57-4b9a-b23f-18d3237bd7ee.jpg";
        else if(id.equals("439325"))
            cardurl = "https://cards.scryfall.io/large/back/7/e/7e0cfe44-9b57-4b9a-b23f-18d3237bd7ee.jpg";
        else if(id.equals("439326"))
            cardurl = "https://cards.scryfall.io/large/front/2/8/28059d09-2c7d-4c61-af55-8942107a7c1f.jpg";
        else if(id.equals("439327"))
            cardurl = "https://cards.scryfall.io/large/back/2/8/28059d09-2c7d-4c61-af55-8942107a7c1f.jpg";
        else if(id.equals("439328"))
            cardurl = "https://cards.scryfall.io/large/front/a/6/a66d5ee9-86a7-4052-a868-8dc6398342b3.jpg";
        else if(id.equals("439329"))
            cardurl = "https://cards.scryfall.io/large/back/a/6/a66d5ee9-86a7-4052-a868-8dc6398342b3.jpg";
        else if(id.equals("439330"))
            cardurl = "https://cards.scryfall.io/large/front/b/b/bb89599a-1883-45da-a87a-25e3f70c5a33.jpg";
        else if(id.equals("439330t"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/462ff49b-a004-4dab-a25b-65cb18c1bbec.jpg";
        else if(id.equals("439331"))
            cardurl = "https://cards.scryfall.io/large/back/b/b/bb89599a-1883-45da-a87a-25e3f70c5a33.jpg";
        else if(id.equals("439331t"))
            cardurl = "https://cards.scryfall.io/large/front/7/a/7a49607c-427a-474c-ad77-60cd05844b3c.jpg";
        else if(id.equals("439332"))
            cardurl = "https://cards.scryfall.io/large/front/b/3/b3c2bd44-4d75-4f61-89c0-1f1ba4d59ffa.jpg";
        else if(id.equals("439333"))
            cardurl = "https://cards.scryfall.io/large/front/4/d/4d553078-afaf-42db-879b-fb4cb4d25742.jpg";
        else if(id.equals("439333t"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/462ff49b-a004-4dab-a25b-65cb18c1bbec.jpg";
        else if(id.equals("439334"))
            cardurl = "https://cards.scryfall.io/large/back/4/d/4d553078-afaf-42db-879b-fb4cb4d25742.jpg";
        else if(id.equals("439335"))
            cardurl = "https://cards.scryfall.io/large/front/c/c/cc750c64-fd83-4b7b-9a40-a99213e6fa6d.jpg";
        else if(id.equals("439336"))
            cardurl = "https://cards.scryfall.io/large/back/c/c/cc750c64-fd83-4b7b-9a40-a99213e6fa6d.jpg";
        else if(id.equals("439337"))
            cardurl = "https://cards.scryfall.io/large/front/6/4/6473e356-2685-4f91-ab42-cca8c6be0816.jpg";
        else if(id.equals("439338"))
            cardurl = "https://cards.scryfall.io/large/back/6/4/6473e356-2685-4f91-ab42-cca8c6be0816.jpg";
        else if(id.equals("439339"))
            cardurl = "https://cards.scryfall.io/large/front/b/1/b101dd14-aff1-4811-bf1b-468930dd2999.jpg";
        else if(id.equals("439339t"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5bd6905-79be-4d2c-a343-f6e6a181b3e6.jpg";
        else if(id.equals("439340"))
            cardurl = "https://cards.scryfall.io/large/back/b/1/b101dd14-aff1-4811-bf1b-468930dd2999.jpg";
        else if(id.equals("439341"))
            cardurl = "https://cards.scryfall.io/large/front/6/3/63b2b7cd-a51d-4e50-b794-a52731196973.jpg";
        else if(id.equals("439342"))
            cardurl = "https://cards.scryfall.io/large/back/6/3/63b2b7cd-a51d-4e50-b794-a52731196973.jpg";
        else if(id.equals("530449t"))
            cardurl = "https://cards.scryfall.io/large/front/6/5/65f8e40f-fb5e-4ab8-add3-a8b87e7bcdd9.jpg";
        else if(id.equals("435173") || id.equals("435174") || id.equals("435176t") || id.equals("435212t") || id.equals("435227") || id.equals("435244")
                || id.equals("435328") || id.equals("435347") || id.equals("435388t") || id.equals("435391") || id.equals("435392t") || id.equals("435393t")
                || id.equals("435402") || id.equals("435409") || id.equals("435410t") || id.equals("435411t") || id.equals("435173t") || id.equals("435174t")
                || id.equals("435181t") || id.equals("435226") || id.equals("435243") || id.equals("435327") || id.equals("435346") || id.equals("435380t")
                || id.equals("435390") || id.equals("435392") || id.equals("435393") || id.equals("435401") || id.equals("435408") || id.equals("435410")
                || id.equals("435411"))
            cardurl = "http://teksport.altervista.org/XLN/" + id + ".jpg";
        else if(id.equals("409741") || id.equals("409760") || id.equals("409790") || id.equals("409826t") || id.equals("409839") || id.equals("409856")
                || id.equals("409868") || id.equals("409901") || id.equals("409913") || id.equals("409946") || id.equals("409962") || id.equals("409976")
                || id.equals("409993") || id.equals("410015t") || id.equals("410027") || id.equals("410049") || id.equals("409742") || id.equals("409773")
                || id.equals("409791") || id.equals("409831") || id.equals("409840") || id.equals("409860t") || id.equals("409869") || id.equals("409903t")
                || id.equals("409923") || id.equals("409947") || id.equals("409968") || id.equals("409977") || id.equals("410007") || id.equals("410016t")
                || id.equals("410031t") || id.equals("410049t") || id.equals("409743") || id.equals("409774") || id.equals("409796") || id.equals("409832")
                || id.equals("409843") || id.equals("409862t") || id.equals("409897") || id.equals("409910") || id.equals("409924") || id.equals("409951")
                || id.equals("409969") || id.equals("409987") || id.equals("410007t") || id.equals("410021") || id.equals("410032t") || id.equals("410050")
                || id.equals("409744") || id.equals("409786") || id.equals("409797") || id.equals("409836") || id.equals("409844") || id.equals("409864")
                || id.equals("409898") || id.equals("409911") || id.equals("409937") || id.equals("409952") || id.equals("409970") || id.equals("409988")
                || id.equals("410008") || id.equals("410022") || id.equals("410033") || id.equals("410050t") || id.equals("409759") || id.equals("409787")
                || id.equals("409805t") || id.equals("409837") || id.equals("409855") || id.equals("409865") || id.equals("409900") || id.equals("409912")
                || id.equals("409938") || id.equals("409961") || id.equals("409971") || id.equals("409992") || id.equals("410008t") || id.equals("410026")
                || id.equals("410034"))
            cardurl = "http://teksport.altervista.org/SOI/" + id + ".jpg";
        else if(id.equals("439401") || id.equals("439471") || id.equals("439625") || id.equals("439628") || id.equals("439631") || id.equals("439633")
                || id.equals("439636") || id.equals("439639") || id.equals("439642") || id.equals("439644t") || id.equals("439646t") || id.equals("439649")
                || id.equals("439651") || id.equals("439654") || id.equals("439438") || id.equals("439502") || id.equals("439626") || id.equals("439629")
                || id.equals("439631t") || id.equals("439634") || id.equals("439637") || id.equals("439640") || id.equals("439643") || id.equals("439645")
                || id.equals("439647") || id.equals("439649t") || id.equals("439652") || id.equals("439456") || id.equals("439536") || id.equals("439627")
                || id.equals("439630") || id.equals("439632") || id.equals("439635") || id.equals("439638") || id.equals("439641") || id.equals("439644")
                || id.equals("439646") || id.equals("439648") || id.equals("439650") || id.equals("439653"))
            cardurl = "http://teksport.altervista.org/UST/" + id + ".jpg";
        else if(set.equals("S00"))
            cardurl = "http://teksport.altervista.org/S00/" + id + ".jpg";
        else if (id.equals("495186"))
            cardurl = "https://cards.scryfall.io/large/front/c/0/c0250dc8-9d4c-428a-9e34-9e3577be4745.jpg";
        else if (id.equals("495187"))
            cardurl = "https://cards.scryfall.io/large/front/4/8/48b8024d-a300-43cb-9dde-6b4cb1fa19f7.jpg";
        else if (id.equals("495188"))
            cardurl = "https://cards.scryfall.io/large/front/d/4/d442c32d-457d-4fef-bba2-33a07bf23125.jpg";
        else if (id.equals("495189"))
            cardurl = "https://cards.scryfall.io/large/front/b/e/be97b691-f9f5-4fb4-8e44-8ffe32d13d03.jpg";
        else if (id.equals("495190"))
            cardurl = "https://cards.scryfall.io/large/front/e/8/e8df0aed-dd2b-4f1e-8dfe-aec07462b1e1.jpg";
        else if (id.equals("495191"))
            cardurl = "https://cards.scryfall.io/large/front/e/f/efc72e9f-2cda-47b9-84fd-4eed88312404.jpg";
        else if (id.equals("495192"))
            cardurl = "https://cards.scryfall.io/large/front/0/4/04833fcc-cef7-4152-8191-c552288c83e4.jpg";
        else if (id.equals("495193"))
            cardurl = "https://cards.scryfall.io/large/front/d/6/d60c9b15-c824-4203-bdda-ff9c041f9e2f.jpg";
        else if (id.equals("495194"))
            cardurl = "https://cards.scryfall.io/large/front/0/5/05347539-de61-4a37-929f-c909e65033f5.jpg";
        else if (id.equals("495195"))
            cardurl = "https://cards.scryfall.io/large/front/b/5/b5757230-08b8-4808-af61-d343f9748fb1.jpg";
        else if (id.equals("495196"))
            cardurl = "https://cards.scryfall.io/large/front/a/e/aecfbd48-7da0-4b44-b9a2-d31412f65eb1.jpg";
        else if (id.equals("495197"))
            cardurl = "https://cards.scryfall.io/large/front/4/9/490f3d74-6144-4cbc-80ed-37cfcdbd159a.jpg";
        else if (id.equals("495198"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b6008794-a7ca-4a3e-b88b-e5dbb9e0f39b.jpg";
        else if (id.equals("495199"))
            cardurl = "https://cards.scryfall.io/large/front/6/9/6954cc66-ab80-4457-b0da-61d80e80e25e.jpg";
        else if (id.equals("495200"))
            cardurl = "https://cards.scryfall.io/large/front/d/5/d52e90d3-d356-4b23-8f5c-a4004b20394c.jpg";
        else if (id.equals("495201"))
            cardurl = "https://cards.scryfall.io/large/front/0/9/09c8c150-a0d8-4254-9169-7697e9c540da.jpg";
        else if (id.equals("495202"))
            cardurl = "https://cards.scryfall.io/large/back/0/9/09c8c150-a0d8-4254-9169-7697e9c540da.jpg";
        else if (id.equals("495203"))
            cardurl = "https://cards.scryfall.io/large/front/7/9/796b5899-97e5-4682-aac8-51378f0c904e.jpg";
        else if (id.equals("495204"))
            cardurl = "https://cards.scryfall.io/large/front/1/5/151bdf3a-4445-43b1-8cea-2737c13d9dee.jpg";
        else if (id.equals("495205"))
            cardurl = "https://cards.scryfall.io/large/front/3/0/309b2cb5-b9a8-417d-b5ae-0a7d03ff93f0.jpg";
        else if (id.equals("495206"))
            cardurl = "https://cards.scryfall.io/large/front/0/3/03d6d8a4-c51d-4b4a-86e7-df9e9c7a171d.jpg";
        else if (id.equals("495207"))
            cardurl = "https://cards.scryfall.io/large/front/c/f/cf4a4aba-3391-4259-9a5f-a163a45d943c.jpg";
        else if (id.equals("495208"))
            cardurl = "https://cards.scryfall.io/large/front/0/d/0d0954df-07f0-430d-90ee-d1fe40af546f.jpg";
        else if (id.equals("495209"))
            cardurl = "https://cards.scryfall.io/large/front/9/b/9b5bc5d7-c0f8-4632-adb7-dd3b75a3d87d.jpg";
        else if (id.equals("495210"))
            cardurl = "https://cards.scryfall.io/large/front/c/3/c344a3cd-43e0-4333-83ec-081f0e39530a.jpg";
        else if (id.equals("495210t")) //Plant 0/1
            cardurl = "https://cards.scryfall.io/large/front/d/0/d03d87f5-0ac6-45ca-a54b-6a36132a8eae.jpg";
        else if (id.equals("495205t") || id.equals("297399t") || id.equals("297400t")) //Insect 1/1
            cardurl = "https://cards.scryfall.io/large/front/8/4/84da9c36-5d9c-4e29-b6cc-c5c10e490f2e.jpg";
        else if (id.equals("495188t")) //Cat Beast 2/2
            cardurl = "https://cards.scryfall.io/large/front/e/2/e2c91781-acf9-4cff-be1a-85148ad2a683.jpg";
        else if (id.equals("425847"))
            cardurl = "https://cards.scryfall.io/large/front/2/e/2e376bdf-076c-471a-9408-b36fc5b8405b.jpg";
        else if (id.equals("293395") || id.equals("29339510"))
            cardurl = "https://cards.scryfall.io/large/front/9/4/940509ec-8f58-4593-a598-142a827f55b0.jpg";
        else if (id.equals("17498") || id.equals("1749810"))
            cardurl = "https://cards.scryfall.io/large/front/4/c/4c663245-dfb6-4d92-8ac7-ffe3d5d12187.jpg";
        else if (id.equals("51974") || id.equals("5197410"))
            cardurl = "https://cards.scryfall.io/large/front/c/c/ccdda4dd-f2e3-419e-9f4d-15d7270e27ee.jpg";
        else if (id.equals("52495"))
            cardurl = "https://cards.scryfall.io/large/front/5/c/5cf835fb-4953-486c-aed2-2208ca31df31.jpg";
        else if (id.equals("5249510"))
            cardurl = "https://cards.scryfall.io/large/back/5/c/5cf835fb-4953-486c-aed2-2208ca31df31.jpg";
        else if (id.equals("52473"))
            cardurl = "https://cards.scryfall.io/large/front/8/7/870e6492-3e4d-4680-9a78-a99782039876.jpg";
        else if (id.equals("5247310"))
            cardurl = "https://cards.scryfall.io/large/back/8/7/870e6492-3e4d-4680-9a78-a99782039876.jpg";
        else if (id.equals("52137"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/466f7f14-72b7-46c9-b8d6-a99bf92c4089.jpg";
        else if (id.equals("5213710"))
            cardurl = "https://cards.scryfall.io/large/back/4/6/466f7f14-72b7-46c9-b8d6-a99bf92c4089.jpg";
        else if (id.equals("52530"))
            cardurl = "https://cards.scryfall.io/large/front/b/f/bfc92a35-9e40-4a7b-a7cb-f0b4537ea996.jpg";
        else if (id.equals("5253010"))
            cardurl = "https://cards.scryfall.io/large/back/b/f/bfc92a35-9e40-4a7b-a7cb-f0b4537ea996.jpg";
        else if (id.equals("52704"))
            cardurl = "https://cards.scryfall.io/large/front/7/2/72887f7f-4156-4b88-aef5-b96dea57903e.jpg";
        else if (id.equals("5270410"))
            cardurl = "https://cards.scryfall.io/large/back/7/2/72887f7f-4156-4b88-aef5-b96dea57903e.jpg";
        else if(id.equals("296818"))
            cardurl = "https://cards.scryfall.io/large/back/6/3/6317573e-d892-48ce-bba4-76f9f632ed2b.jpg";
        else if(id.equals("296817"))
            cardurl = "https://cards.scryfall.io/large/back/7/e/7e6b3fb3-897b-4665-b053-a29f25850b25.jpg";
        else if(id.equals("296594"))
            cardurl = "https://cards.scryfall.io/large/front/6/3/6317573e-d892-48ce-bba4-76f9f632ed2b.jpg";
        else if(id.equals("296486"))
            cardurl = "https://cards.scryfall.io/large/front/7/e/7e6b3fb3-897b-4665-b053-a29f25850b25.jpg";
        else if(id.equals("296764t") || id.equals("534957t") || id.equals("535010t") || id.equals("534872t") || id.equals("534839t")
                || id.equals("534774t") || id.equals("540708t") || id.equals("546993t")) // Clue
            cardurl = "https://cards.scryfall.io/large/front/f/2/f2c859e1-181e-44d1-afbd-bbd6e52cf42a.jpg";
        else if(id.equals("296695t")) //Squirrel 1/1
            cardurl = "https://cards.scryfall.io/large/front/9/7/977ddd05-1aae-46fc-95ce-866710d1c5c6.jpg";
        else if(id.equals("296549t")) // Djinn Monk 2/2
            cardurl = "https://cards.scryfall.io/large/front/f/2/f2e8077e-4400-4923-afe6-6ff5a51b5e91.jpg";
        else if(id.equals("296439t")) //Kraken 8/8
            cardurl= "https://cards.scryfall.io/large/front/c/a/ca17c7b2-180a-4bd1-9ab2-152f8f656dba.jpg";
        else if(id.equals("999993")) // Day
            cardurl = "https://cards.scryfall.io/large/front/9/c/9c0f7843-4cbb-4d0f-8887-ec823a9238da.jpg";
        else if(id.equals("999994")) // Night
            cardurl = "https://cards.scryfall.io/large/back/9/c/9c0f7843-4cbb-4d0f-8887-ec823a9238da.jpg";
        else if(id.equals("534752"))
            cardurl = "https://cards.scryfall.io/large/front/5/4/54d4e7c3-294d-4900-8b70-faafda17cc33.jpg";
        else if(id.equals("534753"))
            cardurl = "https://cards.scryfall.io/large/back/5/4/54d4e7c3-294d-4900-8b70-faafda17cc33.jpg";
        else if(id.equals("534754"))
            cardurl = "https://cards.scryfall.io/large/front/6/1/6109b54e-56c5-4014-9f6d-d5f7a0fd725d.jpg";
        else if(id.equals("534755"))
            cardurl = "https://cards.scryfall.io/large/back/6/1/6109b54e-56c5-4014-9f6d-d5f7a0fd725d.jpg";
        else if(id.equals("534756"))
            cardurl = "https://cards.scryfall.io/large/front/4/a/4adee830-62fd-4ab4-b1c6-a8bbe15331d1.jpg";
        else if(id.equals("534757"))
            cardurl = "https://cards.scryfall.io/large/back/4/a/4adee830-62fd-4ab4-b1c6-a8bbe15331d1.jpg";
        else if(id.equals("534760"))
            cardurl = "https://cards.scryfall.io/large/front/0/d/0dbac7ce-a6fa-466e-b6ba-173cf2dec98e.jpg";
        else if(id.equals("534761"))
            cardurl = "https://cards.scryfall.io/large/back/0/d/0dbac7ce-a6fa-466e-b6ba-173cf2dec98e.jpg";
        else if(id.equals("534767"))
            cardurl = "https://cards.scryfall.io/large/front/2/0/20e94e17-2e4c-41cd-8cc5-39ab41037287.jpg";
        else if(id.equals("534768"))
            cardurl = "https://cards.scryfall.io/large/back/2/0/20e94e17-2e4c-41cd-8cc5-39ab41037287.jpg";
        else if(id.equals("534772"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a204c2a3-a899-4b70-8825-7e085b655ed0.jpg";
        else if(id.equals("534773"))
            cardurl = "https://cards.scryfall.io/large/back/a/2/a204c2a3-a899-4b70-8825-7e085b655ed0.jpg";
        else if(id.equals("534783"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2704743-2e23-40b9-a367-c73d2db45afc.jpg";
        else if(id.equals("534784"))
            cardurl = "https://cards.scryfall.io/large/back/d/2/d2704743-2e23-40b9-a367-c73d2db45afc.jpg";
        else if(id.equals("534785"))
            cardurl = "https://cards.scryfall.io/large/front/2/d/2d3687e2-09e0-4753-aa02-88a19bde3330.jpg";
        else if(id.equals("534786"))
            cardurl = "https://cards.scryfall.io/large/back/2/d/2d3687e2-09e0-4753-aa02-88a19bde3330.jpg";
        else if(id.equals("534800"))
            cardurl = "https://cards.scryfall.io/large/front/3/6/36e71d16-0964-489d-bea2-9cec7991fc99.jpg";
        else if(id.equals("534801"))
            cardurl = "https://cards.scryfall.io/large/back/3/6/36e71d16-0964-489d-bea2-9cec7991fc99.jpg";
        else if(id.equals("534804"))
            cardurl = "https://cards.scryfall.io/large/front/0/3/03a3ea4b-d292-4602-985f-7a7971ca73ec.jpg";
        else if(id.equals("534805"))
            cardurl = "https://cards.scryfall.io/large/back/0/3/03a3ea4b-d292-4602-985f-7a7971ca73ec.jpg";
        else if(id.equals("534807"))
            cardurl = "https://cards.scryfall.io/large/front/a/b/abff6c81-65a4-48fa-ba8f-580f87b0344a.jpg";
        else if(id.equals("534808"))
            cardurl = "https://cards.scryfall.io/large/back/a/b/abff6c81-65a4-48fa-ba8f-580f87b0344a.jpg";
        else if(id.equals("534816"))
            cardurl = "https://cards.scryfall.io/large/front/e/b/eb34c472-c6ff-4d83-ac8b-a8f279593f98.jpg";
        else if(id.equals("534817"))
            cardurl = "https://cards.scryfall.io/large/back/e/b/eb34c472-c6ff-4d83-ac8b-a8f279593f98.jpg";
        else if(id.equals("534823"))
            cardurl = "https://cards.scryfall.io/large/front/e/7/e79269af-63eb-43d2-afee-c38fa14a0c5b.jpg";
        else if(id.equals("534824"))
            cardurl = "https://cards.scryfall.io/large/back/e/7/e79269af-63eb-43d2-afee-c38fa14a0c5b.jpg";
        else if(id.equals("534826"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/caa57b63-bb11-45e8-8795-de92ca61f4f1.jpg";
        else if(id.equals("534827"))
            cardurl = "https://cards.scryfall.io/large/back/c/a/caa57b63-bb11-45e8-8795-de92ca61f4f1.jpg";
        else if(id.equals("534832"))
            cardurl = "https://cards.scryfall.io/large/front/8/3/832288fd-8031-4c2b-ad3e-b1ec9f94d379.jpg";
        else if(id.equals("534833"))
            cardurl = "https://cards.scryfall.io/large/back/8/3/832288fd-8031-4c2b-ad3e-b1ec9f94d379.jpg";
        else if(id.equals("534836"))
            cardurl = "https://cards.scryfall.io/large/front/9/9/999038b3-7d64-4554-b341-0675d4af8d8b.jpg";
        else if(id.equals("534837"))
            cardurl = "https://cards.scryfall.io/large/back/9/9/999038b3-7d64-4554-b341-0675d4af8d8b.jpg";
        else if(id.equals("534846"))
            cardurl = "https://cards.scryfall.io/large/front/a/b/ab17c8fa-4c06-4542-848a-e3f2f9f47c27.jpg";
        else if(id.equals("534847"))
            cardurl = "https://cards.scryfall.io/large/back/a/b/ab17c8fa-4c06-4542-848a-e3f2f9f47c27.jpg";
        else if(id.equals("534852"))
            cardurl = "https://cards.scryfall.io/large/front/7/b/7b63f2ae-5bfd-452f-b1f5-8459bcecd3bb.jpg";
        else if(id.equals("534853"))
            cardurl = "https://cards.scryfall.io/large/back/7/b/7b63f2ae-5bfd-452f-b1f5-8459bcecd3bb.jpg";
        else if(id.equals("534860"))
            cardurl = "https://cards.scryfall.io/large/front/5/d/5db99746-8aee-42b8-acb0-ed69933d0ff8.jpg";
        else if(id.equals("534861"))
            cardurl = "https://cards.scryfall.io/large/back/5/d/5db99746-8aee-42b8-acb0-ed69933d0ff8.jpg";
        else if(id.equals("534863"))
            cardurl = "https://cards.scryfall.io/large/front/0/a/0a3c8532-92f5-41db-92b4-a871aa05e0c7.jpg";
        else if(id.equals("534864"))
            cardurl = "https://cards.scryfall.io/large/back/0/a/0a3c8532-92f5-41db-92b4-a871aa05e0c7.jpg";
        else if(id.equals("534870"))
            cardurl = "https://cards.scryfall.io/large/front/b/b/bbdad18e-e262-41f9-b252-1cbdcdd1b5f9.jpg";
        else if(id.equals("534871"))
            cardurl = "https://cards.scryfall.io/large/back/b/b/bbdad18e-e262-41f9-b252-1cbdcdd1b5f9.jpg";
        else if(id.equals("534875"))
            cardurl = "https://cards.scryfall.io/large/front/d/a/daa2a273-488f-4285-a069-ad159ad2d393.jpg";
        else if(id.equals("534876"))
            cardurl = "https://cards.scryfall.io/large/back/d/a/daa2a273-488f-4285-a069-ad159ad2d393.jpg";
        else if(id.equals("534877"))
            cardurl = "https://cards.scryfall.io/large/front/e/6/e6dd05f0-a3c0-4bd6-a1d1-a74540623093.jpg";
        else if(id.equals("534878"))
            cardurl = "https://cards.scryfall.io/large/back/e/6/e6dd05f0-a3c0-4bd6-a1d1-a74540623093.jpg";
        else if(id.equals("534882"))
            cardurl = "https://cards.scryfall.io/large/front/0/f/0f6e668d-2502-4e82-b4c2-ef34c9afa27e.jpg";
        else if(id.equals("534883"))
            cardurl = "https://cards.scryfall.io/large/back/0/f/0f6e668d-2502-4e82-b4c2-ef34c9afa27e.jpg";
        else if(id.equals("534894"))
            cardurl = "https://cards.scryfall.io/large/front/5/5/55f0666a-5c3e-492b-b4ea-42fa7f24661b.jpg";
        else if(id.equals("534895"))
            cardurl = "https://cards.scryfall.io/large/back/5/5/55f0666a-5c3e-492b-b4ea-42fa7f24661b.jpg";
        else if(id.equals("534901"))
            cardurl = "https://cards.scryfall.io/large/front/d/4/d4054ae6-0227-4d99-8cb5-72e8b5d0b726.jpg";
        else if(id.equals("534902"))
            cardurl = "https://cards.scryfall.io/large/back/d/4/d4054ae6-0227-4d99-8cb5-72e8b5d0b726.jpg";
        else if(id.equals("534915"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2feb859-bfae-4bc4-8181-5737dd5c3b08.jpg";
        else if(id.equals("534916"))
            cardurl = "https://cards.scryfall.io/large/back/d/2/d2feb859-bfae-4bc4-8181-5737dd5c3b08.jpg";
        else if(id.equals("534918"))
            cardurl = "https://cards.scryfall.io/large/front/b/e/be91fcba-4599-4ecb-824d-55112096c34a.jpg";
        else if(id.equals("534919"))
            cardurl = "https://cards.scryfall.io/large/back/b/e/be91fcba-4599-4ecb-824d-55112096c34a.jpg";
        else if(id.equals("534921"))
            cardurl = "https://cards.scryfall.io/large/front/3/5/35fdb976-291c-4824-9518-dd8c9f93fcde.jpg";
        else if(id.equals("534922"))
            cardurl = "https://cards.scryfall.io/large/back/3/5/35fdb976-291c-4824-9518-dd8c9f93fcde.jpg";
        else if(id.equals("534936"))
            cardurl = "https://cards.scryfall.io/large/front/a/3/a33af331-0746-4adf-935a-bf61ff9d8d4b.jpg";
        else if(id.equals("534937"))
            cardurl = "https://cards.scryfall.io/large/back/a/3/a33af331-0746-4adf-935a-bf61ff9d8d4b.jpg";
        else if(id.equals("534939"))
            cardurl = "https://cards.scryfall.io/large/front/4/1/41b6381f-4ff8-49e9-bf00-cfe32851318b.jpg";
        else if(id.equals("534940"))
            cardurl = "https://cards.scryfall.io/large/back/4/1/41b6381f-4ff8-49e9-bf00-cfe32851318b.jpg";
        else if(id.equals("534941"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2a5b43d-e21b-4294-9ea2-5bd0264e71d3.jpg";
        else if(id.equals("534942"))
            cardurl = "https://cards.scryfall.io/large/back/d/2/d2a5b43d-e21b-4294-9ea2-5bd0264e71d3.jpg";
        else if(id.equals("534945"))
            cardurl = "https://cards.scryfall.io/large/front/1/d/1d7b2d05-ce5c-4b73-8fa6-d9b69619d58c.jpg";
        else if(id.equals("534946"))
            cardurl = "https://cards.scryfall.io/large/back/1/d/1d7b2d05-ce5c-4b73-8fa6-d9b69619d58c.jpg";
        else if(id.equals("534948"))
            cardurl = "https://cards.scryfall.io/large/front/1/b/1bf48d2b-eb68-4f47-a80a-4751a4fa20a7.jpg";
        else if(id.equals("534949"))
            cardurl = "https://cards.scryfall.io/large/back/1/b/1bf48d2b-eb68-4f47-a80a-4751a4fa20a7.jpg";
        else if(id.equals("534953"))
            cardurl = "https://cards.scryfall.io/large/front/7/1/71ccc444-54c8-4f7c-a425-82bc3eea1eb0.jpg";
        else if(id.equals("534954"))
            cardurl = "https://cards.scryfall.io/large/back/7/1/71ccc444-54c8-4f7c-a425-82bc3eea1eb0.jpg";
        else if(id.equals("534959"))
            cardurl = "https://cards.scryfall.io/large/front/3/8/3849ad37-f80d-4ffc-9240-25a63326b3dd.jpg";
        else if(id.equals("534960"))
            cardurl = "https://cards.scryfall.io/large/back/3/8/3849ad37-f80d-4ffc-9240-25a63326b3dd.jpg";
        else if(id.equals("534967"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a2cda10b-7cd5-4cf5-87bd-c3b8c6aa2b47.jpg";
        else if(id.equals("534968"))
            cardurl = "https://cards.scryfall.io/large/back/a/2/a2cda10b-7cd5-4cf5-87bd-c3b8c6aa2b47.jpg";
        else if(id.equals("534974"))
            cardurl = "https://cards.scryfall.io/large/front/2/8/28e2119b-ed78-4b98-a956-f2b453d0b164.jpg";
        else if(id.equals("534975"))
            cardurl = "https://cards.scryfall.io/large/back/2/8/28e2119b-ed78-4b98-a956-f2b453d0b164.jpg";
        else if(id.equals("534978"))
            cardurl = "https://cards.scryfall.io/large/front/6/0/60e53d61-fcc3-4def-8206-052b46f62deb.jpg";
        else if(id.equals("534979"))
            cardurl = "https://cards.scryfall.io/large/back/6/0/60e53d61-fcc3-4def-8206-052b46f62deb.jpg";
        else if(id.equals("534992"))
            cardurl = "https://cards.scryfall.io/large/front/3/e/3e96f9a6-c215-42b1-aa02-8e6143fe5bd7.jpg";
        else if(id.equals("534993"))
            cardurl = "https://cards.scryfall.io/large/back/3/e/3e96f9a6-c215-42b1-aa02-8e6143fe5bd7.jpg";
        else if(id.equals("534994"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/3983a304-5040-4b8d-945a-bf4ede3104a8.jpg";
        else if(id.equals("534995"))
            cardurl = "https://cards.scryfall.io/large/back/3/9/3983a304-5040-4b8d-945a-bf4ede3104a8.jpg";
        else if(id.equals("535002"))
            cardurl = "https://cards.scryfall.io/large/front/5/0/50d4b0df-a1d8-494f-a019-70ce34161320.jpg";
        else if(id.equals("535003"))
            cardurl = "https://cards.scryfall.io/large/back/5/0/50d4b0df-a1d8-494f-a019-70ce34161320.jpg";
        else if(id.equals("535009"))
            cardurl = "https://cards.scryfall.io/large/front/3/5/35cf2d72-931f-47b1-a1b4-916f0383551a.jpg";
        else if(id.equals("535010"))
            cardurl = "https://cards.scryfall.io/large/back/3/5/35cf2d72-931f-47b1-a1b4-916f0383551a.jpg";
        else if(id.equals("535011"))
            cardurl = "https://cards.scryfall.io/large/front/9/6/965e6bd5-dc32-406c-bc99-ceb15be4d3f2.jpg";
        else if(id.equals("535012"))
            cardurl = "https://cards.scryfall.io/large/back/9/6/965e6bd5-dc32-406c-bc99-ceb15be4d3f2.jpg";
        else if(id.equals("535025"))
            cardurl = "https://cards.scryfall.io/large/front/8/a/8ab5f2e6-0e0a-4f7d-a959-3d07948ff317.jpg";
        else if(id.equals("535026"))
            cardurl = "https://cards.scryfall.io/large/back/8/a/8ab5f2e6-0e0a-4f7d-a959-3d07948ff317.jpg";
        else if(id.equals("535028"))
            cardurl = "https://cards.scryfall.io/large/front/7/8/788288f6-7944-48f4-91b0-f452e209c9ce.jpg";
        else if(id.equals("535029"))
            cardurl = "https://cards.scryfall.io/large/back/7/8/788288f6-7944-48f4-91b0-f452e209c9ce.jpg";
        else if(id.equals("535042"))
            cardurl = "https://cards.scryfall.io/large/front/f/9/f953fad3-0cd1-48aa-8ed9-d7d2e293e6e2.jpg";
        else if(id.equals("535043"))
            cardurl = "https://cards.scryfall.io/large/back/f/9/f953fad3-0cd1-48aa-8ed9-d7d2e293e6e2.jpg";
        else if(id.equals("535053"))
            cardurl = "https://cards.scryfall.io/large/front/1/1/115a9a44-131d-45f3-852a-40fd18e4afb6.jpg";
        else if(id.equals("535054"))
            cardurl = "https://cards.scryfall.io/large/back/1/1/115a9a44-131d-45f3-852a-40fd18e4afb6.jpg";
        else if(id.equals("535062"))
            cardurl = "https://cards.scryfall.io/large/front/a/c/ac83c27f-55d6-4e5a-93a4-febb0c183289.jpg";
        else if(id.equals("535063"))
            cardurl = "https://cards.scryfall.io/large/back/a/c/ac83c27f-55d6-4e5a-93a4-febb0c183289.jpg";
        else if(id.equals("296820"))
            cardurl = "https://cards.scryfall.io/large/front/d/8/d8b718d8-fca3-4b3e-9448-6067c8656a9a.jpg";
        else if(id.equals("296821"))
            cardurl = "https://cards.scryfall.io/large/back/d/8/d8b718d8-fca3-4b3e-9448-6067c8656a9a.jpg";
        else if(id.equals("535002t") || id.equals("534994t") || id.equals("534995t") || id.equals("54047311t") || id.equals("297433t")) // Wolf 2/2
            cardurl = "https://cards.scryfall.io/large/front/3/6/364e04d9-9a8a-49df-921c-7a9bf62dc731.jpg";
        else if(id.equals("534882t") || id.equals("540880t") || id.equals("297208t") || id.equals("296971t") || id.equals("296854t") ||
                id.equals("296840t")) // Human 1/1
            cardurl = "https://cards.scryfall.io/large/front/b/7/b7667345-e11b-4cad-ac4c-84eb1c5656c5.jpg";
        else if(id.equals("534836t") || id.equals("297108t") || id.equals("296970t") || id.equals("296926t")) // Zombie 2/2 Decayed
            cardurl = "https://cards.scryfall.io/large/front/6/a/6adb8607-1066-451d-a719-74ad32358278.jpg";
        else if (id.equals("540753t") || id.equals("546992t")) //Treasure
            cardurl = "https://cards.scryfall.io/large/front/7/2/720f3e68-84c0-462e-a0d1-90236ccc494a.jpg";
        else if(id.equals("540836"))
            cardurl = "https://cards.scryfall.io/large/front/a/7/a7fc0939-6286-44de-a727-c83bfd3fa752.jpg";
        else if(id.equals("540837"))
            cardurl = "https://cards.scryfall.io/large/back/a/7/a7fc0939-6286-44de-a727-c83bfd3fa752.jpg";
        else if(id.equals("540838"))
            cardurl = "https://cards.scryfall.io/large/front/f/8/f88e269e-ff3d-4775-8520-5b7a6dddf23d.jpg";
        else if(id.equals("540839"))
            cardurl = "https://cards.scryfall.io/large/back/f/8/f88e269e-ff3d-4775-8520-5b7a6dddf23d.jpg";
        else if(id.equals("540841"))
            cardurl = "https://cards.scryfall.io/large/front/d/b/db791fb6-b0ff-4ded-bd3d-9447cf398312.jpg";
        else if(id.equals("540842"))
            cardurl = "https://cards.scryfall.io/large/back/d/b/db791fb6-b0ff-4ded-bd3d-9447cf398312.jpg";
        else if(id.equals("540851"))
            cardurl = "https://cards.scryfall.io/large/front/0/e/0ef240aa-2a88-4ec4-888a-918466372adb.jpg";
        else if(id.equals("540852"))
            cardurl = "https://cards.scryfall.io/large/back/0/e/0ef240aa-2a88-4ec4-888a-918466372adb.jpg";
        else if(id.equals("540853"))
            cardurl = "https://cards.scryfall.io/large/front/2/5/25193485-7f41-4b05-9a69-4c112679f97c.jpg";
        else if(id.equals("540854"))
            cardurl = "https://cards.scryfall.io/large/back/2/5/25193485-7f41-4b05-9a69-4c112679f97c.jpg";
        else if(id.equals("540860"))
            cardurl = "https://cards.scryfall.io/large/front/0/3/031c5cff-e579-432a-bcee-864b12eb0558.jpg";
        else if(id.equals("540861"))
            cardurl = "https://cards.scryfall.io/large/back/0/3/031c5cff-e579-432a-bcee-864b12eb0558.jpg";
        else if(id.equals("540864"))
            cardurl = "https://cards.scryfall.io/large/front/4/a/4a708243-42a1-4fa7-8b0b-9d5163da84bb.jpg";
        else if(id.equals("540865"))
            cardurl = "https://cards.scryfall.io/large/back/4/a/4a708243-42a1-4fa7-8b0b-9d5163da84bb.jpg";
        else if(id.equals("540874"))
            cardurl = "https://cards.scryfall.io/large/front/f/1/f1deb24b-3d8f-4251-a901-85eeb891f26f.jpg";
        else if(id.equals("540875"))
            cardurl = "https://cards.scryfall.io/large/back/f/1/f1deb24b-3d8f-4251-a901-85eeb891f26f.jpg";
        else if(id.equals("540880"))
            cardurl = "https://cards.scryfall.io/large/front/2/c/2c3ddb1f-a1de-4fea-9042-5e9caa16ceb2.jpg";
        else if(id.equals("540881"))
            cardurl = "https://cards.scryfall.io/large/back/2/c/2c3ddb1f-a1de-4fea-9042-5e9caa16ceb2.jpg";
        else if(id.equals("540884"))
            cardurl = "https://cards.scryfall.io/large/front/7/3/730e4629-dc54-415d-9493-88885788ca19.jpg";
        else if(id.equals("540885"))
            cardurl = "https://cards.scryfall.io/large/back/7/3/730e4629-dc54-415d-9493-88885788ca19.jpg";
        else if(id.equals("540886"))
            cardurl = "https://cards.scryfall.io/large/front/5/7/57039230-bf5a-4489-9dc1-37e27b17bd84.jpg";
        else if(id.equals("540887"))
            cardurl = "https://cards.scryfall.io/large/back/5/7/57039230-bf5a-4489-9dc1-37e27b17bd84.jpg";
        else if(id.equals("540900"))
            cardurl = "https://cards.scryfall.io/large/front/c/e/ceb84515-7b8f-444d-b6a9-61231621f9b7.jpg";
        else if(id.equals("540901"))
            cardurl = "https://cards.scryfall.io/large/back/c/e/ceb84515-7b8f-444d-b6a9-61231621f9b7.jpg";
        else if(id.equals("540904"))
            cardurl = "https://cards.scryfall.io/large/front/6/b/6b4529c3-8edb-4909-b910-806450a39d2e.jpg";
        else if(id.equals("540905"))
            cardurl = "https://cards.scryfall.io/large/back/6/b/6b4529c3-8edb-4909-b910-806450a39d2e.jpg";
        else if(id.equals("540906"))
            cardurl = "https://cards.scryfall.io/large/front/a/4/a4d3652a-6774-4b16-aa8b-cb11d72ec7aa.jpg";
        else if(id.equals("540907"))
            cardurl = "https://cards.scryfall.io/large/back/a/4/a4d3652a-6774-4b16-aa8b-cb11d72ec7aa.jpg";
        else if(id.equals("540909"))
            cardurl = "https://cards.scryfall.io/large/front/8/2/823ad188-bd56-476d-9853-bed90bfad582.jpg";
        else if(id.equals("540910"))
            cardurl = "https://cards.scryfall.io/large/back/8/2/823ad188-bd56-476d-9853-bed90bfad582.jpg";
        else if(id.equals("540911"))
            cardurl = "https://cards.scryfall.io/large/front/a/3/a3ff628a-ef8e-45c4-84e7-a33ec28f025a.jpg";
        else if(id.equals("540912"))
            cardurl = "https://cards.scryfall.io/large/back/a/3/a3ff628a-ef8e-45c4-84e7-a33ec28f025a.jpg";
        else if(id.equals("540922"))
            cardurl = "https://cards.scryfall.io/large/front/3/c/3c0fae23-1278-499f-9df7-4a29691726b1.jpg";
        else if(id.equals("540923"))
            cardurl = "https://cards.scryfall.io/large/back/3/c/3c0fae23-1278-499f-9df7-4a29691726b1.jpg";
        else if(id.equals("540941"))
            cardurl = "https://cards.scryfall.io/large/front/a/7/a7cbdd54-7685-4921-ab60-dc36e647a4c5.jpg";
        else if(id.equals("540942"))
            cardurl = "https://cards.scryfall.io/large/back/a/7/a7cbdd54-7685-4921-ab60-dc36e647a4c5.jpg";
        else if(id.equals("540944"))
            cardurl = "https://cards.scryfall.io/large/front/e/6/e61b3afa-66e0-4f7b-84bc-7ae2cc6d28d4.jpg";
        else if(id.equals("540945"))
            cardurl = "https://cards.scryfall.io/large/back/e/6/e61b3afa-66e0-4f7b-84bc-7ae2cc6d28d4.jpg";
        else if(id.equals("540947"))
            cardurl = "https://cards.scryfall.io/large/front/6/1/612b2e6e-fe8d-49ad-b845-6fa7fa59ffd1.jpg";
        else if(id.equals("540948"))
            cardurl = "https://cards.scryfall.io/large/back/6/1/612b2e6e-fe8d-49ad-b845-6fa7fa59ffd1.jpg";
        else if(id.equals("540951"))
            cardurl = "https://cards.scryfall.io/large/front/4/6/467c566e-7f6a-40c9-8fd7-da6ae96df56c.jpg";
        else if(id.equals("540952"))
            cardurl = "https://cards.scryfall.io/large/back/4/6/467c566e-7f6a-40c9-8fd7-da6ae96df56c.jpg";
        else if(id.equals("540967"))
            cardurl = "https://cards.scryfall.io/large/front/9/4/946ca338-5f43-4cff-bd93-1b28449c5fdc.jpg";
        else if(id.equals("540968"))
            cardurl = "https://cards.scryfall.io/large/back/9/4/946ca338-5f43-4cff-bd93-1b28449c5fdc.jpg";
        else if(id.equals("540970"))
            cardurl = "https://cards.scryfall.io/large/front/1/3/13a5e5fd-a67a-4c0e-97ae-923bdbc1be20.jpg";
        else if(id.equals("540971"))
            cardurl = "https://cards.scryfall.io/large/back/1/3/13a5e5fd-a67a-4c0e-97ae-923bdbc1be20.jpg";
        else if(id.equals("540977"))
            cardurl = "https://cards.scryfall.io/large/front/7/f/7fb728de-0d6e-4b32-b0c4-edd7382d1391.jpg";
        else if(id.equals("540978"))
            cardurl = "https://cards.scryfall.io/large/back/7/f/7fb728de-0d6e-4b32-b0c4-edd7382d1391.jpg";
        else if(id.equals("540979"))
            cardurl = "https://cards.scryfall.io/large/front/7/1/71f67ac0-7901-4248-9cb7-2200fb8f893e.jpg";
        else if(id.equals("540980"))
            cardurl = "https://cards.scryfall.io/large/back/7/1/71f67ac0-7901-4248-9cb7-2200fb8f893e.jpg";
        else if(id.equals("540989"))
            cardurl = "https://cards.scryfall.io/large/front/c/a/ca5297a5-bcaa-41fd-a397-e44dc4e00ba3.jpg";
        else if(id.equals("540990"))
            cardurl = "https://cards.scryfall.io/large/back/c/a/ca5297a5-bcaa-41fd-a397-e44dc4e00ba3.jpg";
        else if(id.equals("540994"))
            cardurl = "https://cards.scryfall.io/large/front/3/9/397ffd01-c090-4233-9f5a-5f765886d498.jpg";
        else if(id.equals("540995"))
            cardurl = "https://cards.scryfall.io/large/back/3/9/397ffd01-c090-4233-9f5a-5f765886d498.jpg";
        else if(id.equals("540997"))
            cardurl = "https://cards.scryfall.io/large/front/6/3/63d96c52-66ce-4b46-9a0b-7cd9a43f9253.jpg";
        else if(id.equals("540998"))
            cardurl = "https://cards.scryfall.io/large/back/6/3/63d96c52-66ce-4b46-9a0b-7cd9a43f9253.jpg";
        else if(id.equals("541012"))
            cardurl = "https://cards.scryfall.io/large/front/5/e/5eb3a08e-1d31-4ab9-854f-a86b060696ec.jpg";
        else if(id.equals("541013"))
            cardurl = "https://cards.scryfall.io/large/back/5/e/5eb3a08e-1d31-4ab9-854f-a86b060696ec.jpg";
        else if(id.equals("541018"))
            cardurl = "https://cards.scryfall.io/large/front/f/3/f3d1e90b-0c99-46da-b4f6-4b7be27dbd5c.jpg";
        else if(id.equals("541019"))
            cardurl = "https://cards.scryfall.io/large/back/f/3/f3d1e90b-0c99-46da-b4f6-4b7be27dbd5c.jpg";
        else if(id.equals("541024"))
            cardurl = "https://cards.scryfall.io/large/front/0/c/0cbee24c-9147-46cb-a5f9-8d919c021aa4.jpg";
        else if(id.equals("541025"))
            cardurl = "https://cards.scryfall.io/large/back/0/c/0cbee24c-9147-46cb-a5f9-8d919c021aa4.jpg";
        else if(id.equals("541039"))
            cardurl = "https://cards.scryfall.io/large/front/8/7/87a02ac1-c43a-43cc-9c2b-628cfdeb4cbf.jpg";
        else if(id.equals("541040"))
            cardurl = "https://cards.scryfall.io/large/back/8/7/87a02ac1-c43a-43cc-9c2b-628cfdeb4cbf.jpg";
        else if(id.equals("541042"))
            cardurl = "https://cards.scryfall.io/large/front/a/8/a8b85386-462b-46f8-9412-fd47ed1dc1da.jpg";
        else if(id.equals("541043"))
            cardurl = "https://cards.scryfall.io/large/back/a/8/a8b85386-462b-46f8-9412-fd47ed1dc1da.jpg";
        else if(id.equals("541044"))
            cardurl = "https://cards.scryfall.io/large/front/e/6/e641467b-ac2e-4d29-aed7-5cc227c3b1ce.jpg";
        else if(id.equals("541045"))
            cardurl = "https://cards.scryfall.io/large/back/e/6/e641467b-ac2e-4d29-aed7-5cc227c3b1ce.jpg";
        else if(id.equals("541048"))
            cardurl = "https://cards.scryfall.io/large/front/c/0/c0c358b4-5af2-438f-8bd5-beb0ee6b518b.jpg";
        else if(id.equals("541049"))
            cardurl = "https://cards.scryfall.io/large/back/c/0/c0c358b4-5af2-438f-8bd5-beb0ee6b518b.jpg";
        else if(id.equals("541060"))
            cardurl = "https://cards.scryfall.io/large/front/e/7/e7d3012e-1798-4f72-8d9a-b2d16f817502.jpg";
        else if(id.equals("541061"))
            cardurl = "https://cards.scryfall.io/large/back/e/7/e7d3012e-1798-4f72-8d9a-b2d16f817502.jpg";
        else if(id.equals("541066"))
            cardurl = "https://cards.scryfall.io/large/front/5/4/54a4b031-0919-44aa-a35e-68da7a27235a.jpg";
        else if(id.equals("541067"))
            cardurl = "https://cards.scryfall.io/large/back/5/4/54a4b031-0919-44aa-a35e-68da7a27235a.jpg";
        else if(id.equals("541069"))
            cardurl = "https://cards.scryfall.io/large/front/c/7/c7ceaf83-09c0-4492-a75d-4c47bd421858.jpg";
        else if(id.equals("541070"))
            cardurl = "https://cards.scryfall.io/large/back/c/7/c7ceaf83-09c0-4492-a75d-4c47bd421858.jpg";
        else if(id.equals("541071"))
            cardurl = "https://cards.scryfall.io/large/front/0/9/0936761c-7cdc-49ef-8a0d-ed79219f1056.jpg";
        else if(id.equals("541072"))
            cardurl = "https://cards.scryfall.io/large/back/0/9/0936761c-7cdc-49ef-8a0d-ed79219f1056.jpg";
        else if(id.equals("541078"))
            cardurl = "https://cards.scryfall.io/large/front/8/b/8bcaa944-4e45-457c-be9c-07377b6ed08b.jpg";
        else if(id.equals("541079"))
            cardurl = "https://cards.scryfall.io/large/back/8/b/8bcaa944-4e45-457c-be9c-07377b6ed08b.jpg";
        else if(id.equals("541092"))
            cardurl = "https://cards.scryfall.io/large/front/5/f/5fdf5fc4-69c8-4a59-9095-c2feefb64371.jpg";
        else if(id.equals("541093"))
            cardurl = "https://cards.scryfall.io/large/back/5/f/5fdf5fc4-69c8-4a59-9095-c2feefb64371.jpg";
        else if(id.equals("541094"))
            cardurl = "https://cards.scryfall.io/large/front/7/3/73bf2a0a-b97d-4b90-abd6-d1755734ea15.jpg";
        else if(id.equals("541095"))
            cardurl = "https://cards.scryfall.io/large/back/7/3/73bf2a0a-b97d-4b90-abd6-d1755734ea15.jpg";
        else if(id.equals("541098"))
            cardurl = "https://cards.scryfall.io/large/front/7/a/7a743426-6333-4ca6-9207-163b325ba435.jpg";
        else if(id.equals("541099"))
            cardurl = "https://cards.scryfall.io/large/back/7/a/7a743426-6333-4ca6-9207-163b325ba435.jpg";
        else if(id.equals("541103"))
            cardurl = "https://cards.scryfall.io/large/front/8/d/8d233bc5-8a08-4d38-abd4-21a112141afd.jpg";
        else if(id.equals("541104"))
            cardurl = "https://cards.scryfall.io/large/back/8/d/8d233bc5-8a08-4d38-abd4-21a112141afd.jpg";
        else if(id.equals("541105"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cb168e3c-2c78-4e70-a39b-06aa6a47998c.jpg";
        else if(id.equals("541106"))
            cardurl = "https://cards.scryfall.io/large/back/c/b/cb168e3c-2c78-4e70-a39b-06aa6a47998c.jpg";
        else if(id.equals("541107"))
            cardurl = "https://cards.scryfall.io/large/front/8/c/8c927707-abb5-4a9d-ac53-25df182d6e9b.jpg";
        else if(id.equals("541108"))
            cardurl = "https://cards.scryfall.io/large/back/8/c/8c927707-abb5-4a9d-ac53-25df182d6e9b.jpg";
        else if(id.equals("541109"))
            cardurl = "https://cards.scryfall.io/large/front/6/3/63ba8eef-b834-4031-b0a1-0f8505d53813.jpg";
        else if(id.equals("541110"))
            cardurl = "https://cards.scryfall.io/large/back/6/3/63ba8eef-b834-4031-b0a1-0f8505d53813.jpg";
        else if(id.equals("541120"))
            cardurl = "https://cards.scryfall.io/large/front/f/6/f6c0fca5-b759-4543-95e2-8d712aae5281.jpg";
        else if(id.equals("541121"))
            cardurl = "https://cards.scryfall.io/large/back/f/6/f6c0fca5-b759-4543-95e2-8d712aae5281.jpg";
        else if(id.equals("541131"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a27b9d82-f613-4789-9e8b-f37db5597027.jpg";
        else if(id.equals("541132"))
            cardurl = "https://cards.scryfall.io/large/back/a/2/a27b9d82-f613-4789-9e8b-f37db5597027.jpg";
        else if(id.equals("541143t") || id.equals("541129t") || id.equals("541102t") || id.equals("541101t") || id.equals("540959t") ||
                id.equals("541041t") || id.equals("541035t") || id.equals("541025t") || id.equals("541023t") || id.equals("541011t") ||
                id.equals("541002t") || id.equals("541001t") || id.equals("540999t") || id.equals("540989t") || id.equals("540988t") ||
                id.equals("540980t") || id.equals("540979t") || id.equals("540976t") || id.equals("540964t") || id.equals("540962t") ||
                id.equals("540943t") || id.equals("540939t") || id.equals("540928t") || id.equals("540869t") || id.equals("546990t") ||
                id.equals("546987t") || id.equals("541117t") || id.equals("297471t") || id.equals("297457t") || id.equals("297455t") ||
                id.equals("297445t") || id.equals("297430t") || id.equals("297429t") || id.equals("297369t") || id.equals("297366t") ||
                id.equals("297363t") || id.equals("297358t") || id.equals("297351t") || id.equals("297339t") || id.equals("297330t") ||
                id.equals("297329t") || id.equals("297327t") || id.equals("297317t") || id.equals("297316t") || id.equals("297307t") ||
                id.equals("297308t") || id.equals("297304t") || id.equals("297292t") || id.equals("297290t") || id.equals("297287t") ||
                id.equals("297271t") || id.equals("297268t") || id.equals("297267t") || id.equals("297256t") || id.equals("297197t") ||
                id.equals("297506t") || id.equals("297511t")) //Blood
            cardurl = "https://cards.scryfall.io/large/front/a/6/a6f374bc-cd29-469f-808a-6a6c004ee8aa.jpg";
        else if(id.equals("541110t") || id.equals("297438t")) // Vampire 1/1 Black&White
            cardurl = "https://cards.scryfall.io/large/front/7/e/7eee78d3-c65f-4454-bd3c-1c55388422f5.jpg";
        else if(id.equals("541108t") || id.equals("297436t")) // Spirit 4/4 White
            cardurl = "https://cards.scryfall.io/large/front/b/6/b61e7f44-c112-4501-8850-0565fc857397.jpg";
        else if(id.equals("296841"))
            cardurl = "https://cards.scryfall.io/large/front/c/7/c778f191-0a9b-4d1c-97e5-b5fba9af174d.jpg";
        else if(id.equals("296842"))
            cardurl = "https://cards.scryfall.io/large/back/c/7/c778f191-0a9b-4d1c-97e5-b5fba9af174d.jpg";
        else if(id.equals("296843"))
            cardurl = "https://cards.scryfall.io/large/front/a/3/a3d5a0d4-1f7b-4a88-b375-b241c8e5e117.jpg";
        else if(id.equals("296844"))
            cardurl = "https://cards.scryfall.io/large/back/a/3/a3d5a0d4-1f7b-4a88-b375-b241c8e5e117.jpg";
        else if(id.equals("296845"))
            cardurl = "https://cards.scryfall.io/large/front/9/d/9d7c2b1b-7bef-4ea1-a98a-c6a9c7ed8b43.jpg";
        else if(id.equals("296846"))
            cardurl = "https://cards.scryfall.io/large/back/9/d/9d7c2b1b-7bef-4ea1-a98a-c6a9c7ed8b43.jpg";
        else if(id.equals("296849"))
            cardurl = "https://cards.scryfall.io/large/front/7/2/72df9225-eb06-4f3a-94a5-844c9f6869c7.jpg";
        else if(id.equals("296850"))
            cardurl = "https://cards.scryfall.io/large/back/7/2/72df9225-eb06-4f3a-94a5-844c9f6869c7.jpg";
        else if(id.equals("296856"))
            cardurl = "https://cards.scryfall.io/large/front/3/f/3f024371-a802-4e3f-a039-c178a431b0fd.jpg";
        else if(id.equals("296857"))
            cardurl = "https://cards.scryfall.io/large/back/3/f/3f024371-a802-4e3f-a039-c178a431b0fd.jpg";
        else if(id.equals("296861"))
            cardurl = "https://cards.scryfall.io/large/front/9/7/97a12555-b587-4028-ae23-8085052f0aeb.jpg";
        else if(id.equals("296862"))
            cardurl = "https://cards.scryfall.io/large/back/9/7/97a12555-b587-4028-ae23-8085052f0aeb.jpg";
        else if(id.equals("296872"))
            cardurl = "https://cards.scryfall.io/large/front/0/7/07b9edea-96eb-4146-8922-fbefdb8d57af.jpg";
        else if(id.equals("296873"))
            cardurl = "https://cards.scryfall.io/large/back/0/7/07b9edea-96eb-4146-8922-fbefdb8d57af.jpg";
        else if(id.equals("296874"))
            cardurl = "https://cards.scryfall.io/large/front/5/7/57a7da9c-93b9-4a16-a69c-84e6af8bc8be.jpg";
        else if(id.equals("296875"))
            cardurl = "https://cards.scryfall.io/large/back/5/7/57a7da9c-93b9-4a16-a69c-84e6af8bc8be.jpg";
        else if(id.equals("296889"))
            cardurl = "https://cards.scryfall.io/large/front/2/f/2fb7d8bd-2e25-486b-91e7-b7dd777938e2.jpg";
        else if(id.equals("296890"))
            cardurl = "https://cards.scryfall.io/large/back/2/f/2fb7d8bd-2e25-486b-91e7-b7dd777938e2.jpg";
        else if(id.equals("296893"))
            cardurl = "https://cards.scryfall.io/large/front/d/2/d2febf74-59b4-4419-bff2-bdc5d14c10e9.jpg";
        else if(id.equals("296894"))
            cardurl = "https://cards.scryfall.io/large/back/d/2/d2febf74-59b4-4419-bff2-bdc5d14c10e9.jpg";
        else if(id.equals("296896"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a992bbe3-c389-4a4a-927b-dfc01a4cd9be.jpg";
        else if(id.equals("296897"))
            cardurl = "https://cards.scryfall.io/large/back/a/9/a992bbe3-c389-4a4a-927b-dfc01a4cd9be.jpg";
        else if(id.equals("296905"))
            cardurl = "https://cards.scryfall.io/large/front/d/3/d3255a81-d3bf-4961-bb72-698a39e95f28.jpg";
        else if(id.equals("296906"))
            cardurl = "https://cards.scryfall.io/large/back/d/3/d3255a81-d3bf-4961-bb72-698a39e95f28.jpg";
        else if(id.equals("296912"))
            cardurl = "https://cards.scryfall.io/large/front/c/b/cb76958b-9f53-43ff-95b2-00a066beccdc.jpg";
        else if(id.equals("296913"))
            cardurl = "https://cards.scryfall.io/large/back/c/b/cb76958b-9f53-43ff-95b2-00a066beccdc.jpg";
        else if(id.equals("296915"))
            cardurl = "https://cards.scryfall.io/large/front/3/7/37fe7919-1282-4155-94f8-545eba19b189.jpg";
        else if(id.equals("296916"))
            cardurl = "https://cards.scryfall.io/large/back/3/7/37fe7919-1282-4155-94f8-545eba19b189.jpg";
        else if(id.equals("296921"))
            cardurl = "https://cards.scryfall.io/large/front/c/9/c969947c-015f-4374-9210-b055b9782239.jpg";
        else if(id.equals("296922"))
            cardurl = "https://cards.scryfall.io/large/back/c/9/c969947c-015f-4374-9210-b055b9782239.jpg";
        else if(id.equals("296925"))
            cardurl = "https://cards.scryfall.io/large/front/f/0/f033f980-ddc5-4dea-a682-5a823be99aa5.jpg";
        else if(id.equals("296926"))
            cardurl = "https://cards.scryfall.io/large/back/f/0/f033f980-ddc5-4dea-a682-5a823be99aa5.jpg";
        else if(id.equals("296935"))
            cardurl = "https://cards.scryfall.io/large/front/5/3/5305fc4a-30c6-4aaf-b98c-9a2d273014fc.jpg";
        else if(id.equals("296936"))
            cardurl = "https://cards.scryfall.io/large/back/5/3/5305fc4a-30c6-4aaf-b98c-9a2d273014fc.jpg";
        else if(id.equals("296941"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f5809d03-35d8-4975-bcfb-d42616b70129.jpg";
        else if(id.equals("296942"))
            cardurl = "https://cards.scryfall.io/large/back/f/5/f5809d03-35d8-4975-bcfb-d42616b70129.jpg";
        else if(id.equals("296949"))
            cardurl = "https://cards.scryfall.io/large/front/a/3/a394027b-f728-4a2e-9860-98c6ea45c76a.jpg";
        else if(id.equals("296950"))
            cardurl = "https://cards.scryfall.io/large/back/a/3/a394027b-f728-4a2e-9860-98c6ea45c76a.jpg";
        else if(id.equals("296952"))
            cardurl = "https://cards.scryfall.io/large/front/3/3/3327a287-a6db-49a4-9d25-315f3c93a930.jpg";
        else if(id.equals("296953"))
            cardurl = "https://cards.scryfall.io/large/back/3/3/3327a287-a6db-49a4-9d25-315f3c93a930.jpg";
        else if(id.equals("296959"))
            cardurl = "https://cards.scryfall.io/large/front/9/1/91a861d9-318e-43eb-9552-59072d3fca8e.jpg";
        else if(id.equals("296960"))
            cardurl = "https://cards.scryfall.io/large/back/9/1/91a861d9-318e-43eb-9552-59072d3fca8e.jpg";
        else if(id.equals("296964"))
            cardurl = "https://cards.scryfall.io/large/front/6/6/66ef5058-af82-4867-a6b6-9e08274bd0d3.jpg";
        else if(id.equals("296965"))
            cardurl = "https://cards.scryfall.io/large/back/6/6/66ef5058-af82-4867-a6b6-9e08274bd0d3.jpg";
        else if(id.equals("296966"))
            cardurl = "https://cards.scryfall.io/large/front/8/8/88a28a48-d4a1-47ce-9aa0-ee65325029c4.jpg";
        else if(id.equals("296967"))
            cardurl = "https://cards.scryfall.io/large/back/8/8/88a28a48-d4a1-47ce-9aa0-ee65325029c4.jpg";
        else if(id.equals("296971"))
            cardurl = "https://cards.scryfall.io/large/front/4/3/439d4cd6-afca-46d3-8850-898cb9212a26.jpg";
        else if(id.equals("296972"))
            cardurl = "https://cards.scryfall.io/large/back/4/3/439d4cd6-afca-46d3-8850-898cb9212a26.jpg";
        else if(id.equals("296983"))
            cardurl = "https://cards.scryfall.io/large/front/d/0/d0ad3323-1016-42da-b47e-9a61117f755f.jpg";
        else if(id.equals("296984"))
            cardurl = "https://cards.scryfall.io/large/back/d/0/d0ad3323-1016-42da-b47e-9a61117f755f.jpg";
        else if(id.equals("296990"))
            cardurl = "https://cards.scryfall.io/large/front/8/3/832b0d59-37ca-42b3-be37-4f4e633589b0.jpg";
        else if(id.equals("296991"))
            cardurl = "https://cards.scryfall.io/large/back/8/3/832b0d59-37ca-42b3-be37-4f4e633589b0.jpg";
        else if(id.equals("297004"))
            cardurl = "https://cards.scryfall.io/large/front/b/e/be163079-0372-48f8-93a9-b582702bff16.jpg";
        else if(id.equals("297005"))
            cardurl = "https://cards.scryfall.io/large/back/b/e/be163079-0372-48f8-93a9-b582702bff16.jpg";
        else if(id.equals("297007"))
            cardurl = "https://cards.scryfall.io/large/front/c/f/cf183650-13a7-47d6-8ee9-fa8ef3df9f78.jpg";
        else if(id.equals("297008"))
            cardurl = "https://cards.scryfall.io/large/back/c/f/cf183650-13a7-47d6-8ee9-fa8ef3df9f78.jpg";
        else if(id.equals("297010"))
            cardurl = "https://cards.scryfall.io/large/front/8/1/8124a65d-c309-475d-bd8d-4cd45294e13a.jpg";
        else if(id.equals("297011"))
            cardurl = "https://cards.scryfall.io/large/back/8/1/8124a65d-c309-475d-bd8d-4cd45294e13a.jpg";
        else if(id.equals("297025"))
            cardurl = "https://cards.scryfall.io/large/front/0/6/061171f6-7da3-494d-82d1-1406fd7f5439.jpg";
        else if(id.equals("297026"))
            cardurl = "https://cards.scryfall.io/large/back/0/6/061171f6-7da3-494d-82d1-1406fd7f5439.jpg";
        else if(id.equals("297028"))
            cardurl = "https://cards.scryfall.io/large/front/8/f/8ffb9873-4fb1-4644-9dd9-f2eb5f9617ac.jpg";
        else if(id.equals("297029"))
            cardurl = "https://cards.scryfall.io/large/back/8/f/8ffb9873-4fb1-4644-9dd9-f2eb5f9617ac.jpg";
        else if(id.equals("297030"))
            cardurl = "https://cards.scryfall.io/large/front/5/9/59caee03-6b8c-447d-945d-ebf897a8a524.jpg";
        else if(id.equals("297031"))
            cardurl = "https://cards.scryfall.io/large/back/5/9/59caee03-6b8c-447d-945d-ebf897a8a524.jpg";
        else if(id.equals("297034"))
            cardurl = "https://cards.scryfall.io/large/front/b/0/b0d1ab73-7dbe-40d4-856c-f49ce29706fa.jpg";
        else if(id.equals("297035"))
            cardurl = "https://cards.scryfall.io/large/back/b/0/b0d1ab73-7dbe-40d4-856c-f49ce29706fa.jpg";
        else if(id.equals("297037"))
            cardurl = "https://cards.scryfall.io/large/front/d/e/de968157-3567-4df9-80bd-2a541dbaddf6.jpg";
        else if(id.equals("297038"))
            cardurl = "https://cards.scryfall.io/large/back/d/e/de968157-3567-4df9-80bd-2a541dbaddf6.jpg";
        else if(id.equals("297042"))
            cardurl = "https://cards.scryfall.io/large/front/5/c/5c75740e-9688-47e2-91d6-fbf00b3f7fbb.jpg";
        else if(id.equals("297043"))
            cardurl = "https://cards.scryfall.io/large/back/5/c/5c75740e-9688-47e2-91d6-fbf00b3f7fbb.jpg";
        else if(id.equals("297048"))
            cardurl = "https://cards.scryfall.io/large/front/8/e/8e1300df-ccd6-48f4-8f0b-b3c10c4ae0b0.jpg";
        else if(id.equals("297049"))
            cardurl = "https://cards.scryfall.io/large/back/8/e/8e1300df-ccd6-48f4-8f0b-b3c10c4ae0b0.jpg";
        else if(id.equals("297056"))
            cardurl = "https://cards.scryfall.io/large/front/9/c/9ccabbd8-a776-46e8-9e0e-6f917c123037.jpg";
        else if(id.equals("297057"))
            cardurl = "https://cards.scryfall.io/large/back/9/c/9ccabbd8-a776-46e8-9e0e-6f917c123037.jpg";
        else if(id.equals("297063"))
            cardurl = "https://cards.scryfall.io/large/front/5/e/5e391f2d-4a74-4201-b046-d021c0551aba.jpg";
        else if(id.equals("297064"))
            cardurl = "https://cards.scryfall.io/large/back/5/e/5e391f2d-4a74-4201-b046-d021c0551aba.jpg";
        else if(id.equals("297067"))
            cardurl = "https://cards.scryfall.io/large/front/a/9/a9f52ae3-5cff-4a32-896d-a66cff51e97f.jpg";
        else if(id.equals("297068"))
            cardurl = "https://cards.scryfall.io/large/back/a/9/a9f52ae3-5cff-4a32-896d-a66cff51e97f.jpg";
        else if(id.equals("297081"))
            cardurl = "https://cards.scryfall.io/large/front/b/1/b1384d31-9772-41e9-b6f2-0e2d7961f868.jpg";
        else if(id.equals("297082"))
            cardurl = "https://cards.scryfall.io/large/back/b/1/b1384d31-9772-41e9-b6f2-0e2d7961f868.jpg";
        else if(id.equals("297083"))
            cardurl = "https://cards.scryfall.io/large/front/4/7/47f1be55-1a95-49e0-8943-4b8fa2f2f987.jpg";
        else if(id.equals("297084"))
            cardurl = "https://cards.scryfall.io/large/back/4/7/47f1be55-1a95-49e0-8943-4b8fa2f2f987.jpg";
        else if(id.equals("297091"))
            cardurl = "https://cards.scryfall.io/large/front/5/b/5be8204f-8bb3-466c-a0c5-52b25bcdb7ba.jpg";
        else if(id.equals("297092"))
            cardurl = "https://cards.scryfall.io/large/back/5/b/5be8204f-8bb3-466c-a0c5-52b25bcdb7ba.jpg";
        else if(id.equals("297098"))
            cardurl = "https://cards.scryfall.io/large/front/4/d/4df8d711-09e3-43e5-b59e-ea6a92cae726.jpg";
        else if(id.equals("297099"))
            cardurl = "https://cards.scryfall.io/large/back/4/d/4df8d711-09e3-43e5-b59e-ea6a92cae726.jpg";
        else if(id.equals("297100"))
            cardurl = "https://cards.scryfall.io/large/front/0/8/085f7c47-002d-4213-9466-65a7b8e6c0cb.jpg";
        else if(id.equals("297101"))
            cardurl = "https://cards.scryfall.io/large/back/0/8/085f7c47-002d-4213-9466-65a7b8e6c0cb.jpg";
        else if(id.equals("297114"))
            cardurl = "https://cards.scryfall.io/large/front/f/2/f25ce5a4-9dc1-4293-9cde-0af098aa1877.jpg";
        else if(id.equals("297115"))
            cardurl = "https://cards.scryfall.io/large/back/f/2/f25ce5a4-9dc1-4293-9cde-0af098aa1877.jpg";
        else if(id.equals("297117"))
            cardurl = "https://cards.scryfall.io/large/front/6/1/61a15b38-30f9-4d8e-9511-57a1c11304d0.jpg";
        else if(id.equals("297118"))
            cardurl = "https://cards.scryfall.io/large/back/6/1/61a15b38-30f9-4d8e-9511-57a1c11304d0.jpg";
        else if(id.equals("297131"))
            cardurl = "https://cards.scryfall.io/large/front/3/d/3d7b401f-79aa-4584-b42b-e81e15c7065b.jpg";
        else if(id.equals("297132"))
            cardurl = "https://cards.scryfall.io/large/back/3/d/3d7b401f-79aa-4584-b42b-e81e15c7065b.jpg";
        else if(id.equals("297142"))
            cardurl = "https://cards.scryfall.io/large/front/0/4/04c75e86-d5aa-43b0-be30-1a5115b5bc13.jpg";
        else if(id.equals("297143"))
            cardurl = "https://cards.scryfall.io/large/back/0/4/04c75e86-d5aa-43b0-be30-1a5115b5bc13.jpg";
        else if(id.equals("297151"))
            cardurl = "https://cards.scryfall.io/large/front/e/d/ed39ce30-d73c-48a8-a843-a9c1122bfc72.jpg";
        else if(id.equals("297152"))
            cardurl = "https://cards.scryfall.io/large/back/e/d/ed39ce30-d73c-48a8-a843-a9c1122bfc72.jpg";
        else if(id.equals("297164"))
            cardurl = "https://cards.scryfall.io/large/front/f/f/ff47f351-8e1a-4b39-9598-fe4230b14c15.jpg";
        else if(id.equals("297165"))
            cardurl = "https://cards.scryfall.io/large/back/f/f/ff47f351-8e1a-4b39-9598-fe4230b14c15.jpg";
        else if(id.equals("297166"))
            cardurl = "https://cards.scryfall.io/large/front/8/3/836083e8-0723-4131-beee-23c4afd9da73.jpg";
        else if(id.equals("297167"))
            cardurl = "https://cards.scryfall.io/large/back/8/3/836083e8-0723-4131-beee-23c4afd9da73.jpg";
        else if(id.equals("297169"))
            cardurl = "https://cards.scryfall.io/large/front/0/d/0d8fa256-3535-4c46-bc3c-dcee457519ab.jpg";
        else if(id.equals("297170"))
            cardurl = "https://cards.scryfall.io/large/back/0/d/0d8fa256-3535-4c46-bc3c-dcee457519ab.jpg";
        else if(id.equals("297179"))
            cardurl = "https://cards.scryfall.io/large/front/e/4/e45300aa-02c4-4d4d-bf6d-e5d9342b471a.jpg";
        else if(id.equals("297180"))
            cardurl = "https://cards.scryfall.io/large/back/e/4/e45300aa-02c4-4d4d-bf6d-e5d9342b471a.jpg";
        else if(id.equals("297181"))
            cardurl = "https://cards.scryfall.io/large/front/6/b/6be6513d-af90-4eb7-a639-d08d77445cfb.jpg";
        else if(id.equals("297182"))
            cardurl = "https://cards.scryfall.io/large/back/6/b/6be6513d-af90-4eb7-a639-d08d77445cfb.jpg";
        else if(id.equals("297188"))
            cardurl = "https://cards.scryfall.io/large/front/0/6/0650124f-4dbe-450a-bbd1-28b0071c02c5.jpg";
        else if(id.equals("297189"))
            cardurl = "https://cards.scryfall.io/large/back/0/6/0650124f-4dbe-450a-bbd1-28b0071c02c5.jpg";
        else if(id.equals("297192"))
            cardurl = "https://cards.scryfall.io/large/front/3/e/3e2ddb8e-a448-4ae4-8b2f-dd66190805be.jpg";
        else if(id.equals("297193"))
            cardurl = "https://cards.scryfall.io/large/back/3/e/3e2ddb8e-a448-4ae4-8b2f-dd66190805be.jpg";
        else if(id.equals("297202"))
            cardurl = "https://cards.scryfall.io/large/front/f/3/f3393c4b-b0d0-4c0c-a29c-5ff4c6d1c8b4.jpg";
        else if(id.equals("297203"))
            cardurl = "https://cards.scryfall.io/large/back/f/3/f3393c4b-b0d0-4c0c-a29c-5ff4c6d1c8b4.jpg";
        else if(id.equals("297208"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f56277ed-1c80-48c8-8fe0-e2e3a05e7caf.jpg";
        else if(id.equals("297209"))
            cardurl = "https://cards.scryfall.io/large/back/f/5/f56277ed-1c80-48c8-8fe0-e2e3a05e7caf.jpg";
        else if(id.equals("297212"))
            cardurl = "https://cards.scryfall.io/large/front/8/2/82699d05-d36f-4928-9713-a985f417ea54.jpg";
        else if(id.equals("297213"))
            cardurl = "https://cards.scryfall.io/large/back/8/2/82699d05-d36f-4928-9713-a985f417ea54.jpg";
        else if(id.equals("297214"))
            cardurl = "https://cards.scryfall.io/large/front/7/7/77f2c03a-21ba-489a-9d75-6afd30998589.jpg";
        else if(id.equals("297215"))
            cardurl = "https://cards.scryfall.io/large/back/7/7/77f2c03a-21ba-489a-9d75-6afd30998589.jpg";
        else if(id.equals("297228"))
            cardurl = "https://cards.scryfall.io/large/front/7/d/7dc8a0a3-82a3-4feb-900a-04e7289ad716.jpg";
        else if(id.equals("297229"))
            cardurl = "https://cards.scryfall.io/large/back/7/d/7dc8a0a3-82a3-4feb-900a-04e7289ad716.jpg";
        else if(id.equals("297232"))
            cardurl = "https://cards.scryfall.io/large/front/8/2/823a764f-76b5-4340-bdaf-df9ca1d0ffd2.jpg";
        else if(id.equals("297233"))
            cardurl = "https://cards.scryfall.io/large/back/8/2/823a764f-76b5-4340-bdaf-df9ca1d0ffd2.jpg";
        else if(id.equals("297234"))
            cardurl = "https://cards.scryfall.io/large/front/5/2/52640a92-b5c2-4dd8-8cea-f2f6d72ff985.jpg";
        else if(id.equals("297235"))
            cardurl = "https://cards.scryfall.io/large/back/5/2/52640a92-b5c2-4dd8-8cea-f2f6d72ff985.jpg";
        else if(id.equals("297237"))
            cardurl = "https://cards.scryfall.io/large/front/3/d/3dab44c3-17a1-4213-87d9-5ee0c6064d36.jpg";
        else if(id.equals("297238"))
            cardurl = "https://cards.scryfall.io/large/back/3/d/3dab44c3-17a1-4213-87d9-5ee0c6064d36.jpg";
        else if(id.equals("297239"))
            cardurl = "https://cards.scryfall.io/large/front/9/b/9b3fc117-dc46-46bc-a2e2-3f671f5c2b2e.jpg";
        else if(id.equals("297240"))
            cardurl = "https://cards.scryfall.io/large/back/9/b/9b3fc117-dc46-46bc-a2e2-3f671f5c2b2e.jpg";
        else if(id.equals("297250"))
            cardurl = "https://cards.scryfall.io/large/front/9/a/9a6cb130-6d2f-44ba-a390-79dd4e919864.jpg";
        else if(id.equals("297251"))
            cardurl = "https://cards.scryfall.io/large/back/9/a/9a6cb130-6d2f-44ba-a390-79dd4e919864.jpg";
        else if(id.equals("297269"))
            cardurl = "https://cards.scryfall.io/large/front/1/3/1322fdf0-47ee-47b8-9e93-b75fc6901ba2.jpg";
        else if(id.equals("297270"))
            cardurl = "https://cards.scryfall.io/large/back/1/3/1322fdf0-47ee-47b8-9e93-b75fc6901ba2.jpg";
        else if(id.equals("297272"))
            cardurl = "https://cards.scryfall.io/large/front/a/2/a27b48bf-aa80-4159-aa48-8a56c6780860.jpg";
        else if(id.equals("297273"))
            cardurl = "https://cards.scryfall.io/large/back/a/2/a27b48bf-aa80-4159-aa48-8a56c6780860.jpg";
        else if(id.equals("297275"))
            cardurl = "https://cards.scryfall.io/large/front/3/a/3a000aea-6056-4417-a4b2-19202a1a9192.jpg";
        else if(id.equals("297276"))
            cardurl = "https://cards.scryfall.io/large/back/3/a/3a000aea-6056-4417-a4b2-19202a1a9192.jpg";
        else if(id.equals("297279"))
            cardurl = "https://cards.scryfall.io/large/front/f/f/ffa2c10a-edbc-4bce-94c5-0c1622fd5d1a.jpg";
        else if(id.equals("297280"))
            cardurl = "https://cards.scryfall.io/large/back/f/f/ffa2c10a-edbc-4bce-94c5-0c1622fd5d1a.jpg";
        else if(id.equals("297295"))
            cardurl = "https://cards.scryfall.io/large/front/f/5/f52805c6-d226-4e7b-8c3a-767ed522739e.jpg";
        else if(id.equals("297296"))
            cardurl = "https://cards.scryfall.io/large/back/f/5/f52805c6-d226-4e7b-8c3a-767ed522739e.jpg";
        else if(id.equals("297298"))
            cardurl = "https://cards.scryfall.io/large/front/b/6/b60db1f0-5b59-497d-a9b6-e65ca14380d2.jpg";
        else if(id.equals("297299"))
            cardurl = "https://cards.scryfall.io/large/back/b/6/b60db1f0-5b59-497d-a9b6-e65ca14380d2.jpg";
        else if(id.equals("297305"))
            cardurl = "https://cards.scryfall.io/large/front/8/9/89f67781-3213-455b-9f29-90b21809ca52.jpg";
        else if(id.equals("297306"))
            cardurl = "https://cards.scryfall.io/large/back/8/9/89f67781-3213-455b-9f29-90b21809ca52.jpg";
        else if(id.equals("297307"))
            cardurl = "https://cards.scryfall.io/large/front/0/c/0c6c01a9-67f7-4e96-b44e-b28379b41cc2.jpg";
        else if(id.equals("297308"))
            cardurl = "https://cards.scryfall.io/large/back/0/c/0c6c01a9-67f7-4e96-b44e-b28379b41cc2.jpg";
        else if(id.equals("297317"))
            cardurl = "https://cards.scryfall.io/large/front/5/2/5299eaa4-9223-4180-8339-8cf9af3a41b1.jpg";
        else if(id.equals("297318"))
            cardurl = "https://cards.scryfall.io/large/back/5/2/5299eaa4-9223-4180-8339-8cf9af3a41b1.jpg";
        else if(id.equals("297322"))
            cardurl = "https://cards.scryfall.io/large/front/6/a/6adf21b9-b6d5-4c27-b3f2-60ec0ff17416.jpg";
        else if(id.equals("297323"))
            cardurl = "https://cards.scryfall.io/large/back/6/a/6adf21b9-b6d5-4c27-b3f2-60ec0ff17416.jpg";
        else if(id.equals("297325"))
            cardurl = "https://cards.scryfall.io/large/front/a/f/af2efb08-ffba-4d88-b7df-4530f29af635.jpg";
        else if(id.equals("297326"))
            cardurl = "https://cards.scryfall.io/large/back/a/f/af2efb08-ffba-4d88-b7df-4530f29af635.jpg";
        else if(id.equals("297340"))
            cardurl = "https://cards.scryfall.io/large/front/7/f/7f00e6fb-1c2e-433e-a8ac-fd692699c8c7.jpg";
        else if(id.equals("297341"))
            cardurl = "https://cards.scryfall.io/large/back/7/f/7f00e6fb-1c2e-433e-a8ac-fd692699c8c7.jpg";
        else if(id.equals("297346"))
            cardurl = "https://cards.scryfall.io/large/front/3/6/36c22e93-4b37-4912-9491-eff17510b08d.jpg";
        else if(id.equals("297347"))
            cardurl = "https://cards.scryfall.io/large/back/3/6/36c22e93-4b37-4912-9491-eff17510b08d.jpg";
        else if(id.equals("297352"))
            cardurl = "https://cards.scryfall.io/large/front/a/0/a00fb8c2-2bff-48a5-a7a8-a73146f97d73.jpg";
        else if(id.equals("297353"))
            cardurl = "https://cards.scryfall.io/large/back/a/0/a00fb8c2-2bff-48a5-a7a8-a73146f97d73.jpg";
        else if(id.equals("297367"))
            cardurl = "https://cards.scryfall.io/large/front/7/0/70925279-7364-4fbc-a29e-b8711b96a1ed.jpg";
        else if(id.equals("297368"))
            cardurl = "https://cards.scryfall.io/large/back/7/0/70925279-7364-4fbc-a29e-b8711b96a1ed.jpg";
        else if(id.equals("297370"))
            cardurl = "https://cards.scryfall.io/large/front/1/f/1f626db8-58ea-4d1c-a034-2305dac4ae6e.jpg";
        else if(id.equals("297371"))
            cardurl = "https://cards.scryfall.io/large/back/1/f/1f626db8-58ea-4d1c-a034-2305dac4ae6e.jpg";
        else if(id.equals("297372"))
            cardurl = "https://cards.scryfall.io/large/front/6/f/6f1ba822-dff7-430a-b34c-032210d53189.jpg";
        else if(id.equals("297373"))
            cardurl = "https://cards.scryfall.io/large/back/6/f/6f1ba822-dff7-430a-b34c-032210d53189.jpg";
        else if(id.equals("297376"))
            cardurl = "https://cards.scryfall.io/large/front/1/9/19781e71-62b4-45ad-bf71-a95ed9230d30.jpg";
        else if(id.equals("297377"))
            cardurl = "https://cards.scryfall.io/large/back/1/9/19781e71-62b4-45ad-bf71-a95ed9230d30.jpg";
        else if(id.equals("297388"))
            cardurl = "https://cards.scryfall.io/large/front/b/f/bf8d22ec-792a-4b91-a087-d4cae21be743.jpg";
        else if(id.equals("297389"))
            cardurl = "https://cards.scryfall.io/large/back/b/f/bf8d22ec-792a-4b91-a087-d4cae21be743.jpg";
        else if(id.equals("297394"))
            cardurl = "https://cards.scryfall.io/large/front/1/a/1aff161b-253f-4e79-b8b2-e9c16e1d6aec.jpg";
        else if(id.equals("297395"))
            cardurl = "https://cards.scryfall.io/large/back/1/a/1aff161b-253f-4e79-b8b2-e9c16e1d6aec.jpg";
        else if(id.equals("297397"))
            cardurl = "https://cards.scryfall.io/large/front/d/e/de7b7d93-fe69-4b18-b398-cf1a61eed175.jpg";
        else if(id.equals("297398"))
            cardurl = "https://cards.scryfall.io/large/back/d/e/de7b7d93-fe69-4b18-b398-cf1a61eed175.jpg";
        else if(id.equals("297399"))
            cardurl = "https://cards.scryfall.io/large/front/e/7/e7e246b5-9c21-45d1-951b-ab5a47b4e7e4.jpg";
        else if(id.equals("297400"))
            cardurl = "https://cards.scryfall.io/large/back/e/7/e7e246b5-9c21-45d1-951b-ab5a47b4e7e4.jpg";
        else if(id.equals("297406"))
            cardurl = "https://cards.scryfall.io/large/front/5/7/57f1736a-bbbd-4707-9b0c-1063c6dbdbc0.jpg";
        else if(id.equals("297407"))
            cardurl = "https://cards.scryfall.io/large/back/5/7/57f1736a-bbbd-4707-9b0c-1063c6dbdbc0.jpg";
        else if(id.equals("297420"))
            cardurl = "https://cards.scryfall.io/large/front/6/8/68831286-be87-4552-b658-4410a5e96148.jpg";
        else if(id.equals("297421"))
            cardurl = "https://cards.scryfall.io/large/back/6/8/68831286-be87-4552-b658-4410a5e96148.jpg";
        else if(id.equals("297422"))
            cardurl = "https://cards.scryfall.io/large/front/d/1/d19f2cf1-746d-45b9-8601-4956bbda1096.jpg";
        else if(id.equals("297423"))
            cardurl = "https://cards.scryfall.io/large/back/d/1/d19f2cf1-746d-45b9-8601-4956bbda1096.jpg";
        else if(id.equals("297426"))
            cardurl = "https://cards.scryfall.io/large/front/a/d/ad198283-db1a-4312-a95c-f2f889072e32.jpg";
        else if(id.equals("297427"))
            cardurl = "https://cards.scryfall.io/large/back/a/d/ad198283-db1a-4312-a95c-f2f889072e32.jpg";
        else if(id.equals("297431"))
            cardurl = "https://cards.scryfall.io/large/front/4/2/4205f8f8-7b18-4999-ac51-860fab376d79.jpg";
        else if(id.equals("297432"))
            cardurl = "https://cards.scryfall.io/large/back/4/2/4205f8f8-7b18-4999-ac51-860fab376d79.jpg";
        else if(id.equals("297433"))
            cardurl = "https://cards.scryfall.io/large/front/7/9/79fbd424-8668-4e36-8e2f-780362c9915e.jpg";
        else if(id.equals("297434"))
            cardurl = "https://cards.scryfall.io/large/back/7/9/79fbd424-8668-4e36-8e2f-780362c9915e.jpg";
        else if(id.equals("297435"))
            cardurl = "https://cards.scryfall.io/large/front/6/8/680ce9a3-b47a-4f79-8254-f0935266b467.jpg";
        else if(id.equals("297436"))
            cardurl = "https://cards.scryfall.io/large/back/6/8/680ce9a3-b47a-4f79-8254-f0935266b467.jpg";
        else if(id.equals("297437"))
            cardurl = "https://cards.scryfall.io/large/front/7/8/78f57a41-e9a6-4f71-92a0-46c4534c8fca.jpg";
        else if(id.equals("297438"))
            cardurl = "https://cards.scryfall.io/large/back/7/8/78f57a41-e9a6-4f71-92a0-46c4534c8fca.jpg";
        else if(id.equals("297448"))
            cardurl = "https://cards.scryfall.io/large/front/3/4/348f80f6-a5e2-4c01-a368-1ce935af8212.jpg";
        else if(id.equals("297449"))
            cardurl = "https://cards.scryfall.io/large/back/3/4/348f80f6-a5e2-4c01-a368-1ce935af8212.jpg";
        else if(id.equals("297459"))
            cardurl = "https://cards.scryfall.io/large/front/6/6/66f4d0b2-fe17-4cfe-857e-fcee289c89f7.jpg";
        else if(id.equals("297460"))
            cardurl = "https://cards.scryfall.io/large/back/6/6/66f4d0b2-fe17-4cfe-857e-fcee289c89f7.jpg";
        else if(id.equals("297452t")) // 1/1 Human Soldier white/green
            cardurl = "https://cards.scryfall.io/large/front/5/f/5f3c4810-7359-42b7-905f-4845f6d1daf6.jpg";
        else if(id.equals("297312t")) // 1/1 Slug
            cardurl = "https://cards.scryfall.io/large/front/6/e/6e2ae34f-4558-46e0-95c5-e00d813fa355.jpg";
        else if (id.equals("297227t")) //Zombie */* blue
            cardurl = "https://cards.scryfall.io/large/front/5/3/539f4b60-667b-469d-9191-eacaad5c0db1.jpg";
        else if(id.equals("297522t")) // 1/1 Devil red
            cardurl = "https://cards.scryfall.io/large/front/3/e/3e78c4b8-371b-43d7-a315-fb299704aa60.jpg";
        else if(id.equals("297521"))
            cardurl = "https://cards.scryfall.io/large/front/2/f/2f986406-bfe3-4e59-bcb6-839ef5f1fbc4.jpg";
        else if(id.equals("297543"))
            cardurl = "https://cards.scryfall.io/large/back/2/f/2f986406-bfe3-4e59-bcb6-839ef5f1fbc4.jpg";
        else if(id.equals("615848t"))
            cardurl = "https://cards.scryfall.io/large/front/b/f/bf36408d-ed85-497f-8e68-d3a922c388a0.jpg";
        else if(id.equals("615846t"))
            cardurl = "https://cards.scryfall.io/large/front/5/a/5a4649cc-07fb-4ff0-9ac6-846763b799df.jpg";
        else if (id.equals("583789t")) // Powerstone
            cardurl = "https://cards.scryfall.io/large/front/d/4/d45fe4b6-aeaf-4f84-b660-c7b482ed8512.jpg";
        else if(id.equals("612509"))
            cardurl = "https://cards.scryfall.io/large/front/8/4/84c63c27-6095-4fbc-9064-0ef602e1ced8.jpg";
		
        return cardurl;
    }

    public static String getSpecialTokenUrl(String id, String set) {
        String tokenurl = "";

        if(id.equals("296754t") || id.equals("296741t") || id.equals("296730t") || id.equals("296728t") || id.equals("296723t") ||
                id.equals("296696t") ||  id.equals("296697t") || id.equals("296606t")) //Squirrel 1/1
            tokenurl = "https://cards.scryfall.io/large/front/9/7/977ddd05-1aae-46fc-95ce-866710d1c5c6.jpg";
        else if(id.equals("546983t") || id.equals("547250t") || id.equals("612139t") || id.equals("614765t")) // Blood
            tokenurl = "https://cards.scryfall.io/large/front/a/6/a6f374bc-cd29-469f-808a-6a6c004ee8aa.jpg";
        else if(id.equals("545775t")) // Angel 4/4
            tokenurl = "https://cards.scryfall.io/large/front/f/f/ff0335da-631f-46b8-bfa1-b2f210c91f5f.jpg";
        else if(id.equals("546982t")) // Vampire 1/1 Black&White
            tokenurl = "https://cards.scryfall.io/large/front/7/e/7eee78d3-c65f-4454-bd3c-1c55388422f5.jpg";
        else if(id.equals("547230t")) // Bat 1/1
            tokenurl = "https://cards.scryfall.io/large/front/e/5/e5c0f400-41be-488b-be84-b07289b1ef62.jpg";
        else if(id.equals("545715t")) // Spirit 3/3
            tokenurl = "https://i.pinimg.com/564x/b2/00/cb/b200cb9d331d8e019b4a48db99513306.jpg";
        else if(id.equals("540847t")) // Cleric Spirit 4/4
            tokenurl = "https://cards.scryfall.io/large/front/5/2/5212bae5-d768-45ab-aba8-94c4f9fabc79.jpg";
        else if(id.equals("539344t") || id.equals("541114t") || id.equals("545716t") || id.equals("545711t") || id.equals("545720t") ||
                id.equals("545706t") || id.equals("545702t") || id.equals("545709t") || id.equals("545708t") || id.equals("545795t") ||
                id.equals("546978t") || id.equals("546976t") || id.equals("546971t") || id.equals("546970t") || id.equals("547249t") ||
                id.equals("545698t")) // Spirit 1/1
            tokenurl = "https://cards.scryfall.io/large/front/c/8/c865bc02-0562-408c-b18e-0e66da906fc6.jpg";
        else if(id.equals("540460t") || id.equals("540461t") || id.equals("540729t") || id.equals("296988t") || id.equals("296982t") ||
                id.equals("296979t") || id.equals("296968t") || id.equals("296962t") || id.equals("296955t") || id.equals("296933t") ||
                id.equals("296927t") || id.equals("296904t") || id.equals("296902t")) // Zombie 2/2 Decayed
            tokenurl = "https://cards.scryfall.io/large/front/6/a/6adb8607-1066-451d-a719-74ad32358278.jpg";
        else if(id.equals("540749t")) // Human 1/1
            tokenurl = "https://cards.scryfall.io/large/front/b/7/b7667345-e11b-4cad-ac4c-84eb1c5656c5.jpg";
        else if(id.equals("534963t")) // Ooze green X/X
            tokenurl = "https://cards.scryfall.io/large/front/f/a/faa10292-f358-48c1-a516-9a1eecf62b1d.jpg";
        else if(id.equals("534938t")) // Elemental red X/X
            tokenurl = "https://cards.scryfall.io/large/front/c/4/c4052aed-981b-41d0-85f0-20c2599811ba.jpg";
        else if(id.equals("534999t")) // Treefolk green X/X
            tokenurl = "https://cards.scryfall.io/large/front/9/4/94e4345b-61b1-4026-a01c-c9f2036c5c8a.jpg";
        else if(id.equals("296713t")) //Bear 2/2
            tokenurl = "https://cards.scryfall.io/large/front/c/8/c879d4a6-cef5-48f1-8c08-f5b59ec850de.jpg";
        else if(id.equals("296771t") || id.equals("296738t") || id.equals("540468t")) //Spider 1/2
            tokenurl = "https://cards.scryfall.io/large/front/0/1/01591603-d903-419d-9957-cf0ae7f79240.jpg";
        else if(id.equals("296753t") || id.equals("296707t") || id.equals("296708t")) //Beast 4/4
            tokenurl = "https://cards.scryfall.io/large/front/0/6/06b5e4d2-7eac-4ee9-82aa-80a668705679.jpg";
        else if(id.equals("296546t")) //Illusion 1/1
            tokenurl = "https://cards.scryfall.io/large/front/e/b/ebccb29b-8b69-4813-94bb-d96e117b609e.jpg";
        else if(id.equals("296665t") || id.equals("296655t") || id.equals("296615t")) //Goblin 1/1
            tokenurl = "https://cards.scryfall.io/large/front/1/4/1425e965-7eea-419c-a7ec-c8169fa9edbf.jpg";
        else if(id.equals("296562t") || id.equals("296559t") || id.equals("296539t")) //Crab 0/3
            tokenurl = "https://cards.scryfall.io/large/front/7/e/7ef7f37a-b7f5-45a1-8f2b-7097089ca2e5.jpg";
        else if(id.equals("296793t") || id.equals("296786t")) //Phyrexian Germ 0/0
            tokenurl = "https://cards.scryfall.io/large/front/b/5/b53e0681-603e-4180-bc86-3dadf214e61a.jpg";
        else if(id.equals("296787t") || id.equals("296490t")) //Shapeshifter 2/2
            tokenurl = "https://cards.scryfall.io/large/front/a/3/a33fda72-e61d-478f-bc33-ff1a23b5f45b.jpg";
        else if(id.equals("296472t")) //Phyrexian Golem 3/3
            tokenurl = "https://cards.scryfall.io/large/front/7/b/7becaa04-f142-4163-9286-00018b95c4ca.jpg";
        else if (id.equals("296763t") || id.equals("539412t") || id.equals("539375t") || id.equals("539360t")) //Zombie Army 0/0
            tokenurl = "https://cards.scryfall.io/large/front/c/4/c46d82e0-ef99-473c-a09c-8f552db759bf.jpg";
        else if(id.equals("296601t")) //Rat 1/1
            tokenurl = "https://cards.scryfall.io/large/front/1/a/1a85fe9d-ef18-46c4-88b0-cf2e222e30e4.jpg";
        else if (id.equals("296673t")) //Elemental 1/1
            tokenurl = "https://cards.scryfall.io/large/front/e/5/e5b57672-c346-42f5-ac3e-82466a13b957.jpg";
        else if(id.equals("296739t") || id.equals("296516t") || id.equals("539416t")) //Human Soldier 1/1
            tokenurl = "https://cards.scryfall.io/large/front/d/9/d9cbf36e-4044-4f08-9bae-f0dcb2455716.jpg";
        else if(id.equals("296488t")) //Sliver 1/1
            tokenurl = "https://cards.scryfall.io/large/front/d/e/dec96e95-5580-4110-86ec-561007ab0f1e.jpg";
        else if(id.equals("296500t")) //Bird 3/3
            tokenurl = "https://cards.scryfall.io/large/front/5/9/5988dc9e-724f-4645-8769-b94c5ef631b9.jpg";
        else if(id.equals("296737t")) //Elephant 3/3
            tokenurl = "https://cards.scryfall.io/large/front/2/d/2dbccfc7-427b-41e6-b770-92d73994bf3b.jpg";
        else if(id.equals("296582t") || id.equals("296580t") || id.equals("296822t") || id.equals("539390t") || id.equals("539388t") ||
                id.equals("539387t") || id.equals("539384t") || id.equals("539383t") || id.equals("539382t") || id.equals("539377t") ||
                id.equals("539374t") || id.equals("539373t") || id.equals("539371t") || id.equals("539369t") || id.equals("539367t") ||
                id.equals("539366t") || id.equals("539362t")) //Zombie 2/2
            tokenurl = "https://cards.scryfall.io/large/front/8/a/8a73e348-5bf1-4465-978b-3f31408bade9.jpg";
        else if(id.equals("121236t") || id.equals("296511t") || id.equals("296502t") || id.equals("296471t")) //Bird 1/1
            tokenurl = "https://cards.scryfall.io/large/front/e/8/e8a1b1f2-f067-4c8a-b134-4567e4d5a7c6.jpg";
        else if (id.equals("380486t")) //Bird Enchantment 2/2
            tokenurl = "https://www.mtg.onl/static/4952002452e39de9aa2c98b1f0e3765f/4d406/BNG_4_Bird_U_2_2.jpg";
        else if (id.equals("52181t")) //Centaur Enchantment 3/3
            tokenurl = "https://cards.scryfall.io/large/front/9/8/985be507-6125-4db2-b99f-8b61149ffeeb.jpg";
        else if (id.equals("262699t") || id.equals("262875t") || id.equals("262857t") || id.equals("53054t") || id.equals("539403t") ||
                id.equals("547274t")) //Wolf 2/2
            tokenurl = "https://cards.scryfall.io/large/front/4/6/462ff49b-a004-4dab-a25b-65cb18c1bbec.jpg";
        else if(id.equals("378445t")) //Gold
            tokenurl = "https://cards.scryfall.io/large/front/0/c/0ca10abd-8d9d-4c0b-9d33-c3516abdf6b3.jpg";
        else if (id.equals("380482t")) //Satyr 2/2
            tokenurl = "https://cards.scryfall.io/large/front/b/a/baa93038-2849-4c26-ab4f-1d50d276659f.jpg";
        else if (id.equals("184589t") || id.equals("3832t")) //Spirit */*
            tokenurl = "https://www.mtg.onl/static/5681f8f60f717fb528d0d728cab2bd78/4d406/PROXY_Spirit_B_Y_Y.jpg";
        else if (id.equals("368951t") || id.equals("426025t")) //Elemental */*
            tokenurl = "https://cards.scryfall.io/large/front/8/6/8676704a-419e-4a00-a052-bca2ad34ecae.jpg";
        else if (id.equals("380487t") || id.equals("414506t")) //Zombie */*
            tokenurl = "https://cards.scryfall.io/large/front/9/5/95c59642-575f-4356-ae1a-20b90895545b.jpg";
        else if (id.equals("539365t")) //Zombie */* blue
            tokenurl = "https://cards.scryfall.io/large/front/d/7/d791c7af-1ba7-45ab-ad0c-be9ebc9e51f9.jpg";
        else if (id.equals("114917t") || id.equals("52353t")) //Wurm */*
            tokenurl = "https://cdn.shopify.com/s/files/1/0790/8591/products/Token-front-WURM2_ca71d4fd-916a-4757-a31f-2fd1d631d128_800x800.jpg";
        else if(id.equals("455911t") || id.equals("294389t")) //Horror 1/1
            tokenurl = "https://cards.scryfall.io/large/front/7/9/79a71b52-58f9-4945-9557-0fbcbbf5a241.jpg";
        else if(id.equals("234849t") || id.equals("366401t") || id.equals("366340t")
                || id.equals("366375t") || id.equals("460772t")) // Ooze */*
            tokenurl = "https://cards.scryfall.io/large/front/5/8/580d30c8-df27-422d-b73a-2b27caf598eb.jpg";
        else if(id.equals("52973t")) //Rhino 4/4
            tokenurl = "https://cards.scryfall.io/large/front/1/3/1331008a-ae86-4640-b823-a73be766ac16.jpg";
        else if (id.equals("48096t")) //Demon */*
            tokenurl = "https://cards.scryfall.io/large/front/9/c/9ce65279-fc41-40f8-86a0-fdec72a0d91f.jpg";
        else if(id.equals("383290t")) //Treefolk Warrior */*
            tokenurl = "https://cards.scryfall.io/large/front/2/5/2569593a-d2f2-414c-9e61-2c34e8a5832d.jpg";
        else if(id.equals("51984t")) //Vampire Knight 1/1
            tokenurl = "https://cards.scryfall.io/large/front/8/9/8989fdb4-723b-4c80-89b4-930ccac13b22.jpg";
        else if(id.equals("439331t")) //Wolf 1/1
            tokenurl = "https://cards.scryfall.io/large/front/a/5/a53f8031-aaa8-424c-929a-5478538a8cc6.jpg";
        else if(id.equals("52494t") || id.equals("293206t") || id.equals("294605t")) //Golem 3/3
            tokenurl = "https://cards.scryfall.io/large/front/5/0/509be7b3-490d-4229-ba10-999921a6b977.jpg";
        else if(id.equals("294598t")) //Myr 1/1
            tokenurl = "https://cards.scryfall.io/large/front/d/b/dbad9b20-0b13-41b9-a84a-06b691ee6c71.jpg";
        else if(id.equals("423817t") || id.equals("423700t") || id.equals("183017t") || id.equals("383129t") ||
                id.equals("6164t") || id.equals("456522t") || id.equals("456545t") || id.equals("397624t") ||
                id.equals("52637t") || id.equals("52945t") || id.equals("53460t") || id.equals("53473t") ||
                id.equals("420600t") || id.equals("294436t") || id.equals("489333t") || id.equals("495977t") ||
                id.equals("295775t") || id.equals("295714t") || id.equals("295698t") || id.equals("295635t") ||
                id.equals("296365t") || id.equals("296532t") || id.equals("296482t") || id.equals("296470t") || 
                id.equals("545773t")) // Thopter 1/1
            tokenurl = "https://cards.scryfall.io/large/front/5/a/5a4649cc-07fb-4ff0-9ac6-846763b799df.jpg";
        else if (id.equals("53057t") || id.equals("425825t")) //Wurm T1 3/3
            tokenurl = "https://cards.scryfall.io/large/front/b/6/b68e816f-f9ac-435b-ad0b-ceedbe72447a.jpg";
        else if(id.equals("140233t") || id.equals("191239t") || id.equals("205957t") || id.equals("423797t") ||
                id.equals("51861t")) //Avatar */*
            tokenurl = "https://cards.scryfall.io/large/front/8/6/863768b5-3cf9-415c-b4fd-371dc5afee18.jpg";
        else if (id.equals("53461t")) //Contruct 6/12
            tokenurl = "https://cards.scryfall.io/large/front/8/9/8936efa7-c4d0-426d-977b-38c957a9f025.jpg";
        else if (id.equals("185704t")) //Vampire */*
            tokenurl = "https://cards.scryfall.io/large/front/9/6/969eff58-d91e-49e2-a1e1-8f32b4598810.jpg";
        else if(id.equals("78975t")) //Snake 1/1 green
            tokenurl = "https://cards.scryfall.io/large/front/0/3/032e9f9d-b1e5-4724-9b80-e51500d12d5b.jpg";
        else if(id.equals("296823t")) //Snake 1/1 black
            tokenurl = "https://cards.scryfall.io/large/front/0/3/032e9f9d-b1e5-4724-9b80-e51500d12d5b.jpg";
        else if(id.equals("294401t")) //Dragon 1/1
            tokenurl = "https://cards.scryfall.io/large/front/0/b/0bb628da-a02f-4d3e-b919-0c03821dd5f2.jpg";
        else if (id.equals("175105t") || id.equals("295412t")) //Beast 8/8
            tokenurl = "https://cards.scryfall.io/large/front/a/7/a7382e4b-43dc-4b35-8a9e-ab886ea0a981.jpg";
        else if (id.equals("376496t") || id.equals("376549t") || id.equals("294519t")) //Thopter blue 1/1
            tokenurl = "https://cards.scryfall.io/large/front/a/3/a3506ee6-a168-49a4-9814-2858194be60e.jpg";
        else if (id.equals("247202t")) //Elemental 5/5
            tokenurl = "https://cards.scryfall.io/large/front/6/6/66029f69-2dc3-44e3-aa0d-4fe9a33b06f5.jpg";
        else if (id.equals("376546t")) //Elemental 1/1 haste
            tokenurl = "https://cards.scryfall.io/large/front/9/4/94c14f3d-1578-426b-b64b-07c7e88ab572.jpg";
        else if (id.equals("244668t")) //Faerie Rogue 1/1
            tokenurl = "https://cards.scryfall.io/large/front/a/0/a07b4786-1592-42c7-9d3e-d0d66abaed99.jpg";
        else if(id.equals("294507t")) // Giant Warrior 4/4
            tokenurl = "https://cards.scryfall.io/large/front/a/0/a06eea30-810b-4623-9862-ec71c4bed11a.jpg";
        else if(id.equals("294514t")) //Elf Warrior 1/1
            tokenurl = "https://cards.scryfall.io/large/front/c/b/cb8caa61-e294-4501-b357-a44abd77d09a.jpg";
        else if (id.equals("457111t") || id.equals("51931t")) //Rogue 1/1
            tokenurl = "https://cards.scryfall.io/large/front/6/7/67457137-64f2-413d-b62e-658b3f1b1043.jpg";
        else if (id.equals("376578t") || id.equals("152553t")) //Elemental 4/4
            tokenurl = "https://cards.scryfall.io/large/front/f/e/fea0857b-0f9e-4a87-83d7-85723e33f26c.jpg";
        else if (id.equals("153166t")) //Merfolk Wizard 1/1
            tokenurl = "https://cards.scryfall.io/large/front/5/2/526da544-23dd-42b8-8c00-c3609eea4489.jpg";
        else if(id.equals("83236t") || id.equals("45390t") || id.equals("965t") || id.equals("966t") ||
                id.equals("52750t")) //Rukh 4/4
            tokenurl = "https://cards.scryfall.io/large/front/b/5/b5489e26-6aec-4706-9c3e-8454878fa6c3.jpg";
        else if(id.equals("294426t")) //Spirit Warrior */*
            tokenurl = "https://cards.scryfall.io/large/front/f/e/febc7ce0-387f-413c-a387-2952b990ff3f.jpg";
        else if (id.equals("19878t")) //Monkey 2/2
            tokenurl = "https://www.mtg.onl/static/9ce248147e36a52ccc388b3e642839aa/4d406/PROXY_Ape_G_2_2.jpg";
        else if (id.equals("126166t")) //Elf Druid 1/1
            tokenurl = "https://cards.scryfall.io/large/front/4/5/458f44dd-83f1-497e-b5d0-e3417eb9dfec.jpg";
        else if (id.equals("202474t") || id.equals("1098t") || id.equals("2024t") || id.equals("3766t") || id.equals("11183t") || id.equals("902t")) //Djinn 5/5
            tokenurl = "https://media.mtgsalvation.com/attachments/71/116/635032489341076803.jpg";
        else if (id.equals("202590t") || id.equals("2073t") || id.equals("1027t")) // Tetravite
            tokenurl = "https://www.mtg.onl/static/a1f89472f590ea4e9652fe9dfebc1364/4d406/PROXY_Tetravite_1_1.jpg";
        else if (id.equals("3809t") || id.equals("2792t") || id.equals("1422t") || id.equals("159826t")) //Snake Artifact 1/1
            tokenurl = "https://www.mtg.onl/static/b19119feebdd5bed147282d3c643fca9/4d406/PROXY_Snake_1_1.jpg";
        else if (id.equals("407540t") || id.equals("407672t") || id.equals("407525t") || id.equals("293194t")) //Kor Ally 1/1
            tokenurl = "https://cards.scryfall.io/large/front/b/e/be224180-a482-4b94-8a9d-3a92ee0eb34b.jpg";
        else if (id.equals("460768t")) //Lizard 3/3
            tokenurl = "https://cards.scryfall.io/large/front/e/5/e54486a4-f432-4e50-8639-799e036d0657.jpg";
        else if (id.equals("201124t") || id.equals("3118t")) //Starfish 0/1
            tokenurl = "https://www.mtg.onl/static/536f2ee747044be2a06a820132a4b596/4d406/PROXY_Starfish_U_0_1.jpg";
        else if (id.equals("184730t") || id.equals("3192t") || id.equals("3193t")) //Knight Banding 1/1
            tokenurl = "https://www.mtg.onl/static/c88f42f8bd5a7c25aa36902546b690f5/4d406/PROXY_Knight_W_1_1.jpg";
        else if (id.equals("25910t")) //Angel 3/3 black
            tokenurl = "https://www.mtg.onl/static/9b6aafa10fefb5d5e55c6e4d2c1e512c/4d406/PROXY_Angel_B_3_3.jpg";
        else if (id.equals("6142t")) //Beast 2/2
            tokenurl = "https://www.mtg.onl/static/8eed0c2bcb05f3e26cdcc2f3f41d7f42/4d406/PROXY_Beast_G_2_2.jpg";
        else if (id.equals("34929t")) //Cat 1/1
            tokenurl = "https://www.mtg.onl/static/f23f6e35a23174a7fa9106d67d32fef9/4d406/PROXY_Cat_R_1_1.jpg";
        else if (id.equals("1649t") || id.equals("201182t")) //Minor Demon 1/1
            tokenurl = "https://www.mtg.onl/static/ebecf2ca03dfc9e71cc28e6df6b864bb/4d406/PROXY_Minor_Demon_BR_1_1.jpg";
        else if (id.equals("4854t") || id.equals("376556t")) //Carnivore 3/1
            tokenurl = "https://cards.scryfall.io/large/front/8/4/8437b125-6057-4d17-9f2c-28ea56553f84.jpg";
        else if (id.equals("4771t")) //Dog 1/1
            tokenurl = "https://www.cardkingdom.com/images/magic-the-gathering/core-set-2021/dog-token-55913-medium.jpg";
        else if (id.equals("9667t")) //Giant Bird 4/4
            tokenurl = "https://www.mtg.onl/static/abe5178af8ebbe84f5504493a1b5f154/4d406/PROXY_Giant_Chicken_R_4_4.jpg";
        else if (id.equals("74265t")) //Expansion Symbol 1/1
            tokenurl = "https://www.mtg.onl/static/9de9c9d3d17a3a8eb20c9c66b5b9253a/4d406/PROXY_ExpansionSymbol_1_1.jpg";
        else if (id.equals("73953t")) //Giant Teddy 5/5
            tokenurl = "https://cards.scryfall.io/large/front/6/2/628c542b-7579-4070-9143-6f1f7221468f.jpg";
        else if (id.equals("25956t")) //Kavu 3/4
            tokenurl = "https://www.mtg.onl/static/740ce087c4aff57e881b01c28528c8f9/4d406/PROXY_Kavu_B_3_3.jpg";
        else if (id.equals("184598t") || id.equals("2959t")) //Kelp 0/1
            tokenurl = "http://magicplugin.normalitycomics.com/cardimages/lackey/kelp-u-0-1-defender-v4.jpg";
        else if (id.equals("111046t")) //Insect 6/1
            tokenurl = "https://cards.scryfall.io/large/front/0/f/0ff2e2bd-b8e9-4563-85ad-fdbb0607fb7c.jpg";
        else if (id.equals("27634t") || id.equals("3227t") || id.equals("159097t") || id.equals("294453t")) //Hippo 1/1
            tokenurl = "https://www.mtg.onl/static/8b684bdea239d594e296a134f5ec1783/4d406/PROXY_Hippo_G_1_1.jpg";
        else if (id.equals("3148t")) //Splinter 1/1
            tokenurl = "https://www.mtg.onl/static/73cad75db99d3ba716082464bfd85b2e/4d406/PROXY_Splinter_G_1_1.jpg";
        else if(id.equals("26815t") || id.equals("51774t")) //Cat 2/1
            tokenurl = "https://www.mtg.onl/static/8bb68cf125fdcc9d8a21b3dade2f11cb/4d406/PROXY_Cat_B_2_1.jpg";
        else if (id.equals("1534t")) //Wolves of the Hunt 1/1
            tokenurl = "https://www.mtg.onl/static/e34edc351ea7ef08c4c4064d1f890731/4d406/PROXY_Wolves_of_the_Hunt_G_1_1.jpg";
        else if (id.equals("130314t")) //Zombie Goblin 1/1
            tokenurl = "https://www.mtg.onl/static/334463a009d3b5b3068eaf61621870ef/4d406/PROXY_Festering_Goblin_B_1_1.jpg";
        else if (id.equals("116383t")) //Bat 1/2
            tokenurl = "https://cards.scryfall.io/large/front/4/c/4c532e0f-8934-4ad3-bb1a-640abe946e10.jpg";
        else if (id.equals("124344t")) //Cat Warrior 2/2
            tokenurl = "https://cards.scryfall.io/large/front/2/9/29c4e4f2-0040-4490-b357-660d729ad9cc.jpg";
        else if (id.equals("376404t")) //Elemental */*
            tokenurl = "https://cards.scryfall.io/large/front/d/b/db67bc06-b6c9-49a0-beef-4d35842497cb.jpg";
        else if (id.equals("409810t") || id.equals("409805t") || id.equals("409953t") || id.equals("409997t") ||
                id.equals("410032t")  || id.equals("293377t") || id.equals("294345t") || id.equals("295471t") || id.equals("612562t")) //Clue
            tokenurl = "https://cards.scryfall.io/large/front/f/2/f2c859e1-181e-44d1-afbd-bbd6e52cf42a.jpg";
        else if (id.equals("3242t")) //Wall 0/2
            tokenurl = "https://www.mtg.onl/static/18f8f17bbe1f81822efa4bed878b6437/4d406/PROXY_Wall_0_2.jpg";
        else if (id.equals("21382t")) //Elephant */*
            tokenurl = "https://www.mtg.onl/static/b740cce52030bca3b02d2a917152314f/4d406/PROXY_Elephant_G_Y_Y.jpg";
        else if (id.equals("293348t") || id.equals("293058t")) //Eldrazi Horror 3/2
            tokenurl = "https://cards.scryfall.io/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg";
        else if (id.equals("416746t")) //Marit Lage 20/20
            tokenurl = "https://cards.scryfall.io/large/front/f/b/fb248ba0-2ee7-4994-be57-2bcc8df29680.jpg";
        else if (id.equals("46168t")) // Construct */*
            tokenurl = "https://cards.scryfall.io/large/front/c/5/c5eafa38-5333-4ef2-9661-08074c580a32.jpg";
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
                id.equals("295513t") || id.equals("295506t") || id.equals("295502t") || id.equals("293737t")) //Servo 1/1
            tokenurl = "https://cards.scryfall.io/large/front/d/7/d79e2bf1-d26d-4be3-a5ad-a43346ed445a.jpg";
        else if (id.equals("265141t")) //Boar 3/3
            tokenurl = "https://cards.scryfall.io/large/front/8/3/83dcacd3-8707-4354-a1a5-9863d677d67f.jpg";
        else if(id.equals("383077t")) //Saproling */*
            tokenurl = "https://www.mtg.onl/static/018b0db17f54cdd63bd182174fe3ef5b/4d406/PROXY_Saproling_G_Y_Y.jpg";
        else if(id.equals("53274t")) //Bird 1/1
            tokenurl = "https://cards.scryfall.io/large/front/c/f/cf64f834-a645-4db4-a34f-9cab725fc1b1.jpg";
        else if(id.equals("53244t")) //Soldier 1/1
            tokenurl = "https://cards.scryfall.io/large/front/0/a/0a47f751-52f1-4042-85dd-ea222e5d969d.jpg";
        else if(id.equals("53240t") || id.equals("296593t") || id.equals("296479t")) //Spirit 1/1
            tokenurl = "https://cards.scryfall.io/large/front/4/9/4914610d-7d4f-4cf6-98db-c39e79cce25c.jpg";
        else if(id.equals("53299t")) //Thopter Blue 1/1
            tokenurl = "https://cards.scryfall.io/large/front/e/e/eef8b4fc-238f-4c1f-ad98-a1769fd44eab.jpg";
        else if(id.equals("53246t")) //Germ 0/0
            tokenurl = "https://cards.scryfall.io/large/front/6/1/61f94e32-3b22-4c47-b866-1f36a7f3c734.jpg";
        else if(id.equals("53259t")) //Goblin 1/1
            tokenurl = "https://cards.scryfall.io/large/front/9/9/99a6ebce-f391-4642-857a-4dc1466895f3.jpg";
        else if(id.equals("53264t")) //Lizard 8/8
            tokenurl = "https://cards.scryfall.io/large/front/7/0/70345006-5cde-44f8-ab66-9d8163d4c4f6.jpg";
        else if(id.equals("53289t") || id.equals("611982t") || id.equals("611958t")) //Saproling 1/1
            tokenurl = "https://cards.scryfall.io/large/front/0/3/0302fa7d-2e34-4f4a-a84e-7a78febc77f5.jpg";
        else if(id.equals("53300t")) //Construct 1/1
            tokenurl = "https://cards.scryfall.io/large/front/7/c/7c82af53-2de8-4cd6-84bf-fb39d2693de2.jpg";
        else if (id.equals("401697t") || id.equals("401692t") || id.equals("401701t") || id.equals("293619t") || id.equals("294261t") ||
                id.equals("293585t") || id.equals("539400t")) // Eldrazi Spawn 0/1
            tokenurl = "https://cards.scryfall.io/large/front/7/7/7787eae2-7dfb-44ab-8e92-56fdfc0bb39e.jpg";
        else if (id.equals("376397t") || id.equals("107557t")) //Drake Green Blue 2/2
            tokenurl = "https://cards.scryfall.io/large/front/c/0/c06d2c07-7d3e-46e3-86f0-7ceba3b0aee0.jpg";
        else if(id.equals("52398t")) //Illusion 2/2
            tokenurl = "https://cards.scryfall.io/large/front/a/1/a10729a5-061a-4daf-91d6-0f6ce813a992.jpg";
        else if (id.equals("435411t") || id.equals("435410t") || id.equals("611089t") || id.equals("612146t") || id.equals("614935t") ||
                id.equals("614772t") || id.equals("569995t") || id.equals("563217t")) //Treasure
            tokenurl = "https://cards.scryfall.io/large/front/7/2/720f3e68-84c0-462e-a0d1-90236ccc494a.jpg";
        else if (id.equals("611086t") || id.equals("610929t") || id.equals("607138t")|| id.equals("607125t")) //Incubator
            tokenurl = "https://cards.scryfall.io/large/front/2/c/2c5ed737-657b-43bf-b222-941da7579a4a.jpg";
        else if (id.equals("1686t") || id.equals("2881t") || id.equals("201231t")) //Stangg Twin 3/4
            tokenurl = "https://cards.scryfall.io/large/front/e/b/eba90d37-d7ac-4097-a04d-1f27e4c9e5de.jpg";
        else if (id.equals("439843t")) //Golem 4/4
            tokenurl = "https://cards.scryfall.io/large/front/a/7/a7820eb9-6d7f-4bc4-b421-4e4420642fb7.jpg";
        else if(id.equals("447070t") || id.equals("53480t")) //Mowu 3/3
            tokenurl = "https://cards.scryfall.io/large/front/b/1/b10441dd-9029-4f95-9566-d3771ebd36bd.jpg";
        else if(id.equals("53190t")) //Elemental 5/1
            tokenurl = "https://cards.scryfall.io/large/front/0/5/05aa19b7-da04-4845-868e-3ad2edb9a9ba.jpg";
        else if (id.equals("452760t") || id.equals("296508t")) //Angel 4/4 vigilance
            tokenurl = "https://cards.scryfall.io/large/front/a/c/acb271a8-68bb-45e6-9f99-568479e92ea0.jpg";
        else if(id.equals("53453t")) //Mask Enchantment
            tokenurl = "https://cards.scryfall.io/large/front/b/2/b21b5504-c5ef-4dfc-8219-8db90aca7694.jpg";
        else if(id.equals("53438t")) //Myr 2/1
            tokenurl = "https://cards.scryfall.io/large/front/4/8/483c8cd6-288c-49d7-ac28-642132f85259.jpg";
        else if(id.equals("53463t")) //Survivor 1/1
            tokenurl = "https://cards.scryfall.io/large/front/f/4/f4478979-19b6-4524-bbbd-519594c38f5a.jpg";
        else if(id.equals("52149t")) //Soldier 1/1
            tokenurl = "https://cards.scryfall.io/large/front/4/5/45907b16-af17-4237-ab38-9d7537fd30e8.jpg";
        else if (id.equals("89110t") || id.equals("456379t")) //Voja 2/2
            tokenurl = "https://cards.scryfall.io/large/front/2/8/2879010f-b752-4808-8531-d24e612de0d9.jpg";
        else if (id.equals("116384t") || id.equals("376564t") || id.equals("52993t")) //Assembly-Worker 2/2
            tokenurl = "https://cards.scryfall.io/large/front/e/7/e72daa68-0680-431c-a616-b3693fd58813.jpg";
        else if(id.equals("17841t") || id.equals("17850t") || id.equals("17852t") || id.equals("19444t") || id.equals("294101t") ||
                id.equals("294226t")) //Elf Warrior 1/1
            tokenurl = "https://cards.scryfall.io/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg";
        else if(id.equals("383392t") || id.equals("539394t")) //Beast 3/3
            tokenurl = "https://cards.scryfall.io/large/front/3/f/3fc3a29a-280d-4f2c-9a01-8cfead75f583.jpg";
        else if (id.equals("5610t") || id.equals("416754t")) //Minion */*
            tokenurl = "https://cards.scryfall.io/large/front/a/9/a9930d11-4772-4fc2-abbd-9af0a9b23a3e.jpg";
        else if (id.equals("5173t")) //Insect Artifact 1/1
            tokenurl = "https://cards.scryfall.io/large/front/5/4/54ec2cd6-51f6-4e12-af90-fa254f14ad32.jpg";
        else if(id.equals("378521t") || id.equals("52418t")) //Kraken 9/9
            tokenurl= "https://cards.scryfall.io/large/front/d/0/d0cd85cc-ad22-446b-8378-5eb69fee1959.jpg";
        else if(id.equals("52136t")) //Soldier 1/1
            tokenurl= "https://cards.scryfall.io/large/front/4/5/45907b16-af17-4237-ab38-9d7537fd30e8.jpg";
        else if (id.equals("271158t") || id.equals("401703t")) //Hellion 4/4
            tokenurl = "https://cards.scryfall.io/large/front/d/a/da59fb40-b218-452f-b161-3bde15e30c74.jpg";
        else if (id.equals("88973t") || id.equals("368549t")) //Spirit 1/1
            tokenurl = "https://cards.scryfall.io/large/front/b/3/b3c9a097-219b-4aaf-831f-cc0cddbcfaae.jpg";
        else if (id.equals("53454t")) // Zombie 2/2
            tokenurl = "https://cards.scryfall.io/large/front/b/5/b5bd6905-79be-4d2c-a343-f6e6a181b3e6.jpg";
        else if (id.equals("417465t") || id.equals("294137t") || id.equals("296576t")) //Eldrazi Scion 1/1
            tokenurl = "https://cards.scryfall.io/large/front/a/7/a7ba0398-35e1-4733-ad29-e853757d6f24.jpg";
        else if (id.equals("417480t")) //Demon 5/5
            tokenurl = "https://cards.scryfall.io/large/front/5/4/545639fc-e521-41f2-81b2-a671007321eb.jpg";
        else if (id.equals("417481t") || id.equals("293725t")) //Zombie Giant 5/5
            tokenurl = "https://cards.scryfall.io/large/front/b/e/be7e26e1-5db6-49ba-a88e-c79d889cd364.jpg";
        else if (id.equals("417447t")) //Elemental 4/4
            tokenurl = "https://cards.scryfall.io/large/front/f/e/fea0857b-0f9e-4a87-83d7-85723e33f26c.jpg";
        else if(id.equals("220535t") || id.equals("376253t") || id.equals("376390t") || id.equals("53439t") ||
                id.equals("401643t") || id.equals("417451t") || id.equals("417424t") || id.equals("51908t") ||
                id.equals("52593t") || id.equals("53161t") || id.equals("271227t")) // Plant 0/1
            tokenurl = "https://cards.scryfall.io/large/front/f/a/fa0025fa-c530-4151-bcff-48425a4f1db5.jpg";
        else if(id.equals("3392t")) // Wood 0/1
            tokenurl = "https://www.mtg.onl/static/70c0c3608291aaee9517eff9cacd43d6/4d406/PROXY_Wood_G_0_1.jpg";
        else if (id.equals("21381t") || id.equals("40198t"))
            tokenurl = "https://cards.scryfall.io/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg";
        else if (id.equals("461099t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/e/de7ba875-f77b-404f-8b75-4ba6f81da410.jpg";
        else if (id.equals("426909t") || id.equals("426705t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/8/98956e73-04e4-4d7f-bda5-cfa78eb71350.jpg";
        else if (id.equals("426897t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/8/a8f339c6-2c0d-4631-849b-44d4360b5131.jpg";
        else if (id.equals("457139t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/0/105e687e-7196-4010-a6b7-cfa42d998fa4.jpg";
        else if (id.equals("470549t"))
            tokenurl = "https://cards.scryfall.io/large/front/7/7/7711a586-37f9-4560-b25d-4fb339d9cd55.jpg";
        else if (id.equals("113527t") || id.equals("376321t"))
            tokenurl = "https://cards.scryfall.io/large/front/5/b/5b9f471a-1822-4981-95a9-8923d83ddcbf.jpg";
        else if (id.equals("114919t") || id.equals("247519t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/5/b5ddb67c-82fb-42d6-a4c2-11cd38eb128d.jpg";
        else if (id.equals("8862t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/b/dbf33cc3-254f-4c5c-be22-3a2d96f29b80.jpg";
        else if(id.equals("213757t") || id.equals("213734t") || id.equals("221554t") || id.equals("48049t") ||
                id.equals("46160t") || id.equals("47450t") || id.equals("376421t") || id.equals("213725t") ||
                id.equals("52492t") || id.equals("489268t") || id.equals("489458t"))
            tokenurl = "https://cards.scryfall.io/large/front/f/3/f32ad93f-3fd5-465c-ac6a-6f8fb57c19bd.jpg";
        else if (id.equals("247393t") || id.equals("247399t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg";
        else if (id.equals("152998t") || id.equals("152963t") || id.equals("52364t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/5/959ed4bf-b276-45ed-b44d-c757e9c25846.jpg";
        else if (id.equals("46703t") || id.equals("227151t") || id.equals("205298t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/a/0a9a25fd-1a4c-4d63-bbfa-296ef53feb8b.jpg";
        else if (id.equals("394380t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/2/622397a1-6513-44b9-928a-388be06d4022.jpg";
        else if (id.equals("1138t") || id.equals("2074t") || id.equals("640t") || id.equals("3814t") || id.equals("11530t") ||
                id.equals("43t") || id.equals("338t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/7/c75b81b5-5c84-45d4-832a-20c038372bc6.jpg";
        else if (id.equals("275261t") || id.equals("271156t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg";
        else if (id.equals("376455t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/e/9e0eeebf-7c4a-436b-8cb4-292e53783ff2.jpg";
        else if(id.equals("414388t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/8/b8710a30-8314-49ef-b995-bd05454095be.jpg";
        else if(id.equals("382874t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/3/83dcacd3-8707-4354-a1a5-9863d677d67f.jpg";
        else if(id.equals("383065t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/5/8597029c-3b0d-476e-a6ee-48402f815dab.jpg";
        else if(id.equals("414350t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/4/e4439a8b-ef98-428d-a274-53c660b23afe.jpg";
        else if(id.equals("414349t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/4/e4439a8b-ef98-428d-a274-53c660b23afe.jpg";
        else if(id.equals("414429t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/b/dbd994fc-f3f0-4c81-86bd-14ca63ec229b.jpg";
        else if(id.equals("414314t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg";
        else if(id.equals("414313t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/1/11d25bde-a303-4b06-a3e1-4ad642deae58.jpg";
        else if(id.equals("227061t"))
            tokenurl = "https://cards.scryfall.io/large/front/5/f/5f68c2ab-5131-4620-920f-7ba99522ccf0.jpg";
        else if(id.equals("227072t"))
            tokenurl = "https://cards.scryfall.io/large/front/5/f/5f68c2ab-5131-4620-920f-7ba99522ccf0.jpg";
        else if(id.equals("245250t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/5/a53f8031-aaa8-424c-929a-5478538a8cc6.jpg";
        else if(id.equals("245251t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/5/a53f8031-aaa8-424c-929a-5478538a8cc6.jpg";
        else if(id.equals("398441t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/5/e5ccae95-95c2-4d11-aa68-5c80ecf90fd2.jpg";
        else if (id.equals("409826t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/0/e0a12a72-5cd9-4f1b-997d-7dabb65e9f51.jpg";
        else if (id.equals("51939t") || id.equals("52121t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/9/b999a0fe-d2d0-4367-9abb-6ce5f3764f19.jpg";
        else if (id.equals("52110t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/b/0bb628da-a02f-4d3e-b919-0c03821dd5f2.jpg";
        else if (id.equals("473141t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/f/bf36408d-ed85-497f-8e68-d3a922c388a0.jpg";
        else if(id.equals("53180t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/f/1feaa879-ceb3-4b20-8021-ae41d8be9005.jpg";
        else if(id.equals("53118t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/3/03553980-53fa-4256-b478-c7e0e73e2b5b.jpg";
        else if(id.equals("53268t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/c/6c1ffb14-9d92-4239-8694-61d156c9dba7.jpg";
        else if(id.equals("53403t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/e/ae196fbc-c9ee-4dba-9eb3-52209908b898.jpg";
        else if(id.equals("53408t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/e/0e80f154-9409-40fa-a564-6fc296498d80.jpg";
        else if(id.equals("53417t"))
            tokenurl = "https://cards.scryfall.io/large/front/2/9/29c4e4f2-0040-4490-b357-660d729ad9cc.jpg";
        else if(id.equals("53326t"))
            tokenurl = "https://cards.scryfall.io/large/front/7/4/748d267d-9c81-4dc0-92b7-eafb7691c6cc.jpg";
        else if(id.equals("16787t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/8/e8a56b33-f720-4cbf-8015-59b5fd8ff756.jpg";
        else if(id.equals("16759t"))
            tokenurl = "https://cards.scryfall.io/large/front/f/3/f3b5665e-2b97-47c7-bbf9-6549c2c8a9f2.jpg";
        else if(id.equals("456382t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/6/b64c5f80-4676-4860-be0e-20bcf2227405.jpg";
        else if(id.equals("460464t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/4/94ed2eca-1579-411d-af6f-c7359c65de30.jpg";
        else if(id.equals("19461t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/2/d2f51f4d-eb6d-4503-b9a4-559db1b9b16f.jpg";
        else if(id.equals("19471t") || id.equals("19472t"))
            tokenurl = "https://cards.scryfall.io/large/front/3/4/340fb06f-4bb0-4d23-b08c-8b1da4a8c2ad.jpg";
        else if(id.equals("294089t") || id.equals("294717t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/b/8b4f81e2-916f-4af4-9e63-f4469e953122.jpg";
        else if(id.equals("293323t"))
            tokenurl = "https://cards.scryfall.io/large/front/2/f/2f4b7c63-8430-4ca4-baee-dc958d5bd22f.jpg";
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
            tokenurl = "https://cards.scryfall.io/large/front/a/0/a0b5e1f4-9206-40b6-9cf6-331f6a95d045.jpg";
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
            tokenurl = "https://cdn.shopify.com/s/files/1/1601/3103/products/Token_45_2000x.jpg";
        else if (id.equals("405191t"))
            tokenurl = "https://6d4be195623157e28848-7697ece4918e0a73861de0eb37d08968.ssl.cf1.rackcdn.com/108181_200w.jpg";
        else if (id.equals("476402t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/0/60466c78-155e-442b-8022-795e1e9de8df.jpg";
        else if(id.equals("484904t"))
            tokenurl = "https://cards.scryfall.io/large/front/2/1/21e89101-f1cf-4bbd-a1d5-c5d48512e0dd.jpg";
        else if(id.equals("489168t"))
            tokenurl="https://cards.scryfall.io/large/front/d/e/dee1c2ee-d92e-409a-995a-b4c91620c918.jpg";
        else if(id.equals("489401t"))
            tokenurl="https://i.pinimg.com/564x/01/b0/a2/01b0a289e1a28167cbf0f30532328d99.jpg";
        else if(id.equals("489171t"))
            tokenurl = "https://cards.scryfall.io/large/front/4/f/4f8107b3-8539-4b9c-8d0d-c512c940838f.jpg";
        else if(id.equals("489403t") || id.equals("489358t") || id.equals("489372t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/5/959ed4bf-b276-45ed-b44d-c757e9c25846.jpg";
        else if(id.equals("489562t") || id.equals("296282"))
            tokenurl="https://cards.scryfall.io/large/front/c/f/cf9a289f-cd3f-42a0-9296-8c7cc7d01a91.jpg";
        else if(id.equals("489363t"))
            tokenurl="https://cards.scryfall.io/large/front/8/3/83dcacd3-8707-4354-a1a5-9863d677d67f.jpg";
        else if(id.equals("489900t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/6/8676704a-419e-4a00-a052-bca2ad34ecae.jpg";
        else if(id.equals("489695t"))
            tokenurl = "https://cards.scryfall.io/large/front/7/b/7becaa04-f142-4163-9286-00018b95c4ca.jpg";
        else if(id.equals("489907t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/e/9ecc467e-b345-446c-b9b7-5f164e6651a4.jpg";
        else if(id.equals("295082t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/e/ae56d9e8-de05-456b-af32-b5992992ee15.jpg";
        else if(id.equals("496035t") || id.equals("295423t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/d/ed666385-a2e7-4e1f-ad2c-babbfc0c50b3.jpg";
        else if(id.equals("496036t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/1/e1eb3b8a-f9d3-4ce1-b171-ba7b0c3f4830.jpg";
        else if(id.equals("495984t"))
            tokenurl = "https://cards.scryfall.io/large/front/3/3/33bd708d-dc84-44d3-a563-536ade028bd0.jpg";
        else if(id.equals("495971t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/7/67457137-64f2-413d-b62e-658b3f1b1043.jpg";
        else if(id.equals("495958t"))
            tokenurl = "https://cards.scryfall.io/large/front/b/e/be224180-a482-4b94-8a9d-3a92ee0eb34b.jpg";
        else if(id.equals("295356t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/7/c7e7822b-f155-4f3f-b835-ec64f3a71307.jpg";
        else if(id.equals("295376t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/b/cb8caa61-e294-4501-b357-a44abd77d09a.jpg";
        else if(id.equals("295334t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/c/dcee70ef-6285-4f09-8a71-8b7960e8fa99.jpg";
        else if(id.equals("295433t"))
            tokenurl = "https://cards.scryfall.io/large/front/2/f/2f4b7c63-8430-4ca4-baee-dc958d5bd22f.jpg";
        else if(id.equals("295428t"))
            tokenurl = "https://cards.scryfall.io/large/front/5/3/5371de1b-db33-4db4-a518-e35c71aa72b7.jpg";
        else if(id.equals("295377t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/e/ce90c48f-74fb-4e87-9e46-7f8c3d79cbb0.jpg";
        else if(id.equals("295322t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/0/903e30f3-580e-4a14-989b-ae0632363407.jpg";
        else if(id.equals("295234t"))
            tokenurl ="https://cards.scryfall.io/large/front/d/c/dc77b308-9d0c-492f-b3fe-e00d60470767.jpg";
        else if(id.equals("295225t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/9/d9c95045-e806-4933-94a4-cb52ae1a215b.jpg";
        else if(id.equals("295217t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/4/0419a202-6e32-4f0a-a032-72f6c00cae5e.jpg";
        else if(id.equals("295556t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/2/623a08d1-f5ff-48b7-bdb6-54b8d7a4b931.jpg";
        else if(id.equals("503330t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/5/c5ad13b4-bbf5-4c98-868f-4d105eaf8833.jpg";
        else if(id.equals("503754t") || id.equals("503827t"))
            tokenurl = "https://cards.scryfall.io/large/front/4/a/4ae9f454-4f8c-4123-9886-674bc439dfe7.jpg";
        else if(id.equals("503846t"))
            tokenurl = "https://cards.scryfall.io/large/front/3/d/3db39e3b-fad4-4c9b-911f-69883ac7e0e1.jpg";
        else if(id.equals("503821t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/f/ef775ad0-b1a9-4254-ab6f-304558bb77a1.jpg";
        else if(id.equals("508147t") || id.equals("508338t") || id.equals("508160t") || id.equals("508357t") ||
                id.equals("508354t") || id.equals("508349t") || id.equals("508343t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg";
        else if(id.equals("295919t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/1/118d0655-5719-4512-8bc1-fe759669811b.jpg";
        else if(id.equals("518457t") || id.equals("518473t") || id.equals("518468t") || id.equals("518463t") || id.equals("518460t") ||
                id.equals("518422t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/1/910f48ab-b04e-4874-b31d-a86a7bc5af14.jpg";
        else if(id.equals("518467t") || id.equals("518410t") || id.equals("518436t") || id.equals("518308t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/9/c9deae5c-80d4-4701-b425-91853b7ee03b.jpg";
        else if(id.equals("518461t") || id.equals("518432t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/0/d0ddbe3e-4a66-494d-9304-7471232549bf.jpg";
        else if(id.equals("518310t"))
            tokenurl = "https://cards.scryfall.io/large/front/3/d/3d0b9b88-705e-4df0-8a93-3e240b81355b.jpg";
        else if(id.equals("513663t"))
            tokenurl = "https://cards.scryfall.io/large/front/1/a/1a2d027f-8996-4761-a776-47cd428f6779.jpg";
        else if(id.equals("522245t"))
            tokenurl = "https://cards.scryfall.io/large/front/3/7/37e32ba6-108a-421f-9dad-3d03f7ebe239.jpg";
        else if(id.equals("296413t"))
            tokenurl = "https://i.pinimg.com/564x/af/cc/4c/afcc4c87d67c9651838fed09217c7eed.jpg";
        else if(id.equals("296410t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/4/94057dc6-e589-4a29-9bda-90f5bece96c4.jpg";
        else if(id.equals("527539t") || id.equals("527477t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/3/a3a684b7-27e0-4d9e-a064-9e03c6e50c89.jpg";
        else if(id.equals("527351t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/e/ce3c0bd9-8a37-4164-9937-f35d1c210fe8.jpg";
        else if(id.equals("527378t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/6/86881c5f-df5e-4f50-b554-e4c49d5316f9.jpg";
        else if(id.equals("532511t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/3/a378702b-d074-4402-b423-2ca8f44fce7c.jpg";
        else if(id.equals("532519t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/2/62cafc0a-cd02-4265-aa1f-b8a6cb7cc8db.jpg";
        else if(id.equals("532527t"))
            tokenurl = "https://cards.scryfall.io/large/front/8/e/8e3b2942-d1a4-4d27-9d64-65712497ab2e.jpg";
        else if(id.equals("532560t") || id.equals("532659t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/5/95483574-95b7-42a3-b700-616189163b0a.jpg";
        else if(id.equals("531918t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/7/076f934b-a244-45f1-bcb3-7c5e882e9911.jpg";
        else if(id.equals("532539t") || id.equals("531948t"))
            tokenurl = "https://cards.scryfall.io/large/front/4/4/44a4ef4a-a026-424e-88ff-e2bb77aaf05d.jpg";
        else if(id.equals("532482t") || id.equals("532493t") || id.equals("532491t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/4/e43a205e-43ea-4b3e-92ab-c2ee2172a50a.jpg";
        else if(id.equals("532599t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/c/9c8fe0d7-5c40-45fe-b3d8-47852380845e.jpg";
        else if(id.equals("531833t"))
            tokenurl = "https://i.pinimg.com/564x/04/dc/04/04dc041251acb96f97327d67e9c8fe23.jpg";
        else if(id.equals("532489t"))
            tokenurl = "https://cards.scryfall.io/large/front/6/0/60842b1a-6ae7-4b3b-a23f-0d94a3d89884.jpg";
        else if(id.equals("531921t"))
            tokenurl = "https://cards.scryfall.io/large/front/2/3/2300635e-7771-4676-a5a5-29a9d8f49f1a.jpg";
        else if(id.equals("531928t") || id.equals("531933t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/b/0b08d210-01cb-46c5-9150-4dfb47f50ae7.jpg";
        else if(id.equals("297027t"))
            tokenurl = "https://cards.scryfall.io/large/front/c/4/c4052aed-981b-41d0-85f0-20c2599811ba.jpg";
        else if(id.equals("297047t"))
            tokenurl = "https://cards.scryfall.io/large/front/4/1/41bee18a-46d6-4f60-9e19-5fc670a20a4d.jpg";
        else if(id.equals("297175t"))
            tokenurl = "https://cards.scryfall.io/large/front/5/2/5212bae5-d768-45ab-aba8-94c4f9fabc79.jpg";
        else if(id.equals("297052t"))
            tokenurl = "https://cards.scryfall.io/large/front/f/a/faa10292-f358-48c1-a516-9a1eecf62b1d.jpg";
        else if(id.equals("297111t"))
            tokenurl = "https://cards.scryfall.io/large/front/0/f/0f8fe08d-b469-4471-8a7d-cf75850ba312.jpg";
        else if(id.equals("296918t"))
            tokenurl = "https://cards.scryfall.io/large/front/a/0/a0f7b2f0-16d3-4db0-a737-c7b8dcb9d5de.jpg";
        else if(id.equals("297076t"))
            tokenurl = "https://cards.scryfall.io/large/front/f/5/f55c70b8-0fa5-4d08-9061-f53d6f949908.jpg";
        else if(id.equals("297074t"))
            tokenurl = "https://cards.scryfall.io/large/front/d/5/d53e20ee-b43c-45aa-9921-2c6f7ddc27fb.jpg";
        else if(id.equals("297088t"))
            tokenurl = "https://cards.scryfall.io/large/front/9/4/94e4345b-61b1-4026-a01c-c9f2036c5c8a.jpg";
        else if(id.equals("296943t"))
            tokenurl = "https://cards.scryfall.io/large/front/e/5/e5c0f400-41be-488b-be84-b07289b1ef62.jpg";
        else if(id.equals("612523t")) // Feather
            tokenurl = "https://cards.scryfall.io/large/front/e/4/e401e2a8-d0a3-4517-ba20-449a1fff7f85.jpg";
        else if(id.equals("611980t") || id.equals("611956t")) // Dragon Spirit 5/5
            tokenurl = "https://cards.scryfall.io/large/front/a/4/a4c06e08-2026-471d-a6d0-bbb0f040420a.jpg";
        else if (id.equals("583823t") || id.equals("583834t") || id.equals("585771t") || id.equals("586065t") || id.equals("586144t")) // Powerstone
            tokenurl = "https://cards.scryfall.io/large/front/d/4/d45fe4b6-aeaf-4f84-b660-c7b482ed8512.jpg";
			
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
                id.equals("470578") || id.equals("470571") || id.equals("470552") || id.equals("394490") || id.equals("114921") || id.equals("49775") ||
                id.equals("473123") || id.equals("473160") || id.equals("16743")  || id.equals("16741") || id.equals("294493") || id.equals("293253") ||
                id.equals("293198") || id.equals("479634") || id.equals("479702") || id.equals("489837") || id.equals("489861") || id.equals("491359")||
                id.equals("294872") || id.equals("295110") || id.equals("294842") || id.equals("295067") || id.equals("491767") || id.equals("295386") ||
                id.equals("295229") || id.equals("295387") || id.equals("295206") || id.equals("295706") || id.equals("497549") || id.equals("497666") ||
                id.equals("503860") || id.equals("522280") || id.equals("522111") || id.equals("527288") || id.equals("531927") || id.equals("527295") ||
                id.equals("111220") || id.equals("416829") || id.equals("296545") || id.equals("296694") || id.equals("540473") || id.equals("540464") ||
                id.equals("540708") || id.equals("539395") || id.equals("539417") || id.equals("540991") || id.equals("545724") || id.equals("297319") ||
                id.equals("296925") || id.equals("611094") || id.equals("611234") || id.equals("615336") || id.equals("615147") || id.equals("614778") ||
                id.equals("614771") || id.equals("614756") || id.equals("614666") || id.equals("612620") || id.equals("612152") || id.equals("612130") ||
                id.equals("612145") || id.equals("571160") || id.equals("571159") || id.equals("571105") || id.equals("570289")  || id.equals("570288") ||
                id.equals("570250") || id.equals("567228") || id.equals("563151") || id.equals("563150") || id.equals("563105") || id.equals("562899"))
            return false;
        return true;
    }

    public static Document findTokenPage(String imageurl, String name, String set, String[] availableSets, String tokenstats, String color, SDLActivity parent) throws Exception {
        Document doc = null;
        Elements outlinks = null;
        try {
            if(set.equalsIgnoreCase("DBL"))
                set = "VOW";
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
        System.out.println("Warning: Token " + name + " has not been found in " + set + " tokens, i will search for it between any other set in " + imageurl + " (it may take a long time)");
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
            boolean negativeId = false;
            int a = lines.indexOf("primitive=", lastIndex);
            if (a > 0) {
                if (lines.substring(a, lines.indexOf("\n", a)).split("=").length > 1)
                    primitive = lines.substring(a, lines.indexOf("\n", a)).split("=")[1];
            }
            int b = lines.indexOf("id=", lastIndex);
            if (b > 0) {
                if(lines.substring(b, lines.indexOf("\n", b)).contains("id=-"))
                    negativeId = true;
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
                    set.equals("STX") || set.equals("MID") || set.equals("CC2") || set.equals("VOW") ||
                    set.equals("DBL") || set.equals("Y22") || set.equals("MOM") || set.equals("NEO") ||
                    set.equals("SIR"))
                rarity = "";
            if(id != null && !rarity.equals("t") && (negativeId || id.equals("209162") || id.equals("209163") || id.equals("401721") ||
                    id.equals("401722") || id.equals("999902")))
                rarity = "t";
            if(id != null && (id.equals("1750411") || id.equals("5176911") || id.equals("44680711") || id.equals("29530711") ||
                    id.equals("45108910") || id.equals("530447") || id.equals("530448") || id.equals("530449") || id.equals("296817") ||
                    id.equals("296818") || id.equals("29339510") || id.equals("1749810") || id.equals("5197410") || id.equals("5249510") ||
                    id.equals("5247310") || id.equals("5213710") || id.equals("5253010") || id.equals("5270410") || id.equals("57018400") ||
                    id.equals("57018401")))
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
            if(id.equals("491633"))
                mappa.put("491633t", "Angel");
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
            String specialcardurl = getSpecialCardUrl(id, set);
            JSONObject card = findCardJsonById(id);
            if(specialcardurl.isEmpty() && card != null)
                specialcardurl = findCardImageUrl(card, mappa.get(id), id, "large");
            if (!specialcardurl.isEmpty()) {
                URL url = new URL(specialcardurl);
                HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
                if (httpcon == null) {
                    System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + ", i will not download it...");
                    res = mappa.get(id) + " - " + set + File.separator + id + ".jpg\n" + res;
                    continue;
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
                            continue;
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
                    continue;
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
                    continue;
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
                    continue;
                }
                if(card != null && hasToken(id)) {
                    String text = (String) card.get("oracle_text");
                    String nametoken = findTokenName(card, id, "Copy");
                    if (!nametoken.isEmpty() || (text != null && !text.isEmpty() && !text.trim().toLowerCase().contains("nontoken") && ((text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) ||
                            (text.trim().toLowerCase().contains("put") && text.trim().toLowerCase().contains("token"))))) {
                        System.out.println("The card: " + mappa.get(id) + " (" + id + ".jpg) can create a token, i will try to download that image too as " + id + "t.jpg");
                        String specialtokenurl = findTokenImageUrl(card, id, "large", "Copy");
                        if (!specialtokenurl.isEmpty()) {
                            URL urltoken = null;
                            urltoken = new URL(specialtokenurl);
                            HttpURLConnection httpcontoken = (HttpURLConnection) urltoken.openConnection();
                            if (httpcontoken == null) {
                                System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t.jpg), i will not download it...");
                                res = nametoken + " - " + set + File.separator + id + "t.jpg\n" + res;
                                continue;
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
                                        continue;
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
                                continue;
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
                                if (Border > 0)
                                    resizedToken = Bitmap.createBitmap(resizedToken, Border, Border, ImgX - 2 * Border, ImgY - 2 * Border);
                                FileOutputStream fout = new FileOutputStream(tokenimage);
                                resizedToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                fout.close();
                            } catch (Exception e) {
                                System.err.println("Error: Problem resizing token: " + id + "t.jpg, image may be corrupted...");
                                res = nametoken + " - " + set + File.separator + "thumbnails" + File.separator + id + "t.jpg\n" + res;
                                continue;
                            }
                            try {
                                Bitmap yourBitmapTokenthumb = BitmapFactory.decodeFile(tokenimage);
                                Bitmap resizedThumbToken = Bitmap.createScaledBitmap(yourBitmapTokenthumb, ThumbX, ThumbY, true);
                                if (BorderThumb > 0)
                                    resizedThumbToken = Bitmap.createBitmap(resizedThumbToken, BorderThumb, BorderThumb, ThumbX - 2 * BorderThumb, ThumbY - 2 * BorderThumb);
                                FileOutputStream fout = new FileOutputStream(tokenthumbimage);
                                resizedThumbToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                fout.close();
                            } catch (Exception e) {
                                System.err.println("Error: Problem resizing token thumbnail: " + id + "t.jpg, image may be corrupted...");
                                res = nametoken + " - " + set + File.separator + "thumbnails" + File.separator + id + "t.jpg\n" + res;
                            }
                        }
                    }
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
                    || scryset.equals("HTR18") || scryset.equals("HTR19") || scryset.equals("DKM") || scryset.equals("S00") || scryset.equals("XLN")
                    || scryset.equals("SOI") || scryset.equals("UST") || scryset.equals("PLG21") || scryset.equals("J21") || scryset.equals("CC2")
                    || scryset.equals("Q06") || scryset.equals("DBL") || scryset.equals("Y22") | scryset.equals("CLB") || scryset.equals("MOM")
                    || scryset.equals("MOC") || scryset.equals("BRO") || scryset.equals("MAT") || scryset.equals("BRC") || scryset.equals("BRR")
                    || scryset.equals("NEO") || scryset.equals("ONE") || scryset.equals("ONC") || scryset.equals("DMR") || scryset.equals("NEC")
                    || scryset.equals("J22")){
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

            if(targetres.equals("High") && !scryset.equals("TD2") && !scryset.equals("PRM") && !scryset.equals("TD0") && !scryset.equals("PZ2")
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
                    && !scryset.equals("HTR18") && !scryset.equals("HTR19") && !scryset.equals("DKM") && !scryset.equals("S00") && !scryset.equals("XLN")
                    && !scryset.equals("SOI") && !scryset.equals("UST") && !scryset.equals("PLG21") && !scryset.equals("J21") && !scryset.equals("CC2")
                    && !scryset.equals("Q06") && !scryset.equals("DBL") && !scryset.equals("Y22") && !scryset.equals("CLB") && !scryset.equals("MOM")
                    && !scryset.equals("MOC") && !scryset.equals("BRO") && !scryset.equals("MAT") && !scryset.equals("BRC") && !scryset.equals("BRR")
                    && !scryset.equals("NEO") && !scryset.equals("ONE") && !scryset.equals("ONC") && !scryset.equals("DMR") && !scryset.equals("NEC")
                    && !scryset.equals("J22")){
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
                    && !scryset.equals("HTR18") && !scryset.equals("HTR19") && !scryset.equals("DKM") && !scryset.equals("S00") && !scryset.equals("XLN")
                    && !scryset.equals("SOI") && !scryset.equals("UST") && !scryset.equals("PLG21") && !scryset.equals("J21") && !scryset.equals("CC2")
                    && !scryset.equals("Q06") && !scryset.equals("DBL") && !scryset.equals("Y22") && !scryset.equals("CLB") && !scryset.equals("MOM")
                    && !scryset.equals("MOC") && !scryset.equals("BRO") && !scryset.equals("MAT") && !scryset.equals("BRC") && !scryset.equals("BRR")
                    && !scryset.equals("NEO") && !scryset.equals("ONE") && !scryset.equals("ONC") && !scryset.equals("DMR") && !scryset.equals("NEC")
                    && !scryset.equals("J22")){
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
                            || scryset.equals("HTR19") || scryset.equals("DKM") || scryset.equals("S00") || scryset.equals("XLN") || scryset.equals("SOI")
                            || scryset.equals("UST") || scryset.equals("PLG21") || scryset.equals("J21") || scryset.equals("CC2") || scryset.equals("Q06")
                            || scryset.equals("DBL") || scryset.equals("Y22") || scryset.equals("CLB") || scryset.equals("MOM") || scryset.equals("MOC")
                            || scryset.equals("BRO") || scryset.equals("MAT") || scryset.equals("BRC") || scryset.equals("BRR") || scryset.equals("NEO")
                            || scryset.equals("ONE") || scryset.equals("ONC") || scryset.equals("DMR") || scryset.equals("NEC") || scryset.equals("J22")){
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
                    if (hasToken(id) && !text.trim().toLowerCase().contains("nontoken") && ((text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) || (text.trim().toLowerCase().contains("put") && text.trim().toLowerCase().contains("token")))) {
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
                        Elements imgstoken;
                        String specialtokenurl = getSpecialTokenUrl(id + "t", set);
                        if(specialtokenurl.isEmpty())
                            specialtokenurl = getSpecialCardUrl(id + "t", set);
                        if(specialtokenurl.isEmpty() && card != null)
                            specialtokenurl = findTokenImageUrl(card, id, "large", "Copy");
                        if(nametoken.isEmpty() && card != null)
                            nametoken = findTokenName(card, id, "Copy");
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
