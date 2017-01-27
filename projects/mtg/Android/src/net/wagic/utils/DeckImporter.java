package net.wagic.utils;

import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.File;
import java.io.IOException;
import java.util.Scanner;

import android.util.Log;

public class DeckImporter
{
    public static String importDeck( File f, String mypath, String activePath )
    {
        String message = "";
        String deck = "";
        String deckname = "";
        String prefix = "#SB:";
        int cardcount = 0;
        if(f.exists() && !f.isDirectory())
        { 
            deckname = f.getName();
            int pos = deckname.lastIndexOf(".");
            if (pos > 0) 
            {
                deckname = deckname.substring(0, pos);
            }
            deck += "#NAME:"+deckname+"\n"; 
            try
            {
                Scanner scanner = new Scanner(new File(mypath));                
                if (scanner.hasNext()) 
                {
                    while (scanner.hasNext()) 
                    {
                        String line = scanner.nextLine();
                        line = line.trim();
                        if (!line.equals("") && cardcount < 61) // don't write out blank lines
                        {
                            String[] slines = line.split("\\s+");
                            String arranged = "";
                            for (int idx = 1; idx < slines.length; idx++)
                            {
                                arranged += slines[idx] + " ";
                            }
                            if ((isNumeric(slines[0])) && arranged != null)
                            {
                                if (slines[1] != null && slines[1].startsWith("["))
                                {
                                    arranged = arranged.substring(5);
                                    slines[1] = slines[1].replaceAll("\\[", "").replaceAll("\\]", "");
                                    deck += arranged + " (" + renameSet(slines[1]) + ") * " + slines[0] + "\n";
                                } else
                                {
                                    deck += arranged + "(*) * " + slines[0] + "\n";
                                }
                                cardcount += Integer.parseInt(slines[0]);
                            }
                        }
                    }
                    File profile = new File(activePath + "/Res/settings/options.txt");
                    if (profile.exists() && !profile.isDirectory())
                    {
                        String profileName = getActiveProfile(profile);
                        if (profileName != "Missing!")
                        {
                            File rootProfiles = new File(activePath + "/Res/profiles/" + profileName);
                            if (rootProfiles.exists() && rootProfiles.isDirectory())
                            {
                                //save deck
                                int countdeck = 1;
                                File[] files = rootProfiles.listFiles();
                                for (int i = 0; i < files.length; i++)
                                {//check if there is available deck...
                                    if (files[i].getName().startsWith("deck"))
                                        countdeck++;
                                }
                                File toSave = new File(rootProfiles + "/deck" + countdeck + ".txt");
                                try
                                {
                                    FileOutputStream fop = new FileOutputStream(toSave);

                                    // if file doesn't exists, then create it
                                    if (!toSave.exists())
                                    {
                                        toSave.createNewFile();
                                    }
                                    // get the content in bytes
                                    byte[] contentInBytes = deck.getBytes();
                                    fop.write(contentInBytes);
                                    fop.flush();
                                    fop.close();
                                    message = "Import Deck Success!\n" + cardcount + " total cards in this deck\n\n" + deck;
                                } catch (IOException e)
                                {
                                    message = e.getMessage();
                                }
                            } else
                            {
                                message = "Missing Folder!";
                            }
                        }
                    } else
                    {
                        message = "Invalid Profile!";
                    }
                } else
                {
                    message = "No errors, and file EMPTY";
                }
            } catch (IOException e)
            {
                message = e.getMessage();
            }
        }
        return message;
    }
      
    private static boolean isNumeric(String input)
    {
        try
        {
            Integer.parseInt(input);
        }
        catch(NumberFormatException ex)
        {
            return false;
        }
        return true;
    }

    private static String getActiveProfile(File mypath)
    {
        String name = "";
        try
            {
                Scanner scanner = new Scanner(new File(mypath.toString()));                
                if (scanner.hasNext()) 
                {
                    String line = scanner.nextLine();
                    name = line.substring(8);    
                }
                else
                {
                    return "Missing!";
                }
            }
            catch(IOException e)
            {
                return "Missing!";
            }
        return name;
    }

    private static String renameSet(String set)
    {
        if (set == "")
            return "*";
        if (set == "AL")
            return "ALL";
        if (set == "AQ")
            return "ATQ";
        if (set == "AP")
            return "APC";
        if (set == "AN")
            return "ARN";
        if (set == "AE")
            return "ARC";
        if (set == "BR")
            return "BRB";
        if (set == "BD")
            return "BTD";
        if (set == "CH")
            return "CHR";
        if (set == "6E")
            return "6ED";
        if (set == "CS")
            return "CSP";
        if (set == "DS")
            return "DST";
        if (set == "D2")
            return "DD2";
        if (set == "8E")
            return "8ED";
        if (set == "EX")
            return "EXO";
        if (set == "FE")
            return "FEM";
        if (set == "FD")
            return "5DN";
        if (set == "5E")
            return "5ED";
        if (set == "4E")
            return "4ED";
        if (set == "GP")
            return "GPT";
        if (set == "HL")
            return "HML";
        if (set == "IA")
            return "ICE";
        if (set == "IN")
            return "INV";
        if (set == "JU")
            return "JUD";
        if (set == "LG")
            return "LEG";
        if (set == "LE")
            return "LGN";
        if (set == "A")
            return "LEA";
        if (set == "B")
            return "LEB";
        if (set == "MM")
            return "MMQ";
        if (set == "MI")
            return "MIR";
        if (set == "MR")
            return "MRD";
        if (set == "NE")
            return "NEM";
        if (set == "9E")
            return "9ED";
        if (set == "OD")
            return "ODY";
        if (set == "ON")
            return "ONS";
        if (set == "PS")
            return "PLS";
        if (set == "PT")
            return "POR";
        if (set == "P2")
            return "P02";
        if (set == "P3")
            return "PTK";
        if (set == "PR")
            return "PPR";
        if (set == "PY")
            return "PCY";
        if (set == "R")
            return "RV";
        if (set == "SC")
            return "SCG";
        if (set == "7E")
            return "7ED";
        if (set == "ST")
            return "S99";
        if (set == "ST2K")
            return "S00";
        if (set == "SH")
            return "STH";
        if (set == "TE")
            return "TMP";
        if (set == "DK")
            return "DRK";
        if (set == "TO")
            return "TOR";
        if (set == "UG")
            return "UGL";
        if (set == "U")
            return "2ED";
        if (set == "UD")
            return "UDS";
        if (set == "UL")
            return "ULG";
        if (set == "US")
            return "USG";
        if (set == "VI")
            return "VIS";
        if (set == "WL")
            return "WTH";
        else
            return set;
    }
}
