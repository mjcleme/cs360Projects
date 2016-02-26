var http = require('http');

http.get(process.argv[2], function(response) {
    response.on('error', function (err) {console.log('Error');return;});
    response.setEncoding('utf8');
    response.on('data', function (data) {
        console.log(data);
    });
});
