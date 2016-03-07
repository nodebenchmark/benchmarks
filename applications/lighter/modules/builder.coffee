module.exports = (app, settings) ->
	blog = (require __dirname + '/blog')(settings) 	
	media = (require __dirname + '/media')(settings)
	helper = (require __dirname+'/../helper')()
	ObjectId = require('mongodb').ObjectID
	util = require('util')
	
	if process.env.NODE_ENV != 'production'
		fs = require('fs')
		user = require(__dirname + '/user')(settings) 
		fs.readFile __dirname + '/../bin/post.md','utf8', (err, result)->
			blog.delete ()->
				posts = []
				# process logo image.	
				fs.readFile __dirname + '/../public/logo.png', (err, result)->
					buffer = result
					if Buffer.isBuffer(buffer)
						promise = media.create	
							id:new ObjectId()
							slug:'logo.png'
							type:'image/png'
							body:buffer
						promise.then (result)->

				for post in result.split('#post')
					if post != ''
						content = post.split('#block')
						imageUrl = util.format('![Logo](http://localhost:%s/images/%s)',app.get('port'), helper.linkify('logo.png'))
						body = content[1].replace('#logo', imageUrl)
						categories = []
						for category in content[3].split(' ')
							category = category.replace(/^\n*|\n*$/g, '')
							categories.push(category)
						promise = blog.createPost
							title 	:	content[0]
							body 	:	body
							author	:	settings.author
							publish	: 	true
							categories : categories	
							
						promise.then (result)->
							if result.id isnt null
								console.log '[%s]', result.permaLink
				console.log 'BootStrapping with data'
