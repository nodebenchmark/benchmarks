require 'should'      

helper = (require '../helper')()     
media = (require __dirname + '/init').media
fs = require('fs')
path = require('path')

ObjectId = require('mongodb').ObjectID

describe 'Media', ()->
	buffer = null
	before (done)->
		normalizedPath = path.normalize(__dirname + '/../public/logo.png')
		fs.readFile normalizedPath, (err, result)=>
			buffer = result
			if Buffer.isBuffer(buffer)
				promise = media.create	
					id:new ObjectId()
					slug:'logo.png'
					type:'image/png'
					body:buffer
				promise.then (result)=>
					done()

	it 'should assert the content created', (done)->
		url = helper.linkify 'logo.png'
		promise = media.get url 
		promise.then (result)->
			Buffer.isBuffer(result.data).should.be.ok
			result.data.length.should.equal buffer.length
			result.type.should.equal 'image/png'
			for i in [0...result.length]
				result[i].data.should.equal buffer[i]
			done()

	after (done)->
		url = helper.linkify 'logo.png'
		media.remove url, ()->
			done()


