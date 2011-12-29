import zipfile
import os


class ZipUtilities:

    def toZip(self, file, filename):
        zip_file = zipfile.ZipFile(filename, 'w')
        if os.path.isfile(file):
                        zip_file.write(file)
        else:
                        self.addFolderToZip(zip_file, file)
        zip_file.close()

    def addFolderToZip(self, zip_file, folder): 
        zip_file.writestr(folder + '/', '')
        for file in os.listdir(folder):
            if file != '.svn':
                full_path = os.path.join(folder, file)
                if os.path.isfile(full_path):
                        print 'File added: ' + str(full_path)
                        zip_file.write(full_path)
                elif os.path.isdir(full_path):
                        print 'Entering folder: ' + str(full_path)
                        self.addFolderToZip(zip_file, full_path)

def main():
    utilities = ZipUtilities()
    filename = 'core_017.zip'
    if not os.path.isfile('settings/options.txt'):
        os.rename('settings/options.orig.txt', 'settings/options.txt')
    if not os.path.isfile('player/options.txt'):
        os.rename('player/options.orig.txt', 'player/options.txt')
        
    
    zip_file = zipfile.ZipFile(filename, 'w', zipfile.ZIP_DEFLATED)
    utilities.addFolderToZip(zip_file, 'themes')
    utilities.addFolderToZip(zip_file, 'sound')
    utilities.addFolderToZip(zip_file, 'settings')
    utilities.addFolderToZip(zip_file, 'sets')
    utilities.addFolderToZip(zip_file, 'rules')
    utilities.addFolderToZip(zip_file, 'player')
    utilities.addFolderToZip(zip_file, 'packs')
    utilities.addFolderToZip(zip_file, 'lang')
    utilities.addFolderToZip(zip_file, 'graphics')
    utilities.addFolderToZip(zip_file, 'campaigns')
    utilities.addFolderToZip(zip_file, 'ai')
    zip_file.close()

main()
