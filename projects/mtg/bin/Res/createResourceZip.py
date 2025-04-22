import sys
import os
import zipfile
from pyjavaproperties import Properties
from optparse import OptionParser

def createResZipFile(filename): 

        utilities = ZipUtilities()
        rename = False
        if not os.path.isfile('settings/options.txt'):
            os.rename('settings/options.orig.txt', 'settings/options.txt')
            rename = True
        if not os.path.isfile('player/options.txt'):
            os.rename('player/options.orig.txt', 'player/options.txt')
            rename = True

        zip_file = zipfile.ZipFile(filename, 'w', zipfile.ZIP_STORED)
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
        
        if rename:
            os.rename('settings/options.txt', 'settings/options.orig.txt')
            os.rename('player/options.txt', 'player/options.orig.txt')

def getFilename():
    p = Properties();
    p.load(open('../../build.number.properties'));
    minor = p['build.minor'];
    major = p['build.major'];
    point = p['build.point'];
    filename = 'core_' + major + minor + point 
    return filename



def createStandardResFile(filename):
    print('Creating Standard Resource File')
    if not filename:
        filename = getFilename() + '.zip'
    createResZipFile( filename )
    #print('Created Resource Package for Standard Distribution: {0}'.format(filename), file=sys.stderr)
    print >> sys.stderr, 'Created Resource Package for Standard Distribution: {0}'.format( filename)

def createIosResFile(filename):
    print('Preparing Resource Package for iOS')
    utilities = ZipUtilities()
    if not filename:
        filename = getFilename() + '_iOS.zip'
    #createResZipFile( filename )
    zip_file = zipfile.ZipFile(filename, 'a', zipfile.ZIP_STORED)
    zip_file.write("../../iOS/Res/rules/modrules.xml", "rules/modrules.xml", zipfile.ZIP_STORED)
    zip_file.close()

    #print('Created Resource Package for Standard Distribution: {0}'.format(filename), file=sys.stderr)
    print >> sys.stderr, 'Created Resource Package for iOS Distribution: {0}'.format( filename)


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
                        print('File added: ' + str(full_path))
                        zip_file.write(full_path)
                elif os.path.isdir(full_path):
                        print('Entering folder: ' + str(full_path))
                        self.addFolderToZip(zip_file, full_path)


def main():
## using optparse instead of argParse for now since python 2.7 may not be installed.

    parser = OptionParser()
    parser.add_option("-p", "--platform", help="PLATFORM: specify custom build. (eg ios, android, etc)", metavar="PLATFORM", dest="platform")
    parser.add_option("-n", "--name", help="NAME: specify resource file name", metavar="NAME", dest="name")

    (options, args) = parser.parse_args()
	
    if (options.platform):
        if (options.platform == "ios"):
            createIosResFile(options.name)
        else:
            createStandardResFile(options.name)
    else:
        createStandardResFile(options.name)

if __name__ == "__main__":
	main()
