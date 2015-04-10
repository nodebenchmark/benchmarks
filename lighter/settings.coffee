module.exports = ()->
	class Settings
		constructor:(app) ->
			@mongoose = require('mongoose')
			@Promise = require('node-promise').Promise  
			@GridStore = require('mongodb').GridStore
			# init mongo        
			url = process.env.MONGODB_URI || process.env.MONGOLAB_URI || 'mongodb://localhost/lighter'
			@mongoose.connect url
			# if process.env.NODE_ENV isnt 'production'
			# 	@mongoose.set('debug', true)
			@marked = require('marked')			
			@marked.setOptions
				highlight:(code,lang) ->
					hl = require('highlight.js')
					hl.tabReplace = '    '
					(hl.highlightAuto code).value
					
		marked		:	@marked
		mongoose	:	@mongoose
		url			:	'/'
		author 		:	process.env.AUTHOR || 'Editor'
		title		:	process.env.BLOG_TITLE || 'Lighter Blog'
		username	:	process.env.USER || 'admin'
		password	:	process.env.PASSWORD || 'admin' 
		feedUrl		:	process.env.FEED_URL || null
		updated		:	new Date()
		engine		:	'Lighter Blog Engine'
		format: (content) ->
		  @marked(content)
						
	new Settings()
	