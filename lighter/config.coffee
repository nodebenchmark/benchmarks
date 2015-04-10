express = require 'express'
util = require 'util'
config = (app)->
		app.set('port', process.env.PORT || 3000)
		app.set('views', __dirname + '/views')
		app.set('view engine', 'jade')
		app.use(express.favicon(__dirname + '/public/favicon.ico'))
		app.use(express.logger('dev'))
		app.use (req, res, next)->
			app.host = util.format('http://%s/', req.headers['host'])
			data = []
			length = 0
			req.on 'data', (chunk)=>
				length += chunk.length
				data.push(chunk)
				return 
			req.on 'end', ()=>
				req.rawBody = Buffer.concat(data, length)
				next()
				return 
		app.use(app.router)
		app.use(express.urlencoded())
		app.use(express.json())
		app.use(express.methodOverride())
		app.use(require('less-middleware')({ src: __dirname + '/public' }))
		app.use(express.compress())

		if process.env.NODE_ENV is 'production'
			# 1000*60*60*24 = 1 day
			app.use(express.static(__dirname + '/public', { maxAge : 86400000}))
		else
			app.use(express.static(__dirname + '/public'))
		
		app.locals.pretty = true
		
		if 'development' is app.get 'env'
			app.use(express.errorHandler())
	
module.exports = config
