#!/bin/bash
#
# Common variables for CDK Tools
#
TOP_DIR=$(dirname $0)/../..
BUILD_DIR=${TOP_DIR}/build_dir
STAGING_DIR=${TOP_DIR}/staging_dir
OUTPUT_DIR=${TOP_DIR}/bin

mkdir -p "${BUILD_DIR}" "${STAGING_DIR}" "${OUTPUT_DIR}"
