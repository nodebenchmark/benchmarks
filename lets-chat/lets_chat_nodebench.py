#!/usr/bin/python

import requests, json
from functools import partial

lets_chat_api_url = 'http://146.6.177.23:50001'

def post(url, data):
    headers = {'content-type': 'application/json'}
    r = requests.post(url, data=data, headers=headers)
    return r.content

def get(url):
    r = requests.get(url)
    return r.content

def delete(url):
    r = requests.delete(url)
    return r.content

def post_room(roomID):
    data = json.dumps({ 'name' : roomID, 'slug' : roomID, 'description' : roomID})
    return post('%s/rooms' % lets_chat_api_url, data)

def get_room(roomID):
    return get('%s/rooms/%s' % (lets_chat_api_url, str(roomID)))

def delete_room(roomID):
    return delete('%s/rooms/%s' % (lets_chat_api_url, str(roomID)))

def post_message(room, msg):
    data = json.dumps({ 'text' : msg })
    return post('%s/rooms/%s/messages' % (lets_chat_api_url, str(room)), data)

def get_messages(room):
    return get('%s/rooms/%s/messages' % (lets_chat_api_url, str(room)))


user_requests = [
            partial(post_room),
            partial(get_room),
            partial(get_messages),
            partial(post_message, msg="Howdy!"),
            partial(get_messages),
            partial(post_message, msg="Is anyone here?!?"),
            partial(get_messages),
            partial(post_message, msg="Okay, bye!"),
            partial(delete_room)
]
