#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DISTRO="${1:-noble}"
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
DEB_VERSION="${DEB_UPSTREAM_VER}-0ubuntu1~${DISTRO}1"

echo "=========================================="
echo "Publishing to Ubuntu PPA: ${PPA_TARGET}"
echo "Upstream Version: ${DEB_UPSTREAM_VER}"
echo "Debian Version:   ${DEB_VERSION}"
echo "Target Suite:     ${DISTRO}"
echo "GPG Key:          ${GPG_KEY_ID}"
echo "=========================================="

export DEBEMAIL="covarianttensor@gmail.com"
export DEBFULLNAME="Oscar Cerna-Mandujano"

cd "${REPO_ROOT}"

# Create upstream source tarball in parent directory
ORIG_TARBALL="../castersoundboard_${DEB_UPSTREAM_VER}.orig.tar.gz"
echo "Creating upstream source tarball: ${ORIG_TARBALL}..."
git archive --format=tar.gz --prefix="castersoundboard-${DEB_UPSTREAM_VER}/" HEAD -o "${ORIG_TARBALL}"

# Update changelog
cat <<EOF > debian/changelog
castersoundboard (${DEB_VERSION}) ${DISTRO}; urgency=medium

  * Release v${DEB_UPSTREAM_VER} for Ubuntu (${DISTRO}).

 -- ${DEBFULLNAME} <${DEBEMAIL}>  $(date -R)
EOF

# Build signed source package
echo "Building and signing source package..."
if [ -n "${GPG_PASSPHRASE:-}" ]; then
    debuild -S -sa "-k${GPG_KEY_ID}" -p"gpg --batch --no-tty --passphrase ${GPG_PASSPHRASE} --pinentry-mode loopback"
else
    debuild -S -sa "-k${GPG_KEY_ID}"
fi

# Upload to Launchpad
CHANGES_FILE="../castersoundboard_${DEB_VERSION}_source.changes"
echo "Uploading ${CHANGES_FILE} to ${PPA_TARGET}..."
dput "${PPA_TARGET}" "${CHANGES_FILE}"

echo "Successfully submitted to Launchpad PPA: ${PPA_TARGET}"
