#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DISTRO="${1:-noble}"
REVISION="${2:-1}"
GPG_KEY_ID="${GPG_KEY_ID:-94D7A09D038C770677F7610930187CC970FC46B8}"
PPA_TARGET="${PPA_TARGET:-ppa:covarianttensor/castersoundboard}"

# Extract version from VERSION file, git tag, or fallback
FALLBACK_VER="$(cat "${REPO_ROOT}/VERSION" 2>/dev/null || echo "0.0.0")"
VERSION="${CASTER_VERSION:-}"
if [ -z "${VERSION}" ]; then
    if [ "${GITHUB_REF_TYPE:-}" = "tag" ] && [ -n "${GITHUB_REF_NAME:-}" ]; then
        VERSION="${GITHUB_REF_NAME#v}"
    else
        TAG_DESC="$(git describe --tags --always --dirty 2>/dev/null || echo "")"
        if [ -n "${TAG_DESC}" ]; then
            VERSION="${TAG_DESC#v}"
        else
            VERSION="${FALLBACK_VER}"
        fi
    fi
fi
# Clean numeric version for debian upstream (e.g. 2.1.2)
DEB_UPSTREAM_VER="$(echo "${VERSION}" | sed -E 's/^([0-9]+\.[0-9]+\.[0-9]+).*/\1/')"
DEB_VERSION="${DEB_UPSTREAM_VER}-0ubuntu1~${DISTRO}${REVISION}"

echo "=========================================="
echo "Publishing to Ubuntu PPA: ${PPA_TARGET}"
echo "Upstream Version: ${DEB_UPSTREAM_VER}"
echo "Debian Version:   ${DEB_VERSION}"
echo "Target Suite:     ${DISTRO}"
echo "Revision:         ${REVISION}"
echo "GPG Key:          ${GPG_KEY_ID}"
echo "=========================================="

export DEBEMAIL="covarianttensor@gmail.com"
export DEBFULLNAME="Oscar Cerna-Mandujano"

cd "${REPO_ROOT}"

# Obtain upstream orig tarball:
# For revisions > 1, download the exact orig.tar.gz from Launchpad to guarantee matching checksum.
# For revision 1, generate from the upstream git tag (or HEAD fallback).
ORIG_TARBALL="../castersoundboard_${DEB_UPSTREAM_VER}.orig.tar.gz"
PPA_ORIG_URL="https://launchpad.net/~covarianttensor/+archive/ubuntu/castersoundboard/+sourcefiles/castersoundboard/${DEB_UPSTREAM_VER}-0ubuntu1~${DISTRO}1/castersoundboard_${DEB_UPSTREAM_VER}.orig.tar.gz"

if [ "${REVISION}" -gt 1 ] 2>/dev/null; then
    echo "Revision > 1 detected. Downloading matching orig tarball from Launchpad PPA..."
    if curl -sLf -o "${ORIG_TARBALL}" "${PPA_ORIG_URL}"; then
        echo "Successfully downloaded matching orig tarball from Launchpad."
    else
        echo "Warning: Could not download orig tarball from Launchpad, falling back to git archive."
        TAG_REF="v${DEB_UPSTREAM_VER}"
        if ! git rev-parse -q --verify "${TAG_REF}" >/dev/null; then
            TAG_REF="HEAD"
        fi
        git archive --format=tar.gz --prefix="castersoundboard-${DEB_UPSTREAM_VER}/" "${TAG_REF}" -o "${ORIG_TARBALL}"
    fi
else
    echo "Creating upstream source tarball from tag v${DEB_UPSTREAM_VER}..."
    TAG_REF="v${DEB_UPSTREAM_VER}"
    if ! git rev-parse -q --verify "${TAG_REF}" >/dev/null; then
        TAG_REF="HEAD"
    fi
    git archive --format=tar.gz --prefix="castersoundboard-${DEB_UPSTREAM_VER}/" "${TAG_REF}" -o "${ORIG_TARBALL}"
fi

# Update changelog
cat <<EOF > debian/changelog
castersoundboard (${DEB_VERSION}) ${DISTRO}; urgency=medium

  * Release v${DEB_UPSTREAM_VER} for Ubuntu (${DISTRO}).

 -- ${DEBFULLNAME} <${DEBEMAIL}>  $(date -R)
EOF

# For revision 1, include orig tarball (-sa); for subsequent revisions, omit orig tarball (-sd)
SOURCE_FLAG="-sa"
if [ "${REVISION}" -gt 1 ] 2>/dev/null; then
    SOURCE_FLAG="-sd"
    echo "Resetting non-debian files to match upstream tag v${DEB_UPSTREAM_VER}..."
    if git rev-parse -q --verify "v${DEB_UPSTREAM_VER}" >/dev/null; then
        git checkout "v${DEB_UPSTREAM_VER}" -- . ':!debian' || true
    fi
fi

# Build signed source package (-d skips build-deps check for source-only package)
echo "Building and signing source package..."
DEBUILD_FLAGS=("-S" "${SOURCE_FLAG}" "-d" "-k${GPG_KEY_ID}" "-i^(\.git|\.github|scripts|build.*)")
if [ -n "${GPG_PASSPHRASE:-}" ]; then
    debuild "${DEBUILD_FLAGS[@]}" -p"gpg --batch --no-tty --passphrase ${GPG_PASSPHRASE} --pinentry-mode loopback"
else
    debuild "${DEBUILD_FLAGS[@]}"
fi

# Upload to Launchpad
CHANGES_FILE="../castersoundboard_${DEB_VERSION}_source.changes"
echo "Uploading ${CHANGES_FILE} to ${PPA_TARGET}..."
dput "${PPA_TARGET}" "${CHANGES_FILE}"

echo "Successfully submitted to Launchpad PPA: ${PPA_TARGET}"
