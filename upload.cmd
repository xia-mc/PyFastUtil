python setup.py clean
python setup.py sdist bdist_wheel
twine upload --repository testpypi dist/*
