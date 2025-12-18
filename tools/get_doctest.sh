#!/usr/bin/env bash
set -euo pipefail

# Fetches doctest single-header into third_party/doctest
# - Pins to a specific tag for reproducibility
# - Uses curl or wget with basic retries
# - Skips download if already present unless --force is passed

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
DEST_DIR="$ROOT_DIR/third_party/doctest"

# Pin version for stability (update as needed)
DOCTEST_TAG="v2.4.11"
HEADER_URL="https://raw.githubusercontent.com/doctest/doctest/${DOCTEST_TAG}/doctest/doctest.h"
LICENSE_URL="https://raw.githubusercontent.com/doctest/doctest/${DOCTEST_TAG}/LICENSE.txt"

FORCE=0
if [[ ${1-} == "--force" ]]; then
	FORCE=1
fi

download() {
	local url="$1"; local out="$2";
	if command -v curl >/dev/null 2>&1; then
		curl -fsSL --retry 3 --retry-delay 1 "$url" -o "$out"
	elif command -v wget >/dev/null 2>&1; then
		wget -q "$url" -O "$out"
	else
		echo "Error: neither curl nor wget is available to download $url" >&2
		exit 1
	fi
}

mkdir -p "$DEST_DIR"

if [[ $FORCE -eq 0 && -f "$DEST_DIR/doctest.h" ]]; then
	echo "doctest already present at third_party/doctest/doctest.h (use --force to re-download)"
	exit 0
fi

echo "Fetching doctest (${DOCTEST_TAG}) header ..."
tmp_header=$(mktemp)
trap 'rm -f "$tmp_header" "$tmp_license"' EXIT
download "$HEADER_URL" "$tmp_header"
install -m 0644 "$tmp_header" "$DEST_DIR/doctest.h"

echo "Fetching doctest LICENSE ..."
tmp_license=$(mktemp)
download "$LICENSE_URL" "$tmp_license"
install -m 0644 "$tmp_license" "$DEST_DIR/LICENSE.txt"

echo "doctest ${DOCTEST_TAG} ready at third_party/doctest/doctest.h"
