require 'should'
helper = (require '../helper')()

xml2js = require 'xml2js'
util = require('util')

path = require 'path'
fs = require('fs')  

express = require('express')
request = require 'supertest'

app = express()

blog = (require __dirname + '/init').blog

(require path.join(__dirname, '../config'))(app)
(require path.join(__dirname, '../routes'))(app, blog.settings)


describe 'routes', ()->
	request = request(app)
	describe 'find post by permaLink', ()->
		id = '' 
		expected = 'test post'

		beforeEach (done)->
			promise = blog.createPost
				title	: 	expected,
				author 	:	'Mehfuz Hossain'
				body	:	'Empty body'
			promise.then (result) =>
				id = result._id
				done()
		
		it 'should return the post', (done)->
				permaLink =  '/' + helper.linkify(expected)
				post = request.get(permaLink)
				post.expect(200).end (err, res)->
					if err != null
						throw err
					done()
		it 'should return 301 when post is moved', (done)->
			updatedTitle = 'another post'
			promise = blog.updatePost
				id 		: id
				title 	: updatedTitle
				body 	: 'nothing'
			promise.then (result)->
				permaLink =  '/' + helper.linkify(expected)
				post = request.get(permaLink)
				post.expect(301).end (err, res)->
					if err != null
						throw err
					(res.text.indexOf(helper.linkify(updatedTitle)) >= 0).should.be.true
					done()

		afterEach (done)->
			blog.deletePost id, ()->
				done()
