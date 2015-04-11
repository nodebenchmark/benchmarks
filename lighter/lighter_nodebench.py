#!/usr/bin/python

import requests
from functools import partial

lighter_url = "http://146.6.177.23:50002"

def get(dummy, url):
    r = requests.get(url)
    return r.content

user_requests = [
        partial(get,url=lighter_url)
]
