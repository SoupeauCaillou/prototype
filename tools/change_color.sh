#!/bin/bash

if [ $# -ne 4 ]; then
    echo "Usage change_color.sh new_name shirt_color short_color path_to_images"
    exit 1
fi

for image in `ls $4/$path_to_images/white_*png`; do
    new_image=$(echo $image | sed s/white/${1}/)

    convert $image -fill $3 -opaque '#9badb7' $new_image
    convert $new_image -fill $2 -opaque white $new_image
done

