#!/usr/bin/env bash
#
# Copyright (c) 2019 Yao Wei Tjong. All rights reserved.
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

# Check if the host project root is correctly mounted before proceeding further
if [[ ! $PROJECT_DIR || -z $(ls -1 $PROJECT_DIR 2>/dev/null) ]]; then
    echo -e "Error: PROJECT_DIR env-var must be set to project root directory from host.
       See 'script/dockerized.sh' in the Urho3D project as use case sample."
    exit 1
fi

# Enable history-search in bash completion
sed -i '/^#.*history-search/s/^# //' /etc/inputrc

# Delay updating the symlinks to runtime after all compiler toolchains have been installed
/usr/sbin/update-ccache-symlinks

# Delay group and user creation to runtime to match the host GID and host UID
groupadd -g $HOST_GID urho3d && useradd -u $HOST_UID -g $HOST_GID -s /bin/bash urho3d

# Allow 'urho3d' user to write into mounted docker volumes
chmod o+w /home/urho3d

# Ensure ccache is being found first
PATH=/usr/lib/ccache:$PATH

# Use the built-in locale from the docker image
set -a && . /etc/default/locale && set +a

# Execute the command chain (relative to project root) as urho3d
cd /Urho3D && runuser -u root -- "$@"

# vi: set ts=4 sw=4 expandtab:

