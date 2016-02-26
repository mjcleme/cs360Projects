var http = require('http');
var fs = require('fs');

var port = Number(process.argv[2]);
var path = process.argv[3];

var server = http.createServer(function (req, res) {
    var file = fs.createReadStream(path);
    file.pipe(res);
});
server.listen(port);
