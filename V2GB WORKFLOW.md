
**1/ COMBINE ALL .MP4'S IN CURRENT DIRECTORY**
```bash
ffmpeg -f concat -safe 0 -i <(for f in ./*.mp4; do echo "file '$PWD/$f'"; done) -c copy output.mp4
```

**2/ DETERMINE VIDEO RESOLUTION**
```bash
ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 input.mp4
```

Resolution output from `cmd#2` will replace values in `cmd#3`

**3/ PIXELATE AND FORCE PALETTE**
```bash
ffmpeg -i input.mp4 -i palette.png -filter_complex "scale=72:128,scale=720:1280:flags=neighbor[pix];[pix][1:v]paletteuse=dither=bayer:bayer_scale=3" output.mp4
```
***EXTRA***

**CONVERT .MP4 TO GIF**
```bash
ffmpeg -i input.mp4 output.gif
```

**EXTRACT FRAMES FROM GIF**
```bash
convert input.gif frame%d.png
```

**PIXELATE**
```bash
ffmpeg -i input.mp4 -vf "scale=72:128,scale=720:1280:flags=neighbor" output.mp4
```

**CREATE PALETTE FROM SCRATCH**
```bash
magick -size 256x1 xc:transparent palette.png

magick palette.png \
         -fill '#071821' -draw 'rectangle 0,0 64,1' \
         -fill '#306850' -draw 'rectangle 64,0 128,1' \
         -fill '#86c06c' -draw 'rectangle 128,0 192,1' \
         -fill '#e0f8cf' -draw 'rectangle 192,0 256,1' \
         palette.png
```




montage image1.png image2.png -tile 2x1 -geometry 512x512+0+0 -quality 100 -define png:compression-filter=5 -define png:compression-level=9 -define png:compression-strategy=1 output.png


ffmpeg -i input.mp4 -vframes 1 output.png