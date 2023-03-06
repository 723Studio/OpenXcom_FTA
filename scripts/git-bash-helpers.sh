#!/bin/bash

if [[ -z "$1" ]];
then
  echo "alias prefix required"
  exit 1;
fi;

git config alias.$1-clean '!'"f() { (git diff --quiet --cached --exit-code || (echo 'not commited changes'; false)) && (git diff --quiet --exit-code || (echo 'working dir dirty'; false)); }; f"
git config alias.$1-version-bump '!'"f() { vim -T dumb --noplugin -n -c 'silent! %s/\\(\"Extended \\d\\+[.,]\\d\\+\\)\"/\\=submatch(1).\".0\\\"\"/g' -c '%s/\\(\\d\\+[.,]\\d\\+[.,]\\)\\(\\d\\+\\)/\\=submatch(1).eval(submatch(2)+1)/g' -c '%s/(v\\d\\d\\d\\d-\\d\\d-\\d\\d)/\\=\"(v\".strftime(\"%Y-%m-%d\").\")\"/g' -c ':wq' src/version.h && git add src/version.h; }; f"
git config alias.$1-version-bump-commit '!'"f() { git $1-clean && git $1-version-bump && git commit --no-edit -m \"OXCE \$(grep -oP '(?<=Extended )\\d+\\.\\d+(\\.\\d+)?' src/version.h)\"; }; f"
git config alias.$1-version-set '!'"f() { ([[ \$# -eq 2 ]] || (echo 'wrong args num'; false)) && vim -T dumb --noplugin -n -c '%s/\\d\\+[.,]\\d\\+\\([.,]\\)\\d\\+/\\=\"'\$1'\".submatch(1).\"'\$2'\".submatch(1).\"0\"/g' -c '%s/(v\\d\\d\\d\\d-\\d\\d-\\d\\d)/\\=\"(v\".strftime(\"%Y-%m-%d\").\")\"/g' -c ':wq' src/version.h && git add src/version.h; }; f"
git config alias.$1-version-set-commit '!'"f() { git $1-clean && git $1-version-set \"\$@\" && git commit --no-edit -m \"OXCE v\$1.\$2\"; }; f"
