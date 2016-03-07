require 'should'      
helper = (require '../helper')()     

xml2js = require 'xml2js'      
util = require('util')                               

path = require 'path'
fs = require('fs')  

express = require('express')
request = require 'supertest'

app = express()         

category = (require __dirname + '/init').category
catModel = category.category 

(require path.join(__dirname, '../config'))(app)
(require path.join(__dirname, '../routes'))(app, category.settings)

describe 'category', ()->
	id = ''
	date = ''
	before (done)->
		obj = new Date()
		date = util.format('%s-%s-%s', obj.getMonth(), obj.getDate(), obj.getFullYear())
		cat = new catModel
			title 		: date
			permaLink 	: '/test'

		cat.save (err, data) =>
			id = data._id
			done()

	it 'should not create duplicate entry for refresh category ', (done)->
		promise = category.refresh(date)
		promise.then (result)=>
			result.permaLink.should.equal '/test'
			done()

	after (done)->
		catModel.remove
			title : date , ()=>
				done()