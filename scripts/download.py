import shutil, os, re, sys, zipfile
from distutils.dir_util import copy_tree
os.chdir(os.path.dirname(os.path.realpath(sys.argv[0])) + "/..")
import twlib

def unzip(filename, where):
	try:
		z = zipfile.ZipFile(filename, "r")
	except:
		return False

	# remove extraction folder, if it exists
	if os.path.exists(z.namelist()[0]):
		shutil.rmtree(z.namelist()[0])

	# extract files
	for name in z.namelist():
		z.extract(name, where)
	z.close()
	return z.namelist()[0]

def downloadAll(targets):
	version = "update-icu"
	url = "https://github.com/MrCosmo666/MmoTee-Libs/archive/{}.zip".format(version)

	# download and unzip
	src_package_libs = twlib.fetch_file(url)
	if not src_package_libs:
		print("couldn't download libs")
		sys.exit(-1)
	libs_dir = unzip(src_package_libs, ".")
	if not libs_dir:
		print("couldn't unzip libs")
		sys.exit(-1)
	libs_dir = "MmoTee-Libs-{}".format(version)

	if "boost" in targets:
		copy_tree(libs_dir + "/boost/", "other/boost/")
	if "curl" in targets:
		copy_tree(libs_dir + "/curl/", "other/curl/")
	if "freetype" in targets:
		copy_tree(libs_dir + "/freetype/", "other/freetype/")
	if "icu" in targets:
		copy_tree(libs_dir + "/icu/", "other/icu/")
	if "mysql" in targets:
		copy_tree(libs_dir + "/mysql/", "other/mysql/")
	if "openssl" in targets:
		copy_tree(libs_dir + "/openssl/", "other/openssl/")
	if "opus" in targets:
		copy_tree(libs_dir + "/opus/", "other/opus/")
	if "sdl" in targets:
		copy_tree(libs_dir + "/sdl/", "other/sdl/")
	if "discordgamesdk" in targets:
		copy_tree(libs_dir + "/discordgamesdk/", "other/discordgamesdk/")

	# cleanup
	try:
		shutil.rmtree(libs_dir)
		os.remove(src_package_libs)
	except: pass

def main():
    import argparse
    p = argparse.ArgumentParser(description="Download dep library and header files for Windows.")
    p.add_argument("targets", metavar="TARGET", nargs='+', 
        choices=["boost", "curl", "freetype", "icu", "mysql", "openssl", "opus", "sdl", "discordgamesdk"], 
        help='Target to download.'
    )
    args = p.parse_args()

    downloadAll(args.targets)

if __name__ == '__main__':
    main()
