#!/usr/bin/env bash
set -eu

main() {
  while true; do
    local DATE=$(date | tr -d '\n')
    local INFO=$(ps -eo pid,comm,vsz,rss,maj_flt,min_flt | grep demand-paging | grep -v grep)
    if [[ -z "$INFO" ]]; then
      echo "$DATE: target process seems to be finished"
      break
    fi
    echo "$DATE: $INFO"
    sleep 1
  done
}

main
