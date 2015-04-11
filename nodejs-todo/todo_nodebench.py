#!/usr/bin/python

import requests, json
from functools import partial

todo_api_url = 'http://146.6.177.23:50003'

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

def post_todo(todoID, description):
    data = json.dumps({ 'description' : description })
    r = post('%s/todos/create' % todo_api_url, data)
    if todoID not in ids:
        ids[todoID] = [] 
    ids[todoID].append(json.loads(r)['id'])

def delete_todo(todoID):
    data = json.dumps({ 'id' : ids[todoID].pop() })
    return post('%s/todos/delete' % todo_api_url, data)

ids = {}

user_requests = [
    partial(post_todo, description="one one one"),
    partial(post_todo, description="two two two"),
    partial(post_todo, description="three three three"),
    partial(delete_todo),
    partial(delete_todo),
    partial(delete_todo)
]
