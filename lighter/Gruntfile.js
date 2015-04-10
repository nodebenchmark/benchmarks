module.exports = function(grunt) {

  grunt.initConfig({
	  nodemon: {
	    dev: {
	      options: {
	        file: 'server.js',
	        ignoredFiles: ['node_modules/**', 'public/**'],
	        watchedExtensions: ['js']
	      }
	    }
	  }, 
	  watch:{
		  scripts:{
			  files:['**/*.coffee'],
			  tasks:['coffee']
		  }
	  },
	  
	 coffee: {
		 compile: {
			 files: {
				 'modules/blog.js': ['modules/blog.coffee'], // 1:1 compile
				 'modules/builder.js': ['modules/builder.coffee'], // 1:1 compile
				 'modules/category.js': ['modules/category.coffee'], // 1:1 compile
				 'modules/user.js': ['modules/user.coffee'], // 1:1 compile
				 'modules/media.js': ['modules/media.coffee'], // 1:1 compile
				 'modules/request.js': ['modules/request.coffee'], // 1:1 compile
				 'config.js': ['config.coffee'],
				 'helper.js': ['helper.coffee'],
				 'routes.js': ['routes.coffee'],
				 'settings.js': ['settings.coffee'],
				 'schema.js': ['schema.coffee']
			 }
		 }
	 },
	 concurrent: {
  	    dev: {
  	      tasks: ['nodemon', 'watch'],
  	      options: {
  	        logConcurrentOutput: true
  	      }
  	    }
  	 }
});

 grunt.loadNpmTasks('grunt-contrib-coffee');
 grunt.loadNpmTasks('grunt-concurrent');
 grunt.loadNpmTasks('grunt-contrib-watch');
 grunt.loadNpmTasks('grunt-nodemon'); 
 grunt.registerTask('default', ['coffee', 'concurrent']);
};