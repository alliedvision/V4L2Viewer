#!/bin/bash

set -e

if command -v git >/dev/null; then
  GIT_FULL_SHA=$(git log -1 --pretty=format:%H)
  GIT_SHA=${GIT_FULL_SHA:0:12}
  DIRTY=$(git diff --quiet || echo '-dirty')
else
  GIT_SHA="unknown"
fi

if [[ "$TAG_NAME" =~ ^release/V ]]; then
  TAG_NAME=${TAG_NAME#release/V}
fi

if [ -n "$BRANCH_NAME" ]; then
  BRANCH_NAME="$BRANCH_NAME-$GIT_SHA"
fi

LOCAL_VERSION="${TAG_NAME-${BRANCH_NAME-$GIT_SHA}}${DIRTY}"
export LOCAL_VERSION=$(echo "$LOCAL_VERSION" | tr / _)

mkdir -p build

(
  export DESTDIR="V4L2Viewer-$LOCAL_VERSION"
  cd build
  cmake -DCMAKE_BUILD_TYPE=Release ..
  make -j$(nproc) all install 
  tar cjf "$DESTDIR.tar.bz2" $DESTDIR
)

    

