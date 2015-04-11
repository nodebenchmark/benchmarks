#!/usr/bin/python

import requests, json
from functools import partial

wordfinder_api_url = 'http://146.6.177.23:50007'

def post(url, data):
    headers = {'content-type': 'application/json'}
    r = requests.post(url, data=data, headers=headers)
    return r.content

def search(userID, pattern):
    data = json.dumps({ 'pattern' : pattern })
    return post('%s/search' % wordfinder_api_url, data)

user_requests = [
    partial(search, pattern="he__o"),
    partial(search, pattern="st???"),
    partial(search, pattern="/boo/")
]
