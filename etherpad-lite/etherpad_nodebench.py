#!/usr/bin/python

import requests, json, time, sys
from multiprocessing import Pool
from functools import partial
from random import seed, shuffle
from copy import copy, deepcopy
from StringIO import StringIO

etherpad_api_url = "http://146.6.177.23:50000/api/1/"

api_key= "apikey=3a3dfd09ad938e39087d8d761ac0deca75257891df0fc1fd2da963543f6f8bd7"

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

def etherpad_api(func, params):
    return get(etherpad_api_url + func + "?" + "&".join([api_key] + params))

def create_author(name):
    return etherpad_api('createAuthor', ['name=%s'%name])

def create_pad(padID):
    return etherpad_api('createPad', ['padID=%s'%padID])

def delete_pad(padID):
    return etherpad_api('deletePad', ['padID=%s'%padID])

def get_text(padID):
    return etherpad_api('getText', ['padID=%s'%padID])

def set_text(padID, text):
    return etherpad_api('setText', ['padID=%s' % padID, "text=%s" % text])

user_requests = [
            partial(create_pad),
            partial(get_text),
            partial(set_text, text=''),
            partial(set_text, text='hello '),
            partial(set_text, text='world!'),
            partial(delete_pad)
]
#
#if mode == 'parallel':
#    pass
#    # p = Pool(num_threads)
#    # p.map(single_client_execute, range(num_client))
#else: # sequential by default
#    queue = [ i for i in range(num_clients) for j in range(len(user_requests)) ]
#    seed(7) # do no remove this!
#    shuffle(queue)
#    print queue
#    state = [ 0 for i in range(num_clients) ]
#    for padID in queue:
#        user_requests[state[padID]](padID)
#        state[padID] += 1
#
#print all(s == len(user_requests) for s in state)
#print "done"
