rsync -av --progress . ./data --exclude data --exclude .git --exclude .github --exclude dist
python setup.py sdist
