#!/bin/bash

# 입력 인자 체크
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 source.conf target.conf"
    exit 1
fi

# 입력과 출력 파일 설정
source_file=$1
target_file=$2

# 현재 작업 디렉토리와 PHP 실행 경로 가져오기
current_pwd=$(pwd)
php_path=$(which php)

# [pwd]와 [php]를 각각 현재 디렉토리와 PHP 경로로 대체하고, 결과를 새 파일에 저장
sed -e "s|\[pwd\]|$current_pwd|g" -e "s|\[php\]|$php_path|g" "$source_file" > "$target_file"
