#!/usr/bin/env bash

script_dir=$(dirname "$0")

cd "$script_dir" || exit

source venv/bin/activate

export FLASK_APP="server_mock.py"

flask run

cd - || exit