from optparse import OptionParser
from ftplib import FTP

def main():
    parser = OptionParser()
    parser.add_option("-l", "--local", help="FILE: specify local file path to upload", metavar="LOCAL", dest="local")
    parser.add_option("-r", "--remote", help="NAME: specify remote asset name in the release.", metavar="REMOTE", dest="remote")
    parser.add_option("-u", "--url", help="URL: specify url for ftp server", metavar="URL", dest="url")
    parser.add_option("-e", "--user", help="USER: specify username for ftp server", metavar="USER", dest="user")
    parser.add_option("-p", "--password", help="PASSWORD: specify password for ftp server", metavar="PASSWORD", dest="password")

    (options, args) = parser.parse_args()

    if (options.url and options.local and options.remote and options.user and options.password):
        ftp = FTP(options.url, timeout=1200)
        ftp.login(options.user, options.password)
        with open(options.local, 'r') as f:
            ftp.storbinary('STOR %s' % options.remote, f)
        ftp.quit()
        s = 'File ' + options.local + ' has been uploaded as ' + options.remote + '.'
        print s
    else:
        parser.print_help()
        return



if __name__ == "__main__":
        main()
