import sys
import os
import zipfile
from pyjavaproperties import Properties
from optparse import OptionParser

def createWindowsZipFile(filename, buildpath):
    utilities = ZipUtilities()
    zip_file = zipfile.ZipFile(filename, 'w', zipfile.ZIP_STORED)
    zip_file.write('../../../LICENSE')
    zip_file.write('libpng13.dll')
    zip_file.write(buildpath + '/lib/Release/SDL2.dll')
    zip_file.write(buildpath + '/lib/Release/zlib.dll')
    zip_file.write(buildpath + '/bin/Release/Wagic.exe')
    zip_file.write('Res/' + getFilename('core') + '.zip')
    zip_file.close()

def getFilename(filename):
    p = Properties();
    p.load(open('../build.number.properties'));
    minor = p['build.minor'];
    major = p['build.major'];
    point = p['build.point'];
    filename = filename + '-' + major + minor + point
    return filename

def createStandardResFile(buildpath):
    print "Creating Resource File"
    cmd = 'python createResourceZip.py -n ' + getFilename('core') + '.zip'
    os.chdir("Res")
    os.system(cmd)
    os.chdir("..")
    print "Creating Windows Package File"
    filename = 'Wagic-windows.zip'
    createWindowsZipFile( filename, buildpath )
    print >> sys.stderr, 'Created Windows Package: {0}'.format( filename)

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
## using optparse instead of argParse for now since python 2.7 may not be installed.

    parser = OptionParser()
    parser.add_option("-b", "--build", help="BUILD: specify the build directory", metavar="BUILD", dest="build")

    (options, args) = parser.parse_args()
	
    createStandardResFile(options.build)

if __name__ == "__main__":
	main()
