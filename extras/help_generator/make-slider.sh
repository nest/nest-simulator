#!/bin/bash
#

rm slider.txt
MYDIR=$(dirname $0)

IMGDIR=$(cd $MYDIR/py-original/; pwd)

cd $IMGDIR

IMAGES=$(ls *.png)

cd ..

 for a in $IMAGES ; do

    i=${a%%.png}

    title=$(echo $(grep -Pzo ".*\n(---)" $MYDIR/py_sample/$i.md) | sed 's/---*/ /g')
    string=$(echo $(grep -Pzo "(-*--)(\n.*)*" $MYDIR/py_sample/$i.md) | sed 's/---*/ /g' | sed q )
    left=${string:0:200}


    echo	'<div class="advps-slide" style="float: none; list-style: none; position: absolute; width: 860px; z-index: 0; display: none;">
            <a target="_self" href="./py_sample/'$i'/">
             <img width="300" height="200" src="./py_sample/img/'$i'.png"
                class="attachment-medium size-medium wp-post-image"
                alt="'$i'"
                srcset=""
                sizes="(max-width: 300px) 100vw, 300px"></a>

            <div class="advps-excerpt-one" style="width:38%;height:100%;top:0; left:0;">
                <div class="advps-overlay-one" style="background-color:#ffffff; -moz-opacity:0.6;filter:alpha(opacity=60);opacity:0.6;"></div>

                <div class="advps-excerpt-block-one" style="text-align:left;color:#515151;line-height:15px;font-size:14px;-moz-opacity:1;filter:alpha(opacity=100);opacity:1;">

                    <h2 class="advs-title" style="font-size:16px;line-height:16px;margin:5px 0px 10px 0px;color:#515151">
                        <a target="_self" href="./py_sample/'$i'/" style="font-size:16px;line-height:16px;margin:5px 0px 10px 0px;color:#515151">
    '$title'
                        </a>
                    </h2>
                    <p>
    '$left'[…]
                    </p>
                </div>
            </div>
        </div>' >> slider.txt
done
