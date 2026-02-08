#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
cd ${SCRIPT_DIR}

./lotr_analyzer -create-sql > create.sql
mysql --user lotr --password lotr < create.sql

