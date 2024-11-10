@echo off
call .\.venv\Scripts\activate
python ./setup.py build_ext --inplace
call deactivate
