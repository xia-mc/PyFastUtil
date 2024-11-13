@echo off
echo Running on Windows
call .\.venv\Scripts\activate.bat
python ./setup.py build_ext --inplace
call deactivate
