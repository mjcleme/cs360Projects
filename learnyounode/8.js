var http = require('http');

http.get(process.argv[2], function(response) {
    response.on('error', function (err) {console.log('Error');return;});
    response.setEncoding('utf8');
    var allData = '';
    response.on('data', function (data) {
        allData += data;
    });
    response.on('end', function() {
        console.log(allData.length);
        console.log(allData);
    });
});
