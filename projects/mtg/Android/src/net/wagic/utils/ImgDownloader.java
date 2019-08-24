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
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.stream.Stream;

import android.graphics.*;

public class ImgDownloader {

    private static String convertStreamToString(java.io.InputStream inputStream) {
        final int bufferSize = 1024;
        final char[] buffer = new char[bufferSize];
        final StringBuilder out = new StringBuilder();
        try {
            Reader in = new InputStreamReader(inputStream, StandardCharsets.ISO_8859_1);
            for (; ; ) {
                int rsz = in.read(buffer, 0, buffer.length);
                if (rsz < 0)
                    break;
                out.append(buffer, 0, rsz);
            }
        } catch (Exception e) {
        }
        return out.toString();
    }

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

    public static String getSetInfo(String setName, boolean zipped, String path) {
        String cardsfilepath = "";
        boolean todelete = false;
        if (zipped) {
            File resFolder = new File(path + File.separator);
            File[] listOfFile = resFolder.listFiles();
            ZipFile zipFile = null;
            InputStream stream = null;
            java.nio.file.Path filePath = null;
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
                    if (entryName.contains("sets/")) {
                        if (entryName.contains("_cards.dat")) {
                            String[] names = entryName.split("/");
                            if (setName.equalsIgnoreCase(names[1])) {
                                stream = zipFile.getInputStream(entry);
                                byte[] buffer = new byte[1];
                                java.nio.file.Path outDir = Paths.get(path + File.separator);
                                filePath = outDir.resolve("_cards.dat");
                                try {
                                    FileOutputStream fos = new FileOutputStream(filePath.toFile());
                                    BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                                    int len;
                                    while ((len = stream.read(buffer)) != -1) {
                                        bos.write(buffer, 0, len);
                                    }
                                    fos.close();
                                    bos.close();
                                    cardsfilepath = filePath.toString();
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

    public static String getSpecialCardUrl(String id){
        String cardurl = "";
        if(id.equals("15208711"))
            cardurl = "https://img.scryfall.com/cards/large/front/9/c/9c138bf9-8be6-4f1a-a82c-a84938ab84f5.jpg?1562279137";
        else if(id.equals("15208712"))
            cardurl = "https://img.scryfall.com/cards/normal/front/d/4/d453ee89-6122-4d51-989c-e78b046a9de3.jpg?1561758141";
        else if(id.equals("2050321"))
            cardurl = "https://img.scryfall.com/cards/large/front/1/8/18b9c83d-4422-4b95-9fc2-070ed6b5bdf6.jpg?1562701921";
        else if(id.equals("2050322"))
            cardurl = "https://crystal-cdn4.crystalcommerce.com/photos/504053/ooze_token_b.jpg";
        else if(id.equals("22010012"))
            cardurl = "https://img.scryfall.com/cards/normal/front/8/4/84dc847c-7a37-4c7f-b02c-30b3e4c91fb6.jpg?1561757490";
        return cardurl;
    }

    public static String getSpecialTokenUrl(String id){
        String tokenurl = "";
        if(id.equals("75291t"))
            tokenurl = "http://4.bp.blogspot.com/-y5Fanm3qvrU/Vmd4gGnl2DI/AAAAAAAAAWY/FCrS9FTgOJk/s1600/Tatsumasa%2BToken.jpg";
        else if(id.equals("202474t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-884-hr.jpg";
        else if(id.equals("202590t"))
            tokenurl = "https://deckmaster.info/images/cards/AST/-892-hr.jpg";
        else if(id.equals("201124t"))
            tokenurl = "http://i1013.photobucket.com/albums/af260/lovesoldier99/STARFISHTOKEN.jpg";
        else if(id.equals("184735"))
            tokenurl = "https://i.pinimg.com/originals/a9/fb/37/a9fb37bdfa8f8013b7eb854d155838e2.jpg";
        else if(id.equals("184598t"))
            tokenurl = "https://deckmaster.info/images/cards/HM/-2070-hr.jpg";
        else if(id.equals("184589t"))
            tokenurl = "http://d1f83aa4yffcdn.cloudfront.net/TOKEN/2%202%20Black%20Zombie.jpg";
        else if(id.equals("184730t"))
            tokenurl="https://www.mtg.onl/static/c88f42f8bd5a7c25aa36902546b690f5/4d406/PROXY_Knight_W_1_1.jpg";
        else if(id.equals("1649t") || id.equals("201182t"))
            tokenurl = "https://pbs.twimg.com/media/DH9n-2JVwAA0o8z.jpg";
        else if(id.equals("140233t") || id.equals("191239t") || id.equals("205957t"))
            tokenurl = "https://i860.photobucket.com/albums/ab170/mistergreen527/White%20Tokens/WAvatarX-X1.jpg";
        else if(id.equals("1686t") || id.equals("2881t") ||  id.equals("201231t"))
            tokenurl = "https://deckmaster.info/images/cards/A25/-5648-hr.jpg";
        else if(id.equals("121261t"))
            tokenurl = "https://i.pinimg.com/originals/a9/fb/37/a9fb37bdfa8f8013b7eb854d155838e2.jpg";
        else if(id.equals("368951t"))
            tokenurl = "https://d1rw89lz12ur5s.cloudfront.net/photo/facetofacegames/file/36262794e9f37368e7872326715ac806/eletok.jpg";
        else if(id.equals("46168t"))
            tokenurl = "https://deckmaster.info/images/cards/KLD/-3287-hr.jpg";
        else if(id.equals("49026t"))
            tokenurl = "https://www.mtg.onl/static/a9d81341e62e39e75075b573739f39d6/4d406/PROXY_Wirefly_2_2.jpg";
        else if(id.equals("414506t"))
            tokenurl = "https://poromagia.com/media/cache/25/f0/25f0cd307adc18d7655c465408267469.jpg";
        else if(id.equals("6142t"))
            tokenurl = "https://cdn.staticneo.com/w/mtg/c/cd/Beast5.jpg";
        else if(id.equals("126166t"))
            tokenurl = "https://deckmaster.info/images/cards/C14/-487-hr.jpg";
        else if(id.equals("136155t"))
            tokenurl = "http://static1.squarespace.com/static/583dca25ff7c5080991b2c87/583de52de6f2e18631eb2b32/58405d0dbe6594762f5bd8e6/1565969982322/wurm-white.jpg";
        else if(id.equals("107091t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/13/534/635032476540667501.jpg";
        else if(id.equals("452760t"))
            tokenurl = "https://deckmaster.info/images/cards/M19/-6036.jpg";
        else if(id.equals("2959t"))
            tokenurl = "https://deckmaster.info/images/cards/HM/-2070-hr.jpg";
        else if(id.equals("380486t"))
            tokenurl = "https://deckmaster.info/images/cards/BNG/-5-hr.jpg";
        else if(id.equals("380487t"))
            tokenurl = "https://poromagia.com/media/cache/25/f0/25f0cd307adc18d7655c465408267469.jpg";
        else if(id.equals("234849t"))
            tokenurl = "https://deckmaster.info/images/cards/RTR/-61-hr.jpg";
        else if(id.equals("23319t"))
            tokenurl = "https://i860.photobucket.com/albums/ab170/mistergreen527/White%20Tokens/WReflectionX-X1.jpg";
        else if(id.equals("205297t") || id.equals("50104t"))
            tokenurl = "https://www.mtg.onl/static/df30395b530524a3988428d4c0b37161/4d406/PROXY_Pest_0_1.jpg";
        else if(id.equals("3449t"))
            tokenurl = "https://www.mtg.onl/static/8c7fed1a0b8edd97c0fb0ceab24a654f/4d406/PROXY_Goblin_Scout_R_1_1.jpg";
        else if(id.equals("3392t"))
            tokenurl = "https://deckmaster.info/images/cards/DDR/417498-hr.jpg";
        else if(id.equals("3280t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/54/421/635032484680831888.jpg";
        else if(id.equals("3242t"))
            tokenurl = "https://deckmaster.info/images/cards/MI/-2828-hr.jpg";
        else if(id.equals("426025t"))
            tokenurl = "https://cdn.shopify.com/s/files/1/0790/8591/products/Grnelementalfinal_800x800.jpg?v=1476398274";
        else if(id.equals("19878t"))
            tokenurl = "https://www.cardkingdom.com/images/magic-the-gathering/commander-2014/ape-token-zombie-token-blue-65252-medium.jpg";
        else if(id.equals("21381t") || id.equals("40198t"))
            tokenurl = "https://img.scryfall.com/cards/large/back/8/c/8ce60642-e207-46e6-b198-d803ff3b47f4.jpg?1562921132";
        else if(id.equals("265141t"))
            tokenurl = "https://media.mtgsalvation.com/attachments/102/31/635032498723573408.jpg";
        else if(id.equals("24624t"))
            tokenurl = "https://www.mtg.onl/static/6d717cba653ea9e3f6bd1419741671cb/4d406/PROXY_Minion_B_1_1.jpg";
        return tokenurl;
    }

    public static boolean hasToken(String id){
        if(id.equals("456378") || id.equals("2912") || id.equals("1514") || id.equals("364") || id.equals("69") || id.equals("369012") ||
                id.equals("417759") || id.equals("386476") || id.equals("456371") || id.equals("456360") || id.equals("391958") || id.equals("466959") ||
                id.equals("466813") || id.equals("201176") || id.equals("202483") || id.equals("3546") || id.equals("425949") || id.equals("426027") ||
                id.equals("425853") || id.equals("425846") || id.equals("426036") || id.equals("370387") || id.equals("29955") || id.equals("29989") ||
                id.equals("19741") || id.equals("19722") || id.equals("19706") || id.equals("24597") || id.equals("24617") || id.equals("24563"))
            return false;
        return true;
    }

    public static Document findTokenPage(String imageurl, String name, String set, String[] availableSets, String tokenstats) throws Exception {
        Document doc = null;
        Elements outlinks = null;
        try {
            doc = Jsoup.connect(imageurl + "t" + set.toLowerCase()).get();
            if(doc != null) {
                outlinks = doc.select("body a");
                if (outlinks != null) {
                    for (int k = 0; k < outlinks.size(); k++) {
                        String linktoken = outlinks.get(k).attributes().get("href");
                        if (linktoken != null && !linktoken.isEmpty()) {
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if(tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if(stats != null) {
                                    for (int j = 0; j < stats.size(); j++) {
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
        System.out.println("Warning: Token " + name + " has not been found between " + set + " tokens, i will search for it in https://deckmaster.info");
        String json = "";
        try {
            URL url = new URL("https://deckmaster.info/includes/ajax.php?action=cardSearch&searchString=" + name);
            HttpURLConnection httpcon = (HttpURLConnection) url.openConnection();
            if(httpcon != null) {
                httpcon.addRequestProperty("User-Agent", "Mozilla/4.76");
                InputStream stream = httpcon.getInputStream();
                if(stream != null) {
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
        for (int i = 0; i < urls.size(); i++) {
            try {
                Document tokendoc = Jsoup.connect("https://deckmaster.info/card.php?multiverseid=" + urls.get(i)).get();
                if(tokendoc == null)
                    continue;
                Elements stats = tokendoc.select("head meta");
                if(stats != null) {
                    for (int j = 0; j < stats.size(); j++) {
                        if (stats.get(j).attributes().get("content").contains("Token Creature") && stats.get(j).attributes().get("content").toLowerCase().contains(name.toLowerCase())) {
                            if (stats.get(j).attributes().get("content").contains(tokenstats.replace("X/X", "★/★")))
                                return tokendoc;
                            stats = tokendoc.select("body textarea");
                            if (stats != null) {
                                for (int y = 0; y < stats.size(); y++) {
                                    List<Node> nodes = stats.get(y).childNodes();
                                    if (nodes != null) {
                                        for (int p = 0; p < nodes.size(); p++) {
                                            if (stats.get(y).childNode(p).attributes().get("#text").contains(tokenstats))
                                                return tokendoc;
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
        System.out.println("Warning: Token " + name + " has not been found in https://deckmaster.info so i will search for it between any other set in " + imageurl + " (it can take long time)");
        for (int i = 1; i < availableSets.length; i++) {
            String currentSet = availableSets[i].toLowerCase().split(" - ")[0];
            if (!currentSet.equalsIgnoreCase(set)) {
                try {
                    doc = Jsoup.connect(imageurl + "t" + currentSet).get();
                    if(doc == null)
                        continue;
                    outlinks = doc.select("body a");
                    if(outlinks != null) {
                        for (int k = 0; k < outlinks.size(); k++) {
                            String linktoken = outlinks.get(k).attributes().get("href");
                            try {
                                Document tokendoc = Jsoup.connect(linktoken).get();
                                if(tokendoc == null)
                                    continue;
                                Elements stats = tokendoc.select("head meta");
                                if(stats != null) {
                                    for (int j = 0; j < stats.size(); j++) {
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
        System.err.println("Error: Token " + name + " has not been found between any set of " + imageurl);
        throw new Exception();
    }

    public static String DownloadCardImages(String set, String[] availableSets, String targetres, String basePath, String destinationPath) throws IOException {
        String res = "";

        String baseurl = "https://gatherer.wizards.com/Pages/Card/Details.aspx?multiverseid=";
        String imageurl = "https://scryfall.com/sets/";

        Integer ImgX = 0;
        Integer ImgY = 0;
        Integer ThumbX = 0;
        Integer ThumbY = 0;

        if (targetres.equals("HI")) {
            ImgX = 488;
            ImgY = 680;
            ThumbX = 90;
            ThumbY = 128;
        } else if (targetres.equals("LOW")) {
            ImgX = 244;
            ImgY = 340;
            ThumbX = 45;
            ThumbY = 64;
        }

        File baseFolder = new File(basePath);
        File[] listOfFiles = baseFolder.listFiles();
        String currentSet = "";
        for (int f = 1; f < availableSets.length; f++) {
            if (set.equalsIgnoreCase("*.*"))
                currentSet = availableSets[f];
            else
                currentSet = set;
            Map<String, String> mappa = new HashMap<String, String>();
            ZipFile zipFile = null;
            InputStream stream = null;
            java.nio.file.Path filePath = null;
            try {
                zipFile = new ZipFile(basePath + "/" + listOfFiles[0].getName());
                Enumeration<? extends ZipEntry> e = zipFile.entries();
                while (e.hasMoreElements()) {
                    ZipEntry entry = e.nextElement();
                    String entryName = entry.getName();
                    if (entryName != null && entryName.contains("sets/")) {
                        if (entryName.contains("_cards.dat")) {
                            String[] names = entryName.split("/");
                            if (currentSet.equalsIgnoreCase(names[1])) {
                                stream = zipFile.getInputStream(entry);
                                byte[] buffer = new byte[1];
                                java.nio.file.Path outDir = Paths.get(basePath);
                                filePath = outDir.resolve("_cards.dat");
                                try {
                                    FileOutputStream fos = new FileOutputStream(filePath.toFile());
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
                                if (!set.equalsIgnoreCase("*.*"))
                                    f = availableSets.length;
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
            while (lines.contains("[card]")) {
                findStr = "[card]";
                lastIndex = lines.indexOf(findStr);
                String id = null;
                String primitive = null;
                int a = lines.indexOf("primitive=", lastIndex);
                if (a > 0)
                    primitive = lines.substring(a, lines.indexOf("\n", a)).replace("//", "-").split("=")[1];
                int b = lines.indexOf("id=", lastIndex);
                if (b > 0)
                    id = lines.substring(b, lines.indexOf("\n", b)).replace("-", "").split("=")[1];
                int c = lines.indexOf("[/card]", lastIndex);
                if (c > 0)
                    lines = lines.substring(c + 8);
                if (primitive != null && id != null && !id.equalsIgnoreCase("null"))
                    mappa.put(id, primitive);
            }

            File imgPath = new File(destinationPath + set + "/");
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

            File thumbPath = new File(destinationPath + set + "/thumbnails/");
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

            for (int y = 0; y < mappa.size(); y++) {
                String id = mappa.keySet().toArray()[y].toString();
                Document doc = null;
                try{
                    doc = Jsoup.connect(baseurl + id).get();
                } catch(Exception e) {
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl  + id + ", i will retry 2 times more...");
                    try{
                        doc = Jsoup.connect(baseurl + id).get();
                    } catch(Exception e2) {
                        System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl  + id + ", i will retry 1 time more...");
                        try{
                            doc = Jsoup.connect(baseurl + id).get();
                        } catch(Exception e3) {
                            System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl  + id + ", i will not retry anymore...");
                            continue;
                        }
                    }
                }
                if(doc == null){
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl  + id + ", i will not retry anymore...");
                    continue;
                }
                Elements divs = doc.select("body div");
                if(divs == null){
                    System.err.println("Error: Problem reading card (" + mappa.get(id) + ") infos from: " + baseurl  + id + ", i will not retry anymore...");
                    continue;
                }
                String scryset = currentSet;
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
                try {
                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                } catch (Exception e) {
                    System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                    try {
                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                    } catch (Exception e2) {
                        System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                        try {
                            doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                        } catch (Exception e3) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                            res = mappa.get(id) + " - " + currentSet + "/" + id + ".jpg\n" + res;
                            continue;
                        }
                    }
                }
                String specialcardurl = getSpecialCardUrl(id);
                if(!specialcardurl.isEmpty()){
                    URL url = new URL(specialcardurl);
                    InputStream in;
                    try{
                        in = new BufferedInputStream(url.openStream());
                    }catch(Exception ex){
                        System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                        try {
                            in = new BufferedInputStream(url.openStream());
                        } catch (Exception ex2) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                            try {
                                in = new BufferedInputStream(url.openStream());
                            } catch (Exception ex3) {
                                System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                                break;
                            }
                        }
                    }
                    ByteArrayOutputStream out = new ByteArrayOutputStream();
                    byte[] buf = new byte[1024];
                    int n = 0;
                    while (-1 != (n = in.read(buf))) {
                        out.write(buf, 0, n);
                    }
                    out.close();
                    in.close();
                    byte[] response = out.toByteArray();
                    String cardimage = imgPath + "/" + id + ".jpg";
                    String thumbcardimage = thumbPath + "/" + id + ".jpg";
                    FileOutputStream fos = new FileOutputStream(cardimage);
                    fos.write(response);
                    fos.close();

                    Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                    Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                    try {
                        FileOutputStream fout = new FileOutputStream(cardimage);
                        resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmap, ThumbX, ThumbY, true);
                    try {
                        FileOutputStream fout = new FileOutputStream(thumbcardimage);
                        resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }

                    continue;
                }
                if(doc == null){
                    System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not download it...");
                    continue;
                }
                Elements imgs = doc.select("body img");
                if(imgs == null){
                    System.err.println("Error: Problem fetching card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not download it...");
                    continue;
                }
                int k;
                for (k = 0; k < divs.size(); k++)
                    if (divs.get(k).childNodes().size() > 0 && divs.get(k).childNode(0).toString().toLowerCase().contains("card name"))
                        break;
                if (k >= divs.size())
                    continue;
                String cardname = divs.get(k + 1).childNode(0).attributes().get("#text").replace("\r\n", "").trim();

                for (int i = 0; i < imgs.size(); i++) {
                    String title = imgs.get(i).attributes().get("title");
                    if (title.toLowerCase().contains(cardname.toLowerCase())) {
                        String CardImage = imgs.get(i).attributes().get("src");
                        if (CardImage.isEmpty())
                            CardImage = imgs.get(i).attributes().get("data-src");
                        URL url = new URL(CardImage);
                        InputStream in = null;
                        try {
                            in = new BufferedInputStream(url.openStream());
                        } catch (IOException ex) {
                            System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 2 times more...");
                            try {
                                in = new BufferedInputStream(url.openStream());
                            } catch (IOException ex2) {
                                System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will retry 1 time more...");
                                try {
                                    in = new BufferedInputStream(url.openStream());
                                } catch (IOException ex3) {
                                    System.err.println("Error: Problem downloading card: " + mappa.get(id) + "-" + id + " from " + scryset + " on ScryFall, i will not retry anymore...");
                                    break;
                                }
                            }
                        }
                        ByteArrayOutputStream out = new ByteArrayOutputStream();
                        byte[] buf = new byte[1024];
                        int n = 0;
                        while (-1 != (n = in.read(buf))) {
                            out.write(buf, 0, n);
                        }
                        out.close();
                        in.close();
                        byte[] response = out.toByteArray();
                        String cardimage = imgPath + "/" + id + ".jpg";
                        String thumbcardimage = thumbPath + "/" + id + ".jpg";
                        FileOutputStream fos = new FileOutputStream(cardimage);
                        fos.write(response);
                        fos.close();

                        Bitmap yourBitmap = BitmapFactory.decodeFile(cardimage);
                        Bitmap resized = Bitmap.createScaledBitmap(yourBitmap, ImgX, ImgY, true);
                        try {
                            FileOutputStream fout = new FileOutputStream(cardimage);
                            resized.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        Bitmap resizedThumb = Bitmap.createScaledBitmap(yourBitmap, ThumbX, ThumbY, true);
                        try {
                            FileOutputStream fout = new FileOutputStream(thumbcardimage);
                            resizedThumb.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        String text = "";
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
                        if (hasToken(id) &&  ((text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) || (text.trim().toLowerCase().contains("put") && text.trim().toLowerCase().contains("token")))) {
                            boolean tokenfound = false;
                            String arrays[] = text.trim().split(" ");
                            String nametoken = "";
                            String nametocheck = "";
                            String tokenstats = "";
                            for (int l = 1; l < arrays.length - 1; l++) {
                                if (arrays[l].equalsIgnoreCase("creature") && arrays[l + 1].toLowerCase().contains("token")) {
                                    nametoken = arrays[l - 1];
                                    if (l - 3 > 0)
                                        tokenstats = arrays[l - 3];
                                    if (!tokenstats.contains("/")) {
                                        if (l - 4 > 0)
                                            tokenstats = arrays[l - 4];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 5 > 0)
                                            tokenstats = arrays[l - 5];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 6 > 0)
                                            tokenstats = arrays[l - 6];
                                    }
                                    if (!tokenstats.contains("/")) {
                                        if (l - 7 > 0)
                                            tokenstats = arrays[l - 7];
                                    }
                                    if (nametoken.equalsIgnoreCase("artifact")) {
                                        if (l - 2 > 0)
                                            nametoken = arrays[l - 2];
                                        if (l - 4 > 0)
                                            tokenstats = arrays[l - 4];
                                        if (!tokenstats.contains("/")) {
                                            if (l - 5 > 0)
                                                tokenstats = arrays[l - 5];
                                        }
                                        if (!tokenstats.contains("/")) {
                                            if (l - 6 > 0)
                                                tokenstats = arrays[l - 6];
                                        }
                                        if (!tokenstats.contains("/")) {
                                            if (l - 7 > 0)
                                                tokenstats = arrays[l - 7];
                                        }
                                        if (!tokenstats.contains("/")) {
                                            if (l - 8 > 0)
                                                tokenstats = arrays[l - 8];
                                        }
                                    }
                                    if (!tokenstats.contains("/"))
                                        tokenstats = "";
                                    break;
                                } else if (arrays[l].equalsIgnoreCase("put") && arrays[l + 3].toLowerCase().contains("token")) {
                                    nametoken = arrays[l + 2];
                                    for (int j = 1; j < arrays.length - 1; j++) {
                                        if (arrays[j].contains("/"))
                                            tokenstats = arrays[j];
                                    }
                                    break;
                                }
                            }
                            String specialtokenurl = getSpecialTokenUrl(id + "t");
                            Elements imgstoken;
                            if(!specialtokenurl.isEmpty()) {
                                try{
                                    doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                                } catch (Exception ex) {
                                    System.err.println("Error: Problem occurring while searching for token: " + nametoken + "-" + id + "t, i will not download it...");
                                    break;
                                }
                                if(doc == null)
                                    break;
                                imgstoken = doc.select("body img");
                                if(imgstoken == null)
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
                                        doc = findTokenPage(imageurl, nametoken, scryset, availableSets, tokenstats);
                                        tokenfound = true;
                                        nametocheck = nametoken;
                                    } catch (Exception e) {
                                        tokenfound = false;
                                        nametocheck = mappa.get(id);
                                        doc = Jsoup.connect(imageurl + scryset.toLowerCase()).get();
                                    }
                                }
                                if(doc == null)
                                    break;
                                imgstoken = doc.select("body img");
                                if(imgstoken == null)
                                    break;
                            }
                            for (int p = 0; p < imgstoken.size(); p++) {
                                String titletoken = imgstoken.get(p).attributes().get("alt");
                                if (titletoken.isEmpty())
                                    titletoken = imgstoken.get(p).attributes().get("title");
                                if (titletoken.toLowerCase().contains(nametocheck.toLowerCase())) {
                                    String CardImageToken = imgstoken.get(p).attributes().get("src");
                                    if (CardImageToken.isEmpty())
                                        CardImageToken = imgstoken.get(p).attributes().get("data-src");
                                    URL urltoken = new URL(CardImageToken);
                                    if(!specialtokenurl.isEmpty())
                                        urltoken = new URL(specialtokenurl);
                                    HttpURLConnection httpcontoken = (HttpURLConnection) urltoken.openConnection();
                                    if(httpcontoken == null) {
                                        System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will not download it...");
                                        break;
                                    }
                                    httpcontoken.addRequestProperty("User-Agent", "Mozilla/4.76");
                                    InputStream intoken = null;
                                    try {
                                        intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                    } catch (IOException ex) {
                                        System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will retry 2 times more...");
                                        try {
                                            intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                        } catch (IOException ex2) {
                                            System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will retry 1 time more...");
                                            try {
                                                intoken = new BufferedInputStream(httpcontoken.getInputStream());
                                            } catch (IOException ex3) {
                                                System.err.println("Error: Problem downloading token: " + nametoken + "-" + id + "t, i will not retry anymore...");
                                                break;
                                            }
                                        }
                                    }
                                    ByteArrayOutputStream outtoken = new ByteArrayOutputStream();
                                    byte[] buftoken = new byte[1024];
                                    int ntoken = 0;
                                    while (-1 != (ntoken = intoken.read(buftoken))) {
                                        outtoken.write(buftoken, 0, ntoken);
                                    }
                                    outtoken.close();
                                    intoken.close();
                                    byte[] responsetoken = outtoken.toByteArray();
                                    String tokenimage = imgPath + File.separator + id + "t.jpg";
                                    String tokenthumbimage = thumbPath + File.separator + id + "t.jpg";
                                    if (!tokenfound && !id.equals("464007t")) {
                                        System.err.println("Error: Problem downloading token: " + nametoken + " (" + id + "t) i will use the same image of its source card");
                                    }
                                    FileOutputStream fos2 = new FileOutputStream(tokenimage);
                                    fos2.write(responsetoken);
                                    fos2.close();

                                    Bitmap yourBitmapToken = BitmapFactory.decodeFile(tokenimage);
                                    Bitmap resizedToken = Bitmap.createScaledBitmap(yourBitmapToken, ImgX, ImgY, true);
                                    try {
                                        FileOutputStream fout = new FileOutputStream(tokenimage);
                                        resizedToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                    } catch (IOException e) {
                                        e.printStackTrace();
                                    }
                                    Bitmap resizedThumbToken = Bitmap.createScaledBitmap(yourBitmapToken, ThumbX, ThumbY, true);
                                    try {
                                        FileOutputStream fout = new FileOutputStream(tokenthumbimage);
                                        resizedThumbToken.compress(Bitmap.CompressFormat.JPEG, 100, fout);
                                    } catch (IOException e) {
                                        e.printStackTrace();
                                    }

                                    break;
                                }
                            }
                        }

                        break;
                    }
                }
            }
            try {
                try {
                    File oldzip = new File(destinationPath + "/" + set + "/" + set + ".zip");
                    oldzip.delete();
                } catch (Exception e) {
                }
                ZipParameters zipParameters = new ZipParameters();
                zipParameters.setCompressionMethod(CompressionMethod.STORE);
                File folder = new File(destinationPath + set + "/");
                File[] listOfFile = folder.listFiles();
                net.lingala.zip4j.ZipFile zipped = new net.lingala.zip4j.ZipFile(destinationPath + "/" + set + "/" + set + ".zip");
                for (int i = 0; i < listOfFile.length; i++) {
                    if (listOfFile[i].isDirectory()) {
                        zipped.addFolder(listOfFile[i], zipParameters);
                    } else {
                        zipped.addFile(listOfFile[i], zipParameters);
                    }
                }
                File destFolder = new File(destinationPath + set + "/");
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
