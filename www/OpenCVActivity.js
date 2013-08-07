var OpenCVActivity = function() {
	var process = function(processId, params, title, success, error) {
		if (cordova.exec)
			cordova.exec(success, error, 'OpenCVActivity', 
			   'process', [processId, params, title]);
		else
			error("Cordova not present");
	};

	// Gets a list of available image processes to run
	var processList = function(success) {
		if (cordova.exec)
		   cordova.exec(success, function() {
			   success([]);
		   }, 'OpenCVActivity', 'processList', []);
		else
			success([]);
	};
	return {
		process: process, processList: processList
	};
}();

module.exports = OpenCVActivity;