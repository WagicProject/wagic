import sys
import os
import zipfile
from pyjavaproperties import Properties
from optparse import OptionParser
from github3 import login

def checkRelease(repository, remote, branch):
    release = None
    for r in repository.iter_releases():
        if r.name == ('latest-' + branch) :
            release = r
            for a in r.assets :
                if a.name == remote :
                    # need to delete the old release
                    print '!deleting old release! -> ' + r.name
                    r.delete()
                    # need also to delete the tag (reference)
                    ref = repository.ref('tags/latest-' + branch)
                    ref.delete()
                    release = None

    if release is None:
        # now, we recreate a new one
        release = repository.create_release('latest-' + branch, branch, 'latest-' + branch,
            'Latest successful builds of the ' + branch + ' branch automatically uploaded by Travis or AppVeyor CI.',
            False,
            True)

    return release


def suffixFilename(filename, build):
    p = Properties();
    p.load(open('projects/mtg/build.number.properties'));
    minor = p['build.minor'];
    major = p['build.major'];
    point = p['build.point'];
    name, extension = os.path.splitext(filename)
    filename = name + '-' + major + minor + point + '-' + build + extension
    return filename

def main():
    parser = OptionParser()
    parser.add_option("-t", "--token", help="TOKEN: specify authentication token to use", metavar="TOKEN", dest="token")
    parser.add_option("-s", "--sha", help="SHA: specify commit SHA", metavar="SHA", dest="sha")
    parser.add_option("-l", "--local", help="FILE: specify local file path to upload", metavar="LOCAL", dest="local")
    parser.add_option("-r", "--remote", help="NAME: specify remote asset name in the release.", metavar="REMOTE", dest="remote")
    parser.add_option("-b", "--branch", help="BRANCH: specify branch of the commit", metavar="BRANCH", dest="branch")

    (options, args) = parser.parse_args()

    if (options.token and options.sha and options.local and options.remote and (options.branch == 'master' or options.branch == 'travis_mac_osx' or options.branch == 'cmake' )):
        gh = login(token = options.token)
    elif (options.branch != 'master' and options.branch != 'travis_mac_osx'):
        print '!branch is not master or travis_mac_osx! -> ' + options.branch
        print '-will not upload-'
        return
    else:
        parser.print_help()
        return

    repository = gh.repository('WagicProject', 'wagic')
    if(options.branch == 'master' or options.branch == 'travis_mac_osx'):
    	r = checkRelease(repository, options.remote, 'master')
    else:
	r = checkRelease(repository, options.remote, 'cmake')

    filename = options.remote
    with open(options.local, 'rb') as fd:
        asset = r.upload_asset('application/zip', filename , fd)
    s = 'File ' + options.local + ' has been uploaded as ' + asset.name + '.'
    print s

if __name__ == "__main__":
        main()
