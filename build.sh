#!/bin/bash
source ./.venv/Scripts/activate
python ./setup.py build_ext --inplace
deactivate
