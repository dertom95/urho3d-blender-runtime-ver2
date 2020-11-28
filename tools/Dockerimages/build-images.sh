#!/bin/bash

if [ -z "$1" ]; then
	echo "usage: $0 [--new|--update] [--push]"
	echo ""
	echo "--new    : build images from scratch"
	echo "--update : update current image's urho3d, build and create image out of this"
	echo ""
	echo "--push   : automatically push to docker hub"
	exit 1
fi

postfix=""
push="$2"

if [ "$1" == "--update" ]; then
	postfix="-update" 
fi

githash=$(git ls-remote  https://github.com/urho3d/Urho3D.git HEAD | awk '{ print substr($1,1,10) }')

echo urho3d short-hash: $githash 

build_image () {
	echo ""
	image=dertom95/urho3d-$1
	tag=$1-$githash
	full=$image:$tag

	echo "build image: $full"

	check_tag=$(docker images | grep $tag)
	
	if [ -z "$check_tag" ]; then
		if DOCKER_CLI_EXPERIMENTAL=enabled docker manifest inspect $full >/dev/null; then
			check_tag="found"
		fi
	fi

	echo check_tag:$check_tag 
	if [ -z "$check_tag" ]; then
		imagefile=Dockerfile-$1$postfix
		echo docker build -f $imagefile -t $full .
		docker build -f $imagefile -t $full .
		echo docker tag $full dertom95/$1:latest
		docker tag $full dertom95/urho3d-$1:latest

		if [ "$push" == "--push" ]; then
			echo docker push $image
			 docker push $image
		fi			
	else
		echo "docker-tag '$full' already exists. no rebuild"
	fi
}

#build_image "linux"
#build_image "mingw"
#build_image "arm"
#build_image "rpi"
#build_image "web"
build_image "android"
