#!/usr/bin/python

import requests
from functools import partial

lighter_url = "http://146.6.177.23:50002"

def get(dummy, url):
    r = requests.get(url)
    return r.content

user_requests = [
        partial(get,url="%s/" % lighter_url),
        partial(get,url="%s/styles/style.css" % lighter_url),
        partial(get,url="%s/styles/highlight.css" % lighter_url),
        partial(get,url="%s/styles/media.css" % lighter_url),
        partial(get,url="%s/images/line-tile.png" % lighter_url),
        partial(get,url="%s/images/noise.png" % lighter_url),
        partial(get,url="%s/images/header-bottom.png" % lighter_url),
        partial(get,url="%s/styles/fonts/source-sans-pro/SourceSansPro-Regular-webfont.woff" % lighter_url),
        partial(get,url="%s/styles/fonts/source-sans-pro/SourceSansPro-Black-webfont.woff" % lighter_url),
        partial(get,url="%s/favicon.ico" % lighter_url),
]
