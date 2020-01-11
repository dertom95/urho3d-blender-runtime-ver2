#!/usr/bin/env bash
#
# Copyright (c) 2008-2019 the Urho3D project.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

if [[ $# -eq 0 ]]; then echo "Usage: dockerized.sh native|mingw|android|web not:rpi|arm [command [params]]"; exit 1; fi

PROJECT_DIR=$(cd "${0%/*}/.." || exit 1; pwd)
if [[ ! $DBE_TAG ]]; then
    # If the command failed or not on a tag then use 'master' by default; TRAVIS_COMMIT should be empty for non-CI usage
    DBE_TAG=$(git describe --exact-match "$TRAVIS_COMMIT" 2>/dev/null || echo master)
fi
BuildEnvironment=-$1; shift
BuildEnvironment=${BuildEnvironment/-base}

sudo docker pull dertom95/urho3d$BuildEnvironment:$DBE_TAG

# shellcheck disable=SC1083
if [[ $(sudo docker version -f {{.Client.Version}}) =~ ^([0-9]+)\.0*([0-9]+)\. ]] && (( BASH_REMATCH[1] * 100 + BASH_REMATCH[2] >= 1809 )); then
	echo     sudo docker run -it --rm -h fishtank \
        -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" -e PROJECT_DIR="$PROJECT_DIR" $NEW_URHO3D_HOME \
        --mount type=bind,source="$PROJECT_DIR",target="$PROJECT_DIR" \
        -v $PROJECT_DIR/android/launcher-app:/Urho3D/android/launcher-app \
	--mount source="$(id -u).urho3d_home_dir",target=/home/urho3d \
        --name "dockerized$BuildEnvironment" \
        "dertom95/urho3d$BuildEnvironment:$DBE_TAG" "$@"
    	
	sudo docker run -it --rm -h fishtank \
        -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" -e PROJECT_DIR="$PROJECT_DIR" $NEW_URHO3D_HOME \
        --mount type=bind,source="$PROJECT_DIR",target="$PROJECT_DIR" \
        -v $PROJECT_DIR/android/launcher-app:/Urho3D/android/launcher-app \
	--mount source="$(id -u).urho3d_home_dir",target=/home/urho3d \
	--name "dockerized$BuildEnvironment" \
        "dertom95/urho3d$BuildEnvironment:$DBE_TAG" "$@"
else
    echo "CAUTION: UNTESTED"
    echo "CAUTION: UNTESTED"
    # Fallback workaround on older Docker CLI version
    sudo docker run -it --rm -h fishtank \
        -e HOST_UID="$(id -u)" -e HOST_GID="$(id -g)" -e PROJECT_DIR="$PROJECT_DIR" \
        --mount type=bind,source="$PROJECT_DIR",target="$PROJECT_DIR" \
	--mount source="$(id -u).urho3d_home_dir",target=/home/urho3d \
        -v $PROJECT_DIR/android/launcher-app:/Urho3D/android/launcher-app \
	--name "dockerized$BuildEnvironment" \
        "dertom95/urho3d$BuildEnvironment:$DBE_TAG" "$@"
fi

# vi: set ts=4 sw=4 expandtab:
