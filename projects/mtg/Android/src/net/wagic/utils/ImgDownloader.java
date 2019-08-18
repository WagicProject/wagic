package net.wagic.utils;

import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.Enumeration;

import java.io.*;
import java.net.URL;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
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
	} catch (Exception e) {}
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

    public static void DownloadCardImages(String set, String[] availableSets, String targetres, String basePath, String destinationPath) throws IOException {

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

        for (int f = 1; f < availableSets.length; f++) {
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
                    if(entryName.contains("sets/")){
                        if(entryName.contains("_cards.dat")){
			    String[] names = entryName.split("/");
			    if(availableSets[f].equalsIgnoreCase(names[1])){
                                if(set.equalsIgnoreCase(names[1])){
				    stream = zipFile.getInputStream(entry);
				    byte[] buffer = new byte[2048];
				    java.nio.file.Path outDir = Paths.get(basePath);
				    filePath = outDir.resolve("_cards.dat");
                                    try {
				        FileOutputStream fos = new FileOutputStream(filePath.toFile());
                               		BufferedOutputStream bos = new BufferedOutputStream(fos, buffer.length);
                    			int len;
                    			while ((len = stream.read(buffer)) > 0) {
                        			bos.write(buffer, 0, len);
                    			}
					fos.close();
					bos.close();
                		    } catch (Exception ex) {}
				    f = availableSets.length;
				    break;
				} else if(set.equalsIgnoreCase("*.*")){
			            stream = zipFile.getInputStream(entry);
			            break;
                       		}		
                            }
                        }
		    }
		}	    
            }
            catch (IOException ioe) {
                System.out.println("Error opening zip file" + ioe);
            }
            finally {
                try {
                    if (zipFile!=null) {
                        zipFile.close();
                    }
                }
                catch (IOException ioe) {
                    System.out.println("Error while closing zip file" + ioe);
                }
            }

            //File folder = new File(basePath + set + "\\");
            //String filePath = folder.getAbsolutePath() + "\\_cards.dat";
            String lines = readLineByLineJava8(filePath.toString());
	    int totalcards = 0;
            String findStr = "total=";
            int lastIndex = lines.indexOf(findStr);
            String totals = lines.substring(lastIndex, lines.indexOf("\n", lastIndex));
            totalcards = Integer.parseInt(totals.split("=")[1]);        
            for (int i = 0; i < totalcards; i++) {
                findStr = "[card]";
		lastIndex = lines.indexOf(findStr);
		String id = null;
		String primitive = null;
		int a = lines.indexOf("primitive=",lastIndex);
		if(a > 0)
                    primitive = lines.substring(a, lines.indexOf("\n",a)).replace("//", "-").split("=")[1];
                int b = lines.indexOf("id=",lastIndex);
                if(b > 0)
		    id = lines.substring(b, lines.indexOf("\n",b)).replace("-", "").split("=")[1];
                int c = lines.indexOf("[/card]",lastIndex);
                if(c > 0)
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
                Document doc = Jsoup.connect(baseurl + id).get();
                Elements divs = doc.select("body div");
                try {
                    doc = Jsoup.connect(imageurl + set.toLowerCase()).get();
                } catch (Exception e) {
                    System.err.println("The SET: " + set + " has not been found on ScryFall!");
                    y = mappa.size();
                    continue;
                }
                Elements imgs = doc.select("body img");
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
                        InputStream in = new BufferedInputStream(url.openStream());
                        ByteArrayOutputStream out = new ByteArrayOutputStream();
                        byte[] buf = new byte[1024];
                        int n = 0;
                        while (-1 != (n = in.read(buf))) {
                            out.write(buf, 0, n);
                        }
                        out.close();
                        in.close();
                        byte[] response = out.toByteArray();
                        String cardimage = imgPath + "\\" + id + ".jpg";
                        String thumbcardimage = thumbPath + "\\" + id + ".jpg";
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
                        if (text.trim().toLowerCase().contains("create") && text.trim().toLowerCase().contains("creature token")) {
                            boolean tokenfound = false;
                            String arrays[] = text.trim().split(" ");
                            String nametoken = "";
                            for (int l = 1; l < arrays.length - 1; l++) {
                                if (arrays[l].equalsIgnoreCase("creature") && arrays[l + 1].toLowerCase().contains("token")) {
                                    nametoken = arrays[l - 1];
                                    break;
                                }
                            }
                            if (nametoken.isEmpty()) {
                                tokenfound = false;
                                nametoken = mappa.get(id);
                                doc = Jsoup.connect(imageurl + set.toLowerCase()).get();
                            } else {
                                try {
                                    doc = Jsoup.connect(imageurl + "t" + set.toLowerCase()).get();
                                    tokenfound = true;
                                } catch(Exception e) {
                                    tokenfound = false;
                                }
                            }
                            Elements imgstoken = doc.select("body img");
                            for (int p = 0; p < imgstoken.size(); p++) {
                                String titletoken = imgstoken.get(p).attributes().get("title");
                                if (titletoken.toLowerCase().contains(nametoken.toLowerCase())) {
                                    String CardImageToken = imgstoken.get(p).attributes().get("src");
                                    if (CardImageToken.isEmpty())
                                        CardImageToken = imgstoken.get(p).attributes().get("data-src");
                                    URL urltoken = new URL(CardImageToken);
                                    InputStream intoken = new BufferedInputStream(urltoken.openStream());
                                    ByteArrayOutputStream outtoken = new ByteArrayOutputStream();
                                    byte[] buftoken = new byte[1024];
                                    int ntoken = 0;
                                    while (-1 != (ntoken = intoken.read(buftoken))) {
                                        outtoken.write(buftoken, 0, ntoken);
                                    }
                                    outtoken.close();
                                    intoken.close();
                                    byte[] responsetoken = outtoken.toByteArray();
                                    String tokenimage = "";
                                    String tokenthumbimage = "";
                                    if (tokenfound) {
                                        tokenimage = imgPath + "\\" + id + "t.jpg";
                                        tokenthumbimage = thumbPath + "\\" + id + "t.jpg";
                                    } else {
                                        tokenimage = imgPath + "\\" + id + "_tocheck_t.jpg";
                                        tokenthumbimage = thumbPath + "\\" + id + "_tocheck_t.jpg";
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
        }
    }
}
