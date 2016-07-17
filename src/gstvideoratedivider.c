/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2016 S. Volkov <sv99@inbox.ru>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-videoratedivider
 *
 * Simple videorate downsampler. Passthrough only factored frames all other silenly dropped.
 *
 * Default property #GstVideoDivider:factor is 2, passthrough each second frame.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! videoratedivider ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/gst.h>

#include "gstvideoratedivider.h"

const gchar* gst_pad_direction_enum_names [] =
    {
        "GST_PAD_UNKNOWN","GST_PAD_SRC","GST_PAD_SINK"
    };

const gchar*
get_direction_name (GstPadDirection direction)
{
  return gst_pad_direction_enum_names[direction];
}

GST_DEBUG_CATEGORY_STATIC (video_rate_divider_debug);
#define GST_CAT_DEFAULT video_rate_divider_debug

/* Filter signals and args */
enum {
    /* FILL ME */
    LAST_SIGNAL
};

#define DEFAULT_FACTOR 2

enum {
    PROP_0,
    PROP_FACTOR
    /* FILL ME */
};

/* the capabilities of the inputs and outputs */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
);

GST_BOILERPLATE (GstVideoRateDivider, gst_video_rate_divider, GstBaseTransform,
                 GST_TYPE_BASE_TRANSFORM);

static void gst_video_rate_divider_set_property (GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec);
static void gst_video_rate_divider_get_property (GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec);

static GstFlowReturn gst_video_rate_divider_transform_ip (GstBaseTransform * trans,
    GstBuffer * buffer);

static GstCaps *gst_video_rate_divider_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps);

/* GObject vmethod implementations */

static void
gst_video_rate_divider_base_init (gpointer gclass)
{
  GST_DEBUG ("gst_video_rate_divider_base_init");

  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple (element_class,
      "Simple videorate downsampler",
      "Filter/Video",
      "Passthrough only factored frames all other quiet dropped.",
      "S. Volkov <sv99@inbox.ru>");

  gst_element_class_add_static_pad_template (element_class, &src_factory);
  gst_element_class_add_static_pad_template (element_class, &sink_factory);
}

/* initialize the VideoRateDivider's class */
static void
gst_video_rate_divider_class_init (GstVideoRateDividerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_class = GST_BASE_TRANSFORM_CLASS (klass);

  GST_DEBUG ("gst_video_rate_divider_class_init");

  gobject_class->set_property = gst_video_rate_divider_set_property;
  gobject_class->get_property = gst_video_rate_divider_get_property;

  base_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_video_rate_divider_transform_caps);
  base_class->transform_ip =
      GST_DEBUG_FUNCPTR (gst_video_rate_divider_transform_ip);

  g_object_class_install_property (gobject_class, PROP_FACTOR,
      g_param_spec_uint64 ("factor", "Factor", "Downsampling factor",
          1, G_MAXUINT64, DEFAULT_FACTOR, G_PARAM_READWRITE));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_video_rate_divider_init (GstVideoRateDivider *filter,
                             GstVideoRateDividerClass *gclass)
{
  GST_DEBUG ("gst_video_rate_divider_init");

  filter->first_frame = TRUE;
  filter->factor = 2;
  filter->from_rate_numerator = -1;
  filter->from_rate_denominator = 1;
  filter->_counter = 1;

  //gst_base_transform_set_gap_aware (GST_BASE_TRANSFORM (filter), TRUE);
}

/* GstElement vmethod implementations */

static GstFlowReturn
gst_video_rate_divider_transform_ip (GstBaseTransform * trans, GstBuffer * buffer)
{
  GstVideoRateDivider *filter;
  guint64 factor;

  filter = GST_VIDEO_RATE_DIVIDER (trans);
  GST_OBJECT_LOCK (filter);
  factor = filter->factor;
  GST_OBJECT_UNLOCK (filter);

  if (factor < 2)
    {
      /* just push out the incoming buffer without touching it */
      return GST_FLOW_OK;
    }

  /* pass through first frame */
  if (filter->first_frame)
    {
      filter->first_frame = FALSE;
      g_atomic_int_inc (&filter->_counter);
      return GST_FLOW_OK;
    }

  if ((g_atomic_int_get (&filter->_counter)) < factor)
    {
      g_atomic_int_inc (&filter->_counter);
      return GST_BASE_TRANSFORM_FLOW_DROPPED;
    }
  else
    {
      g_atomic_int_set (&filter->_counter, 1);
      return GST_FLOW_OK;
    }
}

static GstCaps *
gst_video_rate_divider_transform_caps (GstBaseTransform * trans,
                               GstPadDirection direction, GstCaps * caps)
{
  GstVideoRateDivider *videorate = GST_VIDEO_RATE_DIVIDER (trans);
  GstCaps *ret;
  GstStructure *s, *s2;
  gint rate_numerator, rate_denominator;

  ret = gst_caps_copy (caps);

  /* Any caps simply return */
  if (gst_caps_is_any (caps))
    {
      GST_DEBUG_OBJECT (trans,
        "transform caps: %" GST_PTR_FORMAT " (direction = %s) ANY",
                        caps, get_direction_name(direction));
      return ret;
    }

  s = gst_caps_get_structure (ret, 0);
  gst_structure_get_fraction (s, "framerate",
                              &rate_numerator, &rate_denominator);
  GST_DEBUG_OBJECT (trans,
      "transform caps: %" GST_PTR_FORMAT " (direction = %s framerate = %d/%d)",
                    caps, get_direction_name(direction), rate_numerator, rate_denominator);

  s2 = gst_structure_copy (s);

  if (direction == GST_PAD_SINK)
    {
      /* correct input flow framerate */
      /* store inpute framerate */
      videorate->from_rate_numerator = rate_numerator;
      videorate->from_rate_denominator = rate_denominator;

      gst_caps_remove_structure (ret, 0);
      gst_structure_set (s2, "framerate", GST_TYPE_FRACTION,
          rate_numerator, rate_denominator * videorate->factor, NULL);
      gst_caps_merge_structure (ret, s2);
    }

  return ret;
}

static void
gst_video_rate_divider_set_property (GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec)
{
  GstVideoRateDivider *filter = GST_VIDEO_RATE_DIVIDER (object);

  GST_OBJECT_LOCK (filter);
  switch (prop_id)
    {
      case PROP_FACTOR:
        filter->factor = g_value_get_uint64 (value);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
  GST_OBJECT_UNLOCK (filter);
}

static void
gst_video_rate_divider_get_property (GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec)
{
  GstVideoRateDivider *filter = GST_VIDEO_RATE_DIVIDER (object);

  GST_OBJECT_LOCK (filter);
  switch (prop_id)
    {
      case PROP_FACTOR:
        g_value_set_uint64 (value, filter->factor);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
  GST_OBJECT_UNLOCK (filter);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
plugin_init (GstPlugin *plugin)
{
  GST_DEBUG_CATEGORY_INIT (video_rate_divider_debug, "videoratedivider",
      0, "Simple videorate downsampler");

  return gst_element_register (plugin, "videoratedivider", GST_RANK_NONE,
      GST_TYPE_VIDEO_RATE_DIVIDER);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "videoratedivider"
#endif

/* gstreamer looks for this structure to register videoratedivider */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "videoratedivider",
    "Simple videorate downsampler",
    plugin_init,
    VERSION,
    "GPL",
    "GStreamer",
    "http://gstreamer.net/"
)
