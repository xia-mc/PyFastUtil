#!/bin/bash
source ./.venv/bin/activate
python ./setup.py build_ext --inplace
deactivate
