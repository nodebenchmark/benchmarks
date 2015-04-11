#!/usr/bin/python

import requests, json
from functools import partial

mud_api_url = 'http://146.6.177.23:50004'

def get(url):
    r = requests.get(url)
    return r.content

def register(userID):
    r = get('%s/register?id=%d' % (mud_api_url, userID))
    return r


def move(userID, direction):
    r = get('%s/move?id=%d&direction=%s' % (mud_api_url, userID, direction))
    return r

ids = {}

user_requests = [
    partial(register),
    partial(move, direction="left"),
    partial(move, direction="down"),
    partial(move, direction="right"),
    partial(move, direction="up"),
]
