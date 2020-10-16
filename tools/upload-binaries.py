from optparse import OptionParser
from uritemplate import URITemplate
import requests
import time

def main():
    parser = OptionParser()
    parser.add_option("-t", "--token", help="TOKEN: specify authentication token to use", metavar="TOKEN", dest="token")
    parser.add_option("-s", "--sha", help="SHA: specify commit SHA", metavar="SHA", dest="sha")
    parser.add_option("-l", "--local", help="FILE: specify local file path to upload", metavar="LOCAL", dest="local")
    parser.add_option("-r", "--remote", help="NAME: specify remote asset name in the release.", metavar="REMOTE", dest="remote")
    parser.add_option("-b", "--branch", help="BRANCH: specify branch of the commit", metavar="BRANCH", dest="branch")

    (options, args) = parser.parse_args()

    if (options.token and options.local and options.remote):
        repo = 'Vitty85/wagic'
        access_token = options.token
        
        r = requests.get('https://api.github.com/repos/{0}/releases/latest'.format(repo))
        
        upload_url = r.json()["upload_url"]
        assets_url = r.json()["assets_url"]
        
        r = requests.get(assets_url)
        assets_num = len(r.json())
        asset_name = options.remote
        asset_id = 0
        for asset in r.json():
            if asset_name == asset["name"]:
                asset_id = asset["id"]

        headers = {
            'Content-Type': 'application/gzip','Authorization': 'Token {0}'.format(access_token)
        }

        asset_url =  'https://api.github.com/repos/{0}/releases/assets'.format(repo) + '/' + str(asset_id)
        r = requests.delete(asset_url, headers = headers, verify=False)
        
        time.sleep(10)
        
        t = URITemplate(upload_url)
        asset_url = t.expand(name = options.remote)
        r = requests.post(asset_url, headers = headers, data = open(options.local, 'rb').read(),verify=False)
        s = 'File ' + options.local + ' has been uploaded as ' + options.remote + '.'
        print s
    else:
        parser.print_help()
        return



if __name__ == "__main__":
        main()