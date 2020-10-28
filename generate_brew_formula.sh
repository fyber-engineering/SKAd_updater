#!/usr/bin/env bash

#set -x

export PROJECT_VERSION=$1
export PROJECT_NAME=$2
export PROJECT_BINARY_DIR=$3
export PROJECT_SOURCE_DIR=$4


export SHA=$(shasum -a 256  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-${PROJECT_VERSION}.tar.gz | cut -d ' ' -f1)
eval "cat <<EOF
$(<${PROJECT_SOURCE_DIR}/homebrew/${PROJECT_NAME}.tpl.rb)
EOF" > ${PROJECT_NAME}.rb

echo "Generated '${PROJECT_NAME}.rb' Homebrew Formula"