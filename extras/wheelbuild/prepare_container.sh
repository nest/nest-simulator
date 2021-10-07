echo "Running a build, can we figure out which one?"
env
echo "Yay, \`python\` is the one being built!"
python -c "import sysconfig; import pprint; pprint.pprint(sysconfig.get_paths())"
