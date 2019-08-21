package net.wagic.utils;

import org.apache.commons.compress.archivers.ArchiveException;
import org.apache.commons.compress.archivers.ArchiveOutputStream;
import org.apache.commons.compress.archivers.ArchiveStreamFactory;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipFile;
import org.apache.commons.compress.utils.IOUtils;

import java.io.*;
import java.util.Collection;
import java.util.Enumeration;



public class SevenZ {

    /**
     * Add all files from the source directory to the destination zip file
     *
     * @param source      the directory with files to add
     * @param destination the zip file that should contain the files
     * @throws IOException      if the io fails
     * @throws ArchiveException if creating or adding to the archive fails
     */
    public void addFilesToZip(File source, File destination) throws IOException, ArchiveException {
        OutputStream archiveStream = new FileOutputStream(destination);
        ArchiveOutputStream archive = new ArchiveStreamFactory().createArchiveOutputStream(ArchiveStreamFactory.ZIP, archiveStream);

	File[] fileList = source.listFiles();
        for (int i = 0; i < fileList.length; i++) {
	    File file = fileList[i];
	    if(!file.isDirectory()){
		String entryName = getEntryName(source, file);
		ZipArchiveEntry entry = new ZipArchiveEntry(entryName);
     	        archive.putArchiveEntry(entry);
	        BufferedInputStream input = new BufferedInputStream(new FileInputStream(file));
		IOUtils.copy(input, archive);
		input.close();
		archive.closeArchiveEntry();
	    } else {
		File[] subfileList = file.listFiles();
                for (int j = 0; j < subfileList.length; j++) {
  		    File subfile = subfileList[j];
		    String entryName = getEntryName(source, subfile);
		    ZipArchiveEntry entry = new ZipArchiveEntry(entryName);
     	            archive.putArchiveEntry(entry);
	            BufferedInputStream input = new BufferedInputStream(new FileInputStream(subfile));
		    IOUtils.copy(input, archive);
		    input.close();
		    archive.closeArchiveEntry();
		}
	    }
        }

        archive.finish();
        archiveStream.close();
    }

    /**
     * Remove the leading part of each entry that contains the source directory name
     *
     * @param source the directory where the file entry is found
     * @param file   the file that is about to be added
     * @return the name of an archive entry
     * @throws IOException if the io fails
     */
    private String getEntryName(File source, File file) throws IOException {
        int index = source.getAbsolutePath().length() + 1;
        String path = file.getCanonicalPath();

        return path.substring(index);
    }
}
