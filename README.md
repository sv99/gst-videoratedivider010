# gst-videoratedivider010

GStreamer 0.10 plugin - simple videorate downsampler. Passthrough only factored frames all other silenly dropped.

factor - frame which be passthrough, default 2 (passthrough each second frame).

My SoC device on the TI Davinci DM365 not allow set input framerate. Base videorate plugin
allow only RGB input flow. I not find simple plugin for downsamling input flow.

## generate sceleton from template

[gst-template on the github](https://github.com/kirankrishnappa/gst-template)

First version based on GstElement not allow change output framerate.
 
Worked version based on GstBaseTransform

```bash
git clone https://github.com/kirankrishnappa/gst-template.git
cd gst-template
# template for gstreamer 0.10
git checkout a94611f
cd gst-plugin/src
../tools/make_element VideoRateDivider gsttransform
```

Also used source code videorate and capsfilter plugins.

## test on osx

Build gstreamer with jpegenc, default install gst-plugins-good010 exist only pngenc.

```bash
brew uninstall gst-plugins-good010
brew install gst-plugins-good010 --with-jpeg
```

```bash
gst-launch -v -e videotestsrc num-buffers=10 ! \
    video/x-raw-rgb,framerate=10/1,width=1024,height=768 ! \
	jpegenc ! \
    multifilesink location=img_%02d.jpeg

GST_DEBUG=videorate:5 gst-launch -v -e videotestsrc num-buffers=10 ! \
    video/x-raw-rgb,framerate=10/1,width=1024,height=768 ! \
	videorate force-fps=5/1 ! \
	jpegenc ! \
    multifilesink location=img_%02d.jpeg
    
gst-launch -v -e videotestsrc num-buffers=10 ! \
    video/x-raw-rgb,framerate=10/1,width=1024,height=768 ! \
	videoratedivider factor=5 ! \
	jpegenc ! \
    multifilesink location=img_%02d.jpeg
```

without install plugin, using --gst-plugin-path
```bash
GST_DEBUG=videoratedivider:5 gst-launch --gst-plugin-path=$HOME/work/v2r/gst-videoratedivider010/lib -v -e videotestsrc num-buffers=10 ! \
    video/x-raw-rgb,framerate=10/1,width=1024,height=768 ! \
	videoratedivider ! \
	jpegenc ! \
    multifilesink location=img_%02d.jpeg

GST_DEBUG=videoratedivider:5 gst-launch --gst-plugin-path=$HOME/work/v2r/gst-videoratedivider010/lib -v -e videotestsrc num-buffers=10 ! \
    video/x-raw-rgb,framerate=10/1,width=1024,height=768 ! \
	videoratedivider factor=5 ! \
	jpegenc ! \
    multifilesink location=img_%02d.jpeg
```
