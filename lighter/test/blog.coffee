require 'should'      
helper = (require '../helper')() 
blog = (require __dirname + '/init').blog

describe 'Blog', ->    
	describe 'find post', ->
		expected = 'test post'
		_id = ''
		beforeEach (done)->
			promise = blog.createPost
				title	: 	expected
				author 	:	'Mehfuz Hossain'
				body	:	'Empty body'
			promise.then (result) =>
				_id = result._id
				done()
							
		it 'should return expected for permaLink', (done)->
	 		promise = blog.findPost helper.linkify('test post')
	 		promise.then (data) ->
				 data.post.title.should.equal expected
				 done() 
		afterEach (done)->
			blog.deletePost _id, ()->
				done() 

	describe 'list post', ->
		expected = 'test post'
		id = ''
		beforeEach (done)->
			promise = blog.createPost
				title	: 	expected
				author 	:	'Mehfuz Hossain'
				body	:	'Empty body'
				publish :	false
			promise.then (result) =>
				id = result._id
				done()					
		it 'should skip draft posts', (done)->
	 		promise= blog.find ''
	 		promise.then (data) ->
	 			for post in data.posts
	 				post._id.should.not.equal id
				done() 
		afterEach (done)->
			blog.deletePost id, ()->
				done() 

	describe 'update post', ->
		id = '' 
		expected = 'test post'
		beforeEach (done)-> 
			promise = blog.createPost
				title		: 	expected
				author 		:	'Mehfuz Hossain'
				body		:	'Empty body'
				permaLink 	: 	'1900/01/test'
			promise.then (result) =>
				id = result._id
				done()

		it 'should not update permaLink when title is same', (done)->
			body = 'updated'
			promise = blog.updatePost
				id 		: id
				title 	: expected
				body 	: body
			promise.then (result)=>
				result.permaLink.should.equal '1900/01/test'
				result.body.should.equal body
				done()

		it 'should update the permalink when title is different', (done)->
			promise = blog.updatePost
				id 		: id
				title 	: 'updated post'
				body 	: 'nothing'
			promise.then (result)=>
				result.permaLink.should.equal helper.linkify('updated post')
				done()
				
		afterEach (done)->
			blog.deletePost id, ()->
				done()
