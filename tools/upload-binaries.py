from optparse import OptionParser
from uritemplate import URITemplate
import requests

def main():
    parser = OptionParser()
    parser.add_option("-t", "--token", help="TOKEN: specify authentication token to use", metavar="TOKEN", dest="token")
    parser.add_option("-s", "--sha", help="SHA: specify commit SHA", metavar="SHA", dest="sha")
    parser.add_option("-l", "--local", help="FILE: specify local file path to upload", metavar="LOCAL", dest="local")
    parser.add_option("-r", "--remote", help="NAME: specify remote asset name in the release.", metavar="REMOTE", dest="remote")
    parser.add_option("-b", "--branch", help="BRANCH: specify branch of the commit", metavar="BRANCH", dest="branch")

    (options, args) = parser.parse_args()

    if (options.token and options.local and options.remote):
        repo = 'WagicProject/wagic'
        access_token = options.token
        r = requests.get('https://api.github.com/repos/{0}/releases/32638811'.format(repo))
        upload_url = r.json()["upload_url"]
        t = URITemplate(upload_url)
        asset_url = t.expand(name = options.remote)
    
        headers = {
            'Content-Type': 'application/gzip','Authorization': 'Token {0}'.format(access_token)
        }

        r = requests.post(asset_url, headers = headers, data = open(options.local, 'rb').read(),verify=False)
        s = 'File ' + options.local + ' has been uploaded as ' + options.remote + '.'
        print s
    else:
        parser.print_help()
        return



if __name__ == "__main__":
        main()