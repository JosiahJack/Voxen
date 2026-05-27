#!/usr/bin/env bash
sed -n 's/^#Models\///p' ./Data/models.txt | sort > /tmp/expected_models.txt
find ./Models -maxdepth 1 -name "*.obj" -exec basename {} \; | sort > /tmp/actual_models.txt
comm -13 /tmp/expected_models.txt /tmp/tmp/actual_models.txt
rm /tmp/expected_models.txt /tmp/actual_models.txt
